#include "RobotDog.h"


RobotDog::RobotDog(int mpu_bus, int mpu_addr, int pca_bus, int pca_addr, int lcd_bus, int lcd_addr)
    : pca(pca_bus, pca_addr, 50), lcd(lcd_bus, lcd_addr), mpu6050(mpu_bus, mpu_addr), hc_sr04{HC_SR04(27, 17), HC_SR04(5, 6)},
    servos{ 
		// Top, Mid, and Low motors for each leg
		CalServo(&pca, 0), CalServo(&pca, 1), CalServo(&pca, 2),	// Back Left
		CalServo(&pca, 3), CalServo(&pca, 4), CalServo(&pca, 5),	// Back Right
		CalServo(&pca, 6), CalServo(&pca, 7), CalServo(&pca, 8),	// Front Right 
		CalServo(&pca, 9), CalServo(&pca, 10), CalServo(&pca, 11)	// Front Left
    },
    legs{
        Leg(&servos[0],-4, &servos[1],  7, &servos[2], 1, 55, 110, 130, false, false),
        Leg(&servos[3],-10, &servos[4], -3, &servos[5], -7, 55, 110, 130,  true, false),
        Leg(&servos[6], 0, &servos[7], -2, &servos[8], -3, 55, 110, 130,  true, true),
        Leg(&servos[9],-8, &servos[10], 6, &servos[11],5, 55, 110, 130, false, true),
	},
    main_body(&legs[Body::LEFTFRONT], &legs[Body::RIGHTFRONT], &legs[Body::LEFTBACK], &legs[Body::RIGHTBACK], 185, 77.5)
{
	for(int i = 0; i < 12; i++)
        servos[i].refresh_fitter(cal_pwm_list, cal_degree_list[servos[i].getChannel()], 20);
    
    running_flag = true;
}

RobotDog::~RobotDog()
{
    running_flag = false;
    sleep(1);
    pthread_join(mpu_thread_id, NULL);
    pthread_join(hcsr04_thread_id, NULL);
}

void* read_thread(void *param) {
    while(true) {
        printf("%f dregrees\n", acos(-*((float*)param))*180/M_PI);
        sleep(1);
    }
}

