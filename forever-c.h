#ifndef FOREVER_C_H
#define FOREVER_C_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <stdarg.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/wait.h>

extern int optopt, optind, opterr, errno;
extern char * optarg;

typedef struct opts {
	int sleep;
	char * desc;
	char * name;
	unsigned char daemon:1;
	unsigned char verbose:1;
	unsigned char exec;
} opts_t;

void usage(char *);
void defaults(opts_t *);
int reset_args(int, int *, char ***);
int logsys(char *, char *, ...);
char * get_readable_wait_status(int);

#endif
