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

#ifndef __RTA_H
#define __RTA_H

#include <drv/regs/rta.h>
#include <drv/regs/irqmp.h>
#include <drv/auto.h>
#include <drv/osal.h>

/** PRIVATE **/
/* Driver private structure. Shall never be referenced by user. */
struct rta_priv {
        volatile struct rta_regs *regs;
        volatile struct irqmp_regs *irqmp_regs;
        uint8_t open;
        uint32_t irq;
};

struct rta_devcfg {
        struct drv_devreg regs;
        struct drv_devreg irqmp_regs;
        struct rta_priv priv;
};

/* Register one device */
int rta_register(struct rta_devcfg *devcfg);
/* Register an array of devices */
int rta_init(struct rta_devcfg *devcfgs[]);

/*
 * Return number of RTA devices available to driver
 */
extern int rta_dev_count(void);

/*
 * Open a RTA device
 *
 * dev_no:      Device number to open
 * return:      Device handle to use with all other rta_ API functions. The
 *              function returns NULL if device can not be opened.
 */
extern struct rta_priv *rta_open(int dev_no);

/*
 * Close a RTA device
 *
 * return: This function always returns 0 (success)
 */
int rta_close(
        struct rta_priv *priv
);


/*
 * Reads status register from RTA device
 *
 * priv: Priv struct for target RTA device
 * return: Value of the register
 */
uint32_t rta_get_status(struct rta_priv *priv);

/*
 * Reads mask register from RTA device
 *
 * priv: Priv struct for target RTA device
 * return: Value of the register
 */
uint32_t rta_get_mask(struct rta_priv *priv);
/*
 * Sets the mask register for a RTA device
 *
 * priv: Priv struct for target RTA device
 *  val: Value to write to register
 * return: Always return BCC_OK
 */
uint32_t rta_set_mask(struct rta_priv *priv, uint32_t val);
/*
 * Reads level register from RTA device
 *
 * priv: Priv struct for target RTA device
 * return: Value of the register
 */
uint32_t rta_get_lvl(struct rta_priv *priv);
/*
 * Sets the level register for a RTA device
 *
 * priv: Priv struct for target RTA device
 *  val: Value to write to register
 * return: Always return BCC_OK
 */
uint32_t rta_set_lvl(struct rta_priv *priv, uint32_t val);
/*
 * Sets the irq register for a RTA device
 *
 * priv: Priv struct for target RTA device
 *  val: Value to write to register
 * return: Always return BCC_OK
 */
uint32_t rta_set_irq(struct rta_priv *priv, uint32_t val);
/*
 * Starts target RTA device
 *
 * priv: Priv struct for target RTA device
 * return: Always return BCC_OK
 */
uint32_t rta_start(struct rta_priv *priv);
/*
 * Sets entry point for target RTA device
 *
 * priv: Priv struct for target RTA device
 * entry: Entry point to set
 * start: Set to non-zero if device should be started immediately
 * return: Always return BCC_OK
 */
uint32_t rta_set_ep(struct rta_priv *priv, uint32_t entry, int start);

/*
 * Reads user bits from the target RTA devices status register
 *
 * priv: Priv struct for target RTA device
 * return: Value of the bits
 */
uint8_t rta_get_usr_bits(struct rta_priv *priv);
/*
 * Sets the user bits from the target RTA devices status register
 *
 * priv: Priv struct for target RTA device
 *  val: Value to write to register
 * return: Always return BCC_OK
 */
uint8_t rta_set_usr_bits(struct rta_priv *priv, uint8_t val);

#endif /* __RTA_H */
