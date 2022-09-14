
/**
 * @file SparkFunMiniMoto.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2022-09-13
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "SparkFunMiniMoto.h"
#include <errno.h>
#include <zephyr.h>
#include <sys/printk.h>
#include <device.h>
#include <drivers/i2c.h>

#define DRIVER_MOTOR_I2C_ADDR 0x50

static const struct device *i2c_dev;
static int I2CWriteBytes(const struct device *i2c_dev, uint16_t addr,
                         uint8_t *data, uint32_t num_bytes);
static int I2CReadBytes(const struct device *i2c_dev, uint16_t addr,
                        uint8_t *data, uint32_t num_bytes);

int mini_motor_init(void)
{
    i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c0));
    if (!device_is_ready(i2c_dev))
    {
        printk("I2C: Device is not ready.\n");
        return -1;
    }

    return 0;
}

// Return the fault status of the DRV8830 chip. Also clears any existing faults.
uint8_t getFault()
{
    uint8_t buffer = 0;
    uint8_t clearFault = 0x80;
    I2CReadBytes(i2c_dev, 0x01, &buffer, 1);
    I2CWriteBytes(i2c_dev, 0x01, &clearFault, 1);
    return buffer;
}

// Send the drive command over I2C to the DRV8830 chip. Bits 7:2 are the speed
//  setting; range is 0-63. Bits 1:0 are the mode setting:
//  - 00 = HI-Z
//  - 01 = Reverse
//  - 10 = Forward
//  - 11 = H-H (brake)
void miniMotor_drive(int speed)
{
    uint8_t regValue = 0x80; // Before we do anything, we'll want to
    //  clear the fault status. To do that
    //  write 0x80 to register 0x01 on the
    //  DRV8830.
    I2CWriteBytes(i2c_dev, 0x01, &regValue, 1); // Clear the fault status.
    regValue = (uint8_t)abs(speed);             // Find the byte-ish abs value of the input
    if (regValue > 63)
    {
        regValue = 63; // Cap the value at 63.
    }
    regValue = regValue << 2; // Left shift to make room for bits 1:0
    if (speed < 0)
    {
        regValue |= 0x01; // Set bits 1:0 based on sign of input.
    }
    else
    {
        regValue |= 0x02;
    }

    I2CWriteBytes(i2c_dev, 0x00, &regValue, 1);
}

// Coast to a stop by hi-z'ing the drivers.
void miniMotor_stop()
{
    uint8_t regValue = 0; // See above for bit 1:0 explanation.

    I2CWriteBytes(i2c_dev, 0x00, &regValue, 1);
}

// Stop the motor by providing a heavy load on it.
void miniMotor_brake()
{
    uint8_t regValue = 0x03; // See above for bit 1:0 explanation.

    I2CWriteBytes(i2c_dev, 0x00, &regValue, 1);
}

/**
 * @brief
 *
 * @param i2c_dev
 * @param addr
 * @param data
 * @param num_bytes
 * @return int
 */
static int I2CReadBytes(const struct device *i2c_dev, uint16_t addr,
                        uint8_t *data, uint32_t num_bytes)
{
    uint8_t wr_addr[2];
    struct i2c_msg msgs[2];

    /* Now try to read back from FRAM */

    /* FRAM address */
    wr_addr[0] = (addr >> 8) & 0xFF;
    wr_addr[1] = addr & 0xFF;

    /* Setup I2C messages */

    /* Send the address to read from */
    msgs[0].buf = wr_addr;
    msgs[0].len = 2U;
    msgs[0].flags = I2C_MSG_WRITE;

    /* Read from device. STOP after this. */
    msgs[1].buf = data;
    msgs[1].len = num_bytes;
    msgs[1].flags = I2C_MSG_READ | I2C_MSG_STOP;

    return i2c_write(i2c_dev, &msgs[0], 2, DRIVER_MOTOR_I2C_ADDR);
}

/**
 * @brief
 *
 * @param i2c_dev
 * @param addr
 * @param data
 * @param num_bytes
 * @return int
 */
static int I2CWriteBytes(const struct device *i2c_dev, uint16_t addr,
                         uint8_t *data, uint32_t num_bytes)
{
    uint8_t wr_addr[2];
    struct i2c_msg msgs[2];

    /* FRAM address */
    wr_addr[0] = (addr >> 8) & 0xFF;
    wr_addr[1] = addr & 0xFF;

    /* Setup I2C messages */

    /* Send the address to write to */
    msgs[0].buf = wr_addr;
    msgs[0].len = 2U;
    msgs[0].flags = I2C_MSG_WRITE;

    /* Data to be written, and STOP after this. */
    msgs[1].buf = data;
    msgs[1].len = num_bytes;
    msgs[1].flags = I2C_MSG_WRITE | I2C_MSG_STOP;

    return i2c_write(i2c_dev, &msgs[0], 2, DRIVER_MOTOR_I2C_ADDR);
}