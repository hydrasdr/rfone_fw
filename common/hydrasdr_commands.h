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
#define HYDRASDR_EXT_CMD_BASE (0x80)
#define HYDRASDR_EXT_CMD_MAX (0x8A)
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
	HYDRASDR_SET_RF_PORT                = HYDRASDR_CMD_MAX,
	/* 29..33 are taken by hydrasdr-host v1.1 (GET_CAPABILITIES, SET_BANDWIDTH, GET_BANDWIDTHS,
	   GET_TEMPERATURE, SET_GAIN) and not implemented here: they stall, the host then falls back */
	/* Stream metadata, timing and debug commands, kept clear of the upstream range */
	HYDRASDR_GET_STREAM_STATUS          = HYDRASDR_EXT_CMD_BASE + 0,
	HYDRASDR_SET_FRAMING                = HYDRASDR_EXT_CMD_BASE + 1,
	HYDRASDR_WATCHDOG                   = HYDRASDR_EXT_CMD_BASE + 2,
	HYDRASDR_SET_UART_BAUD              = HYDRASDR_EXT_CMD_BASE + 3, /* wValue = baud & 0xFFFF, wIndex = baud >> 16 */
	HYDRASDR_UART_WRITE                 = HYDRASDR_EXT_CMD_BASE + 4, /* OUT data = bytes to transmit (max 64) */
	HYDRASDR_SET_CALIBRATION            = HYDRASDR_EXT_CMD_BASE + 5, /* wValue|wIndex<<16 = crystal correction in ppb (int32), applied at once */
	HYDRASDR_GET_CALIBRATION            = HYDRASDR_EXT_CMD_BASE + 6, /* IN: hydrasdr_calibration_t */
	/* Debug access, see hydrasdr_debug: */
	HYDRASDR_MEM_READ                   = HYDRASDR_EXT_CMD_BASE + 7, /* IN: wValue | wIndex << 16 = address, wLength <= 64 bytes */
	HYDRASDR_MEM_WRITE                  = HYDRASDR_EXT_CMD_BASE + 8, /* OUT: same addressing, data = bytes to write */
	HYDRASDR_CALL                       = HYDRASDR_EXT_CMD_BASE + 9, /* OUT: hydrasdr_call_request_t runs a function; IN: hydrasdr_call_result_t */
	HYDRASDR_SET_SOF_DIVIDER            = HYDRASDR_EXT_CMD_MAX /* wValue | wIndex << 16 = divider: tag the first SOF of every USB frame whose number is a multiple of it, 0 = off; cleared at every stream stop */
} hydrasdr_vendor_request;

/* Reply to HYDRASDR_GET_STREAM_STATUS: 8 little-endian uint32 */
/* Crystal correction in effect: the flash block's value */
#define HYDRASDR_CALIBRATION_SOURCE_NONE  (0)
#define HYDRASDR_CALIBRATION_SOURCE_FLASH (1)
#define HYDRASDR_CALIBRATION_SOURCE_HOST  (2)
typedef struct
{
	uint32_t core; /* 0 = M0 (runs in the USB request handler), 1 = M4 (runs in its main loop) */
	uint32_t address; /* function address; the Thumb bit is added by the device */
	uint32_t r[4];    /* arguments */
} hydrasdr_call_request_t;

typedef struct
{
	uint32_t status; /* 0 = done (r0 valid), 1 = still running on the M4, 2 = bad request */
	uint32_t r0;
} hydrasdr_call_result_t;

typedef struct
{
	int32_t correction_ppb;
	uint32_t source; /* HYDRASDR_CALIBRATION_SOURCE_* */
} hydrasdr_calibration_t;

typedef struct
{
	uint32_t captured;      /* chunks captured by the ADC DMA */
	uint32_t delivered; /* chunks whose USB transfer to the host completed */
	uint32_t lost; /* chunks overwritten before they could be sent: the sample stream has gaps */
	uint32_t overruns;
	uint32_t backlog_max; /* worst number of chunks pending at the device */
	uint32_t ring_chunks;   /* chunks the device ring buffer holds */
	uint32_t chunk_bytes;   /* bytes per chunk on USB */
	uint32_t chunk_samples; /* real ADC samples per chunk */
	uint32_t m0_lag_max; /* worst lag of the device's own USB queuing behind the ADC, in chunks */
	uint32_t dma_errors; /* ADC DMA bus errors: the DMA could not write part of the ring */
	uint32_t usb_errors; /* bulk transfers the USB controller retired with an error */
	uint32_t adc_overflows; /* chunks during which the ADC FIFO overflowed */
	uint32_t pps_count; /* PPS edges captured since the stream started */
	uint32_t sof_count; /* USB SOFs tagged since the stream started */
	uint32_t sof_edges; /* USB SOFs captured since the stream started, tagged or not */
	uint32_t sof_divider; /* HYDRASDR_SET_SOF_DIVIDER value in effect, 0 = off */
} hydrasdr_stream_status_t;

