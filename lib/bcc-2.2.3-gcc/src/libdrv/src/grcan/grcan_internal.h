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

#ifndef GRCAN_DEFAULT_BAUD
 /* default to 500kbits/s */
 #define GRCAN_DEFAULT_BAUD 500000
#endif

#ifndef GRCAN_SAMPLING_POINT
 #define GRCAN_SAMPLING_POINT 80
#endif

#define WRAP_AROUND_TX_MSGS 1
#define WRAP_AROUND_RX_MSGS 2

struct grcan_msg {
        unsigned int head[2];
        unsigned char data[8];
};
#define GRCAN_MSG_SIZE sizeof(struct grcan_msg)


extern int state2err[4];
extern struct grlib_canbtrs_ranges grcan_btrs_ranges;
extern struct grlib_canbtrs_ranges grcanfd_nom_btrs_ranges;
extern struct grlib_canbtrs_ranges grcanfd_fd_btrs_ranges;

int grcan_wait_rxdata(struct grcan_priv *pDev, int min);
int grcan_wait_txspace(struct grcan_priv *pDev, int min);

static inline unsigned int grcan_hw_rxavail(
        unsigned int rp,
        unsigned int wp,
        unsigned int size)
{
        if (rp == wp) {
                /* read pointer and write pointer is equal only
                 * when RX buffer is empty.
                 */
                return 0;
        }

        if (wp > rp) {
                return (wp - rp) / GRCAN_MSG_SIZE;
        } else {
                return (size - (rp - wp)) / GRCAN_MSG_SIZE;
        }
}

static inline unsigned int grcan_hw_txspace(
        unsigned int rp,
        unsigned int wp,
        unsigned int size)
{
        unsigned int left;

        if (rp == wp) {
                /* read pointer and write pointer is equal only
                 * when TX buffer is empty.
                 */
                return size / GRCAN_MSG_SIZE - WRAP_AROUND_TX_MSGS;
        }

        /* size - 4 - abs(read-write) */
        if (wp > rp) {
                left = size - (wp - rp);
        } else {
                left = rp - wp;
        }

        return left / GRCAN_MSG_SIZE - WRAP_AROUND_TX_MSGS;
}

#define FUNCDBG()
#define DBG(a, ...)
#define DBGC(a, ...)

#ifdef GRCAN_DMA_BYPASS_CACHE
#define READ_DMA_WORD(address) _grcan_read_nocache((unsigned int)(address))
#define READ_DMA_BYTE(address) _grcan_read_nocache_byte((unsigned int)(address))
static unsigned char __inline__ _grcan_read_nocache_byte(unsigned int address)
{
        unsigned char tmp;
        __asm__ (" lduba [%1]1, %0 "
                : "=r"(tmp)
                : "r"(address)
        );
        return tmp;
}
static unsigned int __inline__ _grcan_read_nocache(unsigned int address)
{
        unsigned int tmp;
        __asm__ (" lda [%1]1, %0 "
                        : "=r"(tmp)
                        : "r"(address)
        );
        return tmp;
}
#else
#define READ_DMA_DOUBLE(address) (*(volatile uint64_t *)(address))
#define READ_DMA_WORD(address) (*(volatile unsigned int *)(address))
#define READ_DMA_BYTE(address) (*(volatile unsigned char *)(address))
#endif
