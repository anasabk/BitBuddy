//-------------------------------MPU6050 Accelerometer and Gyroscope C++ library-----------------------------
//Copyright (c) 2019, Alex Mous
//Licensed under the CC BY-NC SA 4.0


//-----------------------MODIFY THESE PARAMETERS-----------------------

#define GYRO_RANGE 0 //Select which gyroscope range to use (see the table below) - Default is 0
//	Gyroscope Range
//	0	+/- 250 degrees/second
//	1	+/- 500 degrees/second
//	2	+/- 1000 degrees/second
//	3	+/- 2000 degrees/second
//See the MPU6000 Register Map for more information


#define ACCEL_RANGE 0 //Select which accelerometer range to use (see the table below) - Default is 0
//	Accelerometer Range
//	0	+/- 2g
//	1	+/- 4g
//	2	+/- 8g
//	3	+/- 16g
//See the MPU6000 Register Map for more information


//Offsets - supply your own here (calculate offsets with getOffsets function)
//     Accelerometer
#define A_OFF_X 19402
#define A_OFF_Y -2692
#define A_OFF_Z -8625
//    Gyroscope
#define G_OFF_X -733
#define G_OFF_Y 433
#define G_OFF_Z -75

//-----------------------END MODIFY THESE PARAMETERS-----------------------

#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <time.h>
extern "C" {
	#include <linux/i2c-dev.h>
	#include <i2c/smbus.h>
}
#include <cmath>
#include <thread>

#include "pigpio.h"
#include "I2Cdev.h"

#define _POSIX_C_SOURCE 200809L //Used for calculating time

#define TAU 0.05 //Complementary filter percentage
#define RAD_T_DEG 57.29577951308 //Radians to degrees (180/PI)

//Select the appropriate settings
#if GYRO_RANGE == 1
	#define GYRO_SENS 65.5
	#define GYRO_CONFIG 0b00001000
#elif GYRO_RANGE == 2
	#define GYRO_SENS 32.8
	#define GYRO_CONFIG 0b00010000
#elif GYRO_RANGE == 3
	#define GYRO_SENS 16.4
	#define GYRO_CONFIG 0b00011000
#else //Otherwise, default to 0
	#define GYRO_SENS 131.0
	#define GYRO_CONFIG 0b00000000
#endif
#undef GYRO_RANGE


#if ACCEL_RANGE == 1
	#define ACCEL_SENS 8192.0
	#define ACCEL_CONFIG 0b00001000
#elif ACCEL_RANGE == 2
	#define ACCEL_SENS 4096.0
	#define ACCEL_CONFIG 0b00010000
#elif ACCEL_RANGE == 3
	#define ACCEL_SENS 2048.0
	#define ACCEL_CONFIG 0b00011000
#else //Otherwise, default to 0
	#define ACCEL_SENS 16384.0
	#define ACCEL_CONFIG 0b00000000
#endif
#undef ACCEL_RANGE


#define I2C_ADDRESS 0x68 // I2C address of MPU6050

#define MPU6050_ACCEL_XOUT_H 	0x3B
#define MPU6050_ACCEL_XOUT_L 	0x3C
#define MPU6050_ACCEL_YOUT_H 	0x3D
#define MPU6050_ACCEL_YOUT_L 	0x3E
#define MPU6050_ACCEL_ZOUT_H 	0x3F
#define MPU6050_ACCEL_ZOUT_L 	0x40
#define MPU6050_TEMP_OUT_H 		0x41
#define MPU6050_TEMP_OUT_L 		0x42
#define MPU6050_GYRO_XOUT_H 	0x43
#define MPU6050_GYRO_XOUT_L 	0x44
#define MPU6050_GYRO_YOUT_H 	0x45
#define MPU6050_GYRO_YOUT_L 	0x46
#define MPU6050_GYRO_ZOUT_H 	0x47
#define MPU6050_GYRO_ZOUT_L 	0x48

#define MPU6050_PWR_MGMT_1          0x6B

