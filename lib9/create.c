/*
 * create.c - File creation function for POSIX systems
 * 
 * This file avoids including lib9.h to prevent conflicts.
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* Inferno mode constants */
#define OREAD   0
#define OWRITE  1
#define ORDWR   2
#define OEXEC   3
#define DMDIR   0x80000000

int
create(char *f, int mode, int perm)
{
	int m;

	m = 0;
	switch(mode & 3){
	case OREAD:
	case OEXEC:
		m = O_RDONLY;
		break;
	case OWRITE:
		m = O_WRONLY;
		break;
	case ORDWR:
		m = O_RDWR;
		break;
	}
	m |= O_CREAT|O_TRUNC;

	if(perm & DMDIR){
		if(mkdir(f, perm&0777) < 0)
			return -1;
		perm &= ~DMDIR;
		m &= 3;
	}
	return open(f, m, perm);
}
