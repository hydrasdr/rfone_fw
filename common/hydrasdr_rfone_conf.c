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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING. If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */
#include "hydrasdr_conf.h"

#define HYDRASDR_CONF_M0_M4_NB (3)
#define HYDRASDR_CONF_M0_M4_ALT_NB (4)
#define HYDRASDR_CONF_SI5351C_NB (2)
#define NULL_ADDR (0)

typedef struct
{
	hydrasdr_conf_t hydrasdr_conf;

	/* 1st Expansion Conf Point M0/M4 Data (mandatory) */
	m0_m4_conf_t m0_m4_conf[HYDRASDR_CONF_M0_M4_NB];

	/* 2nd Expansion Conf Point M0/M4 ALT Data (optional) */ 
	m0_m4_conf_t m0_m4_alt_conf[HYDRASDR_CONF_M0_M4_ALT_NB];

	/* 3rd Expansion Conf Point SI5351C Data (depending on conf_hw.hardware_type) */
	si5351c_conf_t si5351c_config[HYDRASDR_CONF_SI5351C_NB];

} hydrasdr_rfone_conf_t;

hydrasdr_rfone_conf_t __attribute__ ((section(".nocopy_data"))) hydrasdr_rfone_conf =
{
	/*	hydrasdr_conf_t hydrasdr_conf = */
	{
		/* conf_struct_version_t conf_struct_version = */
		{
			1, 0, 0
		},

		/* hw_conf_t conf_hw = */
		{
			HW_TYPE_HYDRASDR_RFONE,
			"HydraSDR RFOne"
		},

		/* i2c_conf_t i2c_conf = */
		{
				/* Configure I2C0 & IC21 to 400kHz (140MHz/(2*175)=0.4MHz) */
				/* Also i2c0_pll1_ls_hs_conf_val => I2C val (For 400KHz) = (5*FreqPLL1MHz)/4 */
				175, /* uint16_t i2c0_pll1_ls_hs_conf_val; Si5351C */
				175, /* uint16_t i2c1_pll1_hs_conf_val; R82X */
				/* Configure I2C1 to 400kHz (40MHz/(2*50)=0.4MHz) */
				50, /* uint16_t i2c1_pll1_ls_conf_val; R82X */
				0, /* padding */
				0 /* spare */
		}, /* End i2c_conf_t i2c_conf */

		/*
		Clock configuration with GP_CLKIN=20MHz
		Configuration of clocks at init (sys_clock_init())
		*/
		/* const core_clocks_t m4_init_conf = */
		{
			/* Configure PLL0USB to produce 480MHz clock from GP_CLKIN */
			0x040E03FF, // uint32_t pll0_usb_mdiv; SELR SELI SELP MDEC
			0x00000000, // uint32_t pll0_usb_npdiv; PDEC NDEC
			(PLL0USB_CTRL_FLAG_DIRECT_I | PLL0USB_CTRL_FLAG_DIRECT_O), // uint32_t pll0usb_ctrl_flags; DirectI=PLL0USB_CTRL_FLAG_DIRECT_I or/and DirectO=PLL0USB_CTRL_FLAG_DIRECT_O */
			/* PLL1 clock from GP_CLKIN */
			/* PLL1 High Speed Mode Set PLL1 to 20MHz * (6+1) = 140MHz */
			{
				0, // uint32_t pll1_hs_psel;
				0, // uint32_t pll1_hs_nsel;
				6, // uint32_t pll1_hs_msel;
			},
			/* PLL1 Low Speed Mode => Set PLL1 to 20MHz * (1+1) = 40MHz */
			{
				0, // uint32_t pll1_ls_psel;
				0, // uint32_t pll1_ls_nsel;
				1, // uint32_t pll1_ls_msel;
			}
		}, /* End core_clocks_t m4_init_conf */

		/* Configuration for R82X initial values */
		/* Those initial values start from REG_SHADOW_START */
		/* Shall be not set as const as the structure is used as IN & OUT */
		/* r82x_priv_t r82x_conf_rw = */
		{
			25000000, // xtal_freq => 25MHz
			0, // Set at boot to m0_m4_conf_t conf0 -> r82x_if_freq
			100000000, /* Default Freq 100Mhz */
			{
				/* 05 */ 0x90, // LNA manual gain mode, init to 0
				/* 06 */ 0x80,
				/* 07 */ 0x60,
				/* 08 */ 0x80, // Image Gain Adjustment
				/* 09 */ 0x40, // Image Phase Adjustment
				/* 0A */ 0xA8, // Channel filter [0..3]: 0 = widest, f = narrowest - Optimal. Don't touch!
				/* 0B */ 0x0F, // High pass filter - Optimal. Don't touch!
				/* 0C */ 0x40, // VGA control by code, init at 0
				/* 0D */ 0x63, // LNA AGC settings: [0..3]: Lower threshold; [4..7]: High threshold
				/* 0E */ 0x75,
				/* 0F */ 0xF8, // LDO_5V OFF, CLK AGC OFF,
				/* 10 */ 0x7C, // 0x74 TBC
				/* 11 */ 0x42, // 0x4B TBC
				/* 12 */ 0x06, // 0x80 TBC
				/* 13 */ 0x00, // 0x31
				/* 14 */ 0x0F,
				/* 15 */ 0x00,
				/* 16 */ 0xC0,
				/* 17 */ 0xA0,
				/* 18 */ 0x48,
				/* 19 */ 0xCC,
				/* 1A */ 0x60,
				/* 1B */ 0x00,
				/* 1C */ 0x54,
				/* 1D */ 0xAE,
				/* 1E */ 0x0A,
				/* 1F */ 0xC0
			},
			0 /* uint16_t padding */
		}, /* End r82x_priv_t r82x_conf_rw */

		/* 1st Expansion Conf Point M0/M4 */
		(uint16_t)(sizeof(m0_m4_conf_t)), /* uint16_t sizeof_m0_m4_conf_t */
		HYDRASDR_CONF_M0_M4_NB, /* uint16_t nb_m0_m4_conf_t */
		NULL_ADDR, /* m0_m4_conf_t* m0_m4_conf */

		/* 2nd Expansion Conf Point ALT M0/M4 */ 
		(uint16_t)(sizeof(m0_m4_conf_t)), /* uint16_t sizeof_m0_m4_alt_conf_t */
		HYDRASDR_CONF_M0_M4_ALT_NB, /* uint16_t nb_m0_m4_alt_conf_t */
		NULL_ADDR, /* m0_m4_conf_t* m0_m4_alt_conf */

		/* 3rd Expansion Conf Point SI5351C */ 
		(uint16_t)(sizeof(si5351c_conf_t)), /* uint16_t sizeof_si5351c_conf_t */
		HYDRASDR_CONF_SI5351C_NB, /* uint16_t nb_si5351c_conf_t */
		NULL_ADDR /* si5351c_conf_t* si5351c_config */
	},

	/* 1st Expansion Conf Point M0/M4 Data */
	/*
	SampleRate configuration with GP_CLKIN=20MHz from SI5351C CLK7 LPC4370 GP_CLKIN
	Configuration of PLL0AUDIO shall not exceed 80MHz for ADCHS
	For PLL0AUDIO sys_clock_samplerate() set SEL_EXT to 1 => MDEC enabled. Fractional divider not used.
	For PLL0AUDIO see UM10503 Rev1.8 "Fig 34. PLL0 with fractional divider" Page 184 / 1420 for more details.
	Configuration of IDIVB => Integer divider B divider values (1/(IDIV + 1))
	*/
	/* const m0_m4_conf_t m0_m4_conf[HYDRASDR_CONF_M0_M4_NB] = */
	{
		/* Conf 0 => 10MHz IQ(Spurs at -100dBFS) => 10*4*8 = 320Mbps = 40MB/s */
		{
			/*
			core_clock_pll0_idivb_t hydrasdr_m4_conf
			*/
			{
				/* PLL0AUDIO */
				0x00000000, // uint32_t pll0audio_mdiv;
				0x00000000, // uint32_t pll0audio_npdiv;
				0x00000000, // uint32_t pll0audio_ctrl_flags; DirectI=PLL0AUDIO_CTRL_FLAG_DIRECT_I or/and DirectO=PLL0AUDIO_CTRL_FLAG_DIRECT_O */
				/* IDIVB (from GP_CLKIN) */
				0, // uint8_t adchs_idivb; /* 0 to 15 (0 means direct connection GP_CLKIN to ADCHS_CLK) */
				{ 0, 0, 0 } /* uint8_t padding[3] */
			},
			/* m0_conf_t m0_conf */
			{
				5000000, // Freq 20MHz => 10MHz IQ => IF Freq = 5MHz (r82x_if_freq)
				59,	// uint8_t r82x_bw;
				0,// uint8_t padding0;
				0 // uint16_t padding1;
			}
		},
		/* Conf 1 => 5MHz IQ(Spurs at -105dBFS) => 5*4*8 = 160Mbps = 20MB/s */
		{
			/*
			core_clock_pll0_idivb_t hydrasdr_m4_conf
			*/
			{
				/* PLL0AUDIO */
				0x0000001F, // uint32_t pll0audio_mdiv;
				0x0000000C, // uint32_t pll0audio_npdiv;
				PLL0AUDIO_CTRL_FLAG_DIRECT_I, // uint32_t pll0audio_ctrl_flags; DirectI=PLL0AUDIO_CTRL_FLAG_DIRECT_I or/and DirectO=PLL0AUDIO_CTRL_FLAG_DIRECT_O */
				/* IDIVB not used set it to 0 */
				0, // uint8_t adchs_idivb; /* 0 to 15 (0 means direct connection GP_CLKIN to ADCHS_CLK) */
				{ 0, 0, 0 } /* uint8_t padding[3] */
			},
			/* m0_conf_t m0_conf */
			{
				2500000, // Freq 10MHz => 5MHz IQ => IF Freq = 2.5MHz (r82x_if_freq)
				31, // uint8_t r82x_bw;
				0, // uint8_t padding0;
				0 // uint16_t padding1;
			}
		},
		/* Conf 2 => 2.5MHz IQ(Spurs at -100dBFS) => 2.5*4*8 = 80Mbps = 10MB/s */
		{
			/*
			core_clock_pll0_idivb_t hydrasdr_m4_conf
			*/
			{
				/* PLL0AUDIO */
				0x00000000, // uint32_t pll0audio_mdiv;
				0x00000000, // uint32_t pll0audio_npdiv;
				0x00000000, // uint32_t pll0audio_ctrl_flags; DirectI=PLL0AUDIO_CTRL_FLAG_DIRECT_I or/and DirectO=PLL0AUDIO_CTRL_FLAG_DIRECT_O */
				/* IDIVB not used set it to 0 */
				3, // uint8_t adchs_idivb; /* 0 to 15 (0 means direct connection GP_CLKIN to ADCHS_CLK) */
				{ 0, 0, 0 } /* uint8_t padding[3] */
			},
			/* m0_conf_t m0_conf */
			{
				1250000, // Freq 5MHz => 2.5MHz IQ => IF Freq = 1.25MHz (r82x_if_freq)
				0, // uint8_t r82x_bw;
				0, // uint8_t padding0;
				0 // uint16_t padding1;
			}
		}
	}, /* End m0_m4_conf_t m0_m4_conf[HYDRASDR_CONF_M0_M4_NB] */

	/* 2nd Expansion Conf Point M0/M4 ALT Data */ 
	/*
	SampleRate configuration with GP_CLKIN=20MHz from SI5351C CLK7 LPC4370 GP_CLKIN
	Configuration of PLL0AUDIO shall not exceed 80MHz for ADCHS
	For PLL0AUDIO sys_clock_samplerate() set SEL_EXT to 1 => MDEC enabled. Fractional divider not used.
	For PLL0AUDIO see UM10503 Rev1.8 "Fig 34. PLL0 with fractional divider" Page 184 / 1420 for more details.
	Configuration of IDIVB => Integer divider B divider values (1/(IDIV + 1))
	*/
	/* const m0_m4_conf_t m0_m4_alt_conf[HYDRASDR_CONF_M0_M4_ALT_NB] = */
	{
		/* Conf 0 => 12 MSPS */
		{
			/*
			core_clock_pll0_idivb_t hydrasdr_m4_conf
			*/
			{
				/* PLL0AUDIO */
				0x000003FF, // uint32_t pll0audio_mdiv;
				0x0000000E, // uint32_t pll0audio_npdiv;
				PLL0AUDIO_CTRL_FLAG_DIRECT_I, // uint32_t pll0audio_ctrl_flags; DirectI=PLL0AUDIO_CTRL_FLAG_DIRECT_I or/and DirectO=PLL0AUDIO_CTRL_FLAG_DIRECT_O */
				/* IDIVB not used set it to 0 */
				0, // uint8_t adchs_idivb; /* 0 to 15 (0 means direct connection GP_CLKIN to ADCHS_CLK) */
				{ 0, 0, 0 } /* uint8_t padding[3] */
			},
			/* m0_conf_t m0_conf */
			{
				6000000, // Freq 24MHz => 12MHz IQ => IF Freq = 6MHz (r82x_if_freq)
				63, // uint8_t r82x_bw;
				0,// uint8_t padding0;
				0 // uint16_t padding1;
			}
		},
		/* Conf 1 => 8MHz IQ(Spurs at -90dBFS for test purpose) => 8*4*8 = 256Mbps = 32MB/s */
		{
			/*
			core_clock_pll0_idivb_t hydrasdr_m4_conf
			*/
			{
				/* PLL0AUDIO */
				0x0000003F, // uint32_t pll0audio_mdiv;
				0x0000000E, // uint32_t pll0audio_npdiv;
				PLL0AUDIO_CTRL_FLAG_DIRECT_I, // uint32_t pll0audio_ctrl_flags; DirectI=PLL0AUDIO_CTRL_FLAG_DIRECT_I or/and DirectO=PLL0AUDIO_CTRL_FLAG_DIRECT_O */
				/* IDIVB not used set it to 0 */
				0, // uint8_t adchs_idivb; /* 0 to 15 (0 means direct connection GP_CLKIN to ADCHS_CLK) */
				{ 0, 0, 0 } /* uint8_t padding[3] */
			},
			/* m0_conf_t m0_conf */
			{
				4000000, // Freq 20MHz => 8MHz IQ => IF Freq = 4MHz (r82x_if_freq)
				57, // uint8_t r82x_bw;
				0,// uint8_t padding0;
				0 // uint16_t padding1;
			}
		},
		/* Conf 2 => 6 MSPS IQ(Spurs at -80dBFS for test purpose) => 6*4*8 = 192Mbps = 24MB/s */
		{
			/*
			core_clock_pll0_idivb_t hydrasdr_m4_conf
			*/
			{
				/* PLL0AUDIO */
				0x0000007F, // uint32_t pll0audio_mdiv;
				0x00000018, // uint32_t pll0audio_npdiv;
				PLL0AUDIO_CTRL_FLAG_DIRECT_I, // uint32_t pll0audio_ctrl_flags; DirectI=PLL0AUDIO_CTRL_FLAG_DIRECT_I or/and DirectO=PLL0AUDIO_CTRL_FLAG_DIRECT_O */
				/* IDIVB not used set it to 0 */
				0, // uint8_t adchs_idivb; /* 0 to 15 (0 means direct connection GP_CLKIN to ADCHS_CLK) */
				{ 0, 0, 0 } /* uint8_t padding[3] */
			},
			/* m0_conf_t m0_conf */
			{
				3000000, // Freq 12MHz => 6MHz IQ => IF Freq = 3MHz (r82x_if_freq)
				32, // uint8_t r82x_bw;
				0, // uint8_t padding0;
				0	// uint16_t padding1;
			}
		},
		/* Conf 3 => 4.096 MSPS IQ (For test purpose) => 4.096*4*8 = 131.072Mbps = 16.384MB/s	*/
		{
			/*
				core_clock_pll0_idivb_t hydrasdr_m4_conf
			*/
			{
				/* PLL0AUDIO */
				0x00004924, // uint32_t pll0audio_mdiv;
				0x0003F006, // uint32_t pll0audio_npdiv;
				0x00000000, // uint32_t pll0audio_ctrl_flags; DirectI=PLL0AUDIO_CTRL_FLAG_DIRECT_I or/and DirectO=PLL0AUDIO_CTRL_FLAG_DIRECT_O */
				/* IDIVB not used set it to 0 */
				0, // uint8_t adchs_idivb; /* 0 to 15 (0 means direct connection GP_CLKIN to ADCHS_CLK) */
				{ 0, 0, 0 } /* uint8_t padding[3] */
			},
			/* m0_conf_t m0_conf */
			{
				2048000, // Freq 8.192MHz => 4.096MHz IQ => IF Freq = 2.048MHz (r82x_if_freq)
				25, // uint8_t r82x_bw;
				0, // uint8_t padding0;
				0 // uint16_t padding1;
			}
		}
	}, /* End m0_m4_conf_t m0_m4_alt_conf[HYDRASDR_CONF_M0_M4_ALT_NB] */

	/* 3rd Expansion Conf Point SI5351C Data */ 
	/* si5351c_conf_t si5351c_config[HYDRASDR_CONF_SI5351C_NB] = */
	{
		/* Conf 0 (HYDRASDR_SI5351C_CONFIG_XTAL) XTAL 25MHZ CLK */
		{
			/* Respective value for register 0 to register 180
			The registers are generated using a custom program that optimizes the phase noise by using the integer mode.
			SI5351C Configuration details (XTAL 25MHz):
			PLL_A = Unused
			PLL_B = 25 MHz * 32 = 800 MHz
			Channel 0 = 25MHz (Direct TCXO) R82x (Drive Strength 2mA)
			Channel 1 to 6 = Disabled/Not Used
			Channel 7 = 20MHz (PLL_B / 40) LPC4370 (Drive Strength 2mA)
			*/
			/* conf */
			{ 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0 - 9
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 10 - 19
				0x00, 0x00, 0x00, 0x6C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 20 - 29
				0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x0E, 0x00, 0x00, // 30 - 39
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 40 - 49
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 50 - 59
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 60 - 69
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 70 - 79
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 80 - 89
				0x00, 0x0A, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 90 - 99
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 100 - 109
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 110 - 119
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 120 - 129
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 130 - 139
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 140 - 149
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 150 - 159
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 160 - 169
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 170 - 179
			}
		},
		/* Conf 1 (HYDRASDR_SI5351C_CONFIG_CLKIN) HYDRASDR_RFONE CLKIN 10MHz CLK */
		{
			/*
			SI5351C Configuration details (CLKIN 10MHz):
			PLL_A = 10 MHz * 80 = 800 MHz
			PLL_B = 10 MHz * 64 = 640 MHz
			Channel 0 = 25MHz (PLL_A / 32) R82X (Drive Strength 2mA)
			Channel 1 to 6 = Disabled/Not Used
			Channel 7 = 20MHz (PLL_B / 32) LPC4370 (Drive Strength 2mA)
			*/
			/* conf */
			{
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0 - 9
				0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x4C, 0x00, 0x00, 0x00, // 10 - 19
				0x00, 0x00, 0x40, 0x6C, 0x00, 0x00, 0x00, 0x01, 0x00, 0x26, // 20 - 29
				0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x1E, 0x00, 0x00, // 30 - 39
				0x00, 0x00, 0x00, 0x01, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, // 40 - 49
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 50 - 59
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 60 - 69
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 70 - 79
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 80 - 89
				0x00, 0x08, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 90 - 99
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 100 - 109
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 110 - 119
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 120 - 129
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 130 - 139
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 140 - 149
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 150 - 159
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 160 - 169
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 170 - 179
			}
		}
	} /* End si5351c_conf_t si5351c_config[HYDRASDR_CONF_SI5351C_NB] */
};
