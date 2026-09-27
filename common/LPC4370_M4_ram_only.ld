/*
 * This file is part of the libopencm3 project.

 * Copyright 2012 Michael Ossmann <mike@ossmann.com>
 * Copyright 2012 Jared Boone <jared@sharebrained.com>
 * Copyright 2013-2024 Benjamin Vernoux <bvernoux@hydrasdr.com>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

/* Linker script for HydraSDR (LPC4370, 1M or 4MB SPI flash, 282K SRAM). */
MEMORY
{
  /* rom is really the shadow region that points to SPI flash or elsewhere */
  rom (rx)  : ORIGIN = 0x00000000, LENGTH =  1M
	ram_local1 (rwx) : ORIGIN = 0x10000000, LENGTH =  32K
	ram_local1_usb (rwx) : ORIGIN = 0x10008000, LENGTH =  96K
	ram_local2_usb (rwx) : ORIGIN = 0x10080000, LENGTH =  64K
	ram_local2 (rwx) : ORIGIN = 0x10090000, LENGTH =  8K
  /* LPC437x only: Reserved for M0 subsystem SRAM 16KB + 2KB  */
  ram_local_m0sub (rwx) : ORIGIN = 0x18000000, LENGTH =  18K
	ram_ahb1   (rwx) : ORIGIN = 0x20000000, LENGTH =  16K
	ram_ahb2   (rwx) : ORIGIN = 0x20004000, LENGTH =  32K
	ram_ahb2_usb (rwx) : ORIGIN = 0x2000C000, LENGTH =  16K
}

cm0_exec_baseaddr = ORIGIN(ram_ahb2);	/* Used by M4 to load code from M4 RAM to M0 final exec/run addr */
cm0s_exec_baseaddr = ORIGIN(ram_local_m0sub);	/* Used by M4 to load code from M4 RAM to M0Sub final exec/run addr */

/* Include the common ld script. */
INCLUDE libopencm3_lpc43xx_ram_only.ld
