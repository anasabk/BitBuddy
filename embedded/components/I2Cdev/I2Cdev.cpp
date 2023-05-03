#include "I2Cdev.h"


I2Cdev::I2Cdev(int bus, int addr)
{
    fd = i2cOpen(bus, addr, 0);
}

I2Cdev::~I2Cdev()
{
}

/** Read a single bit from an 8-bit device register. 
 * All of the bits in the result other than the one 
 * scanned will be 0.
 * 
 * @param reg_addr 8-bit address of the register in the i2c device
 * @param bit_num Bit position to read (0-7)
 * @param data Container for single bit value.
 * @return Status of read operation (true = success)
 */
bool I2Cdev::read_bit(uint8_t reg_addr, uint8_t bit_num, uint8_t *data) {
	uint8_t b;
    bool sflag = read_byte(reg_addr, &b);

	if(sflag)
    	*data = b & (1 << bit_num);
    
	return sflag ;
}

/** Read multiple bits from an 8-bit device register.
 * @param reg_addr Register reg_addr to read from
 * @param bitStart First bit position to read (0-7)
 * @param length Number of bits to read (not more than 8)
 * @param data Container for right-aligned value (i.e. '101' read from any bitStart position will equal 0x05)
 * @return Status of read operation (true = success)
 */
bool I2Cdev::read_bits(uint8_t reg_addr, uint8_t bitStart, uint8_t length, uint8_t *data) {
    // 01101001 read byte
    // 76543210 bit numbers
    //    xxx   args: bitStart=4, length=3
    //    010   masked
    //   -> 010 shifted
    uint8_t count, b;
    if ((count = read_byte(reg_addr, &b)) != 0) {
        uint8_t mask = ((1 << length) - 1) << (bitStart - length + 1);
        b &= mask;
        b >>= (bitStart - length + 1);
        *data = b;
    }
    return count;
}

/** Read single byte from an 8-bit device register.
 * @param reg_addr Register reg_addr to read from
 * @param data Container for byte value read from device
 * @return Status of read operation (true = success)
 */
bool I2Cdev::read_byte(uint8_t reg_addr, uint8_t *data) {
    return read_bytes(reg_addr, 1, data) > 0;
}

/** Read multiple bytes from an 8-bit device register.
 * @param reg_addr First register reg_addr to read from
 * @param length Number of bytes to read
 * @param data Buffer to store read data in
 * @return Numbers of bytes received.
 */
int8_t I2Cdev::read_bytes(uint8_t reg_addr, uint8_t length, uint8_t *data) {
	return i2cReadI2CBlockData(fd, reg_addr, (char*)data, length);
}

/** write a single bit in an 8-bit device register.
 * @param reg_addr Register reg_addr to write to
 * @param bit_num Bit position to write (0-7)
 * @param value New bit value to write
 * @return Status of operation (true = success)
 */
bool I2Cdev::write_bit(uint8_t reg_addr, uint8_t bit_num, uint8_t data) {
    uint8_t b;
    read_byte(reg_addr, &b);
    b = (data != 0) ? (b | (1 << bit_num)) : (b & ~(1 << bit_num));
    return write_byte(reg_addr, b);
}

/** Write multiple bits in an 8-bit device register.
 * @param reg_addr Register reg_addr to write to
 * @param bitStart First bit position to write (0-7)
 * @param length Number of bits to write (not more than 8)
 * @param data Right-aligned value to write
 * @return Status of operation (true = success)
 */
bool I2Cdev::write_bits(uint8_t reg_addr, uint8_t bitStart, uint8_t length, uint8_t data) {
    //      010 value to write
    // 76543210 bit numbers
    //    xxx   args: bitStart=4, length=3
    // 00011100 mask byte
    // 10101111 original value (sample)
    // 10100011 original & ~mask
    // 10101011 masked | value
    uint8_t b = 0;
    if (read_byte(reg_addr, &b) != 0) {
        uint8_t mask = ((1 << length) - 1) << (bitStart - length + 1);
        data <<= (bitStart - length + 1); // shift data into correct position
        data &= mask; // zero all non-important bits in data
        b &= ~(mask); // zero all important bits in existing byte
        b |= data; // combine data with existing byte
        return write_byte(reg_addr, b);
    } else {
        return false;
    }
}

/** Write single byte to an 8-bit device register.
 * @param reg_addr Register address to write to
 * @param data New byte value to write
 * @return Status of operation (true = success)
 */
bool I2Cdev::write_byte(uint8_t reg_addr, uint8_t data) {
	i2cWriteByteData(fd, reg_addr, data);
	return true;
}

/** Write single byte to an 8-bit device register.
 * @param reg_addr Register address to write to
 * @param length Number of bytes to write
 * @param data Array of bytes to write
 * @return Status of operation (true = success)
 */
bool I2Cdev::write_bytes(uint8_t reg_addr, uint8_t length, uint8_t *data){
    i2cWriteBlockData(fd, reg_addr, (char*)data, length);
	return true;
}
