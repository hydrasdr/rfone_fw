/*
 * Copyright 2016-2024 Benjamin Vernoux <bvernoux@hydrasdr.com>
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

#ifndef __HYDRASDR_CALIB_H__
#define __HYDRASDR_CALIB_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define HYDRASDR_FLASH_CALIB_OFFSET (0x20000) /* After 128KB (0 to 128KB reserved for firmware) */
#define HYDRASDR_FLASH_CALIB_HEADER (0xCA1B0001)

/* For each configuration the index corresponds to uint32_t */
typedef struct
{
  uint32_t header; /* Shall be equal to HYDRASDR_FLASH_CALIB_HEADER */
  uint32_t timestamp; /* Epoch Unix Time Stamp */
  int32_t correction_ppb;
} hydrasdr_calib_t;

#ifdef __cplusplus
}
#endif

#endif /* __HYDRASDR_CALIB_H__ */
