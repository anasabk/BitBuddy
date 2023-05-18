#include <iostream>
#include <chrono>
#include <fstream>
#include <time.h>
#include <sched.h>
#include "MPU6050.h"

int main() {
    // Real-time scheduling
    struct sched_param param;
    param.sched_priority = 99; // Set priority to maximum
    if (sched_setscheduler(0, SCHED_FIFO, &param) != 0) {
        std::cerr << "sched_setscheduler error!" << std::endl;
        return 1;
    }

    // Initalize
    MPU6050 mpu(1, I2C_ADDRESS);

    MPU6050::MPU6050_data_t data;
    mpu.read_data(&data);

    // Test connection
    if (data.x_accel == 0 && data.y_accel == 0 && data.z_accel == 0 && data.x_rot == 0 && data.y_rot == 0 && data.z_rot == 0) {
        std::cerr << "MPU6050 connection error!" << std::endl;
        return 1;
    }

    std::ofstream outputFile("sensorData.txt");

    auto startTime = std::chrono::system_clock::now();
    while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - startTime).count() < 5) {
        mpu.read_data(&data);

        auto timeNow = std::chrono::system_clock::now();
        std::time_t systemTime = std::chrono::system_clock::to_time_t(timeNow);

        outputFile << "Time: " << std::ctime(&systemTime);
        outputFile << "AccelX: " << data.x_accel << ", AccelY: " << data.y_accel << ", AccelZ: " << data.z_accel << std::endl;
        outputFile << "GyroX: " << data.x_rot << ", GyroY: " << data.y_rot << ", GyroZ: " << data.z_rot << std::endl;

        struct timespec sleepTime;
        sleepTime.tv_sec = 0;
        sleepTime.tv_nsec = 10000000; // 10 ms in nanoseconds
        nanosleep(&sleepTime, nullptr); // 100Hz = 10ms delay
    }

    outputFile.close();

    return 0;
}
