/*
 * linux-compat.h - Linux compatibility layer for Inferno OS
 * 
 * This header provides compatibility definitions for building
 * Inferno OS components on Linux systems.
 * 
 * Copyright (c) 2026 Inferno OS Contributors
 * 
 * IMPORTANT: This file should be included FIRST in lib9.h before other includes.
 * 
 * The approach here is to:
 * 1. Define Plan 9 types that are missing (u32int, etc.)
 * 2. NOT rename POSIX functions via macros (that causes conflicts)
 * 3. Instead, we'll suppress the conflicting declarations via compiler flags
 */
#ifndef _LINUX_COMPAT_H
#define _LINUX_COMPAT_H

#ifdef __linux__

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

#endif /* __linux__ */

#endif /* _LINUX_COMPAT_H */
