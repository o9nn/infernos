/*
 * getuser-posix.c - Get current user name for POSIX systems
 * 
 * This file avoids including lib9.h to prevent conflicts.
 */
#include <pwd.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

char*
getuser(void)
{
	struct passwd *p;

	static char *user = 0;

	if (!user) {
		p = getpwuid(getuid());
		if (p && p->pw_name) {
			user = malloc(strlen(p->pw_name)+1);
			if (user)
				strcpy(user, p->pw_name);
		}
	}
	if(!user)
		user = "unknown";
	return user;
}