#define MPU6050_RA_CONFIG           0x1A
#define MPU6050_RA_GYRO_CONFIG      0x1B
#define MPU6050_RA_ACCEL_CONFIG     0x1C

#define MPU6050_GYRO_FS_250         0x00
#define MPU6050_GYRO_FS_500         0x01
#define MPU6050_GYRO_FS_1000        0x02
#define MPU6050_GYRO_FS_2000        0x03

#define MPU6050_ACCEL_FS_2          0x00
#define MPU6050_ACCEL_FS_4          0x01
#define MPU6050_ACCEL_FS_8          0x02
#define MPU6050_ACCEL_FS_16         0x03

#define MPU6050_GCONFIG_FS_SEL_BIT      4
#define MPU6050_GCONFIG_FS_SEL_LENGTH   2

#define MPU6050_CFG_DLPF_CFG_BIT    2
#define MPU6050_CFG_DLPF_CFG_LENGTH 3

#define MPU6050_DLPF_BW_256         0x00
#define MPU6050_DLPF_BW_188         0x01
#define MPU6050_DLPF_BW_98          0x02
#define MPU6050_DLPF_BW_42          0x03
#define MPU6050_DLPF_BW_20          0x04
#define MPU6050_DLPF_BW_10          0x05
#define MPU6050_DLPF_BW_5           0x06

/*
 * The following registers contain the primary data we are interested in
 * 0x3B MPU6050_ACCEL_XOUT_H
 * 0x3C MPU6050_ACCEL_XOUT_L
 * 0x3D MPU6050_ACCEL_YOUT_H
 * 0x3E MPU6050_ACCEL_YOUT_L
 * 0x3F MPU6050_ACCEL_ZOUT_H
 * 0x40 MPU6050_ACCEL_ZOUT_L
 * 0x41 MPU6050_TEMP_OUT_H
 * 0x42 MPU6050_TEMP_OUT_L
 * 0x43 MPU6050_GYRO_XOUT_H
 * 0x44 MPU6050_GYRO_XOUT_L
 * 0x45 MPU6050_GYRO_YOUT_H
 * 0x46 MPU6050_GYRO_YOUT_L
 * 0x47 MPU6050_GYRO_ZOUT_H
 * 0x48 MPU6050_GYRO_ZOUT_L
 */


class MPU6050 : private I2Cdev {
	

public:
	MPU6050(int8_t bus, int8_t addr, bool run_update_thread = true);
	
	typedef struct {
		float x_rot;
		float x_accel;
		float y_rot;
		float y_accel;
		float z_rot;
		float z_accel;
		float tempr;
	} MPU6050_data_t;
	
	void read_data(MPU6050_data_t *buffer);

	void getAccelRaw(float *x, float *y, float *z);
	void getGyroRaw(float *roll, float *pitch, float *yaw);
	void getAccel(float *x, float *y, float *z);
	void getGyro(float *roll, float *pitch, float *yaw);
	void getOffsets(float *ax_off, float *ay_off, float *az_off, float *gr_off, float *gp_off, float *gy_off);
	int getAngle(int axis, float *result);
	void calibrate();
	bool calc_yaw;

	// /**
	//  * @brief Create a seperate thread that updates 
	//  * readings in the background every dt seconds.
	//  */
	// void run_update_thread();


private:
	void _update();

	float _accel_angle[3];
	float _gyro_angle[3];
	float _angle[3]; //Store all angles (accel roll, accel pitch, accel yaw, gyro roll, gyro pitch, gyro yaw, comb roll, comb pitch comb yaw)

	MPU6050_data_t read_buffer; //Temporary storage variables used in _update()
	MPU6050_data_t cal_offsets;

	int MPU6050_addr;
	int f_dev; //Device file

	float dt; //Loop time (recalculated with each loop)

	struct timespec start,end; //Create a time structure

	bool _first_run = 1; //Variable for whether to set gyro angle to acceleration angle in compFilter

	MPU6050_data_t offsets;

};
