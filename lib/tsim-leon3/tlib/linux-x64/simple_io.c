/**************************************************************/
/*** Routines that should be provided by the I/O sub-system ***/
/**************************************************************/

#include "tsim.h"
#include "simple_io.h"
#include <stdio.h>

static int (*ioread)(int, int *, int *);
static int (*iowrite)(int, int *, int *, int);
static void (*sigio)();

void tsim_set_ioread(int (*func)(int, int *, int *)) {ioread = func;}
void tsim_set_iowrite(int (*func)(int, int *, int *, int)) {iowrite = func;}
void tsim_set_sigio(void (*func)()) {sigio = func;}

static int io_read(address, data, ws) 
unsigned int address, *data, *ws; 
{ 
  if (ioread) return(ioread(address, data, ws));
  else return (0);
}

static int io_write(address, data, ws, size) 
unsigned int address, *data, *ws, size; 
{ 
  if (iowrite) return(iowrite(address, data, ws, size));
  else return (0);
}

static void io_sigio(address, data, ws, size) 
unsigned int address, *data, *ws, size; 
{ 
  if (sigio) sigio();
}

static struct io_subsystem libio = {
	NULL,	/* io_init */
	NULL,	/* io_exit */
	NULL,		/* io_reset */
	NULL,		/* io_restart */
	io_read,	/* io_read */
	io_write,	/* io_write */
	NULL,	/* get_io_ptr */
	NULL,	/* command */
	io_sigio,		/* sigio */
	NULL,	/* save */
	NULL	/* restore */
};

/* THIS IS WHERE THE I/O SYSTEM IS ATTACHED */

struct io_subsystem *iosystem = &libio;

/* AHB dummy, remove if real AHB module is used */
struct ahb_subsystem *ahbsystem = NULL;

