#include "RobotDog.h"
#include <signal.h>


extern "C" int main(int argc, char *argv[]) {
	if(argc < 2) {
		printf("Invalid number of arguments, aborting ...");
		return 0;
	}

	if (gpioInitialise() < 0) {
		printf("Failure...");
		exit(-1);
	}

	RobotDog robot(1, MPU6050_DEF_I2C_ADDRESS, 1, 0x40, 1, 0x27, argv[1]);
	robot.run();

	gpioTerminate();
}