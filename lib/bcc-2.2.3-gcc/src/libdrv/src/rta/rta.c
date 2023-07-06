/*
 * Copyright (c) 2019, Cobham Gaisler AB
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdint.h>
#include <drv/regs/rta.h>
#include <drv/gr716b/rta.h>
#include <bcc/bsp_pnp.h>

#include <stdio.h>

static int dev_count;
static struct drv_list devlist = { NULL, NULL };

static inline unsigned int READ_REG(volatile unsigned int *addr)
{
        return *addr;
}
static inline unsigned int WRITE_REG(volatile unsigned int *addr, unsigned int val)
{
        *addr = val;
        return BCC_OK;
}

int rta_dev_count(void)
{
        return dev_count;
}

int rta_register(struct rta_devcfg *devcfg)
{
        drv_list_addtail(&devlist, &devcfg->regs.node);
        devcfg->priv.open = 0;
        dev_count++;
        return DRV_OK;
}

int rta_init(struct rta_devcfg *devcfgs[])
{
        struct rta_devcfg **dev = &devcfgs[0];

        while (*dev) {
                rta_register(*dev);
                dev++;
        }
        return DRV_OK;
}

int rta_close(struct rta_priv *priv)
{
        priv->open = 0;
        return DRV_OK;
}

struct rta_priv *rta_open(int dev_no)
{
        if (dev_no < 0) {
                return NULL;
        }
        if (dev_no >= dev_count) {
                return NULL;
        }

        struct rta_devcfg *dev =
            (struct rta_devcfg *) drv_list_getbyindex(&devlist, dev_no);
        struct rta_priv *priv = &dev->priv;

        uint8_t popen;

        popen = osal_ldstub(&priv->open);
        if (popen) {
                return NULL;
        }

        priv->regs = (struct rta_regs *)dev->regs.addr;
        priv->irqmp_regs = (struct irqmp_regs *)dev->irqmp_regs.addr;
        priv->irq = dev->regs.interrupt;

        return priv;

}

uint32_t rta_get_status(struct rta_priv *priv)
{
        volatile struct rta_regs *const regs = priv->regs;
        return READ_REG(&regs->status);
}

uint8_t rta_get_usr_bits(struct rta_priv *priv)
{
        return ((rta_get_status(priv) >> 28) & 0xf);
}
uint8_t rta_set_usr_bits(struct rta_priv *priv, uint8_t val)
{
        volatile struct rta_regs *const regs = priv->regs;
        WRITE_REG(&regs->status, (val << 28));
        return DRV_OK;
}

uint32_t rta_get_lvl(struct rta_priv *priv)
{
        volatile struct rta_regs *const regs = priv->regs;
        return READ_REG(&regs->lvl);
}

uint32_t rta_set_lvl(struct rta_priv *priv, uint32_t val)
{
        volatile struct rta_regs *const regs = priv->regs;
        WRITE_REG(&regs->lvl, val);
        return DRV_OK;
}

uint32_t rta_get_mask(struct rta_priv *priv)
{
        volatile struct rta_regs *const regs = priv->regs;
        return READ_REG(&regs->mask);
}

uint32_t rta_set_mask(struct rta_priv *priv, uint32_t val)
{
        volatile struct rta_regs *const regs = priv->regs;
        WRITE_REG(&regs->mask, val);
        return DRV_OK;
}

uint32_t rta_set_irq(struct rta_priv *priv, uint32_t val)
{
        volatile struct rta_regs *const regs = priv->regs;
        WRITE_REG(&regs->irq, val);
        return DRV_OK;
}

uint32_t rta_start(struct rta_priv *priv)
{
        volatile struct irqmp_regs *const regs = priv->irqmp_regs;
        WRITE_REG(&regs->mpstat, 1);
        return DRV_OK;
}

uint32_t rta_set_ep(struct rta_priv *priv, uint32_t entry, int start)
{
        volatile struct irqmp_regs *const regs = priv->irqmp_regs;
        uint32_t result = ((entry & ~0x3) | !!start);
        WRITE_REG(&regs->resetaddr[0], result);
        return DRV_OK;
}

