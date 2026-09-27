/*
 * Copyright 2015 Ian Gilmour <ian@sdrsharp.com>
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

#include <core.h>
#include <libopencm3/lpc43xx/creg.h>
#include <libopencm3/lpc43xx/rgu.h>
#include "adchs.h"

#define RESET_CTRL1_ADCHS_SHIFT (28)
#define RESET_CTRL1_ADCHS (1 << RESET_CTRL1_ADCHS_SHIFT)
#define STATUS0_CLEAR (0x7D)
#define STATUS0_CLEAR_MASK (0x7F)
#define STATUS1_CLEAR_MASK (0x1FFFFFFF)

typedef struct
{
  uint32_t src_addr;
  uint32_t dst_addr;
  uint32_t next_lli;
  uint32_t control;
} t_gpdma_lli;


/* Allocate aligned buffer on 16bytes for DMA LLI */
t_gpdma_lli adchs_dma_lli[ADCHS_DMA_NUM_LLI] __attribute__ ((aligned(16)));

void ADCHS_DMA_init_stop(void)
{
  uint32_t i;

  /* Halt channel 0 and let its FIFO drain before disabling it: a channel
     stopped under way (packing 2 ends its transfers on a partial burst)
     can leave the next stream stuck at its second linked-list item */
  if(LPC_GPDMA->C0CONFIG & 0x1)
  {
    LPC_GPDMA->C0CONFIG |= (0x1 << 18); /* Halt */
    for(i = 0; i < 100000 && (LPC_GPDMA->C0CONFIG & (0x1 << 17)); i++); /* Active */
    LPC_GPDMA->C0CONFIG &= ~0x1;
  }

  /* clear all interrupts on channel 0 */
  LPC_GPDMA->INTTCCLEAR = 0x01;
  LPC_GPDMA->INTERRCLR = 0x01;

  /* Setup the DMAMUX */
  CREG_DMAMUX &= ~(0x3<<(ADCHS_DMA_WRITE*2));
  CREG_DMAMUX |= 0x3<<(ADCHS_DMA_WRITE*2);
  CREG_DMAMUX &= ~(0x3<<(ADCHS_DMA_READ*2));
  CREG_DMAMUX |= 0x3<<(ADCHS_DMA_READ*2);

  LPC_GPDMA->CONFIG = 0x01; /* Enable DMA channels, little endian */
  while ( !(LPC_GPDMA->CONFIG & 0x01) );

  /* Disable Channel before to change settings */
  LPC_GPDMA->CONFIG = 0x00; /* Disable DMA channels, little endian */
}

void ADCHS_DMA_init(uint8_t packed, uint32_t header_bytes)
{
  const uint32_t slot_bytes = HYDRASDR_STREAM_SLOT_SIZE;
  uint32_t nb_dma_transfer;
  int i;

  ADCHS_DMA_init_stop();

  for(i=0; i<ADCHS_DMA_NUM_LLI; i++)
  {
    uint32_t offset = 0;
    uint32_t bytes = slot_bytes;

    if(packed == HYDRASDR_PACKING_8BIT)
    {
      offset = header_bytes;
      bytes = slot_bytes - 2 * header_bytes;
    }
    else if(packed)
    {
      offset = header_bytes;
      bytes = ((((slot_bytes / 4) * 3) - header_bytes) / 3) * 4;
    }
    else if((i & 1) == 0)
    {
      offset = header_bytes;
      bytes = slot_bytes - header_bytes;
    }

    // The size of the transfer is in multiples of 32bit copies (hence the /4)
    // and must be even multiples of ADC_FIFO_LEVEL.
    nb_dma_transfer = bytes / 4;

    adchs_dma_lli[i].src_addr = ADCHS_DMA_READ_SRC;
    adchs_dma_lli[i].dst_addr = (uint32_t)hydrasdr_stream_slot(i) + offset;
    /* Modulo with round rubin last LLI point to First in infinite loop */
    adchs_dma_lli[i].next_lli = (uint32_t)(&adchs_dma_lli[(i+1)%ADCHS_DMA_NUM_LLI]);

    if(packed == HYDRASDR_PACKING_TIMESTAMP_ON)
    {
      adchs_dma_lli[i].dst_addr += 4; // have to skip the packet header
      adchs_dma_lli[i].control = ( (nb_dma_transfer-1) << 0) |
                                 (0x2 << 12)  |
                                 (0x1 << 15)  | // 4x burst because of offset
                                 (0x2 << 18)  |
                                 (0x2 << 21)  |
                                 (0x1 << 24)  |
                                 (0x1 << 25)  |
                                 (0x0 << 26)  |
                                 (0x1 << 27)  |
                                 (0x0UL << 31);
    }
    else
    {
      adchs_dma_lli[i].control = ( (nb_dma_transfer) << 0) |
                                 (0x2 << 12)  |
                                 (0x2 << 15)  |
                                 (0x2 << 18)  |
                                 (0x2 << 21)  |
                                 (0x1 << 24)  |
                                 (0x1 << 25)  |
                                 (0x0 << 26)  |
                                 (0x1 << 27)  |
                                 (0x0UL << 31);
    }

    if(packed || (i & 1))
    {
      adchs_dma_lli[i].control |= (0x1UL << 31);
    }
  }

  LPC_GPDMA->C0SRCADDR = adchs_dma_lli[0].src_addr;
  LPC_GPDMA->C0DESTADDR = adchs_dma_lli[0].dst_addr;
  LPC_GPDMA->C0CONTROL = adchs_dma_lli[0].control;
  LPC_GPDMA->C0LLI     = (uint32_t)(&adchs_dma_lli[1]); // must be pointing to the second LLI as the first is used when initializing
  LPC_GPDMA->C0CONFIG =  (0x1)        |          // Enable bit
                         (ADCHS_DMA_READ << 1) |
                         (0x0 << 6)   |
                         (0x2 << 11)  |
                         (0x1 << 14)  |
                         (0x1 << 15);

  LPC_GPDMA->CONFIG = 0x01; /* Enable DMA channels, little endian */
}

