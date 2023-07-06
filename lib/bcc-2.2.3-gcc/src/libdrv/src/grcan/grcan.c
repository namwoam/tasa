/*
 * Copyright 2018 Cobham Gaisler AB
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <string.h>

#include <bcc/ambapp_ids.h>

#include <drv/grcan.h>
#include <drv/osal.h>

#include "grcan_internal.h"

/* GRCAN nominal boundaries for baud-rate paramters */
struct grlib_canbtrs_ranges grcan_btrs_ranges = {
        .max_scaler = 256*8, /* scaler is multiplied by BPR in steps 1,2,4,8 */
        .has_bpr = 1,
        .divfactor = 2,
        .min_tseg1 = 1,
        .max_tseg1 = 15,
        .min_tseg2 = 2,
        .max_tseg2 = 8,
};

/* GRCANFD nominal boundaries */
struct grlib_canbtrs_ranges grcanfd_nom_btrs_ranges = {
        .max_scaler = 256,
        .has_bpr = 0,
        .divfactor = 1,
        .min_tseg1 = 2,
        .max_tseg1 = 63,
        .min_tseg2 = 2,
        .max_tseg2 = 16,
};

/* GRCANFD flexible baud-rate boundaries */
struct grlib_canbtrs_ranges grcanfd_fd_btrs_ranges = {
        .max_scaler = 256,
        .has_bpr = 0,
        .divfactor = 1,
        .min_tseg1 = 1,
        .max_tseg1 = 15,
        .min_tseg2 = 2,
        .max_tseg2 = 8,
};

static int dev_count;
static struct drv_list devlist = { NULL, NULL };


int grcan_register(struct grcan_devcfg *devcfg)
{
        drv_list_addtail(&devlist, &devcfg->regs.node);
        devcfg->priv.open = 0;
        dev_count++;
        if (devcfg->regs.device_id == GAISLER_GRCANFD) {
                devcfg->priv.fd_capable = 1;
        }
        return DRV_OK;
}

int grcan_init(struct grcan_devcfg *devcfgs[])
{
        struct grcan_devcfg **dev = &devcfgs[0];

        while (*dev) {
                grcan_register(*dev);
                dev++;
        }
        return DRV_OK;
}

#define FUNCDBG()
#define DBG(a, ...)
#define DBGC(a, ...)

#define WRAP_AROUND_TX_MSGS 1

#ifndef GRCAN_DEFAULT_BAUD
 /* default to 500kbits/s */
 #define GRCAN_DEFAULT_BAUD 500000
#endif

#ifndef GRCAN_SAMPLING_POINT
 #define GRCAN_SAMPLING_POINT 80
#endif

int state2err[4] = {
        [STATE_STOPPED]  = GRCAN_RET_NOTSTARTED,
        [STATE_STARTED]  = GRCAN_RET_OK,
        [STATE_BUSOFF]   = GRCAN_RET_BUSOFF,
        [STATE_AHBERR]   = GRCAN_RET_AHBERR,
};

static void grcan_hw_reset(volatile struct grcan_regs *regs);

static void grcan_hw_config(
        struct grcan_priv *priv,
        struct grcan_config *conf);

static void grcan_hw_accept(
        volatile struct grcan_regs *regs,
        struct grcan_filter *afilter);

static void grcan_hw_sync(
        volatile struct grcan_regs *regs,
        struct grcan_filter *sfilter);

static void grcan_interrupt(void *arg);

static inline unsigned int READ_REG(volatile unsigned int *addr)
{
        return *addr;
}

static void grcan_hw_reset(volatile struct grcan_regs *regs)
{
        regs->ctrl = GRCAN_CTRL_RESET;
}

