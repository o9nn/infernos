/*
 * linux-compat.h - Linux compatibility layer for Inferno OS
 * 
 * This header provides compatibility definitions for building
 * Inferno OS components on Linux systems.
 * 
 * Copyright (c) 2026 Inferno OS Contributors
 * 
 * This file should be included FIRST in lib9.h before other includes.
 */
#ifndef _LINUX_COMPAT_H
#define _LINUX_COMPAT_H

#ifdef __linux__

/*
 * Prevent conflicts with POSIX/glibc functions
 * These renames must happen BEFORE any system headers are included
 */
#define stat    inferno_stat
#define fstat   inferno_fstat
#define lstat   inferno_lstat
#define open    inferno_open
#define create  inferno_create
#define strdup  inferno_strdup
#define strtoll inferno_strtoll
#define strtoul inferno_strtoul
#define pow10   inferno_pow10

/*
 * Plan 9/Inferno type definitions
 * These types are used throughout the Inferno codebase
 * and must be defined before fcall.h is included
 */
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

#endif /* _PLAN9_TYPES_DEFINED */

/*
 * Use proper pointer-sized integers for casts
 * This fixes warnings about pointer-to-int-cast on 64-bit systems
 */
#include <stdint.h>
typedef intptr_t    intptr;
typedef uintptr_t   uintptr;

/*
 * Fix execl signature to match POSIX
 * The Inferno declaration uses (char*, ...) but POSIX requires (const char*, const char*, ...)
 */
#define EXECL_POSIX_COMPAT 1

#endif /* __linux__ */

#endif /* _LINUX_COMPAT_H */
