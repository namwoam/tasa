/* I/O module that can be used both as a loadable module for TSIM */

#include "tsim.h"
#include <stdio.h>
#include <string.h>
#include <malloc.h>

#ifdef __CYGWIN32__
struct sim_interface simif; 
struct io_interface ioif; 
#endif

/*
/*** Example routines for a simple I/O system ***/


#define FLASH_START 0x10000000
#define FLASH_SIZE  0x100000
#define FLASH_END   0x11000000
static char *flash;

/* called on simulator exit */
static void io_exit() 
{
   printf("io_exit: releasing flash memory\n");
   free(flash);
}

static void io_init(struct sim_interface sif, struct io_interface iif) 
{

#ifdef __CYGWIN32__
/* Do not remove, needed when compiling on Cygwin! */
   simif = sif;
   ioif = iif;
#endif

   /* flash memory is allocated. Need to be double-word aligned */
   flash = (char *) calloc(FLASH_SIZE/8,8);
   printf("io_init: %d Kb flash allocated at 0x%x\n",FLASH_SIZE/1024, FLASH_START);
   printf("time = %d\n",(long)simif.simtime());
};

/* I/O read/write. Called on I/O or undecoded access. Should return 0 on
   success, 1 on memory error (MEXC will be generated). Data should be
   retuned in *data, number of BUSRDY waitstates should be returned in *ws.
   Size is encoded as: 0=byte, 1=half-word, 2=word, 3=doubel-word  */

static int io_read(address, data, ws) 
unsigned int address, *data, *ws; 
{ 
    *ws = 4;
    printf("io_read at 0x%08X\n", address);
    if ((address >= FLASH_START) && (address < FLASH_END)) {
	address &= (FLASH_SIZE-1);
	*data = *((int *) &flash[address & ~3]);
	return(0);
    } else
	return(1);
}

static int io_write(address, data, ws, size) 
unsigned int address, *data, *ws, size; 
{ 
    *ws = 4;
    if ((address >= FLASH_START) && (address < FLASH_END)) {
	address &= (FLASH_SIZE-1);
        switch(size) {
	case 0: 
#ifdef HOST_LITTLE_ENDIAN
	    address ^= 0x3;
#endif
	    flash[address] = (unsigned char) *data;
	    break;
	case 1: 
#ifdef HOST_LITTLE_ENDIAN
	    address ^= 0x2;
#endif
	    *((unsigned short *) &flash[address]) = (unsigned short) *data;
	    break;
	case 2: 
	    *((unsigned int *) &flash[address]) = (unsigned int) *data;
	    break;
	case 3: 
	    *((unsigned int *) &flash[address]) = (unsigned int) *data;
	    *((unsigned int *) &flash[address+4]) = (unsigned int) data[1];
	    *ws = 8;
	}
	return(0);
    } else
	return(1);
}

/* check if requested address is valid, and return memory pointer /*
/* return -1 on failure */
static char * get_io_ptr(unsigned int address, int size)
{
    if ((address >= FLASH_START) && ((address+size) < FLASH_END)) {
	address &= (FLASH_SIZE-1);
	return(&flash[address]);
    } else
	return((char *) -1);
}

/* example DMA transfer triggered by tick call */
static int dmaraddr = 0x40000000;
static int dmawaddr = FLASH_START + FLASH_SIZE/2;

static void dma_tick()
{
	int dmabuf[2];

	printf("io:dma_tick\n");
	printf("time = %d\n",(long)simif.simtime());
	ioif.dma_read(dmaraddr,dmabuf,2);
	printf("dma_read:  @0x%08x = %08x %08x\n", dmaraddr, dmabuf[0], dmabuf[1]);
	ioif.dma_write(dmawaddr,dmabuf,2);
	printf("dma_write: @0x%08x = %08x %08x\n", dmawaddr, dmabuf[0], dmabuf[1]);
	dmaraddr += 8;
	dmawaddr += 8;
	simif.event(dma_tick, 0, 1000);
}

/* I/O specific command decoding */
static int io_command(char *cmd)
{
    char *cmd1;
    int dmaen;

    if (((cmd1 = strtok (cmd, " \t\n\r")) != NULL)
      && (strncmp (cmd1, "dma", strlen(cmd1)) == 0))
      {
	if ((cmd1 = strtok (NULL, " \t\n\r")) != NULL)
	  {
            if (strncmp (cmd1, "enable", strlen(cmd1)) == 0) {
	        dmaen = 1;
		simif.stop_event(dma_tick);
		simif.event(dma_tick, 0, 1);
		printf("DMA started\n");
            } else if (strncmp (cmd1, "disable", strlen(cmd1)) == 0)
		simif.stop_event(dma_tick);
	    return(1);
	  }
      }
     return(0);
}

/* I/O save state (dummy)  */
void
io_save(char *fname)
{
    printf("io_save: save to %s.ios\n", fname);
}

/* I/O restore state (dummy)  */
void
io_restore(char *fname)
{
    printf("io_restore: restore from %s.ios\n", fname);
}

static struct io_subsystem test = {
	io_init,	/* io_init */
	io_exit,	/* io_exit */
	NULL,		/* io_reset */
	NULL,		/* io_restart */
	io_read,	/* io_read */
	io_write,	/* io_write */
	get_io_ptr,	/* get_io_ptr */
	io_command,	/* command */
	NULL,		/* sigio */
	io_save,	/* save */
	io_restore	/* restore */
};

/* THIS IS WHERE THE I/O SYSTEM IS ATTACHED */
__declspec(dllexport)
struct io_subsystem *iosystem = &test;

/* struct io_subsystem *iosystem = NULL; */
