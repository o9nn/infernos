/*
 * seek.c - File seek function for POSIX systems
 * Include system headers first to avoid conflicts
 */
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include "lib9.h"

vlong
seek(int fd, vlong where, int from)
{
	return lseek(fd, where, from);
}
