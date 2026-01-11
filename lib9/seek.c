/*
 * seek.c - File seek function for POSIX systems
 * 
 * This file avoids including lib9.h to prevent conflicts.
 */
#include <sys/types.h>
#include <unistd.h>

typedef long long vlong;

vlong
seek(int fd, vlong where, int from)
{
	return lseek(fd, where, from);
}