static int grcan_hw_start(struct grcan_priv *priv)
{
        unsigned int tmp;
        (void) sizeof tmp;

        FUNCDBG();

        /* Check that memory has been allocated successfully */
        if (!priv->tx || !priv->rx)
                return DRV_NOMEM;

        /* Configure FIFO configuration register
         * and Setup timing
         */
        if (priv->config_changed) {
                grcan_hw_config(priv, &priv->config);
                priv->config_changed = 0;
        }

        /* Setup receiver */
        priv->regs->rx0addr = (unsigned int)priv->rx;
        priv->regs->rx0size = priv->rxbuf_size;

        /* Setup Transmitter */
        priv->regs->tx0addr = (unsigned int)priv->tx;
        priv->regs->tx0size = priv->txbuf_size;

        /* Setup acceptance filters */
        grcan_hw_accept(priv->regs, &priv->afilter);

        /* Sync filters */
        grcan_hw_sync(priv->regs, &priv->sfilter);

        /* Clear status bits */
        tmp = READ_REG(&priv->regs->stat);
        priv->regs->stat = 0;

        /* Setup IRQ handling */

        /* Clear all IRQs */
        tmp = READ_REG(&priv->regs->pir);
        priv->regs->picr = 0x1ffff;

        priv->regs->imr = (
                GRCAN_RXAHBERR_IRQ |
                GRCAN_TXAHBERR_IRQ |
                GRCAN_OFF_IRQ |
                0
        );

        /* Enable receiver/transmitter */
        priv->regs->rx0ctrl = GRCAN_RXCTRL_ENABLE;
        priv->regs->tx0ctrl = GRCAN_TXCTRL_ENABLE;

        /* Enable HurriCANe core */
        priv->regs->ctrl = GRCAN_CTRL_ENABLE;

        /* Leave transmitter disabled, it is enabled when
         * trying to send something.
         */
        return DRV_OK;
}

static void grcan_hw_stop(struct grcan_priv *priv)
{
        FUNCDBG();

        /* Mask all IRQs */
        priv->regs->imr = 0;

        /* Disable receiver & transmitter */
        priv->regs->rx0ctrl = 0;
        priv->regs->tx0ctrl = 0;
}

static void grcan_hw_config(struct grcan_priv *priv, struct grcan_config *conf)
{
        unsigned int config = 0;
        volatile struct grcan_regs *regs = priv->regs;
        /* Reset HurriCANe Core */
        regs->ctrl = 0;

        if (conf->silent)
                config |= GRCAN_CFG_SILENT;

        if (conf->abort)
                config |= GRCAN_CFG_ABORT;

        if (conf->selection.selection)
                config |= GRCAN_CFG_SELECTION;

        if (conf->selection.enable0)
                config |= GRCAN_CFG_ENABLE0;

        if (conf->selection.enable1)
                config |= GRCAN_CFG_ENABLE1;

        /* Timing */
        if (!priv->fd_capable) {
                config |= (conf->timing.bpr << GRCAN_CFG_BPR_BIT) & GRCAN_CFG_BPR;
                config |= (conf->timing.rsj << GRCAN_CFG_RSJ_BIT) & GRCAN_CFG_RSJ;
                config |= (conf->timing.ps1 << GRCAN_CFG_PS1_BIT) & GRCAN_CFG_PS1;
                config |= (conf->timing.ps2 << GRCAN_CFG_PS2_BIT) & GRCAN_CFG_PS2;
                config |=
                (conf->timing.scaler << GRCAN_CFG_SCALER_BIT) & GRCAN_CFG_SCALER;
        } else {
                regs->nbtr =
                        (conf->timing.scaler << GRCANFD_NBTR_SCALER_BIT) |
                        (conf->timing.ps1 << GRCANFD_NBTR_PS1_BIT) |
                        (conf->timing.ps2 << GRCANFD_NBTR_PS2_BIT) |
                        (conf->timing.rsj << GRCANFD_NBTR_SJW_BIT);
                regs->fdbtr =
                        (conf->timing_fd.scaler << GRCANFD_FDBTR_SCALER_BIT) |
                        (conf->timing_fd.ps1 << GRCANFD_FDBTR_PS1_BIT) |
                        (conf->timing_fd.ps2 << GRCANFD_FDBTR_PS2_BIT) |
                        (conf->timing_fd.sjw << GRCANFD_FDBTR_SJW_BIT);
        }

        /* Write configuration */
        regs->conf = config;

        /* Enable HurriCANe Core */
        regs->ctrl = GRCAN_CTRL_ENABLE;
}

