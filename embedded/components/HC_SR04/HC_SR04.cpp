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
    gpioWrite(trig, PI_HIGH);
    usleep(20);
    gpioWrite(trig, PI_LOW);

    double depart = time_time();
    double arrive = depart;

    while(gpioRead(echo) == PI_LOW);
    arrive = time_time();

    return (arrive - depart) * SPEED_OF_SOUND / 2;
}
