#include <i2c_Helper.h>

// static unsigned char i2cError = 0;
i2cStatus CheckI2C()
{
    return (i2cStatus)i2cError;
}

//*************************************************************************************************************************
//*************************************************************************************************************************

// TODO create function overloads that takes a TwoWire object as a parameter to enable non-default bus usage (change existing functions to call to new overloaded ones with default bus)

// used to read 1,2,and 4 bytes: i2c_read(starting register,number of bytes to read)
sensor_mem_handler i2c_read(const unsigned char reg, const unsigned char number_of_bytes_to_read, const unsigned char bus_address)
{
    sensor_mem_handler temp;
    Wire.beginTransmission(bus_address); // call the device by its ID number
    Wire.write(reg);                     // transmit the register that we will start from
    i2cError = Wire.endTransmission();   // end the I2C data transmission

    Wire.requestFrom(bus_address, number_of_bytes_to_read); // call the device and request to read X bytes
    for (int i = number_of_bytes_to_read; i > 0; i--)
    {
        temp.i2c_data[i - 1] = Wire.read();
    } // with this code we read multiple bytes in reverse
    return temp;
}

// Read Single Byte
int i2c_read(const unsigned char reg, const unsigned char bus_address)
{
    Wire.beginTransmission(bus_address); // call the device by its ID number
    Wire.write(reg);                     // transmit the register that we will start from
    i2cError = Wire.endTransmission();   // end the I2C data transmission

    Wire.requestFrom(bus_address, (uint8_t)1); // call the device and request to read X bytes
    return Wire.read();
}
//*************************************************************************************************************************
//*************************************************************************************************************************

uint8_t i2c_write_byte(const unsigned char reg, const unsigned char data, const unsigned char bus_address)
{                                        // used to write a single byte to a register: i2c_write_byte(register to write to, byte data)
    Wire.beginTransmission(bus_address); // call the device by its ID number
    Wire.write(reg);                     // transmit the register that we will start from
    Wire.write(data);                    // write the byte to be written to the register
    i2cError = Wire.endTransmission();   // end the I2C data transmission
    return i2cError;
}

//*************************************************************************************************************************
//*************************************************************************************************************************

// used to write a 2 bytes to a register: i2c_write_long(register to start at, long data )
uint8_t i2c_write_int(const unsigned char reg, const uint16_t data, const unsigned char bus_address)
{
    sensor_mem_handler temp;
    temp.asInt = data;

    Wire.beginTransmission(bus_address); // call the device by its ID number
    Wire.write(reg);                     // transmit the register that we will start from
    for (int i = 2; i > 0; i--)
    { // with this code we write multiple bytes in reverse
        Wire.write(temp.i2c_data[i - 1]);
    }
    i2cError = Wire.endTransmission(); // end the I2C data transmission
    return i2cError;
}

//*************************************************************************************************************************
//*************************************************************************************************************************

// used to write a 4 bytes to a register: i2c_write_long(register to start at, long data )
uint8_t i2c_write_long(const unsigned char reg, const uint32_t data, const unsigned char bus_address)
{
    sensor_mem_handler temp;
    temp.asLong = data;

    Wire.beginTransmission(bus_address); // call the device by its ID number
    Wire.write(reg);                     // transmit the register that we will start from
    for (int i = 4; i > 0; i--)
    { // with this code we write multiple bytes in reverse
        Wire.write(temp.i2c_data[i - 1]);
    }
    i2cError = Wire.endTransmission(); // end the I2C data transmission
    return i2cError;
}

enum Endianness
{
    Big,
    Little
};

constexpr Endianness GetEndianess()
{
#ifdef __BYTE_ORDER__
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return Little;
#else
    return Big;
#endif
#else
#ifdef __AVR__
    return Little;
#else
    uint16_t TestValue = 1;
    return static_cast<Endianness>(reinterpret_cast<uint8_t *>(&TestValue)[0] == TestValue);
#endif
#endif
}

template <typename T>
union DataConverter
{
    T value;
    uint8_t bytes[sizeof(T)];
};

template <typename T>
i2cStatus i2c_write(const uint8_t reg, const T data, const uint8_t bus_address, const Endianness device_endianess)
{
    static_assert(sizeof(T) > 0, "Data type cannot have zero size for I2C transfer.");
    DataConverter<T> converter;
    converter.value = data;

    Wire.beginTransmission(bus_address);
    Wire.write(reg);
    if (device_endianess == GetEndianess())
    {
        for (size_t i = 0; i < sizeof(T); i++)
        {
            Wire.write(converter.bytes[i]);
        }
    }
    else
    {
        for (int i = sizeof(T) - 1; i >= 0; i--)
        {
            Wire.write(converter.bytes[i]);
        }
    }
    return Wire.endTransmission();
}

template <typename T>
i2cStatus i2c_read(const uint8_t reg, T &data, const uint8_t bus_address, const Endianness device_endianess)
{
    static_assert(sizeof(T) > 0, "Data type cannot have zero size for I2C transfer.");
    Wire.beginTransmission(bus_address);
    Wire.write(reg);
    auto i2cError = Wire.endTransmission();
    if (i2cError != 0)
    {
        return i2cError;
    }

    Wire.requestFrom(bus_address, sizeof(T));
    if (Wire.available() != sizeof(T))
    {
        return i2cStatus::timeout;
    }

    DataConverter<T> converter;
    if (device_endianess == GetEndianess())
    {
        for (size_t i = 0; i < sizeof(T); i++)
        {
            converter.bytes[i] = Wire.read();
        }
    }
    else
    {
        for (int i = sizeof(T) - 1; i >= 0; i--)
        {
            converter.bytes[i] = Wire.read();
        }
    }
    data = converter.value;
    return i2cStatus::success;
}