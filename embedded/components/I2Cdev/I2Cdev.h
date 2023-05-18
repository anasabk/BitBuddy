#ifndef I2CDEV_H
#define I2CDEV_H


// #include "pigpio.h"
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string>
#include <unistd.h>


class I2Cdev
{
public:
    I2Cdev(int bus, int addr);
    ~I2Cdev();

    bool read_bit(uint8_t regAddr, uint8_t bitNum, uint8_t *data);
    bool read_bits(uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t *data);
    bool read_byte(uint8_t regAddr, uint8_t *data);
    int8_t read_bytes(uint8_t regAddr, uint8_t length, uint8_t *data);

    bool write_bit(uint8_t regAddr, uint8_t bitNum, uint8_t data);
    bool write_bits(uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t data);
    bool write_byte(uint8_t regAddr, uint8_t data);
    bool write_bytes(uint8_t regAddr, uint8_t length, uint8_t *data);

protected:
    int fd;
};


#endif
