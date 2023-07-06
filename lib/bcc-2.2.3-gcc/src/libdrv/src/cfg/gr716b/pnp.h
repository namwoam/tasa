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

#ifndef DRV_PNP_H
#define DRV_PNP_H

/* AMBA Plug&Play information for GR716 */

#define GAISLER_RTA_0_PNP_APB                         0x62040000
#define GAISLER_RTA_0_PNP_APB_MASK                    0x00000100
#define GAISLER_RTA_0_PNP_APB_IRQ                     60
#define GAISLER_RTA_0_PNP_VERSION                     0

#define GAISLER_RTA_1_PNP_APB                         0x72040000
#define GAISLER_RTA_1_PNP_APB_MASK                    0x00000100
#define GAISLER_RTA_1_PNP_APB_IRQ                     61
#define GAISLER_RTA_1_PNP_VERSION                     0

#define GAISLER_RTA_0_IRQMP_PNP_APB                   0x62000000
#define GAISLER_RTA_0_IRQMP_APB_MASK                  0x00000400
#define GAISLER_RTA_0_IRQMP_PNP_VERSION               4

#define GAISLER_RTA_1_IRQMP_PNP_APB                   0x72000000
#define GAISLER_RTA_1_IRQMP_APB_MASK                  0x00000400
#define GAISLER_RTA_1_IRQMP_PNP_VERSION               4

#define GAISLER_RTA_0_GPTIMER_0_PNP_APB               0x62010000
#define GAISLER_RTA_0_GPTIMER_0_PNP_APB_MASK          0x00000100
#define GAISLER_RTA_0_GPTIMER_0_PNP_APB_IRQ           1
#define GAISLER_RTA_0_GPTIMER_0_PNP_VERSION           1

#define GAISLER_RTA_1_GPTIMER_0_PNP_APB               0x72010000
#define GAISLER_RTA_1_GPTIMER_0_PNP_APB_MASK          0x00000100
#define GAISLER_RTA_1_GPTIMER_0_PNP_APB_IRQ           1
#define GAISLER_RTA_1_GPTIMER_0_PNP_VERSION           1

#endif /* DRV_PNP_H */