static void grcan_hw_accept(
        volatile struct grcan_regs *regs,
        struct grcan_filter *afilter
)
{
        /* Disable Sync mask totaly (if we change scode or smask
         * in an unfortunate way we may trigger a sync match)
         */
        regs->rx0mask = 0xffffffff;

        /* Set Sync Filter in a controlled way */
        regs->rx0code = afilter->code;
        regs->rx0mask = afilter->mask;
}

static void grcan_hw_sync(volatile struct grcan_regs *regs, struct grcan_filter *sfilter)
{
        /* Disable Sync mask totaly (if we change scode or smask
         * in an unfortunate way we may trigger a sync match)
         */
        regs->smask = 0xffffffff;

        /* Set Sync Filter in a controlled way */
        regs->scode = sfilter->code;
        regs->smask = sfilter->mask;
}

int grcan_istxdone(struct grcan_priv *priv)
{
        unsigned int rp, wp;
        FUNCDBG();

        /* loop until all data in circular buffer has been read by hw.
         * (write pointer != read pointer )
         *
         * Hardware doesn't update write pointer - we do
         */
        wp = READ_REG(&priv->regs->tx0wr);
        rp = READ_REG(&priv->regs->tx0rd);
        if (wp == rp) {
                return 1;
        }
        return 0;
}

int grcan_dev_count(void)
{
        return dev_count;
}

extern struct grcan_priv *grcan_open_userbuf(
        int dev_no,
        void *rxbuf,
        int rxbuf_size,
        void *txbuf,
        int txbuf_size
)
{
        struct grcan_priv *priv;
        void *ret;
        struct grlib_canbtrs_ranges *br;

        FUNCDBG();

        if (dev_no < 0 || dev_count <= dev_no) {
                return NULL;
        }

        struct grcan_devcfg *dev =
            (struct grcan_devcfg *) drv_list_getbyindex(&devlist, dev_no);
        priv = &dev->priv;

        uint8_t popen = osal_ldstub(&priv->open);
        if (popen) {
                return NULL;
        }

        SPIN_INIT(&priv->devlock, "thegrcan");

        priv->regs = (void *)dev->regs.addr;
        priv->corefreq_hz = osal_busfreq();
        priv->irq = dev->regs.interrupt;
        priv->started = STATE_STOPPED;
        priv->config_changed = 1;
        priv->config.silent = 0;
        priv->config.abort = 0;
        priv->config.selection.selection = 0;
        priv->config.selection.enable0 = 0;
        priv->config.selection.enable1 = 1;
        priv->rxa = priv->txa = NULL;
        priv->rx = rxbuf;
        priv->tx = txbuf;
        priv->txbuf_size = txbuf_size;
        priv->rxbuf_size = rxbuf_size;

        DBG("rxbuf_size: %d, txbuf_size: %d\n", txbuf_size, rxbuf_size);

        /* Default to accept all messages */
        priv->afilter.mask = 0x00000000;
        priv->afilter.code = 0x00000000;

        /* Default to disable sync messages (only trigger when id is set to all ones) */
        priv->sfilter.mask = 0xffffffff;
        priv->sfilter.code = 0x00000000;

        /* Calculate default timing register values */
        if (priv->fd_capable)
                br = &grcanfd_nom_btrs_ranges;
        else
                br = &grcan_btrs_ranges;
        grlib_canbtrs_calc_timing(
                GRCAN_DEFAULT_BAUD, priv->corefreq_hz, GRCAN_SAMPLING_POINT,
                br, (struct grlib_canbtrs_timing *)&priv->config.timing);

        /* Clear statistics */
        memset(&priv->stats,0,sizeof(struct grcan_stats));

        grcan_hw_reset(priv->regs);

        ret = priv;
        return ret;
}

int grcan_close_userbuf(struct grcan_priv *priv)
{
        FUNCDBG();

        grcan_stop(priv);

        grcan_hw_reset(priv->regs);

        /* Mark Device as closed */
        priv->open = 0;

        return 0;
}

int grcan_canfd_capable(struct grcan_priv *priv)
{
        return priv->fd_capable;
}

