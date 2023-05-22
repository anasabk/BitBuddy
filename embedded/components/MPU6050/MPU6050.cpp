//-------------------------------MPU6050 Accelerometer and Gyroscope C++ library-----------------------------
//Copyright (c) 2019, Alex Mous
//Licensed under the CC BY-NC SA 4.0

//Include the header file for this class
#include "MPU6050.h"


MPU6050::MPU6050(int8_t bus, int8_t addr, bool run_update_thread) 
	: I2Cdev(bus, addr)
{
	int status;

	MPU6050_addr = addr;
	dt = 0.009; //Loop time (recalculated with each loop)
	_first_run = 1; //Variable for whether to set gyro angle to acceleration angle in compFilter
	calc_yaw = false;

	// fd = i2cOpen(1, addr, 0);

    write_byte(MPU6050_PWR_MGMT_1, 0);
    write_bits(MPU6050_RA_GYRO_CONFIG, MPU6050_GCONFIG_FS_SEL_BIT, MPU6050_GCONFIG_FS_SEL_LENGTH, MPU6050_ACCEL_FS_2);
    write_bits(MPU6050_RA_ACCEL_CONFIG, MPU6050_GCONFIG_FS_SEL_BIT, MPU6050_GCONFIG_FS_SEL_LENGTH, MPU6050_GYRO_FS_250);
    write_bits(MPU6050_RA_CONFIG, MPU6050_CFG_DLPF_CFG_BIT, MPU6050_CFG_DLPF_CFG_LENGTH, MPU6050_DLPF_BW_5);

	// i2cWriteByteData(f_dev, 0x6b, 0b00000000);
	// i2cWriteByteData(f_dev, 0x1a, 0b00000011);
	// i2cWriteByteData(f_dev, 0x19, 0b00000100);
	// i2cWriteByteData(f_dev, 0x1b, GYRO_CONFIG);
	// i2cWriteByteData(f_dev, 0x1c, ACCEL_CONFIG);

	// i2cWriteByteData(f_dev, 0x00, 0b10000001);
	// i2cWriteByteData(f_dev, 0x01, 0b00000001);
	// i2cWriteByteData(f_dev, 0x02, 0b10000001);
	// i2cWriteByteData(f_dev, 0x06, 0b00000000); 
	// i2cWriteByteData(f_dev, 0x07, 0b00000000);
	// i2cWriteByteData(f_dev, 0x08, 0b00000000);
	// i2cWriteByteData(f_dev, 0x09, 0b00000000);
	// i2cWriteByteData(f_dev, 0x0A, 0b00000000);
	// i2cWriteByteData(f_dev, 0x0B, 0b00000000);

	if (run_update_thread){
		std::thread(&MPU6050::_update, this).detach(); //Create a seperate thread, for the update routine to run in the background, and detach it, allowing the program to continue
	}
}

void MPU6050::getGyroRaw(float *x, float *y, float *z) {
	// uint8_t data[6];

    // // select_reg(MPU6050_ACCEL_XOUT_H);
    // read_bytes(MPU6050_ACCEL_XOUT_H, 6, data);

    // *x = ((int16_t) ((data[0] << 8) | data[1])) / 131.0;
    // *y = ((int16_t) ((data[2] << 8) | data[3])) / 131.0;
    // *z = ((int16_t) ((data[4] << 8) | data[5])) / 131.0;

	// int16_t temp_x = i2cReadByteData(f_dev, 0x43) << 8 | i2cReadByteData(f_dev, 0x44); //Read X registers
	// int16_t temp_y = i2cReadByteData(f_dev, 0x45) << 8 | i2cReadByteData(f_dev, 0x46); //Read Y registers
	// int16_t temp_z = i2cReadByteData(f_dev, 0x47) << 8 | i2cReadByteData(f_dev, 0x48); //Read Z registers

	uint8_t tempx[2], tempy[2], tempz[2];

	read_byte(MPU6050_GYRO_XOUT_H, &tempx[1]);
	read_byte(MPU6050_GYRO_XOUT_L, &tempx[0]);
	read_byte(MPU6050_GYRO_YOUT_H, &tempy[1]);
	read_byte(MPU6050_GYRO_YOUT_L, &tempy[0]);
	read_byte(MPU6050_GYRO_ZOUT_H, &tempz[1]);
	read_byte(MPU6050_GYRO_ZOUT_L, &tempz[0]);

	*x = ((int16_t) ((tempx[1] << 8) | tempx[0])) / GYRO_SENS; //Roll on X axis
	*y = ((int16_t) ((tempy[1] << 8) | tempy[0])) / GYRO_SENS; //Pitch on Y axis
	*z = ((int16_t) ((tempz[1] << 8) | tempz[0])) / GYRO_SENS; //Yaw on Z axis
}

