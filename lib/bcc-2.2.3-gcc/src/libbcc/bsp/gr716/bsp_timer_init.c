/*
 * Copyright (c) 2017, Cobham Gaisler AB
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
#include "bcc/bcc_param.h"
#include "bcc/regs/gptimer.h"

extern const unsigned int __bsp_sysfreq;

int __bcc_timer_init(void)
{
        if (0 == __bcc_timer_handle)    { return BCC_NOT_AVAILABLE; }
        if (0 == __bsp_sysfreq)         { return BCC_NOT_AVAILABLE; }

        volatile struct gptimer_regs *regs = (void *) __bcc_timer_handle;
        volatile struct gptimer_timer_regs *tmr0 = &regs->timer[0];
        uint32_t scaler;

        scaler = (__bsp_sysfreq / 1000000) - 1;
        regs->scaler_value = scaler;
        regs->scaler_reload = scaler;
        tmr0->counter = ~0;
        tmr0->reload = ~0;
        tmr0->ctrl = (
                GPTIMER_CTRL_LD |
                GPTIMER_CTRL_RS |
                GPTIMER_CTRL_EN
        );

        return BCC_OK;
}

