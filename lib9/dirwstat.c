/*
 * dirwstat.c - Directory write stat functions for POSIX systems
 * 
 * This file avoids including lib9.h to prevent conflicts.
 */
#include <sys/types.h>
#include <sys/stat.h>

/* Minimal Dir structure definition */
typedef struct Dir Dir;
struct Dir {
	unsigned short type;
	unsigned int dev;
	/* ... other fields not needed for these stubs */
};

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
