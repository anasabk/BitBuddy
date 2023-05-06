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
}


int CalServo::getChannel() {
    return channel;
}