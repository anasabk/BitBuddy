#include "common.h"


void wait_real_dl(struct timespec *timeNow, long ms) {
    timeNow->tv_nsec += ms * 1000000;
    while (timeNow->tv_nsec >= 1000000000L) {
        timeNow->tv_nsec -= 1000000000L;
        timeNow->tv_sec++;}
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, timeNow, nullptr);
}

void wait_real(long ms) {
    struct timespec timeNow;
    clock_gettime(CLOCK_MONOTONIC, &timeNow);
    timeNow.tv_nsec += ms * 1000000;
    while (timeNow.tv_nsec >= 1000000000L) {
        timeNow.tv_nsec -= 1000000000L;
        timeNow.tv_sec++;}
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &timeNow, nullptr);
}
