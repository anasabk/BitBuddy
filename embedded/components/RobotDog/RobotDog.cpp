#include "RobotDog.h"


RobotDog::RobotDog(int mpu_bus, int mpu_addr, int pca_bus, int pca_addr, int lcd_bus, int lcd_addr)
    : pca(pca_bus, pca_addr), lcd(lcd_bus, lcd_addr), mpu6050(mpu_bus, mpu_addr), hc_sr04{HC_SR04(27, 17), HC_SR04(5, 6)},
    servos{ 
		// Top, Mid, and Low motors for each leg
		{CalServo(&pca, 6), CalServo(&pca, 7), CalServo(&pca, 8)},		// Front Right 
		{CalServo(&pca, 9), CalServo(&pca, 10), CalServo(&pca, 11)},	// Front Left
		{CalServo(&pca, 3), CalServo(&pca, 4), CalServo(&pca, 5)},		// Back Right
		{CalServo(&pca, 0), CalServo(&pca, 1), CalServo(&pca, 2)}		// Back Left
    }
{
	for(int i = 0; i < 4; i++)
		for(int j = 0; j < 3; j++)
			servos[i][j].refresh_fitter(cal_pwm_list, cal_degree_list[servos[i][j].getChannel()], 20);
    

}

RobotDog::~RobotDog()
{
}

void RobotDog::run() {
	pthread_t mpu_thread_id;
	pthread_create(&mpu_thread_id, NULL, mpu6050_thread, (void*)this);

        // Real-time scheduling
    struct sched_param param;
    param.sched_priority = 99; // Set priority to maximum
    if (sched_setscheduler(0, SCHED_FIFO, &param) != 0) {
        std::cerr << "sched_setscheduler error!" << std::endl;
        return;
    }

    std::ofstream outputFile("sensorData.txt");

    // Get current time
    struct timespec timeNow;
    clock_gettime(CLOCK_MONOTONIC, &timeNow);

    while (true) {
        std::time_t systemTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

        outputFile << "Time: " << std::ctime(&systemTime);
        outputFile << "AccelX: " << mpu_buff.x_accel << ", AccelY: " << mpu_buff.y_accel << ", AccelZ: " << mpu_buff.z_accel << std::endl;
        outputFile << "GyroX: " << mpu_buff.x_rot << ", GyroY: " << mpu_buff.y_rot << ", GyroZ: " << mpu_buff.z_rot << std::endl;

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

    return;
}

void* RobotDog::mpu6050_thread(void* args) {
    RobotDog *robot = (RobotDog*)args;

	struct sched_param param;
    param.sched_priority = 99; // Set priority to maximum
    if (sched_setscheduler(0, SCHED_FIFO, &param) != 0) {
        std::cerr << "sched_setscheduler error!" << std::endl;
        return NULL;
    }

    robot->mpu6050.read_data(&robot->mpu_buff);

    // Test connection
    if (robot->mpu_buff.x_accel == 0 && 
		robot->mpu_buff.y_accel == 0 && 
		robot->mpu_buff.z_accel == 0 && 
		robot->mpu_buff.x_rot == 0 && 
		robot->mpu_buff.y_rot == 0 && 
		robot->mpu_buff.z_rot == 0) 
	{
        std::cerr << "MPU6050 connection error!" << std::endl;
        return NULL;
    }

    // Get current time
    struct timespec timeNow;
    clock_gettime(CLOCK_MONOTONIC, &timeNow);

    long dt_ns = 1000000000L / MPU6050_SAMPLE_FREQ_HZ;

    while (true) {
    	robot->mpu6050.read_data(&robot->mpu_buff);

        // Add 10ms to current time
        timeNow.tv_nsec += dt_ns; // 10 ms in nanoseconds

        // Handle overflow
        while (timeNow.tv_nsec >= 1000000000L) {
            timeNow.tv_nsec -= 1000000000L;
            timeNow.tv_sec++;
        }

        // Sleep until the next 10ms point
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &timeNow, nullptr);
    }

    return NULL;
}

void* RobotDog::HCSR04_thread(void* args) {
    RobotDog *robot = (RobotDog*)args;

    struct sched_param param;
    param.sched_priority = 99; // Set priority to maximum
    if (sched_setscheduler(0, SCHED_FIFO, &param) != 0) {
        std::cerr << "sched_setscheduler error!" << std::endl;
        return NULL;
    }

    // Check all HC_SR04 sensors
    for (int i = 0; i < NUM_HCSR04; i++) {
        robot->hc_sr04[i].get_distance();

        // Test connection
        if (robot->hc_sr04[i].get_distance() == 0) {
            std::cerr << "HC_SR04[" << i << "] connection error!" << std::endl;
            return NULL;
        }
    }

    // Get current time
    struct timespec timeNow;
    clock_gettime(CLOCK_MONOTONIC, &timeNow);

    long dt_ns = 1000000000L / HC_SR04_SAMPLE_FREQ_HZ;

    while (true) {
        for (int i = 0; i < NUM_HCSR04; i++) {
            robot->front_dist[NUM_HCSR04] = robot->hc_sr04[i].get_distance();

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

    return NULL;
}