int grcan_start(struct grcan_priv *priv)
{
        FUNCDBG();

        if (grcan_get_state(priv) == STATE_STARTED) {
                return GRCAN_RET_INVARG;
        }

        if ( (grcan_hw_start(priv)) != DRV_OK ){
                return GRCAN_RET_NOTSTARTED;
        }

        /* Read and write are now open... */
        priv->started = STATE_STARTED;
        DBGC(DBG_STATE, "STOPPED|BUSOFF|AHBERR->STARTED\n");

        /* Register interrupt routine and enable IRQ at IRQ ctrl */
        int ret;
        ret = osal_isr_register(
                &priv->isr_ctx,
                priv->irq,
                grcan_interrupt,
                priv
        );
        if (DRV_OK != ret) {
                grcan_stop(priv);
                return GRCAN_RET_NOTSTARTED;
        }

        return GRCAN_RET_OK;
}

int grcan_stop(struct grcan_priv *priv)
{
        SPIN_IRQFLAGS(oldLevel);

        FUNCDBG();

        if (priv->started == STATE_STOPPED)
                return GRCAN_RET_INVARG;

        SPIN_LOCK_IRQ(&priv->devlock, oldLevel);
        if (priv->started == STATE_STARTED) {
                grcan_hw_stop(priv);
                DBGC(DBG_STATE, "STARTED->STOPPED\n");
        } else {
                /*
                 * started == STATE_[STOPPED|BUSOFF|AHBERR] so grcan_hw_stop()
                 * might already been called from ISR.
                 */
                DBGC(DBG_STATE, "[STOPPED|BUSOFF|AHBERR]->STOPPED\n");
        }
        priv->started = STATE_STOPPED;
        SPIN_UNLOCK_IRQ(&priv->devlock, oldLevel);

        /* Disable interrupts */
        osal_isr_unregister(
                &priv->isr_ctx,
                priv->irq,
                grcan_interrupt,
                priv
        );

        return GRCAN_RET_OK;
}

int grcan_get_state(struct grcan_priv *priv)
{
        FUNCDBG();

        return priv->started;
}

int grcan_set_silent(struct grcan_priv *priv, int silent)
{
        FUNCDBG();

        if (priv->started == STATE_STARTED)
                return -1;

        priv->config.silent = silent;
        priv->config_changed = 1;

        return 0;
}

int grcan_set_abort(struct grcan_priv *priv, int abort)
{
        FUNCDBG();

        if (priv->started == STATE_STARTED)
                return -1;

        priv->config.abort = abort;
        /* This Configuration parameter doesn't need HurriCANe reset
         * ==> no priv->config_changed = 1;
         */

        return 0;
}

int grcan_set_selection(struct grcan_priv *priv, const struct grcan_selection *selection)
{
        FUNCDBG();

        if (priv->started == STATE_STARTED)
                return -1;

        if ( !selection )
                return -2;

        priv->config.selection = *selection;
        priv->config_changed = 1;

        return 0;
}

int grcan_get_stats(struct grcan_priv *priv, struct grcan_stats *stats)
{
        SPIN_IRQFLAGS(oldLevel);

        FUNCDBG();

        if ( !stats )
                return -1;

        SPIN_LOCK_IRQ(&priv->devlock, oldLevel);
        *stats = priv->stats;
        SPIN_UNLOCK_IRQ(&priv->devlock, oldLevel);

        return 0;
}

int grcan_clr_stats(struct grcan_priv *priv)
{
        SPIN_IRQFLAGS(oldLevel);

        FUNCDBG();

        SPIN_LOCK_IRQ(&priv->devlock, oldLevel);
        memset(&priv->stats,0,sizeof(struct grcan_stats));
        SPIN_UNLOCK_IRQ(&priv->devlock, oldLevel);

        return 0;
}

int grcan_set_afilter(struct grcan_priv *priv, const struct grcan_filter *filter)
{
        FUNCDBG();

        if ( !filter ){
                /* Disable filtering - let all messages pass */
                priv->afilter.mask = 0x0;
                priv->afilter.code = 0x0;
        }else{
                /* Save filter */
                priv->afilter = *filter;
        }
        /* Set hardware acceptance filter */
        grcan_hw_accept(priv->regs,&priv->afilter);

        return 0;
}

