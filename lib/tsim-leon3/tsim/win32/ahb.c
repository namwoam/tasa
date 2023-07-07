
/*** Example routines for a simple AHB module ***/
/* Copyright Gaisler Research, 2003. All right reserved. */

#include "tsim.h"
#include <stdio.h>
#include <string.h>
#include <malloc.h>

#ifdef __CYGWIN32__
struct proc_interface procif; 
#endif

/*** Example routines for a simple AHB module ***/

#define FLASH_START 0x0
#define FLASH_SIZE  0x100000
#define FLASH_END   (FLASH_START + FLASH_SIZE)
#define RAM_START 0x40000000
#define RAM_SIZE  0x400000
#define RAM_END   (RAM_START + RAM_SIZE)

static int  report_irq = 0;
static char *flash;
static char *ram;

/* called on simulator exit */
static void ahb_exit() 
{
   printf("ahb_exit: releasing flash memory\n");
   free(flash);
}

static void ahb_init(struct proc_interface pif) 
{
#ifdef __CYGWIN32__
	/* Do not remove, needed when compiling on Cygwin! */
   procif = pif;
#endif
   /* flash memory is allocated. Need to be double-word aligned */
   flash = (char *) calloc(FLASH_SIZE/8,8);
   printf("ahb_init: %d Kb flash allocated at 0x%x\n",FLASH_SIZE/1024, FLASH_START);
   ram = (char *) calloc(RAM_SIZE/8,8);
   printf("ahb_init: %d Kb ram allocated at 0x%x\n",RAM_SIZE/1024, RAM_START);

};

/* AHB read. Called on each read access. Should return 0 on
   success, 1 on memory error (MEXC will be generated), or -1 if the
   address is not handled by the AHB module. Data is alwasys read
   in 32-bit words. A pointer to the first data word should be
   retuned in *data. The number of AHB waitstates should be returned
   in *ws. ahbacc->num indicates how many words will be read
*/

static int cachereg;
static int ahbread(struct ahb_access *ahbacc) 
{ 
    uint32 address;

    // do ram first to improve sim speed
    if ((ahbacc->address >= RAM_START) && 
       ((ahbacc->address + ahbacc->rnum*4) < RAM_END))
    {
	address = ahbacc->address & (RAM_SIZE-1);
	ahbacc->data = ((uint32 *) &ram[address & ~3]);
	ahbacc->ws = 3 + ahbacc->rnum;  // emulate some access timing
	ahbacc->cache = 1;
    } else if ((ahbacc->address >= FLASH_START) && 
       ((ahbacc->address + ahbacc->rnum*4) < FLASH_END))
    {
	address = ahbacc->address & (FLASH_SIZE-1);
	ahbacc->data = ((uint32 *) &flash[address & ~3]);
	ahbacc->ws = 3 + ahbacc->rnum*2;  // emulate some access timing
	ahbacc->cache = 1;
    } else if (ahbacc->address == 0x80000014) {
	procif.cctrl(&cachereg, 1); //emulate cache control reg as demo
	ahbacc->data = &cachereg;
	ahbacc->cache = 0;  // don't cache CCR !!!
	ahbacc->ws = 3;
    } else if (ahbacc->address == 0x800000F4) {
	*ahbacc->data = RAM_SIZE; //needed to simulate LECCS apps without mkprom
	ahbacc->ws = 3;
    } else if (ahbacc->address == 0x800000F8) {
	*ahbacc->data = FLASH_SIZE; //needed to simulate LECCS apps without mkprom
	ahbacc->ws = 3;
    } else {
	return(-1); // chain default LEON peripherals
    }
    return(0); // return OK
}

