#ifndef ROBOTDOG_H
#define ROBOTDOG_H


#include <iostream>
#include <fstream>

#include "CalServo.h"
#include "PCA9685.h"
#include "LCD.h"
#include "MPU6050.h"
#include "HC_SR04.h"
#include "legIK.h"
#include "bodyFK.h"


#define MPU6050_GYRO_RANGE_MODE MPU6050_GYRO_FS_250
#define MPU6050_ACCEL_RANGE_MODE MPU6050_ACCEL_FS_2
#define MPU6050_SAMPLE_FREQ_HZ 100
#define NUM_HCSR04 2

#define VIDEO_PORT 8082
#define JOYSTICK_PORT 8081
#define TELEM_PORT 8086
#define SWITCH_PORT 8080


class RobotDog
{
public:
    RobotDog(int mpu_bus, int mpu_addr, int pca_bus, int pca_addr, int lcd_bus, int lcd_addr, char *cs_ip_addr);
    ~RobotDog();

    void run();

private:
    PCA9685 pca;
    LCD lcd;

    pthread_t mpu_thread_id;
    MPU6050 mpu6050;

    pthread_t hcsr04_thread_id;
    HC_SR04 hc_sr04[2]; // 0: left / 1: right

    CalServo servos[12];
    Body main_body;

    struct {
        float front_dist[2];
        MPU6050::MPU6050_data_t mpu_buff;
    } sensor_data;

    int switch_server_fd;
    int js_server_fd;
    int cam_server_fd;

    bool mode_flag;
    pthread_t control_thread_id;

    pid_t video_streamer;

    char cs_addr[24];
    char video_addr[64];
    char * vid_args[12] = {
        "libcamera-vid", "-n", "-t", "0", "--codec", "mjpeg", "--inline", "--framerate", "7", "-o", NULL, NULL
    };

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
    static void* control_thread(void *param);
    static void* telem_thread(void *param);
    static void* pos_thread(void *param);

    enum symb {
        UNKNOWN = -1,
        ONOFF,
        MODE,
        POSE
    };

    struct Axes {
        float x, y;
    };
};


#endif
