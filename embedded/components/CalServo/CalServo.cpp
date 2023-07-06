#include "CalServo.h"


CalServo::CalServo(PCA9685* cont_p, int channel) {
    this->controller = cont_p;
    this->channel = channel;
    this->fitter_a = 0;
    this->fitter_b = 1;
    this->last_rad = 0;
}

CalServo::~CalServo() {

}

void CalServo::refresh_fitter(const int* pwm_list, const float* rad_list, int data_len) {
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
            sumX += rad_list[i];
            sumY += pwm_list[i];
            sumXSquare += rad_list[i] * rad_list[i];
            sumXY += rad_list[i] * pwm_list[i];
        }

        //Calculate and store the constants of the linear equation.
        fitter_a = (float) ((sumY * sumXSquare - sumX * sumXY) / (data_len * sumXSquare - sumX * sumX));
        fitter_b = (float) ((data_len * sumXY - sumX * sumY) / (data_len * sumXSquare - sumX * sumX));
    }
}

void CalServo::refresh_fitter(const int* pwm_list, const int* deg_list, int data_len) {
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
            sumX += deg_list[i]*M_PI/180;
            sumY += pwm_list[i];
            sumXSquare += pow(deg_list[i]*M_PI/180, 2);
            sumXY += deg_list[i]*M_PI/180 * pwm_list[i];
        }

        //Calculate and store the constants of the linear equation.
        fitter_a = (float) ((sumY * sumXSquare - sumX * sumXY) / (data_len * sumXSquare - sumX * sumX));
        fitter_b = (float) ((data_len * sumXY - sumX * sumY) / (data_len * sumXSquare - sumX * sumX));
    }

    printf("fitted: a=%lf, b=%lf\n", fitter_a, fitter_b);
}

void CalServo::set_PWM(int pwm_us) {
    controller->set_pwm_us(channel, pwm_us);
}

void CalServo::set_rad(float rad) {
    int pwm_us = fitter_a + fitter_b * rad;
    controller->set_pwm_us(channel, pwm_us);
    last_rad = rad;
}

void CalServo::set_rad_off(float offset) {
    int pwm_us = fitter_a + fitter_b * (last_rad + offset);
    controller->set_pwm_us(channel, pwm_us);
    last_rad += offset;
}

void CalServo::sweep(float start, float dest, int dur_ms) {
    if(start == dest)
        return;
    
    int dt_ns = dur_ms / abs(dest - start) * 1000000 / 0.0175;
    int dir = (dest - start) > 0 ? 1 : -1;
    float current = start;
    
    struct timespec timeNow;
    clock_gettime(CLOCK_MONOTONIC, &timeNow);
    
    while(current*dir < dest*dir) {
        set_rad(current);
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

void CalServo::sweep(float dest, int dur_ms) {
    if(last_rad == -1)
        return;

    if(dest > 180 || dest < 0) {
        printf("servo %d: Limit reached, degree: %d\n", channel, dest);
        return;
    }
    
    sweep(last_rad, dest, dur_ms);
}

void CalServo::sweep_offset(float offset, int dur_ms) {
    if(last_rad == -1)
        return;

    if(offset == 0) {
        set_rad(last_rad);
        return;
    }

    sweep(last_rad, last_rad + offset, dur_ms);
}


int CalServo::getChannel() {
    return channel;
}

float CalServo::get_last_rad() {
    return last_rad;
}
