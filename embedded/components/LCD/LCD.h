#ifndef LCD_H_
#define LCD_H_

#include "I2Cdev.h"
#include <cstdint>
#include <cstdarg>
#include <ostream>
#include <chrono>
#include <thread>


class LCD : public I2Cdev {
public:
	LCD(const uint8_t bus, uint8_t addr, uint8_t width = 16, bool backlight_on = true);
	virtual ~LCD();
	void setPosition(const uint8_t x, const uint8_t y);
	void putChar(const uint8_t bits);
	void puts (const char *str);
	void clear();
	void goHome();
    void enableBacklight(bool backlight_on=true);
    bool getBacklight();
    void enableCursor(bool enable=true);
    void enableBlinking(bool enable=true);
    LCD&  operator<<(const char *chain) ;
    void scrollDisplayRight(bool right=true);
    void autoScroll(bool enable=true);
    void printf(const char *,...);

private:
    const uint8_t m_lcdRowOffset[4] = {0x80, 0xC0, 0x14, 0x54};
	int16_t m_i2cHandle;
	bool m_backlight_on;
	uint8_t m_BL;
	uint8_t m_displayFunction; // Mode 4/8bits ; #lines ; font
    uint8_t m_displayControl;
    uint8_t m_displayMode;
    
	void init();
	void write4bits(uint8_t value);
    void sendCommand(const uint8_t cmd);
    void sendByte(uint8_t lsb, uint8_t msb);
};

#endif /* LCD_H_ */



