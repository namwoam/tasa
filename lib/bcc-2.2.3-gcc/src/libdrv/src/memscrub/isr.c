#include "priv.h"

static void memscrub_isr(
	void *arg
)
{
	struct memscrub_priv *priv = arg;
	uint32_t fadr, ahbstatus, status, mask;

	/* Get hardware status */
	ahbstatus = REG_READ(&priv->regs->ahbstatus);
	if ((ahbstatus & (AHBS_NE|AHBS_DONE)) == 0){
		return;
	}

	/* IRQ generated by MEMSCRUB core... handle it here */

	/* Get Failing address */
	fadr = REG_READ(&priv->regs->ahbfailing);

	/* Get Status */
	status = REG_READ(&priv->regs->status);

	/* Clear error status */
	mask = 0;
	/* Clear CECNT only if we crossed the CE threshold*/
	if ((ahbstatus & AHBS_CE) == 0){
		/* Don't clear the CECNT */
		mask |= AHBS_CECNT;
	}
	/* Clear UECNT only if we crossed the UE threshold*/
	if ((ahbstatus & (AHBS_NE|AHBS_CE|AHBS_SBC|AHBS_SEC)) != AHBS_NE){
		/* Don't clear the UECNT */
		mask |= AHBS_UECNT;
	}
	REG_WRITE(&priv->regs->ahbstatus, ahbstatus & mask);
	REG_WRITE(&priv->regs->status,0);

	/* Let user handle error */
	(priv->isr)((void *) priv->isr_arg, fadr, ahbstatus, status);

	return;
}

int memscrub_isr_register(
	struct memscrub_priv *priv,
	memscrub_isr_t isr,
	void * data
)
{
	unsigned int ethres, ahberc, config;

	if (priv==NULL){
		DBG("MEMSCRUB not init.\n");
		return MEMSCRUB_ERR_ERROR;
	}

	if (isr==NULL){
		DBG("MEMSCRUB wrong pointer.\n");
		return MEMSCRUB_ERR_EINVAL;
	}

	/* Mask interrupts */
	ethres = REG_READ(&priv->regs->ethres);
	REG_WRITE(&priv->regs->ethres, ethres & ~(ETHRES_RECTE | ETHRES_BECTE));

	ahberc = REG_READ(&priv->regs->ahberc);
	REG_WRITE(&priv->regs->ahberc, ahberc & ~(AHBERC_CECTE | AHBERC_UECTE));

	config = REG_READ(&priv->regs->config);
	REG_WRITE(&priv->regs->config, config & ~(CONFIG_IRQD));

	/* Install IRQ handler if needed */
	if (priv->isr == NULL){
		priv->isr=isr;
		priv->isr_arg=data;
		osal_isr_register(
			&priv->isr_ctx,
			priv->interrupt,
			memscrub_isr,
			priv
		);
	} else {
		priv->isr=isr;
		priv->isr_arg=data;
	}

	/* Unmask interrupts */
	REG_WRITE(&priv->regs->ethres, ethres);

	REG_WRITE(&priv->regs->ahberc, ahberc);

	REG_WRITE(&priv->regs->config, config);

	return MEMSCRUB_ERR_OK;
}

int memscrub_isr_unregister(
	struct memscrub_priv *priv
)
{
	unsigned int ethres, ahberc, config;

	if (priv==NULL){
		DBG("MEMSCRUB not init.\n");
		return MEMSCRUB_ERR_ERROR;
	}

	if (priv->isr==NULL){
		DBG("MEMSCRUB wrong pointer.\n");
		return MEMSCRUB_ERR_EINVAL;
	}

	/* Mask interrupts */
	ethres = REG_READ(&priv->regs->ethres);
	REG_WRITE(&priv->regs->ethres, ethres & ~(ETHRES_RECTE | ETHRES_BECTE));

	ahberc = REG_READ(&priv->regs->ahberc);
	REG_WRITE(&priv->regs->ahberc, ahberc & ~(AHBERC_CECTE | AHBERC_UECTE));

	config = REG_READ(&priv->regs->config);
	REG_WRITE(&priv->regs->config, config & ~(CONFIG_IRQD));

	/* Uninstall IRQ handler if needed */
	osal_isr_unregister(
		&priv->isr_ctx,
		priv->interrupt,
		memscrub_isr,
		priv
	);

	/* Uninstall user ISR */
	priv->isr=NULL;
	priv->isr_arg=NULL;

	return MEMSCRUB_ERR_OK;
}
