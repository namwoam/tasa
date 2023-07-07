
/* TSIM functions that can be used by I/O sub-system */
/* all functions are exported via struct 'simif' at the bottom */

/* the endianess of the host is defined in end.h */
#include "end.h"

#ifndef unix
#define EXTERN __declspec(dllimport) 
#else
#define EXTERN extern
#endif

#ifdef WIN32
#define	SIGTRAP	5	/* trace trap (not reset when caught) */
#define	SIGHUP	1	/* hangup */
#endif

/* type definitions */
typedef short int int16;	/* 16-bit signed int */
typedef unsigned short int uint16;	/* 16-bit unsigned int */
typedef int     int32;		/* 32-bit signed int */
typedef unsigned int uint32;	/* 32-bit unsigned int */
typedef float   float32;	/* 32-bit float */
typedef double  float64;	/* 64-bit float */
#ifdef __GNUC__
typedef unsigned long long uint64;
#else
typedef __int64 uint64;
#endif
struct sim_options {
    uint32	phys_ram;
    uint32	phys_rom;
    float64	freq;
    float64	wdfreq;
    uint32	phys_sdram;
};

/*****************************************************************
 * exported TSIM functions and variables                         *
 *****************************************************************/
struct sim_interface {
    struct sim_options  *options;	/* sis command-line options */
    uint64   (*simtime)();              	/* current simulator time */
    void     (*event)(void (*cfunc)(), uint32 arg, uint64 offset);
    void     (*stop_event)(void (*cfunc)());
    uint32     *irl;			/* interrup request level */
    void     (*sys_reset)();		/* reset processor */
    void     (*sim_stop)();		/* stop simulation */
};

/* I/O module functions and variables */
/* dma_read, dma_write:  address indicates
   start address, *data points to first data in transfer, num indicates
   number of transfers. Note that only word-transfers are allowed.
   On error (MEXC), 1 is returned, otherwise 0 */

struct io_interface {
    void   (*set_irq)(uint32 irq, uint32 level);  /* generate external interrupt */
    int    (*dma_read)(uint32 addr, uint32 *data, uint32 num);
    int    (*dma_write)(uint32 addr, uint32 *data, uint32 num);
};

EXTERN struct sim_interface simif; /* exported simulator functions */
EXTERN struct io_interface ioif;   /* exported processor interface */

/* structure of function to be provided by I/O subsystem */
struct io_subsystem {
    /* called once on start-up */
    void   (*io_init)(struct sim_interface sif, struct io_interface iif);
    void   (*io_exit)();	/* called once on exit */
    void   (*io_reset)();	/* called on processor reset */
    void   (*io_restart)();	/* called on simulator restart */
    int	   (*io_read)(unsigned int addr, int *data, int *ws);
    int	   (*io_write)(unsigned int addr, int *data, int *ws, int size);
    char   *(*get_io_ptr)(unsigned int addr, int size);
    int    (*command)(char * cmd); /* I/O specific commands */
    void   (*sigio)();		/* called when SIGIO occurs */
    void   (*save)(char * fname); /* save state */
    void   (*restore)(char * fname); /* restore state */
};

/* structure of function to be provided by an external co-processor */
struct cp_interface {
    void   (*cp_init)();	/* called once on start-up */
    void   (*cp_exit)();	/* called once on exit */
    void   (*cp_reset)();	/* called on processor reset */
    void   (*cp_restart)();	/* called on simulator restart */
    uint32 (*cp_reg)(int reg, uint32 data, int read);
    int    (*cp_load)(int reg, uint32 data, int *hold);
    int    (*cp_store)(int reg, uint32 *data, int *hold);
    int	   (*cp_exec)(uint32 pc, uint32 inst, int *hold);
    int	   (*cp_cc)(int *cc, int *hold);  /* get condition codes */
    int    *cp_status;		/* unit status */
    void   (*cp_print)();	/* print registers */
    int    (*command)(char * cmd); /* CP specific commands */
};

