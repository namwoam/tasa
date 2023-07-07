
/* Standalone TSIM application using simple_io module */

#include <signal.h>
#include <sys/fcntl.h>
#include "tsim.h"

/* forward declarations */

void myprint (char *st);
int iowrite (unsigned int address, unsigned int *data, unsigned int *ws,
	     int size);
int ioread (unsigned int address, unsigned int *data, unsigned int *ws);

/* main */

int
main (argc, argv)
     int argc;
     char **argv;
{

    int stat;
    unsigned int regs[NUMREGS];

    tsim_set_diag (myprint);
    tsim_set_ioread (ioread);
    tsim_set_iowrite (iowrite);
    tsim_init ("");
//    tsim_init ("-nouart"); /* disable uarts if you want to handle them self */
    tsim_cmd ("lo test");
    stat = tsim_cmd ("go");
    switch (stat)
      {
      case 0:
	  break;
      case SIGINT:
	  printf ("\b\bInterrupt!\n");
      case SIGHUP:
	  printf (" Stopped at time %ll (%1.3e s)\n", simif.simtime (),
		  (double) (simif.simtime ()) / simif.options->freq /
		  1000000.0);
	  break;
      case SIGTRAP:
	  tsim_get_regs (regs);
	  printf ("breakpoint at 0x%08x reached\n", regs[PC]);
	  break;
      case SIGSEGV:
	  tsim_get_regs (regs);
	  printf ("IU in error mode (%d)\n", (regs[TBR] >> 4) & 0x0ff);
	  stat = 0;
	  printf (" %ll ", simif.simtime ());
	  tsim_disas (regs[PC], 1);
	  break;
      case SIGTERM:
	  printf ("\nProgram exited normally.\n");
	  stat = 0;
	  break;
      default:
	  break;
      }
    tsim_cmd ("perf");
    tsim_exit (0);
}

/* custom diagnostics print routine */

void
myprint (char *st)
{
    printf ("%s", st);
}

/* custom I/O write handler */
int
iowrite (unsigned int address, unsigned int *data, unsigned int *ws, int size)
{
    *ws = 0;
    if ((address & 0xffff0000) == 0x01f80000) /* handle UARTs */
	switch (address & 0xff)
	  {
	  case 0xE0:
	      putchar (*data);
	      ioif.set_irq (5, 4);	/* SPARC irq 4 */
	      break;
	  case 0xE4:
	      putchar (*data);
	      ioif.set_irq (5, 5);	/* SPARC irq 5 */
	      break;
	  }
    else
	printf ("iowrite: %x, %x\n", address, *data); /* everything else */

    return (0);
}

/* custom I/O read handler */
int
ioread (unsigned int address, unsigned int *data, unsigned int *ws)
{
    *ws = 0;
    if ((address & 0xffff0000) == 0x01f80000) /* handle UARTs */
	switch (address & 0xff)
	  {
	  case 0xE0:
	  case 0xE4:
	      *data = 0;
	      break;
	  case 0xE8:
	      *data = 0x70007;
	      break;
	  }
    else
	printf ("ioread: %x\n", address); /* everything else */

    return (0);
}