void RobotDog::run() {
    pthread_t temp;
	pthread_create(&mpu_thread_id, NULL, mpu6050_thread, (void*)this);
	pthread_create(&hcsr04_thread_id, NULL, HCSR04_thread, (void*)this);
    pthread_create(&temp, NULL, read_thread, (void*)(&this->mpu_buff.x_accel));
    
    servos[0].set_degree(86);
    servos[1].set_degree(128);
    servos[2].set_degree(4);
    servos[3].set_degree(80);
    servos[4].set_degree(61);
    servos[5].set_degree(176);
    servos[6].set_degree(90);
    servos[7].set_degree(119);
    servos[8].set_degree(176);
    servos[9].set_degree(82);
    servos[10].set_degree(69);
    servos[11].set_degree(12);

    sleep(2);
    main_body.sit_down();
    sleep(2);
    main_body.stand_up();
    sleep(2);
    main_body.pose(0, 0, 0, 50, 0, 170);
    sleep(2);
    main_body.pose(0, 0, 0, -50, 0, 170);
    sleep(2);
    main_body.pose(0, 0, 0, 0, 50, 170);
    sleep(2);
    main_body.pose(0, 0, 0, 0, -50, 170);
    sleep(2);
    main_body.pose(0, 0, M_PI/6, 0, 0, 170);
    sleep(2);
    

    // Initialize and start the servo_thread
    // servo_params* params = new servo_params;
    // pthread_t servo_thread_id[12];
    // for (int i = 0; i < 12; ++i) {
    //     // Allocate memory
    //     params->args = this;
    //     params->servo_id = i;

    //     // Create the thread
    //     pthread_create(&servo_thread_id[i], NULL, &RobotDog::servo_thread, params);
    // }


    // // Real-time scheduling
    // struct sched_param param;
    // param.sched_priority = 99; // Set priority to maximum
    // if (sched_setscheduler(0, SCHED_FIFO, &param) != 0) {
    //     std::cerr << "sched_setscheduler error!" << std::endl;
    //     return;
    // }

    // std::ofstream outputFile("sensorData.txt");

    // // Get current time
    // struct timespec timeNow;
    // clock_gettime(CLOCK_MONOTONIC, &timeNow);

    // while (true) {
    //     std::time_t systemTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    //     outputFile << "Time: " << std::ctime(&systemTime);
    //     outputFile << "AccelX: " << mpu_buff.x_accel << ", AccelY: " << mpu_buff.y_accel << ", AccelZ: " << mpu_buff.z_accel << std::endl;
    //     outputFile << "GyroX: " << mpu_buff.x_rot << ", GyroY: " << mpu_buff.y_rot << ", GyroZ: " << mpu_buff.z_rot << std::endl;

    //     // Add 10ms to current time
    //     timeNow.tv_nsec += 10000000L; // 10 ms in nanoseconds
    //     // Handle overflow
    //     while (timeNow.tv_nsec >= 1000000000L) {
    //         timeNow.tv_nsec -= 1000000000L;
    //         timeNow.tv_sec++;
    //     }

    //     // Sleep until the next 10ms point
    //     clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &timeNow, nullptr);

    //     // Check if 5 seconds have passed since the start
    //     if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - std::chrono::system_clock::from_time_t(systemTime)).count() >= 5) {
    //         break;
    //     }
    // }

    // outputFile.close();

    // for (int i = 0; i < 12; ++i) {
    //     // Terminate the thread
    //     pthread_cancel(servo_thread_id[i]);
    //     pthread_join(servo_thread_id[i], NULL);

    //     // Free the memory for the struct
    //     delete params;
    // }


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

    while (robot->running_flag) {
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

    pthread_exit(NULL);
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

    while (robot->running_flag) {
        robot->front_dist[0] = robot->hc_sr04[0].get_distance();
        robot->front_dist[1] = robot->hc_sr04[1].get_distance();

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

    pthread_exit(NULL);
}

// void* RobotDog::servo_thread(void* args) {
//     servo_params *params = (servo_params*) args;
//     RobotDog *robot = (RobotDog*) params->args;

//     struct sched_param param;
//     param.sched_priority = 99; // Set priority to maximum
//     if (sched_setscheduler(0, SCHED_FIFO, &param) != 0) {
//         std::cerr << "sched_setscheduler error!" << std::endl;
//         return NULL;
//     }

//     int servo_id = params->servo_id;

//     // Get current time
//     struct timespec timeNow;
//     clock_gettime(CLOCK_MONOTONIC, &timeNow);

//     long dt_ns = 1000000000L / SERVO_CMD_FREQ_HZ;

//     while (true) {
//         // Sweep servo[i]
//         robot->servos[servo_id].sweep(robot->servo_buffer[servo_id], robot->dur_buffer[servo_id]);

//         // Add dt_ns to current time
//         timeNow.tv_nsec += dt_ns; // dt_ns in nanoseconds

//         // Handle overflow
//         while (timeNow.tv_nsec >= 1000000000L) {
//             timeNow.tv_nsec -= 1000000000L;
//             timeNow.tv_sec++;
//         }

//         // Sleep until the next dt_ns point
//         clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &timeNow, nullptr);
//     }

//     return NULL;
// }

// void RobotDog::move_forward(int dist_mm) {
//     // for(int i = 0, leg_id = 0; i < dist_mm; i += 25, leg_id -= 2) {
//     //     if(leg_id < 0) leg_id += 3;

// 	// 	// usleep(20000);
// 	// 	// legs[0].move_offset(0, 0, 0);
// 	// 	// legs[1].move_offset(0, 0, 0);
// 	// 	// legs[2].move_offset(0, 0, 0);
// 	// 	// legs[3].move_offset(0, 0, 0);
// 	// 	usleep(20000);
// 	// 	legs[1].move(-50, 55, 100, &servo_buffer[3], &servo_buffer[4], &servo_buffer[5]);
// 	// 	usleep(20000);
// 	// 	legs[1].move(-25, 55, 100, &servo_buffer[3], &servo_buffer[4], &servo_buffer[5]);
// 	// 	usleep(100000);
// 	// 	legs[1].move(-25, 55, 170);
// 	// 	usleep(200000);
// 	// 	legs[0].move_offset(-25, 0, 0);
// 	// 	legs[1].move_offset(-25, 0, 0);
// 	// 	legs[2].move_offset(-25, 0, 0);
// 	// 	legs[3].move_offset(-25, 0, 0);
//     // }
    
//     struct sched_param param;
//     param.sched_priority = 99; // Set priority to maximum
//     if (sched_setscheduler(0, SCHED_FIFO, &param) != 0) {
//         std::cerr << "sched_setscheduler error!" << std::endl;
//         return;
//     }

//     // Get current time
//     struct timespec timeNow;
//     clock_gettime(CLOCK_MONOTONIC, &timeNow);
//     int x, y, z;
//     for(int i = 0, leg_id = 2; i < dist_mm; i += 25, leg_id -= 2) {
//         if(leg_id < 0) leg_id += 3;

// 		usleep(20000);
// 		legs[2].move(35, 55, 100, &servo_buffer[6], &servo_buffer[7], &servo_buffer[8]);
//         dur_buffer[6] = 50;
//         dur_buffer[7] = 50;
//         dur_buffer[8] = 50;

//         timeNow.tv_nsec += 50000; // dt_ns in nanoseconds
//         while (timeNow.tv_nsec >= 1000000000L) {timeNow.tv_nsec -= 1000000000L; timeNow.tv_sec++;}
//         clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &timeNow, nullptr);
        

// 		legs[2].move(60, 55, 100, &servo_buffer[6], &servo_buffer[7], &servo_buffer[8]);
//         dur_buffer[6] = 50;
//         dur_buffer[7] = 50;
//         dur_buffer[8] = 50;

//         timeNow.tv_nsec += 50000; // dt_ns in nanoseconds
//         while (timeNow.tv_nsec >= 1000000000L) {timeNow.tv_nsec -= 1000000000L; timeNow.tv_sec++;}
//         clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &timeNow, nullptr);


// 		legs[2].move(60, 55, 170, &servo_buffer[6], &servo_buffer[7], &servo_buffer[8]);
//         dur_buffer[6] = 50;
//         dur_buffer[7] = 50;
//         dur_buffer[8] = 50;

//         timeNow.tv_nsec += 50000; // dt_ns in nanoseconds
//         while (timeNow.tv_nsec >= 1000000000L) {timeNow.tv_nsec -= 1000000000L; timeNow.tv_sec++;}
//         clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &timeNow, nullptr);


// 		legs[0].move_offset(-25, 0, 0, &servo_buffer[0], &servo_buffer[1], &servo_buffer[2]);
// 		legs[1].move_offset(-25, 0, 0, &servo_buffer[3], &servo_buffer[4], &servo_buffer[5]);
// 		legs[2].move_offset(-25, 0, 0, &servo_buffer[6], &servo_buffer[7], &servo_buffer[8]);
// 		legs[3].move_offset(-25, 0, 0, &servo_buffer[9], &servo_buffer[10], &servo_buffer[11]);

//         timeNow.tv_nsec += 50000; // dt_ns in nanoseconds
//         while (timeNow.tv_nsec >= 1000000000L) {timeNow.tv_nsec -= 1000000000L; timeNow.tv_sec++;}
//         clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &timeNow, nullptr);
        
// 		// usleep(20000);
// 		// legs[0].move_offset(0, 0, 0);
// 		// legs[1].move_offset(0, 0, 0);
// 		// legs[2].move_offset(0, 0, 0);
// 		// legs[3].move_offset(0, 0, 0);
// 		usleep(20000);
// 		legs[0].move(-50, 55, 100, &servo_buffer[0], &servo_buffer[1], &servo_buffer[2]);
// 		usleep(20000);
// 		legs[0].move(-25, 55, 100, &servo_buffer[0], &servo_buffer[1], &servo_buffer[2]);
// 		usleep(100000);
// 		legs[0].move(-25, 55, 170, &servo_buffer[0], &servo_buffer[1], &servo_buffer[2]);
// 		usleep(200000);
// 		legs[0].move_offset(-25, 0, 0, &servo_buffer[0], &servo_buffer[1], &servo_buffer[2]);
// 		legs[1].move_offset(-25, 0, 0, &servo_buffer[3], &servo_buffer[4], &servo_buffer[5]);
// 		legs[2].move_offset(-25, 0, 0, &servo_buffer[6], &servo_buffer[7], &servo_buffer[8]);
// 		legs[3].move_offset(-25, 0, 0, &servo_buffer[9], &servo_buffer[10], &servo_buffer[11]);

// 		// usleep(20000);
// 		// legs[0].move_offset(0, 0, 0);
// 		// legs[1].move_offset(0, 0, 0);
// 		// legs[2].move_offset(0, 0, 0);
// 		// legs[3].move_offset(0, 0, 0);
// 		usleep(20000);
// 		legs[3].move(35, 55, 100, &servo_buffer[9], &servo_buffer[10], &servo_buffer[11]);
// 		usleep(20000);
// 		legs[3].move(60, 55, 100, &servo_buffer[9], &servo_buffer[10], &servo_buffer[11]);
// 		usleep(100000);
// 		legs[3].move(60, 55, 170, &servo_buffer[9], &servo_buffer[10], &servo_buffer[11]);
// 		usleep(200000);
// 		legs[0].move_offset(-25, 0, 0, &servo_buffer[0], &servo_buffer[1], &servo_buffer[2]);
// 		legs[1].move_offset(-25, 0, 0, &servo_buffer[3], &servo_buffer[4], &servo_buffer[5]);
// 		legs[2].move_offset(-25, 0, 0, &servo_buffer[6], &servo_buffer[7], &servo_buffer[8]);
// 		legs[3].move_offset(-25, 0, 0, &servo_buffer[9], &servo_buffer[10], &servo_buffer[11]);

// 		// usleep(20000);
// 		// legs[0].move_offset(0, 0, 0);
// 		// legs[1].move_offset(0, 0, 0);
// 		// legs[2].move_offset(0, 0, 0);
// 		// legs[3].move_offset(0, 0, 0);
// 		usleep(20000);
// 		legs[1].move(-50, 55, 100, &servo_buffer[3], &servo_buffer[4], &servo_buffer[5]);
// 		usleep(20000);
// 		legs[1].move(-25, 55, 100, &servo_buffer[3], &servo_buffer[4], &servo_buffer[5]);
// 		usleep(100000);
// 		legs[1].move(-25, 55, 170, &servo_buffer[3], &servo_buffer[4], &servo_buffer[5]);
// 		usleep(200000);
// 		legs[0].move_offset(-25, 0, 0, &servo_buffer[0], &servo_buffer[1], &servo_buffer[2]);
// 		legs[1].move_offset(-25, 0, 0, &servo_buffer[3], &servo_buffer[4], &servo_buffer[5]);
// 		legs[2].move_offset(-25, 0, 0, &servo_buffer[6], &servo_buffer[7], &servo_buffer[8]);
// 		legs[3].move_offset(-25, 0, 0, &servo_buffer[9], &servo_buffer[10], &servo_buffer[11]);


//         // Add dt_ns to current time
//         timeNow.tv_nsec += 50000; // dt_ns in nanoseconds

//         // Handle overflow
//         while (timeNow.tv_nsec >= 1000000000L) {
//             timeNow.tv_nsec -= 1000000000L;
//             timeNow.tv_sec++;
//         }

//         // Sleep until the next dt_ns point
//         clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &timeNow, nullptr);
// 	}
// }
