/*
 * dirstat-posix.c - Directory stat functions for POSIX systems
 * 
 * This file avoids including lib9.h to prevent conflicts between
 * Inferno's function declarations and POSIX/glibc declarations.
 * Instead, we include only what we need directly.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

/* Minimal definitions from lib9.h/kern.h that we need */
#define nil ((void*)0)

typedef unsigned int uint;
typedef unsigned long ulong;
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef long long vlong;
typedef unsigned long long uvlong;

/* Qid structure */
typedef struct Qid Qid;
struct Qid {
	uvlong path;
	ulong vers;
	uchar type;
};

/* Dir structure */
typedef struct Dir Dir;
struct Dir {
	/* system-modified data */
	ushort type;	/* server type */
	uint dev;	/* server subtype */
	/* file data */
	Qid qid;	/* unique id from server */
	ulong mode;	/* permissions */
	ulong atime;	/* last read time */
	ulong mtime;	/* last write time */
	vlong length;	/* file length */
	char *name;	/* last element of path */
	char *uid;	/* owner name */
	char *gid;	/* group name */
	char *muid;	/* last modifier name */
};

/* Constants */
#define QTDIR 0x80
#define QTFILE 0x00

/* External functions we need - declare with correct signatures */
extern void werrstr(char*, ...);
extern char* strrchr(const char*, int);

static char nullstring[] = "";
static char Enovmem[] = "out of memory";

static Dir*
statconv(struct stat *s, char *name)
{
	struct passwd *p;
	struct group *g;
	Dir *dir;
	int str;
	char *n;

	str = 0;
	p = getpwuid(s->st_uid);
	if(p)
		str += strlen(p->pw_name)+1;
	g = getgrgid(s->st_gid);
	if(g)
		str += strlen(g->gr_name)+1;
	dir = malloc(sizeof(Dir)+str);
	if(dir == nil){
		werrstr(Enovmem);
		return nil;
	}
	n = (char*)dir+sizeof(Dir);
	dir->name = name;
	dir->uid = dir->gid = dir->muid = nullstring;
	if(p){
		dir->uid = n;
		strcpy(n, p->pw_name);
		n += strlen(p->pw_name)+1;
	}
	if(g){
		dir->gid = n;
		strcpy(n, g->gr_name);
	}
	dir->qid.type = S_ISDIR(s->st_mode)? QTDIR: QTFILE;
	dir->qid.path = s->st_ino;
	dir->qid.vers = s->st_mtime;
	dir->mode = (dir->qid.type<<24)|(s->st_mode&0777);
	dir->atime = s->st_atime;
	dir->mtime = s->st_mtime;
	dir->length = s->st_size;
	dir->dev = s->st_dev;
	dir->type = S_ISFIFO(s->st_mode)? '|': 'M';
	return dir;
}

Dir*
dirfstat(int fd)
{
	struct stat sbuf;

	if(fstat(fd, &sbuf) < 0)
		return nil;
	return statconv(&sbuf, nullstring);
}

Dir*
dirstat(char *f)
{
	struct stat sbuf;
	char *p;

	if(stat(f, &sbuf) < 0)
		return nil;
	p = strrchr(f, '/');
	if(p)
		p++;
	else
		p = nullstring;
	return statconv(&sbuf, p);
}
