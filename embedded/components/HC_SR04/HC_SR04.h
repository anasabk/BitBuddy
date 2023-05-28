#ifndef HC_SR04_H
#define HC_SR04_H


#include "pigpio.h"
#include "unistd.h"

#define SPEED_OF_SOUND 343.0 // mm/ms

class HC_SR04
{
public:
    HC_SR04(int trig, int echo);
    ~HC_SR04();

    float get_distance();

private:
    int trig;
    int echo;
};


#endif
