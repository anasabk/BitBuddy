#include "I2Cdev.h"


I2Cdev::I2Cdev(int bus, int addr)
{
    // fd = i2cOpen(bus, addr, 0);
    
    char filename[20];
    snprintf(filename, 19, "/dev/i2c-%d", bus);
    fd = open(filename, O_RDWR);
    if (fd < 0) {
        /* ERROR HANDLING; you can check errno to see what went wrong */
        // perror("Opening i2c file")
        exit(1);
    }

    if (ioctl(fd, I2C_SLAVE, addr) < 0) {
        /* ERROR HANDLING; you can check errno to see what went wrong */
        // perror("Setting i2c address");
        exit(1);
    }
}

I2Cdev::~I2Cdev()
{
    close(fd);
}

int I2Cdev::read_bit(uint8_t reg_addr, uint8_t bit_num, uint8_t *data) {
	uint8_t b;
    int count = read_byte(reg_addr, &b);

	if(count)
    	*data = b & (1 << bit_num);
    
	return count ;
}

int I2Cdev::read_bits(uint8_t reg_addr, uint8_t bitStart, uint8_t length, uint8_t *data) {
    /** 
     * 01101001: The byte to be read.
     * 76543210: bit numbers.
     *    xxx  : args: bitStart=4, length=3.
     *    010  : masked to get only the required bits.
     * -> 010  : shifted to the left to get the value.
     * 00000010: The result.
     */

    uint8_t count, b;
    if ((count = read_byte(reg_addr, &b)) != 0) {
        uint8_t mask = ((1 << length) - 1) << (bitStart - length + 1);
        b &= mask;
        b >>= (bitStart - length + 1);
        *data = b;
    }
    return count;
}

int I2Cdev::read_byte(uint8_t reg_addr, uint8_t *data) {
    return read_bytes(reg_addr, 1, data) > 0;
}

int I2Cdev::read_bytes(uint8_t reg_addr, uint8_t length, uint8_t *data) {
	// return i2cReadI2CBlockData(fd, reg_addr, (char*)data, length);
    return i2c_smbus_read_i2c_block_data(fd, reg_addr, length, data);
}

int I2Cdev::write_bit(uint8_t reg_addr, uint8_t bit_num, uint8_t data) {
    uint8_t b;
    read_byte(reg_addr, &b);
    b = (data != 0) ? (b | (1 << bit_num)) : (b & ~(1 << bit_num));
    return write_byte(reg_addr, b);
}

int I2Cdev::write_bits(uint8_t reg_addr, uint8_t bitStart, uint8_t length, uint8_t data) {
    /**
     *      010 value to write
     * 76543210 bit numbers
     *    xxx   args: bitStart=4, length=3
     * 00011100 mask byte
     * 10101111 original value (sample)
     * 10100011 original & ~mask
     * 10101011 masked | value
     */
    
    uint8_t count, b = 0;
    if ((count = read_byte(reg_addr, &b)) != 0) {
        // Contains 1's in the place of the bits that will be written.
        uint8_t mask = ((1 << length) - 1) << (bitStart - length + 1); 
        data <<= (bitStart - length + 1);   // shift data into correct position
        data &= mask;                       // zero all non-important bits in data
        b &= ~(mask);                       // zero all important bits in existing byte
        b |= data;                          // combine data with existing byte
        return write_byte(reg_addr, b);
    } else {
        return count;
    }
}

int I2Cdev::write_byte(uint8_t reg_addr, uint8_t data) {
	// i2cWriteByteData(fd, reg_addr, data);
    return i2c_smbus_write_byte_data(fd, reg_addr, data);
}

int I2Cdev::write_byte(uint8_t data) {
	// i2cWriteByteData(fd, reg_addr, data);
    return i2c_smbus_write_byte(fd, data);
}

int I2Cdev::write_bytes(uint8_t reg_addr, uint8_t length, uint8_t *data){
    // i2cWriteBlockData(fd, reg_addr, (char*)data, length);
    return i2c_smbus_write_block_data(fd, reg_addr, length, data);
}
