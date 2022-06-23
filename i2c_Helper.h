#ifndef i2c_Helper_H
#define i2c_Helper_H

#include <Wire.h>
#include <stdint.h>

enum i2cStatus
{
    success,
    BufferOverflow, // 1 .. length to long for buffer
    addressNACK,    // 2 .. address send, NACK received
    dataNACK,       // 3 .. data send, NACK received
    other,          // 4 .. other twi error (lost bus arbitration, bus error, ..)
    timeout,
};
static unsigned char i2cError = 0;
i2cStatus CheckI2C();

union sensor_mem_handler // declare the use of a union data type
{
    uint8_t i2c_data[4]; // define a 4 byte array in the union
    uint8_t asByte;
    uint16_t asInt;
    uint32_t asLong = 0; // define an long in the union
};

sensor_mem_handler i2c_read(const unsigned char reg, const unsigned char number_of_bytes_to_read, const unsigned char bus_address);
int i2c_read(const unsigned char reg, const unsigned char bus_address);
uint8_t i2c_write_byte(const unsigned char reg, const unsigned char data, const unsigned char bus_address);
uint8_t i2c_write_long(const unsigned char reg, const unsigned long data, const unsigned char bus_address);

#endif