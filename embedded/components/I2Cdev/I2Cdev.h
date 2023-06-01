#ifndef I2CDEV_H
#define I2CDEV_H


// #include "pigpio.h"
#include <string>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

extern "C" {
    #include <linux/i2c-dev.h>
    #include <i2c/smbus.h>
}

class I2Cdev
{
public:
    I2Cdev(int bus, int addr);
    ~I2Cdev();

    /**
     * @brief Read a single bit from an 8-bit device register. 
     * All of the bits in the result other than the one 
     * scanned will be 0.
     * 
     * @param reg_addr 8-bit address of the register in the i2c device.
     * @param bit_num Bit position to read (0-7).
     * @param data Container for single bit value.
     * @return Status of read operation (true = success).
     * 
     * @note Bit numbers: 76543210
     */
    int read_bit(uint8_t regAddr, uint8_t bitNum, uint8_t *data);

    /**  
     * @brief Read multiple bits from an 8-bit device register.
     * 
     * @param reg_addr Register reg_addr to read from.
     * @param bitStart First bit position to read (0-7).
     * @param length Number of bits to read (not more than 8).
     * @param data Container for right-aligned value (i.e. '101' read from any bitStart position will equal 0x05).
     * @return Status of read operation (true = success).
     * 
     * @note Bit numbers: 76543210
     */
    int read_bits(uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t *data);

    /**
     * @brief Read single byte from an 8-bit device register.
     * 
     * @param reg_addr Register reg_addr to read from.
     * @param data Container for byte value read from device.
     * @return Status of read operation (true = success).
     */
    int read_byte(uint8_t regAddr, uint8_t *data);

    /**
     * @brief Read multiple bytes from an 8-bit device register.
     * 
     * @param reg_addr First register reg_addr to read from.
     * @param length Number of bytes to read.
     * @param data Buffer to store read data in.
     * @return Numbers of bytes received.
     */
    int read_bytes(uint8_t regAddr, uint8_t length, uint8_t *data);

    /** 
     * @brief write a single bit in an 8-bit device register.
     * 
     * @param reg_addr Register reg_addr to write to.
     * @param bit_num Bit position to write (0-7).
     * @param value New bit value to write.
     * @return Status of operation (true = success).
     * 
     * @note Bit numbers: 76543210
     */
    int write_bit(uint8_t regAddr, uint8_t bitNum, uint8_t data);

    /**
     * @brief Write multiple bits in an 8-bit device register.
     * 
     * @param reg_addr Register reg_addr to write to.
     * @param bitStart First bit position to write (0-7).
     * @param length Number of bits to write (not more than 8).
     * @param data Right-aligned value to write.
     * @return Status of operation (true = success).
     * 
     * @note Bit numbers: 76543210
     */
    int write_bits(uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t data);

    /** 
     * @brief Write single byte directly to the device.
     * 
     * @param data New byte value to write.
     * @return Status of operation (true = success).
     */
    int write_byte(uint8_t data);

    /** 
     * @brief Write single byte to an 8-bit device register.
     * 
     * @param reg_addr Register address to write to.
     * @param data New byte value to write.
     * @return Status of operation (true = success).
     */
    int write_byte(uint8_t regAddr, uint8_t data);

    /** 
     * @brief Write single byte to an 8-bit device register.
     * 
     * @param reg_addr Register address to write to.
     * @param length Number of bytes to write.
     * @param data Array of bytes to write.
     * @return Status of operation (true = success).
     */
    int write_bytes(uint8_t regAddr, uint8_t length, uint8_t *data);


protected:
    
    int fd; // The file descriptor of the I2C bus.
};


#endif