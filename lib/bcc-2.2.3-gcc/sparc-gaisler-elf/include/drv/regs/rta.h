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

#ifndef RTA_REGS_H
#define RTA_REGS_H

/**
 * @file
 *
 * @brief Register description for RTA devices
 *
 */

#include <stdint.h>

/**
 * @brief GAISLER_UNKNOWN_0
 *
 * address: 0x62040000
 *
 * Offset | Name       | Description
 * ------ | ---------- | --------------------------------
 * 0x0000 | status     | RTA status and mailbox status register
 * 0x0004 | lvl        | RTA status and mailbox interrupt level detection conf reg
 * 0x0008 | irq        | RTA status and mailbox interrupt register
 * 0x000c | mask       | RTA status and mailbox mask register for interrupt generation
 */
struct rta_regs {
  /**
   * @brief RTA status and mailbox status register
   *
   * offset: 0x0000
   *
   * Bit    | Name       | Description
   * ------ | ---------- | --------------------------------
   * 31:28  | usr        | User defined status bits
   *    27  |  lw        | Local watchdog has triggered local interrupt
   *    26  |  li        | Local interrupt has occured
   *    25  |  lr        | Processor has been resumed
   * 24:12  |   -        | Reserved
   *    11  |  dm        | DSU mode
   *    10  |  de        | DSU enable
   *     9  |  fc        | Fault counter
   *     8  |  ic        | Instruction counter
   *     7  |  bp        | BP missed
   *     6  |  du        | DU Counter
   *     5  |  su        | SU state
   *     4  |  ip        | Interrupt pending
   *     3  |  pd        | Power down state
   *     2  |  id        | IDLE state
   *     1  |  ha        | Halt
   *     0  |  er        | Error
   */
  uint32_t status;
  /**
   * @brief RTA status and mailbox interrupt level detection conf reg
   *
   * offset: 0x0004
   *
   */
  uint32_t lvl;
  /**
   * @brief RTA status and mailbox interrupt register
   *
   * offset: 0x0008
   *
   */
  uint32_t irq;
  /**
   * @brief RTA status and mailbox mask register for interrupt generation
   *
   * offset: 0x000c
   *
   */
  uint32_t mask;
} __attribute__((packed));


#endif /* RTA_REGS_H */