#define NUMREGS 72
enum regnames { 
    G0, G1, G2, G3, G4, G5, G6, G7,
    O0, O1, O2, O3, O4, O5, SP, O7,
    L0, L1, L2, L3, L4, L5, L6, L7,
    I0, I1, I2, I3, I4, I5, FP, I7,
    F0, F1, F2, F3, F4, F5, F6, F7,
    F8, F9, F10, F11, F12, F13, F14, F15,
    F16, F17, F18, F19, F20, F21, F22, F23,
    F24, F25, F26, F27, F28, F29, F30, F31,
    Y, PSR, WIM, TBR, PC, NPC, FPSR, CPSR
};

#define TSIM_COV_EXEC 	1
#define TSIM_COV_WRITE	2
#define TSIM_COV_READ	4

EXTERN int  tsim_init (char *option);	/* initialise tsim with optional params. */
EXTERN int  tsim_cmd (char *cmd);	/* execute tsim command */
EXTERN void tsim_exit (int val);	/* should be called before program exit */
EXTERN void tsim_get_regs (unsigned int *regs);	/* get SPARC registers */
EXTERN void tsim_set_regs (unsigned int *regs);	/* set SPARC registers */
EXTERN void tsim_disas(unsigned int addr, int num);	/* disassemble memory */
EXTERN void tsim_read(unsigned int addr, unsigned int *data);	/* read memory */
EXTERN void tsim_write(unsigned int addr, unsigned int data);	/* write memory */
EXTERN void tsim_set_diag (void(*cfunc)(char *));	/* set output files */
EXTERN void tsim_set_callback (void (*cfunc)(void));
EXTERN void tsim_gdb (unsigned char (*inchar)(), void (*outchar)(unsigned char c));
EXTERN void tsim_stop_event (void(*cfunc)(), int arg, int op);	/* remove event */
EXTERN void tsim_inc_time (uint64 leap);	/* increment simulator time */
EXTERN void tsim_trap (int (*trap)(int tt), void (*rett));	/* callback on trap */
EXTERN void tsim_cov_get(int start, int end, char *ptr); /* get coverage */
EXTERN void tsim_cov_set(int start, int end, char val);  /* set coverage */
EXTERN int tsim_lastbp (int *addr);		/* last break/watchpoint addr */


struct ahb_access {
    uint32 address;
    uint32 *data;
    uint32 ws;
    uint32 rnum;
    uint32 wsize;
    uint32 cache;
};

struct ahb_subsystem {
    void   (*init)();	/* called once on start-up */
    void   (*exit)();	/* called once on exit */
    void   (*reset)();	/* called on processor reset */
    void   (*restart)();	/* called on simulator restart */
    int	   (*read)(struct ahb_access *access);
    int	   (*write)(struct ahb_access *access);
    char   *(*get_io_ptr)(unsigned int addr, int size);
    int    (*command)(char * cmd); /* I/O specific commands */
    int   (*sigio)();		/* called when SIGIO occurs */
    void   (*save)(char * fname); /* save state */
    void   (*restore)(char * fname); /* restore state */
    int   (*intack)(int level); /* interrupt acknowledge */
};

struct proc_interface {
    void   (*set_irl)(int level);  /* generate external interrupt */
    void   (*cache_snoop)(uint32 addr);
    void   (*cctrl)(uint32 *data, uint32 read);
    void   (*power_down)();
};

struct ins_interface {
    uint32          psr;	/* IU registers */
    uint32          tbr;
    uint32          wim;
    uint32          g[8];
    uint32          r[128];
    uint32          y;
    uint32          pc;
    uint32          npc;
    uint32          inst;	/* Current instruction */
    uint32          icnt;	/* Clock cycles in curr inst */
    uint32          asr17;
    uint32          asr18;
};

EXTERN int tsim_ext_ins (int (*func) (struct ins_interface *r)); /* custom instruction */

EXTERN struct proc_interface procif;   /* exported processor interface */
