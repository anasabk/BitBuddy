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

    // Initialize
    MPU6050 mpu(1, I2C_ADDRESS);

    MPU6050::MPU6050_data_t data;
    mpu.read_data(&data);

    // Test connection
    if (data.x_accel == 0 && data.y_accel == 0 && data.z_accel == 0 && data.x_rot == 0 && data.y_rot == 0 && data.z_rot == 0) {
        std::cerr << "MPU6050 connection error!" << std::endl;
        return 1;
    }

    std::ofstream outputFile("sensorData.txt");

    // Get current time
    struct timespec timeNow;
    clock_gettime(CLOCK_MONOTONIC, &timeNow);

    while (true) {
        mpu.read_data(&data);

        std::time_t systemTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

        outputFile << "Time: " << std::ctime(&systemTime);
        outputFile << "AccelX: " << data.x_accel << ", AccelY: " << data.y_accel << ", AccelZ: " << data.z_accel << std::endl;
        outputFile << "GyroX: " << data.x_rot << ", GyroY: " << data.y_rot << ", GyroZ: " << data.z_rot << std::endl;

        // Add 10ms to current time
        timeNow.tv_nsec += 10000000L; // 10 ms in nanoseconds
        // Handle overflow
        while (timeNow.tv_nsec >= 1000000000L) {
            timeNow.tv_nsec -= 1000000000L;
            timeNow.tv_sec++;
        }

        // Sleep until the next 10ms point
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &timeNow, nullptr);

        // Check if 5 seconds have passed since the start
        if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - std::chrono::system_clock::from_time_t(systemTime)).count() >= 5) {
            break;
        }
    }

    outputFile.close();

    return 0;
}
