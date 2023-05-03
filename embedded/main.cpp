#include "PCA9685.h"
#include "unistd.h"
#include "MPU6050.h"
#include "LCD.h"
#include "CalServo.h"
#include <cstdio>
#include "cstring"

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

int main() {
	// if (gpioInitialise() < 0) {
	// 	printf("Failure...");
	// 	exit(-1);
	// }
	// LCD lcd(1, 0x27);
	// lcd.printf("Hello World");

    // PCA9685 pca(1, 0x40);
	// /** 
	//  * \verbatim
	//  * 		Numbers of servo channels
	//  * 			   of each leg
	//  * 				 | Top | Mid | Low |
	//  * 				 |  0  |  1  |  2  |
	//  * --------------|-----|-----|-----|
	//  * Front Right 0 |	6  |  7	 |	8  |
	//  * Front Left  1 |	9  |  10 |	11 |
	//  * Back Right  2 |	3  |  4	 |	5  |
	//  * Back Left   3 |	0  |  1	 |	2  |
	//  * \endverbatim
	//  */
	// CalServo servo[4][3] {
	// 	// Top, Mid, and Low motors for each leg
	// 	{CalServo(&pca, 6), CalServo(&pca, 7), CalServo(&pca, 8)},		// Front Right 
	// 	{CalServo(&pca, 9), CalServo(&pca, 10), CalServo(&pca, 11)},	// Front Left
	// 	{CalServo(&pca, 3), CalServo(&pca, 4), CalServo(&pca, 5)},		// Back Right
	// 	{CalServo(&pca, 0), CalServo(&pca, 1), CalServo(&pca, 2)}		// Back Left
	// };

    // pca.set_pwm_freq(50);
	// usleep(2000000);
	
	// // servo[0][0].refresh_fitter(pwm_list, degree_list[6], 20);
	// // servo[0][1].refresh_fitter(pwm_list, degree_list[7], 20);
	// // servo[0][2].refresh_fitter(pwm_list, degree_list[8], 20);
	// // servo[1][0].refresh_fitter(pwm_list, degree_list[9], 20);
	// // servo[1][1].refresh_fitter(pwm_list, degree_list[10], 20);
	// // servo[1][2].refresh_fitter(pwm_list, degree_list[11], 20);
	// // servo[2][0].refresh_fitter(pwm_list, degree_list[3], 20);
	// // servo[2][1].refresh_fitter(pwm_list, degree_list[4], 20);
	// // servo[2][2].refresh_fitter(pwm_list, degree_list[5], 20);
	// // servo[3][0].refresh_fitter(pwm_list, degree_list[0], 20);
	// // servo[3][1].refresh_fitter(pwm_list, degree_list[1], 20);
	// // servo[3][2].refresh_fitter(pwm_list, degree_list[2], 20);

	// printf("Calibrating\n");

	// for(int i = 0; i < 4; i++)
	// 	for(int j = 0; j < 3; j++)
	// 		servo[i][j].refresh_fitter(pwm_list, degree_list[servo[i][j].getChannel()], 20);

	// printf("Calibrated\n");

	// // for(int i = 0; i < 4; i++)
	// // 	for(int j = 0; j < 3; j++)
	// // 		servo[i][j].set_degree(90);

	// int dest_servo = 0, dest_degree = 0;
    // while(true) {
	// 	scanf("%d %d", &dest_servo, &dest_degree);
	// 	servo[3][dest_servo].set_degree(dest_degree);
    // }

	// printf("Finished moving\n");

    
	// gpioTerminate();
	// return 0;

	// printf("Moving\n");
	// double degree[20];
	// int i = 0;
    // while(i < 20) {
    //     servo.set_PWM(pwm_list[i]);
	// 	scanf("%lf", &degree[i]);
	// 	i++;
    // }
	// printf("Calibrating\n");
	// servo.refresh_fitter(pwm_list, degree, 20);

	// // int servo_data_fd = open("servo_data.txt", O_RDWR | O_APPEND | O_CREAT, S_IRWXU);
	// // char buffer[128];
	// // for(int i = 0; i < 19; i++) {
	// // 	sprintf(buffer, "%d, ", degree[i]);
	// // 	write(servo_data_fd, buffer, strlen(buffer));
	// // }
	// // sprintf(buffer, "%d", degree[19]);
	// // write(servo_data_fd, buffer, sizeof(int));
	// // close(servo_data_fd);

	// printf("Calibrated\n");
	// int dest_degree = 0;
    // while(true) {
	// 	scanf("%d", &dest_degree);
    //     servo.set_degree(dest_degree);
    // }


	if (gpioInitialise() < 0) {
		printf("Failure...");
		exit(-1);
	}
	MPU6050 device(1, 0x68);

	float ax, ay, az, gr, gp, gy; //Variables to store the accel, gyro and angle values

	sleep(1); //Wait for the MPU6050 to stabilize

	/*
	//Calculate the offsets
	std::cout << "Calculating the offsets...\n    Please keep the accelerometer level and still\n    This could take a couple of minutes...";
	device.getOffsets(&ax, &ay, &az, &gr, &gp, &gy);
	std::cout << "Gyroscope R,P,Y: " << gr << "," << gp << "," << gy << "\nAccelerometer X,Y,Z: " << ax << "," << ay << "," << az << "\n";
	*/

	//Read the current yaw angle
	device.calc_yaw = true;

	while(1) {
		device.getAngle(0, &gr);
		device.getAngle(1, &gp);
		device.getAngle(2, &gy);
		std::cout << "Current angle around the roll axis: " << gr << "\n";
		std::cout << "Current angle around the pitch axis: " << gp << "\n";
		std::cout << "Current angle around the yaw axis: " << gy << "\n";
		usleep(250000); //0.25sec
	}

	//Get the current accelerometer values
	device.getAccel(&ax, &ay, &az);
	std::cout << "Accelerometer Readings: X: " << ax << ", Y: " << ay << ", Z: " << az << "\n";

	//Get the current gyroscope values
	device.getGyro(&gr, &gp, &gy);
	std::cout << "Gyroscope Readings: X: " << gr << ", Y: " << gp << ", Z: " << gy << "\n";

	gpioTerminate();
	return 0;
}
