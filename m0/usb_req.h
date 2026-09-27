/*
 * Copyright 2014-2024 Benjamin Vernoux <bvernoux@hydrasdr.com>
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

#ifndef __USB_REQ_H__
#define __USB_REQ_H__

#include "usb_type.h"

extern usb_endpoint_t usb_endpoint_bulk_in;
extern usb_endpoint_t usb_endpoint_bulk_out;
extern usb_endpoint_t usb_endpoint_control_in;
extern usb_endpoint_t usb_endpoint_control_out;
extern usb_device_t usb_device;

extern uint8_t sample_rate_conf_no;

typedef struct {
  uint64_t freq_hz;
} set_freq_params_t;

/* Last frequency the host asked for */
extern set_freq_params_t set_freq_params;

typedef enum
{
  DEFERRED_NONE = 0,
  DEFERRED_R82X_READ,
  DEFERRED_R82X_WRITE,
  DEFERRED_SET_FREQ,
  DEFERRED_LNA_GAIN,
  DEFERRED_MIXER_GAIN,
  DEFERRED_VGA_GAIN,
  DEFERRED_LNA_AGC,
  DEFERRED_MIXER_AGC,
  DEFERRED_SET_RF_PORT
} deferred_kind_t;

typedef struct
{
  volatile uint32_t pending;
  deferred_kind_t kind;
  usb_endpoint_t* endpoint;
  uint32_t index;
  uint32_t value;
} deferred_job_t;

extern deferred_job_t deferred_job;
void deferred_job_run(void); /* main loop */

void usb_req_init(void);

void usb_streaming_disable(void);

#endif//__USB_REQ_H__
