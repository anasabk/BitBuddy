#include <iostream>
#include <chrono>
#include <fstream>
#include <time.h>
#include <sched.h>
#include <string.h>

int main() {
    // Real-time scheduling
    struct sched_param param;
    param.sched_priority = 99; // Set priority to maximum
    if (sched_setscheduler(0, SCHED_FIFO, &param) != 0) {
        std::cerr << "sched_setscheduler error!" << strerror(errno) << std::endl;
        return 1;
    }

    std::ofstream outputFile("sensorData.txt");

    // Get current time
    struct timespec timeNow;
    clock_gettime(CLOCK_MONOTONIC, &timeNow);

    while (true) {
        std::time_t systemTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

        outputFile << "Time: " << std::ctime(&systemTime) << ", Hello" << std::endl;

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