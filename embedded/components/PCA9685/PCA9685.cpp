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
 * Name        : PCA9685.cpp
 * Author      : Georgi Todorov
 * Version     :
 * Created on  : Dec 9, 2012
 *
 * Copyright © 2012 Georgi Todorov  <terahz@geodar.com>
 */

#include "PCA9685.h"
#include "pigpio.h"
#include "unistd.h"
#include "cmath"

//! Constructor takes bus and address arguments
/*!
 \param bus the bus to use in /dev/i2c-%d.
 \param address the device address on bus
 */
PCA9685::PCA9685(int bus, int address) {
	i2c_fd = i2cOpen(bus, address, 0);
	// reset();
	// set_pwm_freq(1000);

	set_all_pwm(0,0);
	// i2c_fd->WriteRegisterByte(MODE2, OUTDRV);
	// i2c_fd->WriteRegisterByte(MODE1, ALLCALL);
	i2cWriteByteData(i2c_fd, MODE2, OUTDRV);
	i2cWriteByteData(i2c_fd, MODE1, ALLCALL);
	usleep(5'000);
	// auto mode1_val = i2c_dev->ReadRegisterByte(MODE1);
	auto mode1_val = i2cReadByteData(i2c_fd, MODE1);
	mode1_val &= ~SLEEP;
	// i2c_dev->WriteRegisterByte(MODE1, mode1_val);
	i2cWriteByteData(i2c_fd, MODE1, mode1_val);
	usleep(5'000);
}

PCA9685::~PCA9685() {
	// delete i2c;
}

//! Sets PCA9685 mode to 00
void PCA9685::reset() {
	// i2c->write_byte(MODE1, 0x00); //Normal mode
	// i2c->write_byte(MODE2, 0x04); //totem pole
	i2cWriteByte(MODE1, 0x00);
	i2cWriteByte(MODE2, 0x04);
}

//! Set the frequency of PWM
/*!
 \param freq desired frequency. 40Hz to 1000Hz using internal 25MHz oscillator.
 */
void PCA9685::set_pwm_freq(int freq_hz) {
	frequency = freq_hz;

	auto prescaleval = 2.5e7; //    # 25MHz
	prescaleval /= 4096.0; //       # 12-bit
	prescaleval /= freq_hz;
	prescaleval -= 1.0;

	auto prescale = static_cast<int>(std::round(prescaleval));

	// const auto oldmode = i2c_dev->ReadRegisterByte(MODE1);
	const auto oldmode = i2cReadByteData(i2c_fd, MODE1);

	auto newmode = (oldmode & 0x7F) | SLEEP;

	// i2c_dev->WriteRegisterByte(MODE1, newmode);
	// i2c_dev->WriteRegisterByte(PRESCALE, prescale);
	// i2c_dev->WriteRegisterByte(MODE1, oldmode);

	i2cWriteByteData(i2c_fd, MODE1, newmode);
	i2cWriteByteData(i2c_fd, PRESCALE, prescale);
	i2cWriteByteData(i2c_fd, MODE1, oldmode);
	usleep(5'000);
	// i2c_dev->WriteRegisterByte(MODE1, oldmode | RESTART);
	i2cWriteByteData(i2c_fd, MODE1, oldmode | RESTART);
}

void PCA9685::set_all_pwm(const uint16_t on, const uint16_t off) {
//   i2c_dev->WriteRegisterByte(ALL_LED_ON_L, on & 0xFF);
//   i2c_dev->WriteRegisterByte(ALL_LED_ON_H, on >> 8);
//   i2c_dev->WriteRegisterByte(ALL_LED_OFF_L, off & 0xFF);
//   i2c_dev->WriteRegisterByte(ALL_LED_OFF_H, off >> 8);

  i2cWriteByteData(i2c_fd, ALL_LED_ON_L, on & 0xFF);
  i2cWriteByteData(i2c_fd, ALL_LED_ON_H, on >> 8);
  i2cWriteByteData(i2c_fd, ALL_LED_OFF_L, off & 0xFF);
  i2cWriteByteData(i2c_fd, ALL_LED_OFF_H, off >> 8);
}

//! PWM a single channel
/*!
 \param channel channel (1-16) to set PWM value for
 \param value 0-4095 value for PWM
 */
void PCA9685::set_pwm(uint8_t channel, int value) {
	set_pwm(channel, 0, value);
}

//! PWM a single channel with custom on time
/*!
 \param led channel (1-16) to set PWM value for
 \param on_value 0-4095 value to turn on the pulse
 \param off_value 0-4095 value to turn off the pulse
 */
void PCA9685::set_pwm(uint8_t channel, int on_value, int off_value) {
	// i2c->write_byte(LED0_ON_L + LED_MULTIPLYER * (led - 1), on_value & 0xFF);
	// i2c->write_byte(LED0_ON_H + LED_MULTIPLYER * (led - 1), on_value >> 8);
	// i2c->write_byte(LED0_OFF_L + LED_MULTIPLYER * (led - 1), off_value & 0xFF);
	// i2c->write_byte(LED0_OFF_H + LED_MULTIPLYER * (led - 1), off_value >> 8);

  	const auto channel_offset = 4 * channel;
	i2cWriteByte(LED0_ON_L + channel_offset, on_value & 0xFF);
	i2cWriteByte(LED0_ON_H + channel_offset, on_value >> 8);
	i2cWriteByte(LED0_OFF_L + channel_offset, off_value & 0xFF);
	i2cWriteByte(LED0_OFF_H + channel_offset, off_value >> 8);
}

//! Get current PWM value
/*!
 \param led channel (1-16) to get PWM value from
 */
int PCA9685::get_pwm(uint8_t led){
	int ledval = 0;
	// ledval = i2c->read_byte(LED0_OFF_H + LED_MULTIPLYER * (led-1));
	ledval = i2cReadByte(LED0_OFF_H + LED_MULTIPLYER * (led-1));
	ledval = ledval & 0xf;
	ledval <<= 8;
	// ledval += i2c->read_byte(LED0_OFF_L + LED_MULTIPLYER * (led-1));
	ledval += i2cReadByte(LED0_OFF_L + LED_MULTIPLYER * (led-1));
	return ledval;
}

void PCA9685::set_pwm_ms(const int channel, const double ms) {
	auto period_ms = 1000.0 / frequency;
	auto bits_per_ms = 4096 / period_ms;
	auto bits = ms * bits_per_ms;
	set_pwm(channel, 0, bits);
}
