/*
 * srf08-ultrasonic-sensor.c - Driver for the SRF08 ultrasonic ranger and the LPC2387 chip via the i2c interface.
 * Copyright (C) 2013 Zakaria Kasmi <zkasmi@inf.fu-berlin.de>
 *
 * This source code is licensed under the  LGPLv2 license, See the file LICENSE for more details.
 */

/**
 * @file
 * @internal
 * @brief	Driver for the SRF08 ultrasonic ranger and the LPC2387 chip using the i2c interface.
 *
 * @author      Freie Universität Berlin, Computer Systems & Telematics
 * @author	Zakaria Kasmi <zkasmi@inf.fu-berlin.de>
 * @version     $Revision: 3854 $
 *
 * @note	$Id: srf08-ultrasonic-sensor.c 3854 2013-06-21 15:30:01Z zkasmi $
 *
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "hwtimer.h"
#include "srf08-ultrasonic-sensor.h"
#include "i2c.h"


bool srf08_init(uint8_t i2c_interface, uint32_t baud_rate) {
	if (i2c_initialize(i2c_interface, (uint32_t) I2CMASTER, 0, baud_rate, NULL)
			== false) /* initialize I2C */
			{
		puts("fatal error! happened in i2cInitialize() \n");
		while (1)
			; /* Fatal error */
	} else {
		i2c_enable_pull_up_resistor(i2c_interface);
		//i2c_disable_pull_up_resistor(i2c_interface);
		return true;
	}

}


void srf08_set_range(uint8_t max_range) {
	uint8_t tx_buff[1];
	tx_buff[0] = max_range;
	i2c_write(SRF08_I2C_INTERFACE, SRF08_DEFAULT_ADDR, SRF08_RANGE_REG, tx_buff,
			1);
}

void srf08_set_gain(uint8_t gain) {
	uint8_t tx_buff[1];
	tx_buff[0] = gain;
	i2c_write(SRF08_I2C_INTERFACE, SRF08_DEFAULT_ADDR, SRF08_GAIN_REG, tx_buff,
			1);
}

uint8_t srf08_get_range(void) {
	uint8_t rx_buff[1];
	i2c_read(SRF08_I2C_INTERFACE, SRF08_DEFAULT_ADDR, SRF08_RANGE_REG, rx_buff,
			1);

	return rx_buff[0];
}

uint8_t srf08_get_gain(void) {
	uint8_t rx_buff[1];
	i2c_read(SRF08_I2C_INTERFACE, SRF08_DEFAULT_ADDR, SRF08_GAIN_REG, rx_buff,
			1);

	return rx_buff[0];
}

void srf08_start_ranging(void) {
	puts("Ultrasonic SRF08 engine is started");
	bool status = false;
	uint8_t reg_size = 1;
	uint8_t range_high_byte = 0;
	uint8_t range_low_byte = 0;
	uint32_t distance = 0;
	uint8_t rx_buff[reg_size];
	uint8_t tx_buff[reg_size];
	tx_buff[0] = SRF08_REAL_RANGING_MODE_CM;
	uint8_t register_location;

	//wait due to calibration
	hwtimer_wait(HWTIMER_TICKS(700000));
	printf("Actual range = %d\n", srf08_get_range());
	printf("Actual gain = %d\n", srf08_get_gain());


	while (1) {

		status = i2c_write(SRF08_I2C_INTERFACE, SRF08_DEFAULT_ADDR,
				SRF08_COMMAND_REG, tx_buff, reg_size);
		if (!status) {
			puts("Write the ranging command to the i2c-interface is failed");
			break;
		}

		hwtimer_wait(HWTIMER_TICKS(70000));

		// Read all echo buffers
		for (register_location = 2; register_location < MAX_REGISTER_NUMBER;
				register_location += 2) {

			//read the high echo byte
			status = i2c_read(SRF08_I2C_INTERFACE, SRF08_DEFAULT_ADDR,
					register_location, rx_buff, reg_size);
			if (!status) {
				puts(
						"Read the the high echo byte from the i2c-interface is failed");
				break;
			}
			range_high_byte = rx_buff[0];

			//read the low echo byte
			status = i2c_read(SRF08_I2C_INTERFACE, SRF08_DEFAULT_ADDR,
					register_location + 1, rx_buff, reg_size);
			if (!status) {
				puts(
						"Read the the low echo byte from the i2c-interface is failed");
				break;
			}

			range_low_byte = rx_buff[0];

			if ((range_high_byte == 0) && (range_low_byte == 0)) {
				break;
			} else {
				distance = (range_high_byte << 8) | range_low_byte;
				printf("distance = %lu cm , echo%d\n", distance, register_location/2);
			}
			hwtimer_wait(HWTIMER_TICKS(500000));
		}
	}

}

