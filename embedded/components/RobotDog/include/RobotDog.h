#ifndef ROBOTDOG_H
#define ROBOTDOG_H


#include <iostream>
#include <fstream>

#include "CalServo.h"
#include "PCA9685.h"
#include "LCD.h"
#include "MPU6050.h"
#include "HC_SR04.h"
#include "inv_kinematics.h"


#define MPU6050_GYRO_RANGE_MODE MPU6050_GYRO_FS_250
#define MPU6050_ACCEL_RANGE_MODE MPU6050_ACCEL_FS_2
#define MPU6050_SAMPLE_FREQ_HZ 300
#define HC_SR04_SAMPLE_FREQ_HZ 20
#define SERVO_CMD_FREQ_HZ 50
#define NUM_HCSR04 2


class RobotDog
{
public:
    RobotDog(int mpu_bus, int mpu_addr, int pca_bus, int pca_addr, int lcd_bus, int lcd_addr);
    ~RobotDog();

    void run();

    void move_forward (int dist_mm);

private:
    PCA9685 pca;
    LCD lcd;
    MPU6050 mpu6050;
    HC_SR04 hc_sr04[2];
    CalServo servos[12];
    Leg legs[4];

    MPU6050::MPU6050_data_t mpu_buff;
    int front_dist[2];

    typedef struct servo_params {
        void* args;
        int servo_id;
    } servo_params;

    // {Back left, Back right, Front right, Front left} {x, y, z}
    double leg_move_buffer[4][3];
    int movement_speed;

    // degree
    double servo_buffer[12];
    int dur_buffer[12]{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};


    const int cal_pwm_list[20] = {450, 550, 650, 750, 850, 950, 1050, 1150, 1250, 1350, 1450, 1550, 1650, 1750, 1850, 1950, 2050, 2150, 2250, 2350};

    const int cal_degree_list[12][20] = {
        {0, 8, 16, 26, 36, 45, 53, 63, 72, 80, 88, 96, 104, 112, 120, 130, 139, 148, 158, 168},
        {0, 8, 16, 27, 37, 45, 56, 64, 71, 81, 89, 96, 104, 114, 121, 129, 139, 147, 154, 165},
        {0, 7, 17, 26, 37, 45, 56, 65, 74, 82, 91, 100, 108, 117, 123, 134, 141, 150, 158, 167},

        {0, 8, 18, 26, 36, 45, 55, 63, 72, 80, 89, 97, 100, 113, 120, 130, 139, 148, 156, 165}, 
        {0, 7, 17, 26, 35, 44, 53, 62, 71, 80, 89, 98, 105, 115, 121, 130, 139, 148, 157, 167},
        {0, 7, 17, 26, 35, 44, 53, 62, 71, 80, 89, 98, 105, 115, 121, 130, 139, 148, 157, 167},

        {0, 8, 16, 25, 35, 43, 52, 62, 70, 79, 88, 92, 103, 113, 120, 127, 138, 148, 154, 165},
        {0, 8, 18, 28, 35, 44, 53, 63, 72, 80, 88, 97, 104, 113, 121, 130, 137, 146, 155, 165}, 
        {0, 8, 18, 28, 35, 44, 53, 63, 72, 80, 88, 97, 104, 113, 121, 130, 137, 146, 155, 165}, 

        {0, 8, 16, 24, 35, 45, 52, 63, 72, 78, 90, 97, 105, 114, 122, 130, 140, 148, 157, 167}, 
        {0, 7, 15, 24, 34, 43, 51, 61, 70, 77, 86, 95, 103, 113, 121, 130, 138, 148, 155, 165}, 
        {0, 7, 15, 24, 34, 43, 51, 61, 70, 77, 86, 95, 103, 113, 121, 130, 138, 148, 155, 165}, 
    };

    static void* mpu6050_thread(void* args);
    static void* HCSR04_thread(void*);
    static void* servo_thread(void*);
};


#endif