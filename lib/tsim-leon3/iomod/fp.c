/* This file implements a SPARC FPU using the host's floating-point
   capabilities. Parallel IU/FPU operation and data dependency interlocks
   are not implemented.

   Copyright Gaisler Research, all rights reserved.

   */

#include "end.h"
#include "tsim.h"
#include <math.h>
#include <stdio.h>

#define TRAP_FPEXC 8
#define FSR_TT		0x1C000
#define FP_IEEE		0x04000
#define FP_UNIMP	0x0C000
#define FP_SEQ_ERR	0x10000

#define FSR_QNE 	0x2000
#define FP_EXE_MODE 0
#define	FP_EXC_PE   1
#define FP_EXC_MODE 2
#define FPUQN 1

static int fpmeiko (uint32 pc, uint32 inst, int *hold);

struct fpregtype
{
    float64 fd[16];		/* FPU registers */
    float32 *fs;
    int32 *fsi;
    uint32 fsr;
    int32 fpstate;
    uint32 ftime;
    uint32 fhold;
    uint32 fpq[FPUQN * 2];
    uint32 fpqn;
};

static struct fpregtype fpregs;

static void
cp_init ()
{
    fpregs.fpstate = FP_EXE_MODE;
    fpregs.fpqn = 0;
    fpregs.ftime = 0;
    fpregs.fs = (float32 *) fpregs.fd;
    fpregs.fsi = (int32 *) fpregs.fd;
    fpregs.fsr = 0;
    printf ("fp_init\n");
}

static uint32
cp_reg (int reg, uint32 data, int read)
{
    if (read)
      {
	  if (reg < 32)
	    {
#ifdef HOST_LITTLE_ENDIAN
		return (fpregs.fsi[reg ^ 1]);
#else
		return (fpregs.fsi[reg]);
#endif
	    }
	  else if (reg == 34)
	    {
		return (fpregs.fsr);
	    }
      }
    else
      {
	  if (reg < 32)
	    {
#ifdef HOST_LITTLE_ENDIAN
		fpregs.fsi[reg ^ 1] = data;
#else
		fpregs.fsi[reg] = data;
#endif
	    }
	  else
	    {
		fpregs.fsr = data;
		set_fsr (data);
	    }
      }
}

static int
cp_load (int reg, uint32 data, int *hold)
{
    *hold = 0;
    if (fpregs.fpstate == FP_EXC_PE)
      {
	  fpregs.fpstate = FP_EXC_MODE;
	  return (TRAP_FPEXC);
      }
    if (reg < 32)
      {
#ifdef HOST_LITTLE_ENDIAN
	  fpregs.fsi[reg ^ 1] = data;
#else
	  fpregs.fsi[reg] = data;
#endif

      }
    else
      {
	  fpregs.fsr = data;
	  set_fsr (data);
      }
    return (0);
}

static int
cp_store (int reg, uint32 * data, int *hold)
{
    *hold = 0;
    if (fpregs.fpstate == FP_EXC_PE)
      {
	  fpregs.fpstate = FP_EXC_MODE;
	  return (TRAP_FPEXC);
      }
    if (reg < 32)
      {
#ifdef HOST_LITTLE_ENDIAN
	  *data = fpregs.fsi[reg ^ 1];
#else
	  *data = fpregs.fsi[reg];
#endif
	  return (0);
      }
    else if (reg == 34)
      {
	  *data = fpregs.fsr;
	  return (0);
      }
    else
      {
	  if (!(fpregs.fsr & FSR_QNE))
	    {
		fpregs.fsr = (fpregs.fsr & ~FSR_TT) | FP_SEQ_ERR;
		return (TRAP_FPEXC);
	    }
	  data[0] = fpregs.fpq[0];
	  data[1] = fpregs.fpq[1];
	  fpregs.fsr &= ~FSR_QNE;
	  fpregs.fpstate = FP_EXE_MODE;
	  return (0);
      }
}

static int
cp_cc (int *cc, int *hold)
{
    if (fpregs.fpstate == FP_EXC_PE)
      {
	  fpregs.fpstate = FP_EXC_MODE;
	  return (TRAP_FPEXC);
      }
    *hold = 0;
    *cc = (fpregs.fsr >> 10) & 3;
    return (0);
}


static void
cp_print ()
{

    int i;

    printf ("\n fsr: %08X\n\n", fpregs.fsr);

    for (i = 0; i < 32; i++)
      {
#ifdef HOST_LITTLE_ENDIAN
	  printf (" f%02d  %08x  %14e  ", i, fpregs.fsi[i ^ 1],
		  fpregs.fs[i ^ 1]);
#else
	  printf (" f%02d  %08x  %14e  ", i, fpregs.fsi[i], fpregs.fs[i]);
#endif
	  if (!(i & 1))
	      printf ("%14e\n", fpregs.fd[i >> 1]);
	  else
	      printf ("\n");
      }
    printf ("\n");
}