#define HYDRASDR_FRAME_HEADER_SIZE (96)
#define HYDRASDR_FRAME_MAGIC (0x42445642)
#define HYDRASDR_FRAME_UART_BYTES (15) /* UART bytes per chunk header */
#define HYDRASDR_FRAME_FLAG_PACKED (1 << 0) /* 12-bit packed samples (packing 1 and 2) */
#define HYDRASDR_FRAME_FLAG_8BIT   (1 << 1) /* one byte per sample, the top 8 bits (packing 3) */
#define HYDRASDR_FRAME_FLAG_TIMESTAMP (1 << 2) /* packing 2: the first two packed values are the chunk counter and the ADC status */

typedef struct
{
	uint64_t sample_index; /* index of this chunk's first sample in the ADC sample stream */
	uint32_t magic;          /* HYDRASDR_FRAME_MAGIC */
	uint32_t chunk_index;    /* chunk number since stream start */
	uint32_t sample_count;   /* real samples in this chunk */
	uint32_t flags;          /* HYDRASDR_FRAME_FLAG_* */
	uint32_t lost_chunks; /* chunks the device lost so far, see hydrasdr_stream_status_t */
	uint32_t overrun_chunks; /* chunks the device delivered corrupted so far */
	uint32_t freq_hz; /* tuner frequency the device was set to when this chunk was queued */
	uint32_t pps_sample_index_lo; /* sample index of the last PPS edge captured since the stream started */
	uint32_t pps_sample_index_hi;
	uint32_t pps_fraction; /* position of that edge within the sample, in 1/2^32 sample units */
	uint32_t pps_count; /* PPS edges captured since the stream started */
	uint8_t uart_len; /* bytes received on the auxiliary UART (GNSS module) carried in this chunk */
	uint8_t uart_data[HYDRASDR_FRAME_UART_BYTES];
	/* USB Start-Of-Frame tagging (HYDRASDR_SET_SOF_DIVIDER), independent of the PPS input */
	uint32_t sof_sample_index_lo; /* sample index of the last tagged SOF */
	uint32_t sof_sample_index_hi;
	uint32_t sof_fraction; /* position of that SOF within the sample, in 1/2^32 sample units */
	uint32_t sof_frame; /* its USB frame number, extended past the 11-bit wrap by the device */
	uint32_t sof_count; /* SOFs tagged since the stream started */
	uint32_t reserved[2];    /* zero */
} hydrasdr_frame_header_t;   /* HYDRASDR_FRAME_HEADER_SIZE bytes */

/* Chunk size on the wire, framed or not */
#define HYDRASDR_FRAME_WIRE_UNPACKED (16384)
#define HYDRASDR_FRAME_WIRE_PACKED (6144)
#define HYDRASDR_FRAME_WIRE_8BIT (4096)

/* HYDRASDR_WATCHDOG: wIndex 1 feeds the watchdog and reports */
#define HYDRASDR_WATCHDOG_FLAG_ARMED (1 << 0)             /* the watchdog is running */
#define HYDRASDR_WATCHDOG_FLAG_RESET_BY_WATCHDOG (1 << 1) /* the last reset was a watchdog time-out (best effort) */
#define HYDRASDR_WATCHDOG_FLAG_HOST_OWNED (1 << 2) /* strict mode: the host has made contact, only the host feeds */
#define HYDRASDR_WATCHDOG_FLAG_STRICT (1 << 3) /* firmware built with WATCHDOG=1: the host protocol applies */
#define HYDRASDR_WATCHDOG_SELF_FEEDS (6) /* strict mode: self feeds before contact, about 5.6 s each */

typedef struct
{
	uint32_t flags;           /* HYDRASDR_WATCHDOG_FLAG_* */
	uint32_t timeout_ms;      /* time between feeds the device tolerates */
	uint32_t remaining_ms; /* time left before a reset when the reply was built */
	uint32_t self_feeds_left; /* strict mode: self feeds left before contact; 0xFFFFFFFF when unlimited */
} hydrasdr_watchdog_status_t;

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
