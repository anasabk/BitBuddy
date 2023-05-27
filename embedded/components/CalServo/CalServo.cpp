#include "CalServo.h"
#include <cstdio>


CalServo::CalServo(PCA9685* cont_p, int channel) {
    this->controller = cont_p;
    this->channel = channel;
    this->fitter_a = 0;
    this->fitter_b = 1;
}

CalServo::~CalServo() {

}

void CalServo::refresh_fitter(const int* pwm_list, const int* degree_list, int data_len) {
    //Check if there is no data.
    if(data_len < 1) {
        fitter_a = 0;
        fitter_b = 1;
    }

    else {
        float sumX = 0, 
              sumY = 0, 
              sumXSquare = 0, 
              sumXY = 0;
            
        //Calculate the summations needed by the linear fitting formula.
        for(int i = 0; i < data_len; i++){
            sumX += degree_list[i];
            sumY += pwm_list[i];
            sumXSquare += degree_list[i] * degree_list[i];
            sumXY += degree_list[i] * pwm_list[i];
        }

        //Calculate and store the constants of the linear equation.
        fitter_a = (float) ((sumY * sumXSquare - sumX * sumXY) / (data_len * sumXSquare - sumX * sumX));
        fitter_b = (float) ((data_len * sumXY - sumX * sumY) / (data_len * sumXSquare - sumX * sumX));
    }
}

void CalServo::set_PWM(int pwm_us) {
    controller->set_pwm_us(channel, pwm_us);
}

void CalServo::set_degree(int degree) {
    int pwm_us = fitter_a + fitter_b * degree;
    controller->set_pwm_us(channel, pwm_us);
    last_deg = degree;
}

void CalServo::sweep(int start, int dest, int dur_ms) {
    if(start == dest) {
        set_degree(start);
        return;
    }
    
    int dt = dur_ms / abs(dest - start);
    int dir = (dest - start) > 0 ? 1 : -1;
    int current = start;
    while(current*dir < dest*dir) {
        set_degree(current);
        current += dir;
    }
}

void CalServo::sweep(int offset, int dur_ms) {
    if(last_deg == -1)
        return;

    if(offset == 0) {
        set_degree(last_deg);
        return;
    }

    if(offset + last_deg > 180 || offset + last_deg < 0) {
        printf("Limit reached, degree: %d\n", offset + last_deg);
        return;
    }
    
    int dt_ns = dur_ms / abs(offset) * 1000000;
    int dir = (offset) > 0 ? 1 : -1;
    int current = last_deg;
    int dest = current + offset;
    
    struct timespec timeNow;
    clock_gettime(CLOCK_MONOTONIC, &timeNow);
    
    while(current*dir < dest*dir) {
        set_degree(current);
        current += dir;

        // Add dt_ns to current time
        timeNow.tv_nsec += dt_ns; // dt_ns in nanoseconds

        // Handle overflow
        while (timeNow.tv_nsec >= 1000000000L) {
            timeNow.tv_nsec -= 1000000000L;
            timeNow.tv_sec++;
        }

        // Sleep until the next dt_ns point
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &timeNow, nullptr);
    }
}


int CalServo::getChannel() {
    return channel;
}
