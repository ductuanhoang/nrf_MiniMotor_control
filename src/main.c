/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <zephyr.h>
#include <sys/printk.h>
#include <device.h>
#include <drivers/i2c.h>
#include "mini_motor/SparkFunMiniMoto.h"
/**
 * @file Sample app using the Fujitsu MB85RC256V FRAM through I2C.
 */


void main(void)
{
	printk("Hello World! %s\n", CONFIG_BOARD);
	mini_motor_init();
	miniMotor_drive(100);
}
