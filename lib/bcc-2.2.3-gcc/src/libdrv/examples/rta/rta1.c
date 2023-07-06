#include <stdio.h>
#include <bcc/bcc.h>
#include <bcc/bcc_param.h>
#include <drv/gr716b/rta.h>
#include <drv/timer.h>
#include <drv/gr716b/timer.h>
#include <bcc/bsp_pnp.h>

void *rta_self;
int ticks = 0;

int __bcc_int_init(void) {
    __bcc_int_handle = GAISLER_RTA_1_IRQMP_PNP_APB;
    __bcc_int_irqmp_eirq = 1;
    return 0;
}

void seq_seq_1()
{
    ticks++;
}

int timers_init()
{
    void *timer;
    void *sub_timer1;

    timer_register(&GR716B_RTA1_TMR_CFG_0);
    timer = timer_open(0);
    timer_set_scaler_reload(timer, 0xfff);
    timer_set_scaler(timer, 0xfff);
    timer_set_cfg(timer, 0x0);

    sub_timer1 = timer_sub_open(timer, 1);
    timer_set_reload(sub_timer1, 0x00001020);
    timer_set_ctrl(sub_timer1, (GPTIMER_CTRL_IE | GPTIMER_CTRL_LD | GPTIMER_CTRL_RS | GPTIMER_CTRL_EN));

    bcc_isr_register(2, seq_seq_1, NULL);
    bcc_int_unmask(2);
    return 0;
}

int main ()
{
    rta_init(GR716_RTA_DRV_ALL);
    rta_self = rta_open(1);
    printf("\tRTA1> Hello from rta1\n");
    timers_init();
    while(1) {
        printf("\tRTA1> Tock\n");
        /* Post number of ticks to the mailbox */
        rta_set_usr_bits(rta_self, ticks);
        bcc_power_down();
    }
    return 0;
}

