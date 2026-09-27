/*
 * Copyright 2026 Bertold Van den Bergh <vandenbergh@bertold.org>
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

#ifndef __HYDRASDR_STREAM_H
#define __HYDRASDR_STREAM_H

#include <stdint.h>
#include "hydrasdr_commands.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* ADC -> USB ring buffer, shared by the M4 */
#define HYDRASDR_STREAM_PIECE_SIZE  (16384)
#define HYDRASDR_STREAM_NUM_PIECES  (11)
#define HYDRASDR_STREAM_SLOT_SIZE   (8192) /* one GPDMA linked-list item fills one slot */
#define HYDRASDR_STREAM_NUM_SLOTS   (HYDRASDR_STREAM_NUM_PIECES * 2)

static const uint32_t hydrasdr_stream_pieces[HYDRASDR_STREAM_NUM_PIECES] =
{
  0x10008000, 0x1000C000, 0x10010000, 0x10014000, 0x10018000, 0x1001C000, /* local SRAM 1 */
  0x10080000, 0x10084000, 0x10088000, 0x1008C000,                         /* local SRAM 2 */
  0x2000C000                                                              /* AHB SRAM */
};

static inline uint8_t* hydrasdr_stream_slot(uint32_t slot)
{
  return (uint8_t*)(hydrasdr_stream_pieces[slot / 2] + (slot & 1) * HYDRASDR_STREAM_SLOT_SIZE);
}

/* Unpacked: one chunk is two slots of 16-bit samples */
#define HYDRASDR_STREAM_CHUNK_BYTES_UNPACKED  (2 * HYDRASDR_STREAM_SLOT_SIZE)
/* Packed: one chunk is one slot */
#define HYDRASDR_STREAM_CHUNK_BYTES_PACKED    ((HYDRASDR_STREAM_SLOT_SIZE / 4) * 3)
/* 8-bit mode (packing 3) */
#define HYDRASDR_STREAM_CHUNK_BYTES_8BIT      (HYDRASDR_STREAM_SLOT_SIZE / 2)

/* Packing 2: the chunk counter and the ADC status take the place of the first two packed samples */
#define HYDRASDR_STREAM_TIMESTAMP_SAMPLES     (2)

/* Transfers the M0 can hold in the USB controller at once */
#define HYDRASDR_STREAM_USB_POOL              (4)

/* Stream state, placed at cm4_data_share (AHB SRAM visible to both cores) */
typedef struct
{
  volatile uint32_t captured; /* chunks ready for USB: DMA done and, when packing, packed */
  volatile uint32_t chunk_bytes;  /* bytes per chunk as sent over USB */
  volatile uint32_t chunk_slots; /* DMA slots per chunk: 2 unpacked, 1 packed */
  volatile uint32_t ring_chunks;  /* number of chunks the ring holds */
  volatile uint32_t header_bytes; /* HYDRASDR_FRAME_HEADER_SIZE when framing is on, else 0 */
  volatile uint32_t chunk_samples;/* real ADC samples per chunk */
  volatile uint32_t overruns;
  volatile uint32_t backlog_max; /* highest number of chunks pending at the device */
  volatile uint32_t m0_lag_max;
  volatile uint32_t dma_errors; /* GPDMA error interrupts: the ADC DMA could not write the ring (bus error) */
  volatile uint32_t adc_overflows; /* chunks in which the ADC FIFO overflow flag was set */
  /* PPS capture (M4 timer ISR) */
  volatile uint32_t pps_seq;
  volatile uint32_t pps_count;     /* edges captured since the stream started */
  volatile uint32_t pps_index_lo;  /* sample index of the last edge */
  volatile uint32_t pps_index_hi;
  volatile uint32_t pps_fraction; /* position within that sample, 1/2^32 sample units */
  /* USB Start-Of-Frame capture (M4 timer ISR) */
  volatile uint32_t sof_seq;
  volatile uint32_t sof_count;     /* tagged SOFs since the stream started */
  volatile uint32_t sof_edges;     /* every SOF captured since the stream started */
  volatile uint32_t sof_index_lo;  /* sample index of the last tagged SOF */
  volatile uint32_t sof_index_hi;
  volatile uint32_t sof_fraction;
  volatile uint32_t sof_frame;     /* its USB frame index, see hydrasdr_frame_header_t */
  volatile uint32_t sof_divider;   /* tagging divider in effect, 0 = off */
  volatile uint32_t calib_xtal_nominal;
  volatile int32_t  calib_ppb;
  volatile uint32_t calib_source;
  volatile uint32_t delivered; /* chunks whose USB transfer completed (USB ISR) */
  volatile uint32_t lost; /* chunks never queued because the DMA overwrote them first (main loop) */
  volatile uint32_t queued; /* chunks handed to the USB controller (main loop) */
  volatile uint32_t usb_errors; /* bulk IN transfers the USB controller retired with an error (USB ISR) */
  volatile uint32_t packing; /* hydrasdr_packing_type in effect */
} hydrasdr_stream_state_t;

extern uint32_t cm4_data_share; /* defined in linker script */
#define HYDRASDR_STREAM_STATE ((volatile hydrasdr_stream_state_t *)&cm4_data_share)

#ifdef __cplusplus
}
#endif

#endif /* __HYDRASDR_STREAM_H */
