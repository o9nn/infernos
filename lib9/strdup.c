/*
 * strdup.c - String duplication function
 * Include system headers first for malloc
 */
#include <stdlib.h>
#include <string.h>
#include "lib9.h"

char*
strdup(const char *s) 
{  
	char *os;
	size_t len;

	len = strlen(s) + 1;
	os = malloc(len);
	if(os == 0)
		return 0;
	memcpy(os, s, len);
	return os;
}
