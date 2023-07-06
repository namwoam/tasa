#include <stdio.h>
#include <bcc/bcc.h>
#include <drv/gr716b/rta.h>

volatile int done = 0;

void *rta0;
void *rta1;

void rta1_irq_catch()
{
    /* Get message from the mailbox */
    uint32_t msg = rta_get_usr_bits(rta1);
    printf("cpu0: Got irq from RTA1 with message 0x%08lx\n", msg);
    /* Empty the mailbox */
    rta_set_usr_bits(rta1, 0);
    /* Clear all interrupts */
    rta_set_irq(rta1, -1);
}
void rta0_irq_catch()
{
    uint32_t msg = rta_get_usr_bits(rta0);
    printf("cpu0: Got irq from RTA0 with message 0x%08lx\n", msg);
    rta_set_usr_bits(rta0, 0);
    rta_set_irq(rta0, -1);
    /* Update number of ticks RTA0 has done */
    done = msg;
}

int main ()
{
    rta_init(GR716_RTA_DRV_ALL);
    printf("Found %i registered RTAs in system\n", rta_dev_count());

    rta0 = rta_open(0);
    rta1 = rta_open(1);

    rta_set_ep(rta0, 0x61000038, 0);
    rta_set_lvl(rta0, 0xf0000001);
    rta_set_mask(rta0, 0xf0000000);

    rta_set_lvl(rta1, 0xf0000001);
    rta_set_mask(rta1, 0xf0000000);
    rta_set_ep(rta1, 0x71000038, 0);

    bcc_int_map_set(60, 5);
    bcc_isr_register(5, rta0_irq_catch, NULL);
    bcc_int_map_set(61, 6);
    bcc_isr_register(6, rta1_irq_catch, NULL);

    bcc_int_unmask(5);
    bcc_int_unmask(6);
    printf("cpu0: Starting RTA 0!\n");
    rta_start(rta0);


    while(!done) {
        bcc_power_down();
    }

    printf("cpu0: Starting RTA 1!\n");
    rta_start(rta1);

    /* Loop testing until RTA0 has ticked 10 times */
    while(done != 10) {
        bcc_power_down();
    }

    rta_close(rta0);
    rta_close(rta1);

    printf("Test done\n");
    return 0;
}

