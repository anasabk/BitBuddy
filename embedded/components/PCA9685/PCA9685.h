/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * Name        : PCA9685.h
 * Author      : Georgi Todorov
 * Version     :
 * Created on  : Dec 9, 2012
 *
 * Copyright © 2012 Georgi Todorov  <terahz@geodar.com>
 */

#ifndef _PCA9685_H
#define _PCA9685_H

#include <inttypes.h>

// // Register Definitions
// #define MODE1 0x00			//Mode  register  1
// #define MODE2 0x01			//Mode  register  2
// #define SUBADR1 0x02		//I2C-bus subaddress 1
// #define SUBADR2 0x03		//I2C-bus subaddress 2
// #define SUBADR3 0x04		//I2C-bus subaddress 3
// #define ALLCALLADR 0x05     //LED All Call I2C-bus address
// #define LED0 0x6			//LED0 start register
// #define LED0_ON_L 0x6		//LED0 output and brightness control byte 0
// #define LED0_ON_H 0x7		//LED0 output and brightness control byte 1
// #define LED0_OFF_L 0x8		//LED0 output and brightness control byte 2
// #define LED0_OFF_H 0x9		//LED0 output and brightness control byte 3
#define LED_MULTIPLYER 4	// For the other 15 channels
// #define ALL_LED_ON_L 0xFA    //load all the LEDn_ON registers, byte 0 (turn 0-7 channels on)
// #define ALL_LED_ON_H 0xFB	//load all the LEDn_ON registers, byte 1 (turn 8-15 channels on)
// #define ALL_LED_OFF_L 0xFC	//load all the LEDn_OFF registers, byte 0 (turn 0-7 channels off)
// #define ALL_LED_OFF_H 0xFD	//load all the LEDn_OFF registers, byte 1 (turn 8-15 channels off)
// #define PRE_SCALE 0xFE		//prescaler for output frequency
// #define CLOCK_FREQ 25000000.0 //25MHz default osc clock

// #define OUTDRV 4U
// #define ALLCALL 1U
// #define SLEEP 16U

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
class PCA9685 {
public:
	PCA9685(int bus, int address);
	virtual ~PCA9685();

	void set_pwm_freq(int freq_hz);
	void set_pwm(uint8_t channel, int on, int off);
	void set_pwm(uint8_t channel, int value);
	int get_pwm(uint8_t channel);
	void set_all_pwm(const uint16_t on, const uint16_t off);
	void set_pwm_us(const int channel, const double ms);

private:
	int i2c_fd;
	int frequency = 200;
	void reset(void);
};
#endif