static struct cp_interface test_fpu = {
    cp_init,			/* cp_init */
    NULL,			/* cp_exit */
    NULL,			/* cp_reset */
    cp_init,			/* cp_restart */
    cp_reg,			/* cp_reg  */
    cp_load,			/* cp_load */
    cp_store,			/* cp_store */
    fpmeiko,			/* cp_exec */
    cp_cc,			/* cp_cc */
    &fpregs.fpstate,		/* cp_status */
    cp_print,			/* cp_print */
    NULL			/* cp_command */
};

struct cp_interface *cp = &test_fpu;

#define T_FABSs		4
#define T_FADDs		6
#define T_FADDd		6
#define T_FCMPs		6
#define T_FCMPd		6
#define T_FDIVs		20
#define T_FDIVd		36
#define T_FMOVs		4
#define T_FMULs		7
#define T_FMULd		11
#define T_FNEGs		4
#define T_FSQRTs	39
#define T_FSQRTd	67
#define T_FSUBs		8
#define T_FSUBd		8
#define T_FdTOi		10
#define T_FdTOs		5
#define T_FiTOs		8
#define T_FiTOd		8
#define T_FsTOi		9
#define T_FsTOd		4

#define FABSs	0x09
#define FADDs	0x41
#define FADDd	0x42
#define FCMPs	0x51
#define FCMPd	0x52
#define FCMPEs	0x55
#define FCMPEd	0x56
#define FDIVs	0x4D
#define FDIVd	0x4E
#define FMOVs	0x01
#define FMULs	0x49
#define FMULd	0x4A
#define FNEGs	0x05
#define FSQRTs	0x29
#define FSQRTd	0x2A
#define FSUBs	0x45
#define FSUBd	0x46
#define FdTOi	0xD2
#define FdTOs	0xC6
#define FiTOs	0xC4
#define FiTOd	0xC8
#define FsTOi	0xD1
#define FsTOd	0xC9