int grcan_set_sfilter(struct grcan_priv *priv, const struct grcan_filter *filter)
{
        SPIN_IRQFLAGS(oldLevel);

        FUNCDBG();

        if ( !filter ){
                /* disable TX/RX SYNC filtering */
                priv->sfilter.mask = 0xffffffff;
                priv->sfilter.mask = 0;

                 /* disable Sync interrupt */
                SPIN_LOCK_IRQ(&priv->devlock, oldLevel);
                priv->regs->imr = READ_REG(&priv->regs->imr) & ~(GRCAN_RXSYNC_IRQ|GRCAN_TXSYNC_IRQ);
                SPIN_UNLOCK_IRQ(&priv->devlock, oldLevel);
        }else{
                /* Save filter */
                priv->sfilter = *filter;

                /* Enable Sync interrupt */
                SPIN_LOCK_IRQ(&priv->devlock, oldLevel);
                priv->regs->imr = READ_REG(&priv->regs->imr) | (GRCAN_RXSYNC_IRQ|GRCAN_TXSYNC_IRQ);
                SPIN_UNLOCK_IRQ(&priv->devlock, oldLevel);
        }
        /* Set Sync RX/TX filter */
        grcan_hw_sync(priv->regs,&priv->sfilter);

        return 0;
}

int grcan_get_status(struct grcan_priv *priv, unsigned int *data)
{
        FUNCDBG();

        if ( !data )
                return -1;

        /* Read out the status register from the GRCAN core */
        data[0] = READ_REG(&priv->regs->stat);

        return 0;
}

/* The interrupt is unmasked */
int grcan_txint(struct grcan_priv *priv, int txint)
{
        volatile struct grcan_regs *regs = priv->regs;
        unsigned int imr;
        unsigned int imr_set;
        SPIN_IRQFLAGS(oldLevel);

        SPIN_LOCK_IRQ(&priv->devlock, oldLevel);

        imr_set = 0;
        if (txint == 1) {
                imr_set = GRCAN_TX_IRQ;
        } else if (txint == -1) {
                imr_set = GRCAN_TXEMPTY_IRQ;
        }

        /* Clear pending Tx IRQ */
        regs->picr = imr_set;
        imr = READ_REG(&regs->imr);
        imr &= ~(GRCAN_TX_IRQ | GRCAN_TXEMPTY_IRQ);
        imr |= imr_set;
        regs->imr = imr;

        SPIN_UNLOCK_IRQ(&priv->devlock, oldLevel);

        return 0;
}

int grcan_rxint(struct grcan_priv *priv, int rxint)
{
        volatile struct grcan_regs *regs = priv->regs;
        unsigned int imr;
        unsigned int imr_set;
        SPIN_IRQFLAGS(oldLevel);

        SPIN_LOCK_IRQ(&priv->devlock, oldLevel);

        imr_set = 0;
        if (rxint == 1) {
                imr_set = GRCAN_RX_IRQ;
        } else if (rxint == -1) {
                imr_set = GRCAN_RXFULL_IRQ;
        }

        /* Clear pending Rx IRQ */
        regs->picr = imr_set;
        imr = READ_REG(&regs->imr);
        imr &= ~(GRCAN_RX_IRQ | GRCAN_RXFULL_IRQ);
        imr |= imr_set;
        regs->imr = imr;

        SPIN_UNLOCK_IRQ(&priv->devlock, oldLevel);

        return 0;
}

void grcan_set_isr(
        struct grcan_priv *priv,
        void (*isr)(struct grcan_priv *priv, void *data),
        void *data
)
{
        SPIN_IRQFLAGS(plev);

        SPIN_LOCK_IRQ(&priv->devlock, plev);

        priv->userisr = isr;
        priv->userisr_data = data;

        SPIN_UNLOCK_IRQ(&priv->devlock, plev);
}

/* Error indicators */
#define GRCAN_IRQ_ERRORS \
                (GRCAN_RXAHBERR_IRQ | GRCAN_TXAHBERR_IRQ | GRCAN_OFF_IRQ)
