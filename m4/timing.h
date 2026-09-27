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

/* Timestamps of events in ADC sample units: the PPS input and the USB Start-Of-Frame */
#ifndef __TIMING_H__
#define __TIMING_H__

#include <stdint.h>

void timing_init(void);
/* Call with interrupts disabled immediately before the ADC trigger */
void timing_stream_start(uint32_t sample_rate_hz, uint32_t timer_clock_hz);
void timing_stream_stop(void);
/* Tag the first SOF (microframe 0) of every USB frame whose number is a multiple of the divider, 0 turns it off */
void timing_set_sof_divider(uint32_t divider);

#endif
