#ifndef CALSERVO_H_
#define CALSERVO_H_

#include <cmath>
#include <unistd.h>
#include "PCA9685.h"


class CalServo {
public:
    CalServo(PCA9685* cont_p, int channel);
    ~CalServo();

    void refresh_fitter(const int* pwm_list, const int* degree_list, int data_len);

    void set_PWM(int pwm_us);

    void set_degree(int degree);

    void sweep(int start, int finish, int duration_ms);

    int getChannel();

private:
    int channel;
    PCA9685* controller;
    double fitter_a, fitter_b;
};

#endif