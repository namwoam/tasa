
/* standalone TSIM application using the standard I/O module interface */
#include <signal.h>
#include <sys/fcntl.h>
#include "tsim.h"

void myprint(char *st);

int
main (argc, argv)
     int argc;
     char **argv;
{

    int stat;
    unsigned int regs[NUMREGS];

    tsim_init ("");
    tsim_cmd ("lo stanford.exe");
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
	  tsim_get_regs(regs);
          printf ("breakpoint at 0x%08x reached\n", regs[PC]);
	  break;
      case SIGSEGV:
	  tsim_get_regs(regs);
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
