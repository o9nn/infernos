/*
 * errstr-posix.c - Error string functions for POSIX systems
 * 
 * This file avoids including lib9.h to prevent conflicts.
 */
#include <errno.h>
#include <string.h>
#include <stdarg.h>

typedef unsigned int uint;

#define ERRMAX 128

/* External functions from lib9 */
extern int vseprint(char*, char*, char*, va_list);
extern char* utfecpy(char*, char*, char*);

static char errstring[ERRMAX];

enum
{
	Magic = 0xffffff
};

void
werrstr(char *fmt, ...)
{
	va_list arg;

	va_start(arg, fmt);
	vseprint(errstring, errstring+sizeof(errstring), fmt, arg);
	va_end(arg);
	errno = Magic;
}

void
oserrstr(char *buf, uint nerr)
{
	char *s;

	if(errno != EINTR)
		s = strerror(errno);
	else
		s = "interrupted";
	utfecpy(buf, buf+nerr, s);
}

int
errstr(char *buf, uint nerr)
{
	if(errno == Magic)
		utfecpy(buf, buf+nerr, errstring);
	else
		oserrstr(buf, nerr);
	return 1;
}
