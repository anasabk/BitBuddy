#include "PCA9685.h"
#include "cmath"

/**
 * @brief Construct a new PCA9685::PCA9685 object
 * 
 * @param bus I2C bus.
 * @param address I2C address.
 * @param frequency Initial signal frequency.
 */
PCA9685::PCA9685(int bus, int address, int frequency) 
	: I2Cdev(bus, address) 
{
	reset();
	set_pwm_freq(frequency);
	set_all_pwm(0,0);

	write_byte(MODE2, OUTDRV);
	write_byte(MODE1, ALLCALL);
	usleep(1000);

	uint8_t mode1_val = 0;
	read_byte(MODE1, &mode1_val);
	mode1_val &= ~SLEEP;

	write_byte(MODE1, mode1_val);
	usleep(1000);
}

PCA9685::~PCA9685() {
	set_all_pwm(0, 0);
}

/**
 * @brief Set the PCA9685 to mode 00.
 */
void PCA9685::reset() {
	write_byte(MODE1, 0x00);
	write_byte(MODE2, 0x04);
}

/**
 * @brief Set signal frequency to all channels.
 * 
 * @param freq_hz frequency in Hz.
 */
void PCA9685::set_pwm_freq(int freq_hz) {
	frequency = freq_hz;

	auto prescaleval = 2.5e7; //    # 25MHz
	prescaleval /= 4096.0; //       # 12-bit
	prescaleval /= freq_hz;
	prescaleval -= 1.0;

	auto prescale = static_cast<int>(std::round(prescaleval));

	uint8_t oldmode;
	read_byte(MODE1, &oldmode);

	auto newmode = (oldmode & 0x7F) | SLEEP;

	write_byte(MODE1, newmode);
	write_byte(PRESCALE, prescale);
	write_byte(MODE1, oldmode);
	usleep(1000);

	write_byte(MODE1, oldmode | RESTART);
}

void PCA9685::set_all_pwm(const uint16_t on, const uint16_t off) {
	write_byte(ALL_LED_ON_L, on & 0xFF);
	write_byte(ALL_LED_ON_H, on >> 8);
	write_byte(ALL_LED_OFF_L, off & 0xFF);
	write_byte(ALL_LED_OFF_H, off >> 8);
}

void PCA9685::set_pwm(uint8_t channel, int value) {
	set_pwm(channel, 0, value);
}

void PCA9685::set_pwm(uint8_t channel, int on_value, int off_value) {
  	const int channel_offset = 4 * (channel);
	write_byte(LED0_ON_L + channel_offset, on_value & 0xFF);
	write_byte(LED0_ON_H + channel_offset, on_value >> 8);
	write_byte(LED0_OFF_L + channel_offset, off_value & 0xFF);
	write_byte(LED0_OFF_H + channel_offset, off_value >> 8);
}

/**
 * @brief Get PWM signal given to channel.
 * 
 * @param channel channel number.
 * @return PWM signal.
 */
int PCA9685::get_pwm(uint8_t channel){
	int ledval = 0;
	uint8_t temp;
	
	read_byte(LED0_OFF_H + LED_MULTIPLYER * (channel-1), &temp);
	ledval = temp & 0xf;
	ledval <<= 8;
	
	read_byte(LED0_OFF_L + LED_MULTIPLYER * (channel-1), &temp);
	ledval += temp;
	return ledval;
}

/**
 * @brief Set a PWM signal channel with the length of 
 * the high part in microseconds.
 * 
 * @param channel Channel number.
 * @param us length of the high signal.
 */
void PCA9685::set_pwm_us(int channel, int us) {
	auto period_ms = 1000000.0 / frequency;
	auto bits_per_ms = 4096 / period_ms;
	auto bits = us * bits_per_ms;
	set_pwm(channel, 0, bits);
}
