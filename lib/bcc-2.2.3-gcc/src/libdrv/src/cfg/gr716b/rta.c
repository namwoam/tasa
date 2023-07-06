/*
 * Copyright (c) 2022, Cobham Gaisler AB
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

#include <stddef.h>
#include <drv/gr716b/rta.h>
#include <bcc/ambapp_ids.h>
#include "pnp.h"

#include <stddef.h>
#include <drv/gr716/timer.h>
#include "pnp.h"

struct timer_devcfg GR716B_RTA0_TMR_CFG_0 = {
  .regs = {
    .addr   = GAISLER_RTA_0_GPTIMER_0_PNP_APB,
    .interrupt  = GAISLER_RTA_0_GPTIMER_0_PNP_APB_IRQ
  },
};

struct timer_devcfg GR716B_RTA1_TMR_CFG_0 = {
  .regs = {
    .addr   = GAISLER_RTA_1_GPTIMER_0_PNP_APB,
    .interrupt  = GAISLER_RTA_1_GPTIMER_0_PNP_APB_IRQ
  },
};


struct rta_devcfg *GR716_RTA_DRV_ALL[] = {
        & (struct rta_devcfg) {
                .regs = {
                        .addr       = GAISLER_RTA_0_PNP_APB,
                        .version    = GAISLER_RTA_0_PNP_VERSION,
                        .interrupt  = GAISLER_RTA_0_PNP_APB_IRQ,
                },
                .irqmp_regs = {
                        .addr       = GAISLER_RTA_0_IRQMP_PNP_APB,
                        .version    = GAISLER_RTA_0_IRQMP_PNP_VERSION,
                },
        },
        & (struct rta_devcfg) {
                .regs = {
                        .addr       = GAISLER_RTA_1_PNP_APB,
                        .version    = GAISLER_RTA_1_PNP_VERSION,
                        .interrupt  = GAISLER_RTA_1_PNP_APB_IRQ,
                },
                .irqmp_regs = {
                        .addr       = GAISLER_RTA_1_IRQMP_PNP_APB,
                        .version    = GAISLER_RTA_1_IRQMP_PNP_VERSION,
                },
        },
        NULL
};


