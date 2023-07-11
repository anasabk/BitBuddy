#include "RobotDog.h"
#include <signal.h>


extern "C" int main() {
	if (gpioInitialise() < 0) {
		printf("Failure...");
		exit(-1);
	}

	RobotDog robot(1, MPU6050_DEF_I2C_ADDRESS, 1, 0x40, 1, 0x27);
	robot.run();

	gpioTerminate();
}