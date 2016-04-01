
#ifndef AMIGA_TIME_H
#define AMIGA_TIME_H

#include <sys/time.h>
#include <sys/select.h>
#include <proto/exec.h>
#include <proto/timer.h>

#ifdef _SYS_CLIB2_STDC_H
#define gettimeofday amiga_gettimeofday
void amiga_gettimeofday(struct timeval *__p, struct timezone *__z);
#endif

struct timespec {
    unsigned int tv_sec;
    unsigned int tv_nsec;
} timespec;

#endif
