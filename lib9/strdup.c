/*
 * strdup.c - String duplication function
 * 
 * This file avoids including lib9.h to prevent conflicts.
 */
#include <stdlib.h>
#include <string.h>

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