#define GRCAN_STAT_ERRORS (GRCAN_STAT_AHBERR | GRCAN_STAT_OFF)
/* Warning & RX/TX sync indicators */
#define GRCAN_IRQ_WARNS \
                (GRCAN_ERR_IRQ | GRCAN_OR_IRQ | GRCAN_TXLOSS_IRQ | \
                 GRCAN_RXSYNC_IRQ | GRCAN_TXSYNC_IRQ)
#define GRCAN_STAT_WARNS (GRCAN_STAT_OR | GRCAN_STAT_PASS)

/* Handle the IRQ */
static void grcan_interrupt(void *arg)
{
        struct grcan_priv *priv = arg;
        volatile struct grcan_regs *regs = priv->regs;
        unsigned int status = READ_REG(&regs->pimsr);
        unsigned int canstat = READ_REG(&regs->stat);
        SPIN_ISR_IRQFLAGS(irqflags);

        /* Spurious IRQ call? */
        if ( !status && !canstat )
                return;

        if (priv->started != STATE_STARTED) {
                DBGC(DBG_STATE, "not STARTED (unexpected interrupt)\n");
                regs->picr = status;
                return;
        }

        FUNCDBG();

        if ( (status & GRCAN_IRQ_ERRORS) || (canstat & GRCAN_STAT_ERRORS) ) {
                /* Bus-off condition interrupt
                 * The link is brought down by hardware, we wake all threads
                 * that is blocked in read/write calls and stop futher calls
                 * to read/write until user has called ioctl(fd,START,0).
                 */
                SPIN_LOCK(&priv->devlock, irqflags);
                DBGC(DBG_STATE, "STARTED->BUSOFF|AHBERR\n");
                priv->stats.ints++;
                if ((status & GRCAN_OFF_IRQ) || (canstat & GRCAN_STAT_OFF)) {
                        /* CAN Bus-off interrupt */
                        DBGC(DBG_STATE, "BUSOFF: status: 0x%x, canstat: 0x%x\n",
                                status, canstat);
                        priv->started = STATE_BUSOFF;
                        priv->stats.busoff_cnt++;
                } else {
                        /* RX or Tx AHB Error interrupt */
                        //printk("AHBERROR: status: 0x%x, canstat: 0x%x\n",
                        //      status, canstat);
                        priv->started = STATE_AHBERR;
                        priv->stats.ahberr_cnt++;
                }
                grcan_hw_stop(priv); /* this mask all IRQ sources */
                regs->picr = 0x1ffff; /* clear all interrupts */
                /*
                 * Prevent driver from affecting bus. Driver can be started
                 * again with grcan_start().
                 */
                SPIN_UNLOCK(&priv->devlock, irqflags);

                /*
                 * NOTE: Another interrupt may be pending now so ISR could be
                 * executed one more time aftert this (first) return.
                 */
                return;
        }

        SPIN_LOCK(&priv->devlock, irqflags);

        /* Increment number of interrupts counter */
        priv->stats.ints++;
        if ((status & GRCAN_IRQ_WARNS) || (canstat & GRCAN_STAT_WARNS)) {

                if ( (status & GRCAN_ERR_IRQ) || (canstat & GRCAN_STAT_PASS) ) {
                        /* Error-Passive interrupt */
                        priv->stats.passive_cnt++;
                }

                if ( (status & GRCAN_OR_IRQ) || (canstat & GRCAN_STAT_OR) ) {
                        /* Over-run during reception interrupt */
                        priv->stats.overrun_cnt++;
                }

                if ( status & GRCAN_TXLOSS_IRQ ) {
                        priv->stats.txloss_cnt++;
                }

                if ( status & GRCAN_TXSYNC_IRQ ) {
                        /* TxSync message transmitted interrupt */
                        priv->stats.txsync_cnt++;
                }

                if ( status & GRCAN_RXSYNC_IRQ ) {
                        /* RxSync message received interrupt */
                        priv->stats.rxsync_cnt++;
                }
        }

        SPIN_UNLOCK(&priv->devlock, irqflags);

        /* Clear IRQs */
        regs->picr = status;

        /* Delegate to user function, if installed. */
        if (priv->userisr) {
                (priv->userisr) (priv, priv->userisr_data);
        }
}

