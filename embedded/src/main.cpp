#include "PCA9685.h"
#include "unistd.h"
#include "MPU6050.h"
#include "LCD.h"
#include "CalServo.h"
#include "RobotDog.h"
#include <cstdio>
#include "cstring"
#include "HC_SR04.h"
#include <iostream>
#include <chrono>
#include <fstream>
#include <time.h>
#include <sched.h>
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>
#include "inverse_kinematics.h"

int pwm_list[20] = {450, 550, 650, 750, 850, 950, 1050, 1150, 1250, 1350, 1450, 1550, 1650, 1750, 1850, 1950, 2050, 2150, 2250, 2350};

int degree_list[12][20] = {
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

Leg *legs_g;
CalServo *servos_g;

void *thread_stand(void *val) {
	printf("Standing thread\n");
	servos_g[(int)val].sweep(sit[(int)val], stand[(int)val], 2000);
	pthread_exit(0);
}

void *thread_sit(void *val) {
	printf("Sitting thread\n");
	servos_g[(int)val].sweep(stand[(int)val], sit[(int)val], 2000);
	pthread_exit(0);
}

// HC_SR04 *hcsr04_g;
// LCD *lcd_g;

// void* thread_hcsr04(void *) {
//     // Get current time
//     struct timespec timeNow;
//     clock_gettime(CLOCK_MONOTONIC, &timeNow);

// 	while (true) {
// 		lcd_g->printf("%lf", hcsr04_g->get_distance());
// 		lcd_g->goHome();

// 		        // Add 10ms to current time
//         timeNow.tv_nsec += 10000000L; // 10 ms in nanoseconds
//         // Handle overflow
//         while (timeNow.tv_nsec >= 1000000000L) {
//             timeNow.tv_nsec -= 1000000000L;
//             timeNow.tv_sec++;
//         }

//         // Sleep until the next 10ms point
//         clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &timeNow, nullptr);
// 	}
// }

// void *thread_step(void *val) {
// 	printf("Stepping thread\n");
// 	for(int i =0; i < 5; i++) {
// 		servo_g[(int)val].sweep(step_offset[i][(int)val], 3000);
// 		sleep(2);
// 	}
// 	pthread_exit(0);
// }

extern "C" int main() {
	// RobotDog robot(1, MPU6050_DEF_I2C_ADDRESS, 1, 0x40, 1, 0x27);
	// robot.run();
	
    PCA9685 pca(1, 0x40);
	/** 
	 * \verbatim
	 * 		Numbers of servo channels
	 * 			   of each leg
	 * 				 | Top | Mid | Low |
	 * 				 |  0  |  1  |  2  |
	 * --------------|-----|-----|-----|
	 * Front Right 0 |	6  |  7	 |	8  |
	 * Front Left  1 |	9  |  10 |	11 |
	 * Back Right  2 |	3  |  4	 |	5  |
	 * Back Left   3 |	0  |  1	 |	2  |
	 * \endverbatim
	 */
	CalServo servo[12] {
		// Top, Mid, and Low motors for each leg
		CalServo(&pca, 0), CalServo(&pca, 1), CalServo(&pca, 2),	// Back Left
		CalServo(&pca, 3), CalServo(&pca, 4), CalServo(&pca, 5),	// Back Right
		CalServo(&pca, 6), CalServo(&pca, 7), CalServo(&pca, 8),	// Front Right 
		CalServo(&pca, 9), CalServo(&pca, 10), CalServo(&pca, 11)	// Front Left
	};

	Leg legs[4] {
		Leg(&servo[0],   0, &servo[1], 4.443508, &servo[2], -3.844949, 55, 110, 130, false),
		Leg(&servo[3],  10, &servo[4], -3.443508, &servo[5], 6.844949, 55, 110, 130, true),
		Leg(&servo[6],   0, &servo[7], -4.042452, &servo[8], 13.594, 55, 110, 130, true),
		Leg(&servo[9], -10, &servo[10], 5.957548, &servo[11], 14.594, 55, 110, 130, false),
	};

	servos_g = servo;
	legs_g = legs;

    pca.set_pwm_freq(50);
	usleep(1000000);

	printf("Calibrating ...\n");

	for(int i = 0; i < 12; i++)
		servo[i].refresh_fitter(pwm_list, degree_list[servo[i].getChannel()], 20);

	printf("Moving ...\nStanding ...\n");

	pthread_t temp;
	for(int i = 0; i < 12; i++) {
		pthread_create(&temp, NULL, thread_stand, (void*)i);
	}
	
	// sleep(5);
	// for(int i = 0; i < 12; i++) {
	// 	pthread_create(&temp, NULL, thread_sit, (void*)i);
	// }
	
	// // sleep(5);
	// // for(int i = 0; i < 12; i++) {
	// // 	pthread_create(&temp, NULL, thread_step, (void*)i);
	// // }

	// sleep(4);
	// legs[0].move(-50, 55, 65);
	// legs[1].move(-50, 55, 65);
	// legs[2].move(30, 55, 65);
	// legs[3].move(30, 55, 65);
	
	sleep(4);
	legs[0].move(-50, 55, 170);
	legs[1].move(-50, 55, 170);
	legs[2].move(30, 55, 170);
	legs[3].move(30, 55, 170);

	for(int i = 0; i < 4; i++) {
		usleep(200000);
		legs[1].move(30, 55, 130);
		usleep(200000);
		legs[1].move(60, 55, 130);
		usleep(200000);
		legs[1].move(60, 55, 170);
		usleep(200000);
		legs[0].move_offset(-25, 0, 0);
		legs[1].move_offset(-25, 0, 0);
		legs[2].move_offset(-25, 0, 0);
		legs[3].move_offset(-25, 0, 0);

		usleep(200000);
		legs[2].move(35, 55, 130);
		usleep(200000);
		legs[2].move(60, 55, 130);
		usleep(200000);
		legs[2].move(60, 55, 170);
		usleep(200000);
		legs[0].move_offset(-25, 0, 0);
		legs[1].move_offset(-25, 0, 0);
		legs[2].move_offset(-25, 0, 0);
		legs[3].move_offset(-25, 0, 0);

		usleep(200000);
		legs[0].move(30, 55, 130);
		usleep(200000);
		legs[0].move(60, 55, 130);
		usleep(200000);
		legs[0].move(60, 55, 170);
		usleep(200000);
		legs[0].move_offset(-25, 0, 0);
		legs[1].move_offset(-25, 0, 0);
		legs[2].move_offset(-25, 0, 0);
		legs[3].move_offset(-25, 0, 0);

		usleep(20000);
		legs[3].move(35, 55, 130);
		usleep(200000);
		legs[3].move(60, 55, 130);
		usleep(200000);
		legs[3].move(60, 55, 170);
		usleep(200000);
		legs[0].move_offset(-25, 0, 0);
		legs[1].move_offset(-25, 0, 0);
		legs[2].move_offset(-25, 0, 0);
		legs[3].move_offset(-25, 0, 0);
	}


	uint8_t dest_servo = 0;
	int dest_degree = 0;
    while(true) {
		scanf("%d %d", &dest_servo, &dest_degree);
		for(int i = 0; i < 12; i++){
			printf("%d %d %d\n", i, servo[i].getChannel());
			if(servo[i].getChannel() == dest_servo) {
				servo[i].set_degree(dest_degree);
				break;
			}
		}
    }

	return 0;


	// if (gpioInitialise() < 0) {
	// 	printf("Failure...");
	// 	exit(-1);
	// }

	// MPU6050 device(1, 0x68);
	// MPU6050::MPU6050_data_t data;
	// float ax, ay, az, gr, gp, gy; //Variables to store the accel, gyro and angle values
	// LCD lcd(1, 0x27);

	// sleep(1); //Wait for the MPU6050 to stabilize

	// //Read the current yaw angle
	// device.calc_yaw = true;

	// while(1) {
	// 	device.read_data(&data);
	// 	printf("Accel x: %.3f, y: %.3f, z: %.3f / Gyro x: %3.f, y: %3.f, z: %3.f\n", 
	// 		data.x_accel, 
	// 		data.y_accel,
	// 		data.z_accel,
	// 		data.x_rot,
	// 		data.y_rot,
	// 		data.z_rot
	// 	);

	// 	lcd.setPosition(0, 0);
	// 	lcd.printf("x = %0.3f", data.x_accel);
		
	// 	usleep(500000); //0.25sec
	// }

	// // gpioTerminate();
	// return 0;


	// if (gpioInitialise() < 0) {
	// 	printf("Failure...");
	// 	exit(-1);
	// }

	// HC_SR04 sensor(27, 17);
	// LCD lcd(1, 0x27);

	// while(1) {
	// 	lcd.setPosition(0, 0);
	// 	lcd.printf("%.3f\n", sensor.get_distance());
	// 	printf("distance = %f\n", sensor.get_distance());
	// 	usleep(1000000);
	// }

	// gpioTerminate();
	// return 0;
	

	// if (gpioInitialise() < 0) {
	// 	printf("Failure...");
	// 	exit(-1);
	// }

    // // Real-time scheduling
    // struct sched_param param;
    // param.sched_priority = 99; // Set priority to maximum
    // if (sched_setscheduler(0, SCHED_FIFO, &param) != 0) {
    //     std::cerr << "sched_setscheduler error!" << std::endl;
    //     return 1;
    // }

    // // Initalize
    // MPU6050 mpu(1, I2C_ADDRESS);

    // MPU6050::MPU6050_data_t data;
    // mpu.read_data(&data);

    // // Test connection
    // if (data.x_accel == 0 && data.y_accel == 0 && data.z_accel == 0 && data.x_rot == 0 && data.y_rot == 0 && data.z_rot == 0) {
    //     std::cerr << "MPU6050 connection error!" << std::endl;
    //     return 1;
    // }

    // std::ofstream outputFile("sensorData.txt");

    // auto startTime = std::chrono::system_clock::now();
    // while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - startTime).count() < 5) {
    //     mpu.read_data(&data);

    //     auto timeNow = std::chrono::system_clock::now();
    //     std::time_t systemTime = std::chrono::system_clock::to_time_t(timeNow);

    //     outputFile << "Time: " << std::ctime(&systemTime);
    //     outputFile << "AccelX: " << data.x_accel << ", AccelY: " << data.y_accel << ", AccelZ: " << data.z_accel << std::endl;
    //     outputFile << "GyroX: " << data.x_rot << ", GyroY: " << data.y_rot << ", GyroZ: " << data.z_rot << std::endl;

    //     std::cout << "Time: " << std::ctime(&systemTime);
    //     std::cout << "AccelX: " << data.x_accel << ", AccelY: " << data.y_accel << ", AccelZ: " << data.z_accel << std::endl;
    //     std::cout << "GyroX: " << data.x_rot << ", GyroY: " << data.y_rot << ", GyroZ: " << data.z_rot << std::endl;

    //     // struct timespec sleepTime;
    //     // sleepTime.tv_sec = 0;
    //     // sleepTime.tv_nsec = 10000000; // 10 ms in nanoseconds
    //     // nanosleep(&sleepTime, nullptr); // 100Hz = 10ms delay
	// 	usleep(10000L);
    // }

    // outputFile.close();
	// gpioTerminate();
	// return 0;
}
