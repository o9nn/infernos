/*
 * system- and machine-specific declarations for emu:
 * floating-point save and restore, signal handling primitive, and
 * implementation of the current-process variable `up'.
 */

/*
 * The current-process pointer `up' is stored in pthread thread-specific
 * data when the emulator is built with the USE_PTHREADS scheduler
 * (the default on Linux).  `getup()' is provided by emu/port/kproc-pthreads.c.
 */
extern	Proc*	getup(void);
#define	up	(getup())

typedef	struct	FPU	FPU;

/*
 * This structure must agree with FPsave and FPrestore asm routines
 */
struct FPU
{
	uchar	env[28];
};

typedef sigjmp_buf osjmpbuf;
#define	ossetjmp(buf)	sigsetjmp(buf, 1)

#define	KSTACK	(32*1024)	/* kernel stack size; increased for Linux gethost* functions */
