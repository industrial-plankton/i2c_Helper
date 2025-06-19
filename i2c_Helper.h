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
uint8_t i2c_write_int(const unsigned char reg, const uint16_t data, const unsigned char bus_address);
uint8_t i2c_write_long(const unsigned char reg, const uint32_t data, const unsigned char bus_address);

namespace i2c
{
    enum class Status
    {
        success,
        BufferOverflow, // 1 .. length to long for buffer
        addressNACK,    // 2 .. address send, NACK received
        dataNACK,       // 3 .. data send, NACK received
        other,          // 4 .. other twi error (lost bus arbitration, bus error, ..)
        timeout,
    };

    enum class Endianness
    {
        Big,
        Little
    };

    namespace i2c_internal
    {
        constexpr Endianness GetEndianess()
        {
#ifdef __BYTE_ORDER__
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
            return Endianness::Little;
#else
            return Endianness::Big;
#endif
#else
#ifdef __AVR__
            return Endianness::Little;
#else
            uint16_t TestValue = 1;
            return static_cast<Endianness>(reinterpret_cast<uint8_t *>(&TestValue)[0] == 1);
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
        Status write_internal(const uint8_t reg, const T *data_ptr, const size_t numElements, const uint8_t bus_address, const Endianness device_endianess)
        {
            static_assert(sizeof(T) > 0, "Data type cannot have zero size for I2C transfer.");

            Wire.beginTransmission(bus_address);
            Wire.write(reg);

            DataConverter<T> converter;
            for (size_t j = 0; j < numElements; j++)
            {
                converter.value = data_ptr[j]; // Assign the current element's value

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
            }
            return (i2c::Status)Wire.endTransmission();
        }

        template <typename T>
        Status read_internal(const uint8_t reg, T *data_ptr, const size_t numElements, const uint8_t bus_address, const Endianness device_endianess)
        {
            static_assert(sizeof(T) > 0, "Data type cannot have zero size for I2C transfer.");
            Wire.beginTransmission(bus_address);
            Wire.write(reg);
            auto i2cError = Wire.endTransmission();
            if (i2cError != 0)
            {
                return (i2c::Status)i2cError;
            }

            Wire.requestFrom(bus_address, sizeof(T) * numElements);
            if (Wire.available() != sizeof(T) * numElements)
            {
                return Status::timeout;
            }

            DataConverter<T> converter;
            for (size_t j = 0; j < numElements; j++)
            {
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
                data_ptr[j] = converter.value;
            }

            return Status::success;
        }
    }

    // Careful passing literals to this function, they usually default to int, and you might want to use a char
    template <typename T>
    Status read(const uint8_t reg, T &data, const uint8_t bus_address, const Endianness device_endianess)
    {
        return i2c_internal::read_internal(reg, &data, 1, bus_address, device_endianess);
    }

    template <typename T, size_t N>
    Status read(const uint8_t reg, T (&data)[N], const uint8_t bus_address, const Endianness device_endianess)
    {
        static_assert(N > 0, "Array size cannot be zero for I2C transfer.");
        return i2c_internal::read_internal(reg, data, N, bus_address, device_endianess);
    }

    // Careful passing literals to this function, they usually default to int, and you might want to use a char
    template <typename T>
    Status write(const uint8_t reg, const T data, const uint8_t bus_address, const Endianness device_endianess)
    {
        // Pass the address of the single data item
        return i2c_internal::write_internal(reg, &data, 1, bus_address, device_endianess);
    }

    template <typename T, size_t N>
    Status write(const uint8_t reg, const T (&data)[N], const uint8_t bus_address, const Endianness device_endianess)
    {
        static_assert(N > 0, "Array size cannot be zero for I2C transfer.");
        // Arrays decay to pointers to their first element when passed as arguments
        return i2c_internal::write_internal(reg, data, N, bus_address, device_endianess);
    }
}

#endif