/*
 * rerrstr.c - Read error string function
 * 
 * This file avoids including lib9.h to prevent conflicts.
 */

typedef unsigned int uint;

#define ERRMAX 128

/* External functions */
extern int errstr(char*, uint);
extern char* utfecpy(char*, char*, char*);

void
rerrstr(char *buf, uint nbuf)
{
	char tmp[ERRMAX];

	tmp[0] = 0;
	errstr(tmp, sizeof tmp);
	utfecpy(buf, buf+nbuf, tmp);
	errstr(tmp, sizeof tmp);
}
