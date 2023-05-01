#include "PCA9685.h"
#include "unistd.h"
#include "MPU6050.h"
#include "LCD.h"
#include "CalServo.h"
#include <cstdio>

int pwm_list[20] = {450, 550, 650, 750, 850, 950, 1050, 1150, 1250, 1350, 1450, 1550, 1650, 1750, 1850, 1950, 2050, 2150, 2250, 2350};

int degree_list[1][20] = {
	{0, 8, 16, 26,36, 45, 53, 63, 72, 80, 88, 96, 104, 112, 120, 130, 139, 148, 158, 168}
};

int main() {
	if (gpioInitialise() < 0) {
		printf("Failure...");
		exit(-1);
	}
    PCA9685 pca(1, 0x40);
	CalServo servo(&pca, 0);

	printf("initialized\n");
    pca.set_pwm_freq(50);
    
	printf("Moving\n");
	int pwm[20];
	double degree[20];
	int i = 0;
    while(i < 20) {
		scanf("%d", &pwm[i]);
        servo.set_PWM(pwm[i]);
		scanf("%lf", &degree[i]);
		i++;
    }
	printf("Calibrating\n");
	servo.refresh_fitter(pwm, degree, 20);

	int servo_data_fd = open("servo_data.txt", O_RDWR | O_APPEND | O_CREAT, S_IRWXU);
	char buffer[128];
	for(int i = 0; i < 19; i++) {
		sprintf(buffer, "%d, ", degree[i]);
		write(servo_data_fd, buffer, sizeof(int));
	}
	sprintf(buffer, "%d", degree[19]);
	write(servo_data_fd, buffer, sizeof(int));
	close(servo_data_fd);

	printf("Calibrated\n");
	int dest_degree = 0;
    while(true) {
		scanf("%d", &dest_degree);
        servo.set_degree(dest_degree);
    }

	gpioTerminate();
	return 0;

	// if (gpioInitialise() < 0) {
	// 	printf("Failure...");
	// 	exit(-1);
	// }
	// MPU6050 device(0x68);

	// float ax, ay, az, gr, gp, gy; //Variables to store the accel, gyro and angle values

	// sleep(1); //Wait for the MPU6050 to stabilize

	// /*
	// //Calculate the offsets
	// std::cout << "Calculating the offsets...\n    Please keep the accelerometer level and still\n    This could take a couple of minutes...";
	// device.getOffsets(&ax, &ay, &az, &gr, &gp, &gy);
	// std::cout << "Gyroscope R,P,Y: " << gr << "," << gp << "," << gy << "\nAccelerometer X,Y,Z: " << ax << "," << ay << "," << az << "\n";
	// */

	// //Read the current yaw angle
	// device.calc_yaw = true;

	// while(1) {
	// 	device.getAngle(0, &gr);
	// 	device.getAngle(1, &gp);
	// 	device.getAngle(2, &gy);
	// 	std::cout << "Current angle around the roll axis: " << gr << "\n";
	// 	std::cout << "Current angle around the pitch axis: " << gp << "\n";
	// 	std::cout << "Current angle around the yaw axis: " << gy << "\n";
	// 	usleep(250000); //0.25sec
	// }

	// //Get the current accelerometer values
	// device.getAccel(&ax, &ay, &az);
	// std::cout << "Accelerometer Readings: X: " << ax << ", Y: " << ay << ", Z: " << az << "\n";

	// //Get the current gyroscope values
	// device.getGyro(&gr, &gp, &gy);
	// std::cout << "Gyroscope Readings: X: " << gr << ", Y: " << gp << ", Z: " << gy << "\n";

	// gpioTerminate();
	// return 0;


	// if (gpioInitialise() < 0) {
	// 	printf("Failure...");
	// 	exit(-1);
	// }
	// // Initialize the LCD driver
	// LCD lcd(1, 0x27);
    // lcd.enableCursor();
    // lcd.enableBlinking();
	// while (1) {
    //     lcd.setPosition(0, 0);
    //     lcd.putChar(65);  // Put char 'A'
    //     std::this_thread::sleep_for(std::chrono::seconds(1));
    //     lcd.setPosition(0, 1);
    //     lcd.putChar(66); // Put char 'B'
    //     std::this_thread::sleep_for(std::chrono::seconds(1));
    //     lcd.setPosition(0, 0);
    //     lcd.putChar(67); // Put char 'C'
    //     std::this_thread::sleep_for(std::chrono::seconds(1));
    //     lcd.setPosition(0, 1);
    //     lcd.putChar(68); // Put char 'D'
    //     lcd<<"ABC"; // Put string "ABC"
    //     std::this_thread::sleep_for(std::chrono::seconds(1));
	// }
	
	// gpioTerminate();
	// return 0;
}
