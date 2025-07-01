/*
 * Copyright 2013-2024 Benjamin Vernoux <bvernoux@hydrasdr.com>
 *
 * This file is part of HydraSDR (based on AirSpy project).
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

#ifndef __R82X_H__
#define __R82X_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "hydrasdr_commands.h"

#define R82X_I2C_ADDR (0x3A << 1) // R828D I2C ADDR

#define REG_SHADOW_START 5
#define NUM_REGS 30

/* R82X Clock */
#define CALIBRATION_LO 88000

typedef struct
{
  uint32_t xtal_freq; /* XTAL_FREQ_HZ */
  uint32_t freq;
  uint32_t if_freq;
  uint8_t regs[NUM_REGS];
  uint16_t padding;
} r82x_priv_t;

void r82x_write_single(r82x_priv_t *priv, uint8_t reg, uint8_t val);
uint8_t r82x_read_single(r82x_priv_t *priv, uint8_t reg);

void r82x_startup(r82x_priv_t *priv);
int r82x_init(r82x_priv_t *priv, const uint32_t if_freq);
int r82x_set_freq(r82x_priv_t *priv, uint64_t freq);
int r82x_set_lna_gain(r82x_priv_t *priv, uint8_t gain_index);
int r82x_set_mixer_gain(r82x_priv_t *priv, uint8_t gain_index);
int r82x_set_vga_gain(r82x_priv_t *priv, uint8_t gain_index);
int r82x_set_lna_agc(r82x_priv_t *priv, uint8_t value);
int r82x_set_mixer_agc(r82x_priv_t *priv, uint8_t value);
void r82x_set_if_bandwidth(r82x_priv_t *priv, uint8_t bw);
int r82x_standby(void);
int r82x_set_rf_port(r82x_priv_t *priv, hydrasdr_rf_port_t rf_port);

#ifdef __cplusplus
}
#endif

#endif /* __R82X_H__ */
