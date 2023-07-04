#ifndef COMMON_H
#define COMMON_H

#include <time.h>


void wait_real_dl(struct timespec *timeNow, long ms);

void wait_real(long ms);

#endif
