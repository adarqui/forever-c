#include "forever-c.h"

void usage(char * s) {
	if(!s) s = "nevermind.";

	printf("incorrect usage: %s\n\n\tforever-c <opts> --exec original program arguments\n\n",s);

	printf(
		"\topts:\n"
		"\t\t--name=<name>\n"
		"\t\t--desc=<desc>\n"
		"\t\t--daemon\n"
		"\t\t--sleep=<num>\n"
		"\t\t--exec program to run and it's arguments..\n"
	);
	exit(1);
}

void parse_opts(opts_t * opts, int argc, char ** argv) {

	int c;

	while(1) {

		int option_index = 0;

		static struct option long_options[] = {
			{ "desc", required_argument, 0, 0 },
			{ "name", required_argument, 0, 0 },
			{ "sleep", required_argument, 0, 0 },
			{ "verbose", no_argument, 0, 0 },
			{ "exec", no_argument, 0, 0 },
			{ "daemon", no_argument, 0, 0},
			{ "bg", no_argument, 0, 0 },
			{ 0, 0, 0, 0 }
		};

		c = getopt_long(argc, argv, "", long_options, &option_index);
		if(c < 0) break;

		switch(option_index) {
			case 0: {
				opts->desc = strdup(optarg);
				break;
			}
			case 1: {
				opts->name = strdup(optarg);
				break;
			}
			case 2: {
				opts->sleep = atoi(optarg);
				break;
			}
			case 3: {
				opts->verbose = 1;
				break;
			}
			case 4: {
				opts->exec = optind;
				return;
			}
			case 5:
			case 6: {
				opts->daemon = 1;
				break;
			}
			default: {
				continue;
			}
		}
	}
}


void defaults(opts_t * opts) {
	if(!opts) exit(-1);

	memset(opts,0,sizeof(opts_t));

	opts->sleep = 1;

	return;
}


int reset_args(int index, int * argc, char ***argv) {

	char ** n_argv;

	if(index < 0 || !argc || !argv) return -1;

	*argc = *argc - index;
	n_argv = *argv;
	*argv = &n_argv[index];

	return 0;
}

int logsys(char * who, char * fmt, ...) {
	va_list ap;
	char buf[1024];
	if(!who||!fmt) return -1;
	va_start(ap,fmt);
	memset(buf,0,sizeof(buf));
	snprintf(buf,sizeof(buf)-1,"%s_%s","forever",who);
	openlog(buf,LOG_NDELAY|LOG_NOWAIT|LOG_PERROR|LOG_PID,LOG_DAEMON);
	vsyslog(LOG_CRIT,fmt,ap);
	closelog();
	va_end(ap);
	return 0;
}


char * get_readable_wait_status(int status) {
	int wexit, wexitval, wsig, wsigval, wcore, wstop, wstopval, wcont;
	char buf[1024];

	memset(buf,0,sizeof(buf));

	wexit = wexitval = wsig = wsigval = wcore = wstop = wstopval = wcont = 0;

	wexit = WIFEXITED(status);
	wexitval = WEXITSTATUS(status);
	wsig = WIFSIGNALED(status);

	if(wsig) {
		wsigval = WTERMSIG(status);
	}

	if(wsig) {
		wcore = WCOREDUMP(status);
	}

	wstop = WIFSTOPPED(status);

	if(wstop) {
		wstopval = WSTOPSIG(status);
	}

	wcont = WIFCONTINUED(status);

	snprintf(buf,sizeof(buf)-1,
		"ifexited=%i exitstatus=%i ifsignaled=%i termsig=%i coredump=%i ifstopped=%i stopsig=%i ifcontinued=%i",
		wexit,wexitval,wsig,wsigval,wcore,wstop,wstopval,wcont);

	return strdup(buf);
}


int main(int argc, char *argv[], char *envp[]) {

	opts_t opts;
	pid_t p;
	int status, cnt = 0;

	defaults(&opts);
	parse_opts(&opts,argc,argv);

	if(!opts.name) usage("Specify --name=\"name\"");
	if(!opts.desc) usage("Specify --desc=\"<description>\"");
	if(opts.sleep < 0) usage("--sleep has to be >= 0");
	if(!opts.exec) usage("Specify --exec then your program arguments");

	if(reset_args(opts.exec,&argc,&argv) < 0) usage("reset_args: Failed");

	if(opts.daemon) {
		int n;
		n = daemon(0,0);
		if(n > 0) exit(0);
	}

	while(1) {
		p = fork();
		if(!p) {
			int n = execve(argv[0],argv,envp);
			exit(n);
		} else {
			p = wait(&status);
		}
		
		if(opts.sleep > 0) {
			sleep(opts.sleep);
		}

		cnt++;

		char * wait_status = get_readable_wait_status(status);
		logsys(opts.name, "cnt=%i status=%i %s\n", cnt, status, wait_status);
		if(wait_status) free(wait_status);
	}

	if(opts.name) free(opts.name);
	if(opts.desc) free(opts.desc);

	return 0;
}
