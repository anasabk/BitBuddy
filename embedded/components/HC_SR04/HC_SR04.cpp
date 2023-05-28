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

float HC_SR04::get_distance() {
    // Get current time
    struct timespec time_send;
    struct timespec time_rec;

    gpioWrite(trig, PI_HIGH);
    gpioDelay(10);
    gpioWrite(trig, PI_LOW);


    // double startTime = time_time();
    // double arrivalTime = time_time();

    // while(gpioRead(echo) == PI_LOW) {
    //     startTime = time_time();
    // }

    while(gpioRead(echo) == PI_LOW);
    clock_gettime(CLOCK_MONOTONIC, &time_send);
    while(gpioRead(echo) == PI_HIGH);
    clock_gettime(CLOCK_MONOTONIC, &time_rec);

    // while(gpioRead(echo) == PI_HIGH) {
    //     arrivalTime = time_time();
    // }

    // double timeElapsed = arrivalTime - startTime;
    // double distanceCalculated = (timeElapsed * 34300) / 2;

    double dur_ms = (time_rec.tv_nsec - time_send.tv_nsec) / 1000000;
    double result = (dur_ms * SPEED_OF_SOUND) / 2;

    return result;
}
