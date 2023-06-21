#include "RobotDog.h"
#include <pthread.h>
#include <signal.h>


RobotDog::RobotDog(int mpu_bus, int mpu_addr, int pca_bus, int pca_addr, int lcd_bus, int lcd_addr)
    : pca(pca_bus, pca_addr, 50), lcd(lcd_bus, lcd_addr), hc_sr04{HC_SR04(27, 17), HC_SR04(5, 6)}, mpu6050(mpu_bus, mpu_addr),
    servos{ 
		// Top, Mid, and Low motors for each leg
		CalServo(&pca, 0), CalServo(&pca, 1), CalServo(&pca, 2),	// Back Left
		CalServo(&pca, 3), CalServo(&pca, 4), CalServo(&pca, 5),	// Back Right
		CalServo(&pca, 6), CalServo(&pca, 7), CalServo(&pca, 8),	// Front Right 
		CalServo(&pca, 9), CalServo(&pca, 10), CalServo(&pca, 11)	// Front Left
    },
    main_body(servos, 185, 77.5)
{
	for(int i = 0; i < 12; i++)
        servos[i].refresh_fitter(cal_pwm_list, cal_degree_list[servos[i].getChannel()], 20);
    
    running_flag = true;

    MPU6050::MPU6050_data_t offsets = {
        0.0306, 0.0120, 0.0211, 0, 0.7868, -0.8433, -0.7314
    };
    mpu6050.set_offsets(&offsets);
}

bool running = true;

RobotDog::~RobotDog()
{
    running = false;
    running_flag = false;
    sleep(1);
    pthread_join(mpu_thread_id, NULL);
    pthread_join(hcsr04_thread_id, NULL);
    main_body.sit_down();
    sleep(2);
}

void* read_thread(void *param) {
    MPU6050::MPU6050_data_t *buf = (MPU6050::MPU6050_data_t*)param;
    while(running) {
        printf("accel: x=%.4lf y=%.4lf z=%.4lf / gyro: x=%.4lf y=%.4lf z=%.4lf \n", 
            buf->x_accel,
            buf->y_accel,
            buf->z_accel,
            buf->x_rot,
            buf->y_rot,
            buf->z_rot
        );
        sleep(1);
    }
}

void RobotDog::run() {
    pthread_t temp;
    // mpu6050.calibrate();

	pthread_create(&mpu_thread_id, NULL, mpu6050_thread, (void*)this);
	pthread_create(&hcsr04_thread_id, NULL, HCSR04_thread, (void*)this);
    pthread_create(&temp, NULL, read_thread, (void*)(&this->mpu_buff));
    
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
    // main_body.pose(M_PI/4, 0, 0, 0, 0, 170);
    // sleep(2);
    // main_body.pose(0, M_PI/6, 0, 0, 0, 170);
    // sleep(2);
    // main_body.pose(0, -M_PI/6, 0, 0, 0, 170);
    // sleep(5);
    // main_body.pose(0, 0, M_PI/4, 0, 0, 170);
    // sleep(2);
    main_body.move_forward(0, 200);
    sleep(2);
    main_body.move_forward(0, 200);
    sleep(2);
    // main_body.move_forward(M_PI/24, 0);
    // main_body.move_forward(M_PI/24, 0);
    // main_body.move_forward(M_PI/24, 0);
    // sleep(2);
    // main_body.pose(0, 0, 0, 0,  50, 140);
    // sleep(5);
    // main_body.pose(0, 0, 0, 0, -50, 140);
    // sleep(1);
    // main_body.pose(0, 0, 0, 0, 20, 140);
    // sleep(1);
    // main_body.pose(0, 0, 0, 0, 0, 140);
    // sleep(1);
    // main_body.pose(M_PI/6, 0, 0, 0, 0, 140);
    // sleep(1);
    // main_body.pose(0, M_PI/4, 0, 0, 0, 140);
    // sleep(1);
    // main_body.pose(M_PI/3, 0, 0, 0, 0, 140);
    // sleep(1);
    main_body.recenter();
    sleep(2);
    // main_body.recover();
    // sleep(10);
    
    // uint8_t dest_servo = 0;
	// int dest_degree = 0;
    // while(running_flag) {
	// 	scanf("%d %d", &dest_servo, &dest_degree);
	// 	for(int i = 0; i < 12; i++){
	// 		printf("%d %d %d\n", i, servos[i].getChannel());
	// 		if(servos[i].getChannel() == dest_servo) {
	// 			servos[i].set_degree(dest_degree);
	// 			break;
	// 		}
	// 	}
    // }

    // running_flag = false;

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
