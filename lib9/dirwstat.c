/*
 * dirwstat.c - Directory write stat functions for POSIX systems
 * Include system headers first to avoid conflicts
 */
#include <sys/types.h>
#include <sys/stat.h>
#include "lib9.h"

int
dirfwstat(int fd, Dir *d)
{
	return -1;
}

int
dirwstat(char *name, Dir *d)
{
	return -1;
}
