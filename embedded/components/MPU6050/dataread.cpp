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

    MPU6050 mpu;
    mpu.initialize();

    if (!mpu.testConnection()) {
        std::cerr << "MPU6050 connection error!" << std::endl;
        return 1;
    }

    std::ofstream outputFile("sensorData.txt");

    auto startTime = std::chrono::system_clock::now();
    while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - startTime).count() < 5) {
        int16_t ax, ay, az;
        int16_t gx, gy, gz;

        mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

        auto timeNow = std::chrono::system_clock::now();
        std::time_t systemTime = std::chrono::system_clock::to_time_t(timeNow);

        outputFile << "Time: " << std::ctime(&systemTime);
        outputFile << "AccelX: " << ax << ", AccelY: " << ay << ", AccelZ: " << az << std::endl;
        outputFile << "GyroX: " << gx << ", GyroY: " << gy << ", GyroZ: " << gz << std::endl;

        struct timespec sleepTime;
        sleepTime.tv_sec = 0;
        sleepTime.tv_nsec = 10000000; // 10 ms in nanoseconds
        nanosleep(&sleepTime, nullptr); // 100Hz = 10ms delay
    }

    outputFile.close();

    return 0;
}