void MPU6050::read_data(MPU6050_data_t *buffer) {
	uint8_t data[14];

    // select_reg(MPU6050_ACCEL_XOUT_H);
    read_bytes(MPU6050_ACCEL_XOUT_H, 14, data);

	buffer->x_accel = ((int16_t) ((data[0]  << 8) | data[1])) / ACCEL_SENS;
    buffer->y_accel = ((int16_t) ((data[2]  << 8) | data[3])) / ACCEL_SENS;
    buffer->z_accel = ((int16_t) ((data[4]  << 8) | data[5])) / ACCEL_SENS;
    buffer->tempr  = ((int16_t) ((data[6]  << 8) | data[7])) / 340.0 + 36.53;
    buffer->x_rot  = ((int16_t) ((data[8]  << 8) | data[9])) / GYRO_SENS;
    buffer->y_rot  = ((int16_t) ((data[10] << 8) | data[11]))/ GYRO_SENS;
    buffer->z_rot  = ((int16_t) ((data[12] << 8) | data[13]))/ GYRO_SENS;
}

void MPU6050::getGyro(float *x, float *y, float *z) {
	getGyroRaw(x, y, z);
	*x = ((int16_t) ((*x - cal_offsets.x_rot) * 1000)) / 1000.0;
	*y = ((int16_t) ((*y - cal_offsets.y_rot) * 1000)) / 1000.0;
	*z = ((int16_t) ((*z - cal_offsets.z_rot) * 1000)) / 1000.0;

	// getGyroRaw(roll, pitch, yaw); //Store raw values into variables
	// *roll = round((*roll - G_OFF_X) * 1000.0 / GYRO_SENS) / 1000.0; //Remove the offset and divide by the gyroscope sensetivity (use 1000 and round() to round the value to three decimal places)
	// *pitch = round((*pitch - G_OFF_Y) * 1000.0 / GYRO_SENS) / 1000.0;
	// *yaw = round((*yaw - G_OFF_Z) * 1000.0 / GYRO_SENS) / 1000.0;
}

void MPU6050::getAccelRaw(float *x, float *y, float *z) {
	uint8_t tempx[2], tempy[2], tempz[2];

	read_byte(MPU6050_ACCEL_XOUT_H, &tempx[1]);
	read_byte(MPU6050_ACCEL_XOUT_L, &tempx[0]);
	read_byte(MPU6050_ACCEL_YOUT_H, &tempy[1]);
	read_byte(MPU6050_ACCEL_YOUT_L, &tempy[0]);
	read_byte(MPU6050_ACCEL_ZOUT_H, &tempz[1]);
	read_byte(MPU6050_ACCEL_ZOUT_L, &tempz[0]);

	*x = (tempx[1] << 8 | tempx[0]) / ACCEL_SENS; //Roll on X axis
	*y = (tempy[1] << 8 | tempy[0]) / ACCEL_SENS; //Pitch on Y axis
	*z = (tempz[1] << 8 | tempz[0]) / ACCEL_SENS; //Yaw on Z axis

	// int16_t X = i2cReadByteData(f_dev, 0x3b) << 8 | i2cReadByteData(f_dev, 0x3c); //Read X registers
	// int16_t Y = i2cReadByteData(f_dev, 0x3d) << 8 | i2cReadByteData(f_dev, 0x3e); //Read Y registers
	// int16_t Z = i2cReadByteData(f_dev, 0x3f) << 8 | i2cReadByteData(f_dev, 0x40); //Read Z registers
	// *x = (float)X;
	// *y = (float)Y;
	// *z = (float)Z;
}

void MPU6050::getAccel(float *x, float *y, float *z) {
	getAccelRaw(x, y, z); //Store raw values into variables
	
	//Remove the offset and divide by the accelerometer sensetivity (use 1000 and round() to round the value to three decimal places)
	*x = ((int16_t) ((*x - cal_offsets.x_accel) * 1000)) / 1000.0;
	*y = ((int16_t) ((*y - cal_offsets.y_accel) * 1000)) / 1000.0;
	*z = ((int16_t) ((*z - cal_offsets.z_accel) * 1000)) / 1000.0;
}

