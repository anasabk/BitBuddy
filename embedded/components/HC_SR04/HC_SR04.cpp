#include "HC_SR04.h"


HC_SR04::HC_SR04(int trig, int echo)
{
    this->trig = trig;
    this->echo = echo;

    gpioSetMode(trig, PI_OUTPUT);
    gpioSetMode(echo, PI_INPUT);

    gpioWrite(trig, PI_LOW);
}

HC_SR04::~HC_SR04()
{
}

/**
 * @brief Return the measured distance in millimeters.
 */
float HC_SR04::get_distance() {
    // Get current time
    struct timespec time_send;
    struct timespec time_rec;

    gpioWrite(trig, PI_HIGH);
    clock_gettime(CLOCK_MONOTONIC, &time_send);
    time_send.tv_nsec += 20000;
    if (time_send.tv_nsec >= 1000000000L) {
        time_send.tv_nsec -= 1000000000L;
        time_send.tv_sec++;
    }
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &time_send, nullptr);
    gpioWrite(trig, PI_LOW);

    while(gpioRead(echo) == PI_LOW)
        clock_gettime(CLOCK_MONOTONIC, &time_send);
    while(gpioRead(echo) == PI_HIGH)
        clock_gettime(CLOCK_MONOTONIC, &time_rec);

    double dur_ms = (time_rec.tv_nsec - time_send.tv_nsec) / 1000000.0;
    double result = (dur_ms * SPEED_OF_SOUND) / 2.0;

    return result;
}