void ADCHS_init_stop(void)
{
  /* Reset ADCHS using RGU */
  RESET_CTRL1 = RESET_CTRL1_ADCHS;
  /* Wait end of Reset */
  while( (RESET_ACTIVE_STATUS1 & RESET_CTRL1_ADCHS) != RESET_CTRL1_ADCHS );

  LPC_ADCHS->CLR_EN0 = STATUS0_CLEAR_MASK;
  LPC_ADCHS->CLR_STAT0 = STATUS0_CLEAR_MASK;

  while(LPC_ADCHS->STATUS0 & STATUS0_CLEAR);

  LPC_ADCHS->CLR_EN1 = STATUS1_CLEAR_MASK;
  LPC_ADCHS->CLR_STAT1 = STATUS1_CLEAR_MASK;
  while(LPC_ADCHS->STATUS1);

  LPC_ADCHS->POWER_DOWN = (0<<0);
  LPC_ADCHS->FLUSH = 1;
  LPC_ADCHS->FIFO_CFG = ADC_FIFO_LEVEL<<1 | 0x1;
}

void ADCHS_deinit(void)
{
  ADCHS_DMA_init_stop();
  ADCHS_init_stop();
  LPC_ADCHS->POWER_CONTROL = 0x0;
}

/* Initialized ADCHS for freq between 0 to less than 30MSPS */
static uint32_t adchs_crs = 0;
static uint32_t adchs_dgec = 0;

void ADCHS_set_sample_rate(uint32_t hz)
{
  if(hz <= 20000000) { adchs_crs = 0; adchs_dgec = 0x0; }
  else if(hz <= 30000000) { adchs_crs = 1; adchs_dgec = 0x0; }
  else if(hz <= 50000000) { adchs_crs = 2; adchs_dgec = 0x0; }
  else if(hz <= 65000000) { adchs_crs = 3; adchs_dgec = 0xF; }
  else { adchs_crs = 4; adchs_dgec = 0xE; }
}

void ADCHS_init(void)
{
  uint32_t i;

  ADCHS_init_stop();

  LPC_ADCHS->CONFIG =
  ( 0x1 << 0 ) |
  ( 0x0 << 2 ) |
  ( 0x0  << 4 ) |
  ( 0x0  << 5 ) |
  ( 0x90 << 6 );

  /* Configure and Enable ADCHS for fADC less than 30MS/s */
  LPC_ADCHS->POWER_CONTROL =
  adchs_crs |
  (0x1 << 4)   |
  (0x1 << 10)  |
  (0 << 16)    |
  (1 << 17)    |
  (1 << 18);
  LPC_ADCHS->ADC_SPEED = adchs_dgec * 0x111111; /* the same DGEC for all six channels */

  LPC_ADCHS->FLUSH = 1;

  for(i = 0; i < 5; i++ )
  {
    while( LPC_ADCHS->FIFO_STS ); /* Wait until FIFO empty. */
  }

  /* Configure Threshold A & B to default value (even if not used) */
  LPC_ADCHS->THR_A = 0x00FFF000;
  LPC_ADCHS->THR_B = 0x00FFF000;

  /* Configure Interrupt 0 & 1 Enable register to default value (even if not used) */
  LPC_ADCHS->CLR_EN0   = STATUS0_CLEAR_MASK;
  LPC_ADCHS->CLR_STAT0 = STATUS0_CLEAR_MASK;
  LPC_ADCHS->CLR_EN1   = STATUS1_CLEAR_MASK;
  LPC_ADCHS->CLR_STAT1 = STATUS1_CLEAR_MASK;

}

void ADCHS_desc_init(uint8_t chan_num)
{
  LPC_ADCHS->DSCR_STS =
    (1<<0) |
    (0<<1);

  LPC_ADCHS->DESCRIPTOR_1[0] =
  (chan_num << 0) |
  (0 << 3)        |
  (0 << 4)        |
  (0 << 5)        |
  (0x2 << 6)      |
  (1 << 8)        |
  (0 << 22)       |
  (1 << 24)       |
  (0x1U << 31);

  LPC_ADCHS->DESCRIPTOR_0[0] =
  (chan_num << 0) |
  (0 << 3)        |
  (0 << 4)        |
  (0 << 5)        |
  (0x1 << 6)      |
  (0 << 8)        |
  (0 << 22)       |
  (1 << 24)       |
  (0x1U << 31);

}

