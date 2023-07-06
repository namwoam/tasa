/*
 * Copyright 2022 Cobham Gaisler AB
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

#include <drv/grcan.h>
#include <drv/canbtrs.h>
#include <drv/osal.h>

#include "grcan_internal.h"

/* Uncomment for debug output */
/****************** DEBUG Definitions ********************/
#define DBG_TX 2
#define DBG_RX 4
#define DBG_STATE 8

#define DEBUG_FLAGS (DBG_STATE | DBG_RX | DBG_TX )
/*
#define DEBUG
#define DEBUGFUNCS
*/
/*********************************************************/
static inline unsigned int READ_REG(volatile unsigned int *addr)
{
        return *addr;
}

static int grcan_hw_read_try(
        struct grcan_priv *priv,
        volatile struct grcan_regs *regs,
        struct grcan_canmsg *buffer,
        int max);

static int grcan_hw_write_try(
        struct grcan_priv *priv,
        volatile struct grcan_regs *regs,
        struct grcan_canmsg *buffer,
        int count);

static int grcan_hw_read_try(
        struct grcan_priv *priv,
        volatile struct grcan_regs *regs,
        struct grcan_canmsg * buffer,
        int max
)
{
        int i, j;
        struct grcan_canmsg *dest;
        struct grcan_msg *source, tmp;
        unsigned int wp, rp, size, rxmax, addr;
        int trunk_msg_cnt;

        FUNCDBG();

        wp = READ_REG(&regs->rx0wr);
        rp = READ_REG(&regs->rx0rd);

        /*
         * Due to hardware wrap around simplification write pointer will
         * never reach the read pointer, at least a gap of 8 bytes.
         * The only time they are equal is when the read pointer has
         * reached the write pointer (empty buffer)
         *
         */
        if (wp != rp) {
                /* Not empty, we have received chars...
                 * Read as much as possible from DMA buffer
                 */
                size = READ_REG(&regs->rx0size);

                /* Get number of bytes available in RX buffer */
                trunk_msg_cnt = grcan_hw_rxavail(rp, wp, size);

                /* truncate size if user space buffer hasn't room for
                 * all received chars.
                 */
                if (trunk_msg_cnt > max)
                        trunk_msg_cnt = max;

                /* Read until i is 0 */
                i = trunk_msg_cnt;

                addr = (unsigned int)priv->rx;
                source = (struct grcan_msg *)(addr + rp);
                dest = buffer;
                rxmax = addr + (size - GRCAN_MSG_SIZE);

                /* Read as many can messages as possible */
                while (i > 0) {
                        /* Read CAN message from DMA buffer */
                        tmp.head[0] = READ_DMA_WORD(&source->head[0]);
                        tmp.head[1] = READ_DMA_WORD(&source->head[1]);
                        if (tmp.head[1] & 0x4) {
                                DBGC(DBG_RX, "overrun\n");
                        }
                        if (tmp.head[1] & 0x2) {
                                DBGC(DBG_RX, "bus-off mode\n");
                        }
                        if (tmp.head[1] & 0x1) {
                                DBGC(DBG_RX, "error-passive mode\n");
                        }
                        /* Convert one grcan CAN message to one "software" CAN message */
                        dest->extended = tmp.head[0] >> 31;
                        dest->rtr = (tmp.head[0] >> 30) & 0x1;
                        if (dest->extended) {
                                dest->id = tmp.head[0] & 0x3fffffff;
                        } else {
                                dest->id = (tmp.head[0] >> 18) & 0xfff;
                        }
                        dest->len = tmp.head[1] >> 28;
                        for (j = 0; j < dest->len; j++) {
                                dest->data[j] = READ_DMA_BYTE(&source->data[j]);
                        }

                        /* wrap around if neccessary */
                        source =
                            ((unsigned int)source >= rxmax) ?
                            (struct grcan_msg *)addr : source + 1;
                        dest++; /* straight user buffer */
                        i--;
                }
                {
                        /* A bus off interrupt may have occured after checking priv->started */
                        SPIN_IRQFLAGS(oldLevel);

                        SPIN_LOCK_IRQ(&priv->devlock, oldLevel);
                        if (priv->started == STATE_STARTED) {
                                regs->rx0rd = (unsigned int) source - addr;
                                regs->rx0ctrl = GRCAN_RXCTRL_ENABLE;
                        } else {
                                DBGC(DBG_STATE, "cancelled due to a BUS OFF error\n");
                                trunk_msg_cnt = state2err[priv->started];
                        }
                        SPIN_UNLOCK_IRQ(&priv->devlock, oldLevel);
                }
                return trunk_msg_cnt;
        }
        return 0;
}

