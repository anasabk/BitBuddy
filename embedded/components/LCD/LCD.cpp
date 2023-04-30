#include "LCD.h"
#include "pigpio.h"
#include <cstdint>
#include <cstdarg>
#include <chrono>
#include <thread>


#define LCD_CMD_CLEAR          0x01
#define LCD_CMD_RETURNHOME     0x02
#define LCD_CMD_ENTRYMODESET   0x04
#define LCD_CMD_DISPLAYCONTROL 0x08
#define LCD_CMD_CURSORSHIFT    0x10
#define LCD_CMD_FUNCTIONSET    0x20

#define LCD_CTRL_8BITMODE   0x10
#define LCD_CTRL_2LINE      0x08
#define LCD_CTRL_5x10DOTS   0x04

// flags for display on/off control
#define LCD_DISPLAYON   0x04
#define LCD_CURSORON    0x02
#define LCD_BLINKON     0x01

// flags for display entry mode
#define LCD_ENTRYLEFT           0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01

// flags for display/cursor shift
#define LCD_DISPLAYMOVE     0x08
#define LCD_MOVERIGHT       0x04

// flags for backlight control
#define LCD_BACKLIGHT 0x08


enum I2C_EXPANDER {
    _RS = 0x1,
    _EN = 0x4,
    _B4 = 4
};


LCD::LCD(const uint8_t bus, uint8_t addr, uint8_t width, bool backlight_on) {
    m_i2cHandle = i2cOpen(bus, addr, 0);
    m_backlight_on = backlight_on;
    m_displayFunction = LCD_CMD_FUNCTIONSET | LCD_CTRL_2LINE; // | LCD_4BITMODE;// | LCD_5x8DOTS;
    m_displayControl = LCD_CMD_DISPLAYCONTROL | LCD_DISPLAYON;
    m_displayMode = LCD_CMD_ENTRYMODESET | LCD_ENTRYLEFT;

    init();
    clear();
}

LCD::~LCD() {
    i2cClose(m_i2cHandle);
}

/*
 * Initialise the LCD screen
 */
void LCD::init() const {
    write4bits(0x3 << 4);
    std::this_thread::sleep_for(std::chrono::microseconds(4500));
    write4bits(0x3 << 4);
    std::this_thread::sleep_for(std::chrono::microseconds(4500));
    write4bits(0x3 << 4);
    std::this_thread::sleep_for(std::chrono::microseconds(150));
    write4bits(0x2 << 4);

    sendCommand(m_displayFunction);
    sendCommand(m_displayControl);
    sendCommand(m_displayMode);
    
    goHome();
}

void LCD::sendCommand(const uint8_t cmd) const {
    uint8_t MSN = (cmd >> 4) & 0x0F;
    uint8_t LSN = cmd & 0x0F;
    uint8_t MSb = MSN << _B4;
    uint8_t LSb = LSN << _B4;
    sendByte(MSb, LSb);
}

/*
 * Display a char
 */
void LCD::putChar(const uint8_t bits) const {
    uint8_t MSN = (bits >> 4) & 0x0F;
    uint8_t LSN = bits & 0x0F;
    uint8_t MSb = (MSN << _B4) | _RS;
    uint8_t LSb = (LSN << _B4) | _RS;
    sendByte(MSb, LSb);
}

void LCD::sendByte(uint8_t msb, uint8_t lsb) const {
    if (m_backlight_on) {
        lsb = lsb | LCD_BACKLIGHT;
        msb = msb | LCD_BACKLIGHT;
    }
    write4bits(msb);
    write4bits(lsb);
}

void LCD::write4bits(uint8_t value) const {
    i2cWriteByte(m_i2cHandle, value | _EN);
    std::this_thread::sleep_for(std::chrono::microseconds(1));
    i2cWriteByte(m_i2cHandle, value & ~_EN);
    std::this_thread::sleep_for(std::chrono::microseconds(50));
}

/*
 * Move the cursor to a position
 */
void LCD::setPosition(const uint8_t x, const uint8_t y) const {
    sendCommand(m_lcdRowOffset[y] + x);
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
}

/*
 * Clear the LCD screen
 */
void LCD::clear() const {
    sendCommand(LCD_CMD_CLEAR);
    std::this_thread::sleep_for(std::chrono::milliseconds(2));

}

/*
 * Write a string
 */
void LCD::puts(const char *str) {
    while (*str)
        putChar((uint8_t) *(str++));
}

/*
 * Set the cursor to position 0,0
 */
void LCD::goHome() const {
    sendCommand(LCD_CMD_RETURNHOME);
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
}

/*
 * Enable/Disable backlight
 */
void LCD::enableBacklight(bool backlight_on) {
    m_backlight_on=backlight_on;
    i2cWriteByte(m_i2cHandle, m_backlight_on ? LCD_BACKLIGHT : 0);
}

/*
 * Return the status of backlight
 */
bool LCD::getBacklight() const {
    return m_backlight_on;
}

/*
 * Enable/Disable cursor
 */
void LCD::enableCursor(bool enable) {
    m_displayControl= enable ? (m_displayControl | LCD_CURSORON) : (m_displayControl & ~LCD_CURSORON);
    sendCommand(m_displayControl);
}

/*
 * Enable/Disable cursor blinking
 */
void LCD::enableBlinking(bool enable) {
    m_displayControl= enable ? (m_displayControl | LCD_BLINKON) : (m_displayControl & ~LCD_BLINKON);
    sendCommand(m_displayControl);
}

/*
 * Display a C string using operator <<
 * TODO : accept formatting operations like cout<<
 */
LCD&  LCD::operator<<(const char *chaine)  {
    puts(chaine);
    return *this;
}

/*
 * Scroll display to the left or to the right
 */
void LCD::scrollDisplayRight(bool right) {
    sendCommand(LCD_CMD_CURSORSHIFT | LCD_DISPLAYMOVE | (right ? LCD_MOVERIGHT : 0));
}

/*
 * Enable/Disable autoscroll
 */
void LCD::autoScroll(bool enable){
    m_displayMode =enable ? (m_displayMode | LCD_ENTRYSHIFTINCREMENT) : (m_displayMode & ~LCD_ENTRYSHIFTINCREMENT);
    sendCommand(m_displayMode);
}
/*
 * A C-printf allowing to print a formatted string
 */
void LCD::printf(const char *fmt,...) {
    char buffer[256];
    va_list args;
    va_start (args, fmt);
    vsprintf (buffer,fmt, args);
    puts(buffer);
    va_end (args);
}