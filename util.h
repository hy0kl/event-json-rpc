#ifndef _UTIL_H
#define _UTIL_H

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>

#define SIGNO_END   111

int daemonize(int nochdir, int noclose);

void signal_setup(void);

#endif

