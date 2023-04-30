#include "PCA9685.h"
#include "unistd.h"
#include "MPU6050.h"

int main() {
    // PCA9685 pca(1, 0x7f);

    // pca.setPWMFreq(50.0);
    
    // while(true) {
    //     pca.setPWM(0, 0, 370);
    //     usleep(1'000'000);
    //     pca.setPWM(0, 0, 415);
    //     usleep(1'000'000);
    //     pca.setPWM(0, 0, 460);
    //     usleep(1'000'000);
    //     pca.setPWM(0, 0, 415);
    //     usleep(1'000'000);
    // }


	gpioInitialise();
    MPU6050 device(0x68);

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

	return 0;
}