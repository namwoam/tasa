
/* standalone TSIM application using the I/O module interface */
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include "tsim.h"

#define __erc32__

#ifdef __erc32__
#define MEMBASE 0x2000000
#else
#define MEMBASE 0x40000000
#endif
void myprint (char *st);
void ev3 (int p);
void ev1 (int p) { printf("event handler 1, arg = %i\n", p);}
void ev2 (int p) 
{
    printf("event handler 2, arg = %i\n", p);
    tsim_cmd ("ev");
    printf("remove event handler 3\n");
    tsim_stop_event (ev3, 1, 0);
    tsim_cmd ("ev");
}
void ev3 (int p) { printf("event handler 3, arg = %i\n", p);}

void retthandler()
{
	printf ("rett:\n");
}

int traphandler(int tt)
{
	static int cnt = 4;
	printf ("trap: 0x%x\n", tt);
	if (cnt) {
	    cnt--;
	    return(0);
	} else 
	    return(1);
}

unsigned char
gdbgetchar ()
{
    const char cmd[] = "$g#67+$D#44+";	/* print registers and detach */
    static int index = 0;

    if (cmd[index])
      {
//              putchar(cmd[index]);
	  return (cmd[index++]);
      }
    else
      {
	  printf ("warning: read past gdb command buffer\n");
	  return ('+');
      }
}

void
gdbputchar (unsigned char c)
{
    putchar (c);
}

int
main (argc, argv)
     int argc;
     char **argv;
{

    int stat, data, i, j;
    unsigned int regs[NUMREGS];

    i = tsim_init ("");
    if (!i) exit(1);
    printf (" Current time %llu (%1.3e s)\n", simif.simtime (),
		  (double) (simif.simtime ()) / simif.options->freq /
		  1000000.0);
    printf(" Inrementing time with 14000 ticks\n");
//    tsim_inc_time((uint64) 14000);
    printf (" Current time %llu (%1.3e s)\n", simif.simtime (),
		  (double) (simif.simtime ()) / simif.options->freq /
		  1000000.0);
    printf("adding 7 events:\n");
    simif.event(ev1, 1, 10);
    simif.event(ev1, 1, 10);
    simif.event(ev1, 1, 10);
    simif.event(ev1, 2, 10);
    simif.event(ev1, 3, 10);
    simif.event(ev2, 1, 10);
    simif.event(ev3, 1, 10);
    tsim_cmd ("ev");
    printf("remove event handler 1, arg 1, 1 instance\n");
    tsim_stop_event (ev1, 1, 2);
    tsim_cmd ("ev");
    printf("remove event handler 1, arg 1, all instances\n");
    tsim_stop_event (ev1, 1, 1);
    tsim_cmd ("ev");
    printf("remove event handler 1, any arg, all instances\n");
    tsim_stop_event (ev1, 1, 0);
    tsim_cmd ("ev");

    tsim_trap(traphandler, retthandler);

    tsim_read (MEMBASE, &data);
    printf ("data : %08x\n", data);
    data = 0x01234567;
    tsim_write (MEMBASE + 0x10, data);
    tsim_read (MEMBASE + 0x10, &data);
    printf ("data : %08x\n", data);

    tsim_read (0x0000010, &data);
    printf ("data : %08x\n", data);
    data = 0x01234567;
    tsim_write (0x0000010, data);
    tsim_read (0x0000010, &data);
    printf ("data : %08x\n", data);

    tsim_cmd ("lo stanford.exe");
#ifdef __erc32__
    tsim_cmd ("mem 0x2000000");
#else
    tsim_cmd ("mem 0x40000000");
#endif

    stat = tsim_cmd ("go");
    tsim_trap(NULL, NULL);
    printf("trap handler removed\n");
    while (tsim_cmd ("cont 10 ms") != SIGTERM);
//    stat = tsim_cmd ("cont 400 s");
    switch (stat)
      {
      case 0:
	  break;
      case SIGINT:
	  printf ("\b\bInterrupt!\n");
      case SIGHUP:
	  printf (" Stopped at time %llu (%1.3e s)\n", simif.simtime (),
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
    printf (" Current time %llu (%1.3e s)\n", simif.simtime (),
		  (double) (simif.simtime ()) / simif.options->freq /
		  1000000.0);
    printf(" Inrementing time with 14000000 ticks\n");
    tsim_inc_time((uint64) 14000000);
    printf (" Current time %llu (%1.3e s)\n", simif.simtime (),
		  (double) (simif.simtime ()) / simif.options->freq /
		  1000000.0);
    printf ("reading processor registers through gdb interface\n\n");
    tsim_gdb (gdbgetchar, gdbputchar);

    printf ("\nreading processor registers through tsim_get_regs()\n\n");
    tsim_get_regs (regs);
    for (i=0; i<72; i++) {
	printf("0x%08x  ", regs[i]);
	if ((i&3) == 3) printf("\n");
    }
    regs[2] = 0x01234567;
    regs[3] = 0x76543210;
    tsim_set_regs (regs);
    printf ("\nsetting processor registers 2 and 3 through tsim_set_regs()\n\n");
    tsim_get_regs (regs);
    for (i=0; i<4; i++) {
	printf("0x%08x  ", regs[i]);
	if ((i&3) == 3) printf("\n");
    }
    printf("\n");
    tsim_exit (0);
}
