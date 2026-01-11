/*
 * lib9.h - Plan 9/Inferno C library interface
 * This header provides the basic types and interfaces for the Inferno system.
 */

#ifndef LIB9_H
#define LIB9_H

/* Include Linux compatibility layer FIRST on Linux systems */
#ifdef __linux__
#include "linux-compat.h"
#endif

/* Need va_list for kern.h function declarations */
#include <stdarg.h>
/* Need jmp_buf and setjmp/longjmp */
#include <setjmp.h>

/* Basic Plan 9 types - must be defined before including kern.h */
#define nil ((void*)0)
typedef unsigned short ushort;
typedef unsigned char uchar;
typedef unsigned long ulong;
typedef unsigned int uint;
typedef signed char schar;
typedef long long vlong;
typedef unsigned long long uvlong;
typedef unsigned long uintptr;
typedef uint Rune;

/* Plan 9/Inferno extended types for fcall.h and other headers */
#ifndef _PLAN9_TYPES_DEFINED
#define _PLAN9_TYPES_DEFINED
typedef unsigned char       u8int;
typedef unsigned short      u16int;
typedef unsigned int        u32int;
typedef unsigned long long  u64int;
typedef signed char         s8int;
typedef signed short        s16int;
typedef signed int          s32int;
typedef signed long long    s64int;
#endif

/* Include the full kernel/system interface which provides all constants and functions */
#include <kern.h>

#endif /* LIB9_H */