static int
fpmeiko (uint32 pc, uint32 inst, int *hold)
{
    uint32 opf, tem, accex;
    int32 fcc;
    uint32 ldadj;
    uint32 rd, rs1, rs2;
#ifdef FPUDEBUG
    int dsingle = 0;
    int ddouble = 0;
#endif

    if (fpregs.fpstate == FP_EXC_MODE)
      {
	  fpregs.fsr = (fpregs.fsr & ~FSR_TT) | FP_SEQ_ERR;
	  fpregs.fpstate = FP_EXC_PE;
	  return (0);
      }
    if (fpregs.fpstate == FP_EXC_PE)
      {
	  fpregs.fpstate = FP_EXC_MODE;
	  return (TRAP_FPEXC);
      }
    rs1 = (inst >> 14) & 0x1f;
    rs2 = inst & 0x1f;
    rd = (inst >> 25) & 0x1f;
    opf = (inst >> 5) & 0x1ff;
    fpregs.ftime = 1;

    /* SPARC is big-endian - swap float register address if host is little-endian */

#ifdef HOST_LITTLE_ENDIAN
    rs1 = rs1 ^ 1;
    rs2 = rs2 ^ 1;
    rd = rd ^ 1;
#endif

    switch (opf)
      {
      case FABSs:
	  fpregs.fs[rd] = fabs (fpregs.fs[rs2]);
	  fpregs.ftime += T_FABSs;
	  break;
      case FADDs:
	  fpregs.fs[rd] = fpregs.fs[rs1] + fpregs.fs[rs2];
	  fpregs.ftime += T_FADDs;
	  break;
      case FADDd:
	  fpregs.fd[rd >> 1] = fpregs.fd[rs1 >> 1] + fpregs.fd[rs2 >> 1];
	  fpregs.ftime += T_FADDd;
	  break;
      case FCMPs:
      case FCMPEs:
	  if (fpregs.fs[rs1] == fpregs.fs[rs2])
	      fcc = 3;
	  else if (fpregs.fs[rs1] < fpregs.fs[rs2])
	      fcc = 2;
	  else if (fpregs.fs[rs1] > fpregs.fs[rs2])
	      fcc = 1;
	  else
	      fcc = 0;
	  fpregs.fsr |= 0x0C00;
	  fpregs.fsr &= ~(fcc << 10);
	  fpregs.fhold += T_FCMPs;
	  if ((fcc == 0) && (opf == FCMPEs))
	    {
		fpregs.fpstate = FP_EXC_PE;
		fpregs.fsr = (fpregs.fsr & ~0x1C000) | (1 << 14);
	    }
	  break;
      case FCMPd:
      case FCMPEd:
	  if (fpregs.fd[rs1 >> 1] == fpregs.fd[rs2 >> 1])
	      fcc = 3;
	  else if (fpregs.fd[rs1 >> 1] < fpregs.fd[rs2 >> 1])
	      fcc = 2;
	  else if (fpregs.fd[rs1 >> 1] > fpregs.fd[rs2 >> 1])
	      fcc = 1;
	  else
	      fcc = 0;
	  fpregs.fsr |= 0x0C00;
	  fpregs.fsr &= ~(fcc << 10);
	  fpregs.fhold += T_FCMPd;
	  if ((fcc == 0) && (opf == FCMPEd))
	    {
		fpregs.fpstate = FP_EXC_PE;
		fpregs.fsr = (fpregs.fsr & ~FSR_TT) | FP_IEEE;
	    }
	  break;
      case FDIVs:
	  fpregs.fs[rd] = fpregs.fs[rs1] / fpregs.fs[rs2];
	  fpregs.ftime += T_FDIVs;
	  break;
      case FDIVd:
	  fpregs.fd[rd >> 1] = fpregs.fd[rs1 >> 1] / fpregs.fd[rs2 >> 1];
	  fpregs.ftime += T_FDIVd;
	  break;
      case FMOVs:
	  fpregs.fs[rd] = fpregs.fs[rs2];
	  fpregs.ftime += T_FMOVs;
	  break;
      case FMULs:
	  fpregs.fs[rd] = fpregs.fs[rs1] * fpregs.fs[rs2];
	  fpregs.ftime += T_FMULs;
	  break;
      case FMULd:
	  fpregs.fd[rd >> 1] = fpregs.fd[rs1 >> 1] * fpregs.fd[rs2 >> 1];
	  fpregs.ftime += T_FMULd;
	  break;
      case FNEGs:
	  fpregs.fs[rd] = -fpregs.fs[rs2];
	  fpregs.ftime += T_FNEGs;
	  break;
      case FSQRTs:
	  if (fpregs.fs[rs2] < 0.0)
	    {
		fpregs.fpstate = FP_EXC_PE;
		fpregs.fsr = (fpregs.fsr & ~FSR_TT) | FP_IEEE;
		fpregs.fsr = (fpregs.fsr & 0x1f) | 0x10;
		break;
	    }
	  fpregs.fs[rd] = sqrt (fpregs.fs[rs2]);
	  fpregs.ftime += T_FSQRTs;
	  break;
      case FSQRTd:
	  if (fpregs.fd[rs2 >> 1] < 0.0)
	    {
		fpregs.fpstate = FP_EXC_PE;
		fpregs.fsr = (fpregs.fsr & ~FSR_TT) | FP_IEEE;
		fpregs.fsr = (fpregs.fsr & 0x1f) | 0x10;
		break;
	    }
	  fpregs.fd[rd >> 1] = sqrt (fpregs.fd[rs2 >> 1]);
	  fpregs.ftime += T_FSQRTd;
	  break;
      case FSUBs:
	  fpregs.fs[rd] = fpregs.fs[rs1] - fpregs.fs[rs2];
	  fpregs.ftime += T_FSUBs;
	  break;
      case FSUBd:
	  fpregs.fd[rd >> 1] = fpregs.fd[rs1 >> 1] - fpregs.fd[rs2 >> 1];
	  fpregs.ftime += T_FSUBd;
	  break;
      case FdTOi:
	  fpregs.fsi[rd] = (int) fpregs.fd[rs2 >> 1];
	  fpregs.ftime += T_FdTOi;
	  break;
      case FdTOs:
	  fpregs.fs[rd] = (float32) fpregs.fd[rs2 >> 1];
	  fpregs.ftime += T_FdTOs;
	  break;
      case FiTOs:
	  fpregs.fs[rd] = (float32) fpregs.fsi[rs2];
	  fpregs.ftime += T_FiTOs;
	  break;
      case FiTOd:
	  fpregs.fd[rd >> 1] = (float64) fpregs.fsi[rs2];
	  fpregs.ftime += T_FiTOd;
	  break;
      case FsTOi:
	  fpregs.fsi[rd] = (int) fpregs.fs[rs2];
	  fpregs.ftime += T_FsTOi;
	  break;
      case FsTOd:
	  fpregs.fd[rd >> 1] = fpregs.fs[rs2];
	  fpregs.ftime += T_FsTOd;
	  break;

      default:
	  fpregs.fsr = (fpregs.fsr & ~FSR_TT) | FP_UNIMP;
	  fpregs.fpstate = FP_EXC_PE;
      }

#define  accex  0

    if (fpregs.fpstate == FP_EXC_PE)
      {
	  fpregs.fpq[0] = pc;
	  fpregs.fpq[1] = inst;
	  fpregs.fsr |= FSR_QNE;
      }
    else
      {
	  tem = (fpregs.fsr >> 23) & 0x1f;
	  if (tem & accex)
	    {
		fpregs.fpstate = FP_EXC_PE;
		fpregs.fsr = (fpregs.fsr & ~FSR_TT) | FP_IEEE;
		fpregs.fsr = ((fpregs.fsr & ~0x1f) | accex);
	    }
	  else
	    {
		fpregs.fsr = ((((fpregs.fsr >> 5) | accex) << 5) | accex);
	    }
	  if (fpregs.fpstate == FP_EXC_PE)
	    {
		fpregs.fpq[0] = pc;
		fpregs.fpq[1] = inst;
		fpregs.fsr |= FSR_QNE;
	    }
      }

    *hold = fpregs.ftime / 2;	/* Assign half of the instruction timing to
				   emulate approximate parallel operation */
    return (0);
}
