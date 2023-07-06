#ifndef CALSERVO_H_
#define CALSERVO_H_

#include <cmath>
#include <unistd.h>
#include <cstdio>
#include <sys/time.h>

#include "PCA9685.h"


class CalServo {
public:
    CalServo(PCA9685* cont_p, int channel);
    ~CalServo();

    void refresh_fitter(const int* pwm_list, const float* rad_list, int data_len);

    void refresh_fitter(const int* pwm_list, const int* deg_list, int data_len);

    void set_PWM(int pwm_us);

    void set_rad(float degree);

    void set_rad_off(float offset);

    void sweep(float start, float dest, int duration_ms);

    void sweep(float dest, int duration_ms);

    void sweep_offset(float offset, int duration_ms);

    int getChannel();

    float get_last_rad();

private:
    int channel;
    PCA9685* controller;
    float fitter_a, fitter_b;
    float last_rad;
};

#endif