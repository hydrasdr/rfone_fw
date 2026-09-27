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

MEMORY
{
	ram_local1 (rwx) : ORIGIN = 0x10000000, LENGTH =  32K
	ram_local1_usb (rwx) : ORIGIN = 0x10008000, LENGTH =  96K
	ram_local2_usb (rwx) : ORIGIN = 0x10080000, LENGTH =  64K
	ram_local2 (rwx) : ORIGIN = 0x10090000, LENGTH =  8K
	ram_local_m0sub (rwx) : ORIGIN = 0x18000000, LENGTH =  18K

	ram_ahb1_m0_stack (rwx) : ORIGIN = 0x20000000, LENGTH =  4K
	ram_ahb1_spare    (rwx) : ORIGIN = 0x20001000, LENGTH =  3K
	ram_ahb1_m4_share (rwx) : ORIGIN = 0x20001C00, LENGTH =  1K
	ram_ahb1_adchs (rwx)    : ORIGIN = 0x20002000, LENGTH =  4K
	ram_ahb1_m0_share (rwx) : ORIGIN = 0x20003000, LENGTH =  4K

	ram_ahb2   (rwx) : ORIGIN = 0x20004000, LENGTH =  32K
	ram_ahb2_usb (rwx) : ORIGIN = 0x2000C000, LENGTH =  16K
}

cm4_data_share = ORIGIN(ram_ahb1_m4_share);	/* M4 data shared (write by M4 read by M0 */
adchs_data     = ORIGIN(ram_ahb1_adchs);	/* M4 ADCHS data (write by ADCHS DMA or M4) */
cm0_data_share = ORIGIN(ram_ahb1_m0_share);	/* M0 data shared (write by M0 read by M4) */

cm0_exec_baseaddr = ORIGIN(ram_ahb2);
cm0s_exec_baseaddr = ORIGIN(ram_local_m0sub);

conf_data_share = ORIGIN(ram_ahb1_spare);	/* M4/M0 conf data shared */
conf_data_share_size = LENGTH(ram_ahb1_spare);	/* M4/M0 conf data shared size */

EXTERN (vector_table)

ENTRY(reset_handler)

SECTIONS
{
	.text : {
		*(.vectors)	/* Vector table */
		*(.text*)	/* Program code */
		. = ALIGN(4);
		*(.rodata*)	/* Read-only data */
		. = ALIGN(4);
	} >ram_local1

	.preinit_array : {
		. = ALIGN(4);
		__preinit_array_start = .;
		KEEP (*(.preinit_array))
		__preinit_array_end = .;
	} >ram_local1
	.init_array : {
		. = ALIGN(4);
		__init_array_start = .;
		KEEP (*(SORT(.init_array.*)))
		KEEP (*(.init_array))
		__init_array_end = .;
	} >ram_local1
	.fini_array : {
		. = ALIGN(4);
		__fini_array_start = .;
		KEEP (*(.fini_array))
		KEEP (*(SORT(.fini_array.*)))
		__fini_array_end = .;
	} >ram_local1

	.ARM.extab : {
		*(.ARM.extab*)
	} >ram_local1

	.ARM.exidx : {
		__exidx_start = .;
		*(.ARM.exidx*)
		__exidx_end = .;
	} >ram_local1

	.nocopy_data : {
		. = ALIGN(4);
		_nocopy_data = .;
		*(.nocopy_data)
		. = ALIGN(4);
		_enocopy_data = .;
	} >ram_local1
	_nocopy_data_loadaddr = LOADADDR(.nocopy_data);
	_nocopy_data_rom = 0;
	_enocopy_data_rom = 0;

	. = ALIGN(4);
	_etext = .;
	_text_ram = 0;
	_etext_ram = 0;
	_etext_rom = 0;

	.data : {
		_data = .;
		*(.data*)	/* Read-write initialized data */
		. = ALIGN(4);
		_edata = .;
	} >ram_local1
	_data_loadaddr = LOADADDR(.data);
	_data_rom = 0;
	_edata_rom = 0;

	.bss : {
		_bss = .;
		*(.bss_aligned*)	/* Read-write zero initialized data */
		*(.bss*)	/* Read-write zero initialized data */
		*(COMMON)
		. = ALIGN(4);
		_ebss = .;
	} >ram_local1

	/DISCARD/ : { *(.eh_frame) }

	. = ALIGN(4);
	end = .;

	__StackTop = ORIGIN(ram_local2) + LENGTH(ram_local2) - 32;
	PROVIDE(_stack = __StackTop);
}