static int grcan_hw_write_try(
        struct grcan_priv *priv,
        volatile struct grcan_regs *regs,
        struct grcan_canmsg * buffer,
        int count
)
{
        unsigned int rp, wp, size, txmax, addr;
        int ret;
        struct grcan_msg *dest;
        struct grcan_canmsg *source;
        int space_left;
        unsigned int tmp;
        int i;

        DBGC(DBG_TX, "\n");
        /*FUNCDBG(); */

        rp = READ_REG(&regs->tx0rd);
        wp = READ_REG(&regs->tx0wr);
        size = READ_REG(&regs->tx0size);

        space_left = grcan_hw_txspace(rp, wp, size);

        /* is circular fifo full? */
        if (space_left < 1) {
                return 0;
        }

        /* Truncate size */
        if (space_left > count) {
                space_left = count;
        }
        ret = space_left;

        addr = (unsigned int)priv->tx;

        dest = (struct grcan_msg *)(addr + wp);
        source = (struct grcan_canmsg *) buffer;
        txmax = addr + (size - GRCAN_MSG_SIZE);

        while (space_left > 0) {
                /* Convert and write CAN message to DMA buffer */
                if (source->extended) {
                        tmp = (1 << 31) | (source->id & 0x3fffffff);
                } else {
                        tmp = (source->id & 0xfff) << 18;
                }
                if (source->rtr) {
                        tmp |= (1 << 30);
                }
                dest->head[0] = tmp;
                dest->head[1] = source->len << 28;
                for (i = 0; i < source->len; i++) {
                        dest->data[i] = source->data[i];
                }
                source++;       /* straight user buffer */
                dest =
                    ((unsigned int)dest >= txmax) ?
                    (struct grcan_msg *)addr : dest + 1;
                space_left--;
        }

        /* A bus off interrupt may have occured after checking priv->started */
        SPIN_IRQFLAGS(oldLevel);

        SPIN_LOCK_IRQ(&priv->devlock, oldLevel);
        if (priv->started != STATE_STARTED) {
                DBGC(DBG_STATE, "cancelled due to a BUS OFF error\n");
                ret = state2err[priv->started];
                SPIN_UNLOCK_IRQ(&priv->devlock, oldLevel);
                return ret;
        }

        regs->tx0wr = (unsigned int) dest - addr;
        regs->tx0ctrl = GRCAN_TXCTRL_ENABLE;
        SPIN_UNLOCK_IRQ(&priv->devlock, oldLevel);

        return ret;
}


int grcan_read(struct grcan_priv *priv, struct grcan_canmsg *msg, size_t ucount)
{
        struct grcan_canmsg *dest;
        int nread;
        int req_cnt;

        FUNCDBG();

        dest = msg;
        req_cnt = ucount;

        if ( (!dest) || (req_cnt<1) )
                return GRCAN_RET_INVARG;

        if (priv->started != STATE_STARTED) {
                return GRCAN_RET_NOTSTARTED;
        }

        DBGC(DBG_RX, "grcan_read [%p]: buf: %p len: %u\n", d, msg, (unsigned int) ucount);

        nread = grcan_hw_read_try(priv,priv->regs,dest,req_cnt);
        return nread;
}

int grcan_write(struct grcan_priv *priv, struct grcan_canmsg *msg, size_t ucount)
{
        struct grcan_canmsg *source;
        int nwritten;
        int req_cnt;

        DBGC(DBG_TX,"\n");

        if ((priv->started != STATE_STARTED) || priv->config.silent)
                return GRCAN_RET_NOTSTARTED;

        req_cnt = ucount;
        source = (struct grcan_canmsg *) msg;

        /* check proper length and buffer pointer */
        if (( req_cnt < 1) || (source == NULL) ){
                return GRCAN_RET_INVARG;
        }

        nwritten = grcan_hw_write_try(priv,priv->regs,source,req_cnt);
        return nwritten;
}


int grcan_set_speed(struct grcan_priv *priv, unsigned int speed)
{
        struct grcan_timing timing;
        int ret;

        FUNCDBG();

        /* cannot change speed during run mode */
        if (priv->started == STATE_STARTED)
                return GRCAN_RET_INVSTATE;

        /* get speed rate from argument */
        ret = grlib_canbtrs_calc_timing(
                speed, priv->corefreq_hz, GRCAN_SAMPLING_POINT,
                &grcan_btrs_ranges, (struct grlib_canbtrs_timing *)&timing);
        if ( ret )
                return GRCAN_RET_INVARG;

        /* save timing/speed */
        priv->config.timing = timing;
        priv->config_changed = 1;

        return 0;
}

int grcan_set_btrs(struct grcan_priv *priv, const struct grcan_timing *timing)
{
        FUNCDBG();

        /* Set BTR registers manually
         * Read GRCAN/HurriCANe Manual.
         */
        if (priv->started == STATE_STARTED)
                return -1;

        if ( !timing )
                return -2;

        priv->config.timing = *timing;
        priv->config_changed = 1;

        return 0;
}
