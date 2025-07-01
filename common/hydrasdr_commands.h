/*
 * Copyright (c) 2013-2024 Benjamin Vernoux <bvernoux@hydrasdr.com>
 *
 * This file is part of HydraSDR.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef __HYDRASDR_COMMANDS_H__
#define __HYDRASDR_COMMANDS_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
	RECEIVER_MODE_OFF = 0,
	RECEIVER_MODE_RX = 1
} receiver_mode_t;

// Commands (usb vendor request) shared between Firmware and Host.
#define HYDRASDR_CMD_MAX (28)
typedef enum
{
	HYDRASDR_RESET                      = 0 ,
	HYDRASDR_RECEIVER_MODE              = 1 ,
	HYDRASDR_SI5351C_WRITE              = 2 ,
	HYDRASDR_SI5351C_READ               = 3 ,
	HYDRASDR_R82X_WRITE                 = 4 ,
	HYDRASDR_R82X_READ                  = 5 ,
	HYDRASDR_SPIFLASH_ERASE             = 6 ,
	HYDRASDR_SPIFLASH_WRITE             = 7 ,
	HYDRASDR_SPIFLASH_READ              = 8 ,
	HYDRASDR_BOARD_ID_READ              = 9 ,
	HYDRASDR_VERSION_STRING_READ        = 10,
	HYDRASDR_BOARD_PARTID_SERIALNO_READ = 11,
	HYDRASDR_SET_SAMPLERATE             = 12,
	HYDRASDR_SET_FREQ                   = 13,
	HYDRASDR_SET_LNA_GAIN               = 14,
	HYDRASDR_SET_MIXER_GAIN             = 15,
	HYDRASDR_SET_VGA_GAIN               = 16,
	HYDRASDR_SET_LNA_AGC                = 17,
	HYDRASDR_SET_MIXER_AGC              = 18,
	HYDRASDR_MS_VENDOR_CMD              = 19,
	HYDRASDR_SET_RF_BIAS_CMD            = 20,
	HYDRASDR_GPIO_WRITE                 = 21,
	HYDRASDR_GPIO_READ                  = 22,
	HYDRASDR_GPIODIR_WRITE              = 23,
	HYDRASDR_GPIODIR_READ               = 24,
	HYDRASDR_GET_SAMPLERATES            = 25,
	HYDRASDR_SET_PACKING                = 26,
	HYDRASDR_SPIFLASH_ERASE_SECTOR      = 27,
	HYDRASDR_SET_RF_PORT                = HYDRASDR_CMD_MAX
} hydrasdr_vendor_request;

typedef enum
{
	GPIO_PORT0 = 0,
	GPIO_PORT1 = 1,
	GPIO_PORT2 = 2,
	GPIO_PORT3 = 3,
	GPIO_PORT4 = 4,
	GPIO_PORT5 = 5,
	GPIO_PORT6 = 6,
	GPIO_PORT7 = 7
} hydrasdr_gpio_port_t;

typedef enum
{
	GPIO_PIN0 = 0,
	GPIO_PIN1 = 1,
	GPIO_PIN2 = 2,
	GPIO_PIN3 = 3,
	GPIO_PIN4 = 4,
	GPIO_PIN5 = 5,
	GPIO_PIN6 = 6,
	GPIO_PIN7 = 7,
	GPIO_PIN8 = 8,
	GPIO_PIN9 = 9,
	GPIO_PIN10 = 10,
	GPIO_PIN11 = 11,
	GPIO_PIN12 = 12,
	GPIO_PIN13 = 13,
	GPIO_PIN14 = 14,
	GPIO_PIN15 = 15,
	GPIO_PIN16 = 16,
	GPIO_PIN17 = 17,
	GPIO_PIN18 = 18,
	GPIO_PIN19 = 19,
	GPIO_PIN20 = 20,
	GPIO_PIN21 = 21,
	GPIO_PIN22 = 22,
	GPIO_PIN23 = 23,
	GPIO_PIN24 = 24,
	GPIO_PIN25 = 25,
	GPIO_PIN26 = 26,
	GPIO_PIN27 = 27,
	GPIO_PIN28 = 28,
	GPIO_PIN29 = 29,
	GPIO_PIN30 = 30,
	GPIO_PIN31 = 31
} hydrasdr_gpio_pin_t;

typedef enum
{
	RF_PORT_RX0 = 0, /* RX Channel 0 (default/standard RFIN called ANT) */
	RF_PORT_RX1 = 1, /* RX Channel 1 (called also CABLE1) */
	RF_PORT_RX2 = 2  /* RX Channel 2 (called also CABLE2) */
} hydrasdr_rf_port_t;

#ifdef __cplusplus
} // __cplusplus defined.
#endif

#endif//__HYDRASDR_COMMANDS_H__