static int ahbwrite(struct ahb_access *ahbacc)
{ 
    char *mem = NULL;
    uint32 address;

    if ((ahbacc->address >= RAM_START) && (ahbacc->address < RAM_END)) {
	mem = ram; // do ram first to improve sim speed
	address = ahbacc->address & (RAM_SIZE-1);
    } else if ((ahbacc->address >= FLASH_START) && (ahbacc->address < FLASH_END)) {
	mem = flash;
	address = ahbacc->address & (FLASH_SIZE-1);
    }
    if (mem) {
        switch(ahbacc->wsize) {
	case 0: 
#ifdef HOST_LITTLE_ENDIAN
	    address ^= 0x3;
#endif
	    mem[address] = (unsigned char) *ahbacc->data;
	    break;
	case 1: 
#ifdef HOST_LITTLE_ENDIAN
	    address ^= 0x2;
#endif
	    *((unsigned short *) &mem[address]) = (unsigned short) *ahbacc->data;
	    break;
	case 2: 
	    *((unsigned int *) &mem[address]) = (unsigned int) *ahbacc->data;
	    break;
	case 3: 
	    *((unsigned int *) &mem[address]) = (unsigned int) *ahbacc->data;
	    *((unsigned int *) &mem[address+4]) = (unsigned int) ahbacc->data[1];
	}
    } else if (ahbacc->address == 0x80000014) {
	procif.cctrl(ahbacc->data, 0); //emulate cache control reg as demo
    } else if (ahbacc->address == 0x80000018) {
	procif.power_down(); //emulate power down reg as demo
    } else {
	return(-1); // chain remaining leon APB registers
    }
    ahbacc->ws = 0; // Pretend that write buffer will absorb bus latency
    return(0); // return OK
}

/* check if requested address is valid, and return memory pointer /*
/* return -1 on failure */
static char * get_ahb_ptr(unsigned int address, int size)
{
    if ((address >= FLASH_START) && ((address+size) < FLASH_END)) {
	address &= (FLASH_SIZE-1);
	return(&flash[address]);
    } else if ((address >= RAM_START) && ((address+size) < RAM_END)) {
	address &= (RAM_SIZE-1);
	return(&ram[address]);
    } else
	return((char *) -1);
}

/* I/O specific command decoding */
static int ahb_command(char *cmd)
{
    char *cmd1;
    int dmaen;

    if ((cmd1 = strtok (cmd, " \t\n\r")) != NULL) {
      if (strncmp (cmd1, "help", strlen(cmd1)) == 0) {
	printf("\n ahb.so:               custom help command\n");
        return(0); //return 0 to continue searching for commands, else 1
      } else if (strncmp (cmd1, "ahbdeb", strlen(cmd1)) == 0) {
          report_irq = 1;
      } else if (strncmp (cmd1, "ahbndeb", strlen(cmd1)) == 0) {
          report_irq = 0;
      } else
	  return(0);
    }
    return(1);
}

/* AHB save state (dummy)  */
static void
ahb_save(char *fname)
{
    printf("ahb_save: save to %s.ahs\n", fname);
}

/* AHB restore state (dummy)  */
static void
ahb_restore(char *fname)
{
    printf("ahb_restore: restore from %s.ahs\n", fname);
}

static void ahb_reset()
{
    printf("ahb_reset\n");
}

static void ahb_restart()
{
    printf("ahb_restart:\n");
}

/* AHB interrupt acknowledge - use the LEON default irq handler */
static int ahb_intack(int level)
{
    if (report_irq) {
	printf ("ahb_intack: %d\n", level);
    }
    return (0);  // continue searching for interrupt handler
}

static struct ahb_subsystem ahb_test = {
	ahb_init,	/* init */
	ahb_exit,	/* exit */
	ahb_reset,		/* reset */
	ahb_restart,		/* restart */
	ahbread,	/* read */
	ahbwrite,	/* write */
	get_ahb_ptr,	/* get_io_ptr */
	ahb_command,	/* command */
	NULL,		/* sigio */
	ahb_save,	/* save */
	ahb_restore,	/* restore */
	ahb_intack	/* intack */
};

/* THIS IS WHERE THE I/O SYSTEM IS ATTACHED */
__declspec(dllexport)
struct ahb_subsystem *ahbsystem = &ahb_test;