void MPU6050::getOffsets(float *ax_off, float *ay_off, float *az_off, float *gr_off, float *gp_off, float *gy_off) {
	float gyro_off[3]; //Temporary storage
	float accel_off[3];

	*gr_off = 0, *gp_off = 0, *gy_off = 0; //Initialize the offsets to zero
	*ax_off = 0, *ay_off = 0, *az_off = 0; //Initialize the offsets to zero

	for (int i = 0; i < 10000; i++) { //Use loop to average offsets
		getGyroRaw(&gyro_off[0], &gyro_off[1], &gyro_off[2]); //Raw gyroscope values
		*gr_off = *gr_off + gyro_off[0], *gp_off = *gp_off + gyro_off[1], *gy_off = *gy_off + gyro_off[2]; //Add to sum

		getAccelRaw(&accel_off[0], &accel_off[1], &accel_off[2]); //Raw accelerometer values
		*ax_off = *ax_off + accel_off[0], *ay_off = *ay_off + accel_off[1], *az_off = *az_off + accel_off[2]; //Add to sum
	}

	*gr_off = *gr_off / 10000, *gp_off = *gp_off / 10000, *gy_off = *gy_off / 10000; //Divide by number of loops (to average)
	*ax_off = *ax_off / 10000, *ay_off = *ay_off / 10000, *az_off = *az_off / 10000;

	*az_off = *az_off - ACCEL_SENS; //Remove 1g from the value calculated to compensate for gravity)
}

int MPU6050::getAngle(int axis, float *result) {
	if (axis >= 0 && axis <= 2) { //Check that the axis is in the valid range
		*result = _angle[axis]; //Get the result
		return 0;
	}
	else {
		std::cout << "ERR (MPU6050.cpp:getAngle()): 'axis' must be between 0 and 2 (for roll, pitch or yaw)\n"; //Print error message
		*result = 0; //Set result to zero
		return 1;
	}
}

void MPU6050::_update() { //Main update function - runs continuously
	clock_gettime(CLOCK_REALTIME, &start); //Read current time into start variable

	while (1) { //Loop forever
		read_data(&read_buffer);

		//X (roll) axis
		_accel_angle[0] = atan2(read_buffer.z_accel,read_buffer.y_accel) * RAD_T_DEG - 90.0; //Calculate the angle with z and y convert to degrees and subtract 90 degrees to rotate
		_gyro_angle[0] = _angle[0] + read_buffer.x_rot*dt; //Use roll axis (X axis)

		//Y (pitch) axis
		_accel_angle[1] = atan2(read_buffer.z_accel, read_buffer.x_accel) * RAD_T_DEG - 90.0; //Calculate the angle with z and x convert to degrees and subtract 90 degrees to rotate
		_gyro_angle[1] = _angle[1] + read_buffer.y_rot*dt; //Use pitch axis (Y axis)

		//Z (yaw) axis
		if (calc_yaw) {
			_gyro_angle[2] = _angle[2] + read_buffer.z_rot*dt; //Use yaw axis (Z axis)
		}


		if (_first_run) { //Set the gyroscope angle reference point if this is the first function run
			for (int i = 0; i <= 1; i++) {
				_gyro_angle[i] = _accel_angle[i]; //Start off with angle from accelerometer (absolute angle since gyroscope is relative)
			}
			_gyro_angle[2] = 0; //Set the yaw axis to zero (because the angle cannot be calculated with the accelerometer when vertical)
			_first_run = 0;
		}

		float asum = abs(read_buffer.x_accel) + abs(read_buffer.y_accel) + abs(read_buffer.z_accel); //Calculate the sum of the accelerations
		float gsum = abs(read_buffer.x_rot) + abs(read_buffer.y_rot) + abs(read_buffer.z_rot); //Calculate the sum of the gyro readings

		for (int i = 0; i <= 1; i++) { //Loop through roll and pitch axes
			if (abs(_gyro_angle[i] - _accel_angle[i]) > 5) { //Correct for very large drift (or incorrect measurment of gyroscope by longer loop time)
				_gyro_angle[i] = _accel_angle[i];
			}

			//Create result from either complementary filter or directly from gyroscope or accelerometer depending on conditions
			if (asum > 0.1 && asum < 3 && gsum > 0.3) { //Check that th movement is not very high (therefore providing inacurate angles)
				_angle[i] = (1 - TAU)*(_gyro_angle[i]) + (TAU)*(_accel_angle[i]); //Calculate the angle using a complementary filter
			}
			else if (gsum > 0.3) { //Use the gyroscope angle if the acceleration is high
				_angle[i] = _gyro_angle[i];
			}
			else if (gsum <= 0.3) { //Use accelerometer angle if not much movement
				_angle[i] = _accel_angle[i];
			}
		}

		//The yaw axis will not work with the accelerometer angle, so only use gyroscope angle
		if (calc_yaw) { //Only calculate the angle when we want it to prevent large drift
			_angle[2] = _gyro_angle[2];
		}
		else {
			_angle[2] = 0;
			_gyro_angle[2] = 0;
		}

		clock_gettime(CLOCK_REALTIME, &end); //Save time to end clock
		dt = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9; //Calculate new dt
		clock_gettime(CLOCK_REALTIME, &start); //Save time to start clock
	}
}

