#ifndef _PCA9685_H
#define _PCA9685_H

#include <inttypes.h>
#include "I2Cdev.h"

// // Register Definitions
// #define ALLCALLADR 0x05     //LED All Call I2C-bus address
#define LED_MULTIPLYER 4	// For the other 15 channels

// Registers/etc:
#define MODE1        	 0x00
#define MODE2        	 0x01
#define SUBADR1      	 0x02
#define SUBADR2      	 0x03
#define SUBADR3      	 0x04
#define PRESCALE     	 0xFE
#define LED0_ON_L    	 0x06
#define LED0_ON_H    	 0x07
#define LED0_OFF_L   	 0x08
#define LED0_OFF_H   	 0x09
#define ALL_LED_ON_L 	 0xFA
#define ALL_LED_ON_H 	 0xFB
#define ALL_LED_OFF_L	 0xFC
#define ALL_LED_OFF_H	 0xFD

// Bits:
#define RESTART        0x80
#define SLEEP          0x10
#define ALLCALL        0x01
#define INVRT          0x10
#define OUTDRV         0x04


//! Main class that exports features for PCA9685 chip
class PCA9685 : public I2Cdev {
public:
	PCA9685(int bus, int address, int frequency);
	virtual ~PCA9685();

	void set_pwm_freq(int freq_hz);
	void set_pwm(uint8_t channel, int on, int off);
	void set_pwm(uint8_t channel, int value);
	int get_pwm(uint8_t channel);
	void set_all_pwm(uint16_t on, uint16_t off);
	void set_pwm_us(const int channel, const int us);

private:
	int i2c_fd;
	int frequency = 200;
	void reset(void);
};
#endif