void MPU6050::calibrate() {
    MPU6050_data_t readings, temp_offset;

    offsets.x_rot = 0.0;
    offsets.x_accel = 0.0;
    offsets.y_rot = 0.0;
    offsets.y_accel = 0.0;
    offsets.z_rot = 0.0;
    offsets.z_accel = 0.0;

    while(1) {
        temp_offset.x_rot = 0.0;
        temp_offset.x_accel = 0.0;
        temp_offset.y_rot = 0.0;
        temp_offset.y_accel = 0.0;
        temp_offset.z_rot = 0.0;
        temp_offset.z_accel = 0.0;

        // vTaskDelay(1000 / portTICK_PERIOD_MS);
		usleep(1000000);

        for(int i = 0; i < 500; i++) {
            getAccelRaw(&readings.x_accel, &readings.y_accel, &readings.z_accel);
            getGyroRaw(&readings.x_rot, &readings.y_rot, &readings.z_rot);

            temp_offset.x_rot 	+= readings.x_rot;
            temp_offset.x_accel += readings.x_accel;
            temp_offset.y_rot 	+= readings.y_rot;
            temp_offset.y_accel += readings.y_accel;
            temp_offset.z_rot 	+= readings.z_rot;
            temp_offset.z_accel += readings.z_accel;

            // vTaskDelay(10 / portTICK_PERIOD_MS);
			usleep(10000);
        }

        temp_offset.x_rot 	/= 500;
        temp_offset.x_accel /= 500;
        temp_offset.y_rot 	/= 500;
        temp_offset.y_accel /= 500;
        temp_offset.z_rot 	/= 500;
        temp_offset.z_accel /= 500;
        temp_offset.z_accel -= 0.999;
        
		getAccelRaw(&readings.x_accel, &readings.y_accel, &readings.z_accel);
		getGyroRaw(&readings.x_rot, &readings.y_rot, &readings.z_rot);

        printf("raw accel: x=%.4lf y=%.4lf z=%.4lf / gyro: x=%.4lf y=%.4lf z=%.4lf / cal accel: x=%.4lf y=%.4lf z=%.4lf / gyro: x=%.4lf y=%.4lf z=%.4lf\n", 
                readings.x_accel,
                readings.y_accel,
                readings.z_accel,
                readings.x_rot,
                readings.y_rot,
                readings.z_rot,
                readings.x_accel - temp_offset.x_accel, 
                readings.y_accel - temp_offset.y_accel, 
                readings.z_accel - temp_offset.z_accel,
                readings.x_rot - temp_offset.x_rot, 
                readings.y_rot - temp_offset.y_rot, 
                readings.z_rot - temp_offset.z_rot
        );

        if (readings.x_rot - temp_offset.x_rot <  0.1 && 
            readings.x_rot - temp_offset.x_rot > -0.1 &&
            readings.y_rot - temp_offset.y_rot <  0.1 && 
            readings.y_rot - temp_offset.y_rot > -0.1 &&
            readings.z_rot - temp_offset.z_rot <  0.1 && 
            readings.z_rot - temp_offset.z_rot > -0.1 &&
            readings.x_accel - temp_offset.x_accel <  0.01 && 
            readings.x_accel - temp_offset.x_accel > -0.01 &&
            readings.y_accel - temp_offset.y_accel <  0.01 && 
            readings.y_accel - temp_offset.y_accel > -0.01 &&
            readings.z_accel - temp_offset.z_accel < 1.00 && 
            readings.z_accel - temp_offset.z_accel > 0.99)
        {
            offsets = temp_offset;
            return;
        }
    }
}
