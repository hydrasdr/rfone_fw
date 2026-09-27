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

#include <string.h>

#include <libopencm3/lpc43xx/cgu.h>
#include <libopencm3/lpc43xx/ccu.h>
#include <libopencm3/lpc43xx/gpio.h>
#include <libopencm3/lpc43xx/m4/nvic.h>
#include <libopencm3/lpc43xx/creg.h>
#include <libopencm3/lpc43xx/rgu.h>
#include <libopencm3/lpc43xx/ipc.h>
#include <libopencm3/cm3/scs.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/cm3/vector.h>

#include <core.h>
#include <si5351c.h>
#include <w25q80bv.h>
#include <rom_iap.h>
#include <signal_mcu.h>

#include "adchs.h"
#include "hydrasdr_debug.h"
#include "timing.h"

#include "m0_bin.h"
#include "m0s_bin.h"

#include "hydrasdr_conf.h"
#include <hydrasdr_stream.h>
#include <watchdog.h>

#define DEFAULT_ADCHS_CHAN (0)

#undef DMA_ISR_DEBUG
//#define DMA_ISR_DEBUG

#define SLAVE_TXEV_FLAG ((uint32_t *) 0x40043400)
#define SLAVE_TXEV_QUIT() { *SLAVE_TXEV_FLAG = 0x0; }

extern uint32_t adchs_data; /* defined in linker script */
extern uint32_t cm0_data_share; /* defined in linker script */

volatile int adchs_stopped = 0;
volatile int adchs_started = 0;

volatile hydrasdr_packing_type use_packing = HYDRASDR_PACKING_OFF;
volatile int use_framing = 0;

volatile hydrasdr_stream_state_t * const stream = HYDRASDR_STREAM_STATE;

volatile uint32_t dma_chunks_done = 0;
static uint32_t packed_chunks = 0;
static volatile uint32_t adchs_epoch = 0;


volatile mcore_t *start_adchs = (mcore_t *)(&cm0_data_share);
volatile mcore_t *set_samplerate = (mcore_t *)((&cm0_data_share)+1);
volatile mcore_t *set_packing = (mcore_t *)((&cm0_data_share)+2);
volatile mcore_t *set_framing = (mcore_t *)((&cm0_data_share)+3);
volatile mcore_t *set_sof = (mcore_t *)((&cm0_data_share)+4);
volatile uint32_t *set_sof_divider = (uint32_t *)((&cm0_data_share)+5);

volatile int first_start = 0;

/*
uint32_t nb_cycles[5] = { 0 };
uint32_t data_counter = 0;
*/
#ifdef DMA_ISR_DEBUG
  #define DMA_IRQ_CYCLES_MAX (100)
  #define FREQ_DMA_IRQ_CYCLES_MAX (100)
  typedef struct
  {
    uint32_t adchs_fifo_ovf;
    uint32_t adchs_dscr_error;
    uint32_t adchs_adc_ovf;
    uint32_t adchs_adc_unf;
    uint32_t dma_err_cnt;

    uint32_t dma_irq_cycles[DMA_IRQ_CYCLES_MAX];
    uint32_t dma_irq_cycles_idx;

    uint32_t freq_dma_irq_cycles[FREQ_DMA_IRQ_CYCLES_MAX];
    uint32_t freq_dma_irq_cycles_idx;
  } t_stats_adchs;

  t_stats_adchs stat_adchs = { 0 };
#endif

/*
__attribute__ ((always_inline)) static void pack(uint16_t* input, uint32_t* output, uint32_t length)
{
  uint32_t i;
    
  for (i = 0; i < length; i += 8)
  {    
    register uint32_t t2, t5;
    
    t2 = input[i+2];      
        
    output[0] = ((uint32_t)(input[i] << 20)) | ((uint32_t)(input[i+1] << 8)) | (t2 >> 4);    
    t5 = input[i+5];
    output[1] = ((uint32_t)(t2&0xf) << 28)| ((uint32_t)input[i+3] << 16) | (input[i+4] << 4) | (t5 >> 8);
    output[2] = ((uint32_t)(t5 & 0xff) << 24) | ((uint32_t)input[i+6]<<12) | ((uint32_t)input[i+7]);

    output += 3;
  }
}*/

__attribute__ ((always_inline)) static void pack(uint32_t* input, uint32_t* output, uint32_t length)
{
  register uint32_t *a0 asm("r0") = input;
  register uint32_t *a1 asm("r1") = output;
  register uint32_t a2 asm("r2") = length;

  asm volatile("1:\n\t"
         "ldm.w %0!, {r4, r5, r6, r7}\n\t"
         
         "lsr	r8, r4, #16\n\t"
         "ubfx	r3, r5, #4, #12\n\t"
         "orr	r8, r3, r8, lsl #8\n\t"
         "orr	r8, r8, r4, lsl #20\n\t"
         "lsrs	r3, r5, #16\n\t"
         "lsls	r5, r5, #28\n\t"
         "orr	r5, r5, r3, lsl #16\n\t"
         "orr	r5, r5, r6, lsr #24\n\t"
         "uxth	r9, r6\n\t"
         "orr	r9, r5, r9, lsl #4\n\t"
         "lsrs	r6, r6, #16\n\t"
         "uxth	r10, r7\n\t"
         "lsl	r10, r10, #12\n\t"
         "orr	r10, r10, r6, lsl #24\n\t"	
         "orr	r10, r10, r7, lsr #16\n\t"

         "stm.w %1!, {r8, r9, r10}\n\t"

         "subs	%2, %2, #8\n\t"
         "bne 1b\n\t"
        : "+r"(a0), "+r"(a1), "+r"(a2)
        :: "memory", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10");
        
}

__attribute__ ((always_inline)) static void pack8(uint32_t* input, uint32_t* output, uint32_t length)
{
  register uint32_t *a0 asm("r0") = input;
  register uint32_t *a1 asm("r1") = output;
  register uint32_t a2 asm("r2") = length;

  asm volatile("1:\n\t"
         "ldm.w %0!, {r4, r5, r6, r7, r8, r9, r10, r12}\n\t"
         "lsr r4, r4, #4\n\t"
         "lsr r5, r5, #4\n\t"
         "lsr r6, r6, #4\n\t"
         "lsr r7, r7, #4\n\t"
         "lsr r8, r8, #4\n\t"
         "lsr r9, r9, #4\n\t"
         "lsr r10, r10, #4\n\t"
         "lsr r12, r12, #4\n\t"
         "uxtb16 r4, r4\n\t"
         "uxtb16 r5, r5\n\t"
         "uxtb16 r6, r6\n\t"
         "uxtb16 r7, r7\n\t"
         "uxtb16 r8, r8\n\t"
         "uxtb16 r9, r9\n\t"
         "uxtb16 r10, r10\n\t"
         "uxtb16 r12, r12\n\t"
         "orr r4, r4, r4, lsr #8\n\t"
         "orr r5, r5, r5, lsr #8\n\t"
         "orr r6, r6, r6, lsr #8\n\t"
         "orr r7, r7, r7, lsr #8\n\t"
         "orr r8, r8, r8, lsr #8\n\t"
         "orr r9, r9, r9, lsr #8\n\t"
         "orr r10, r10, r10, lsr #8\n\t"
         "orr r12, r12, r12, lsr #8\n\t"
         "pkhbt r4, r4, r5, lsl #16\n\t"
         "pkhbt r5, r6, r7, lsl #16\n\t"
         "pkhbt r6, r8, r9, lsl #16\n\t"
         "pkhbt r7, r10, r12, lsl #16\n\t"
         "stm.w %1!, {r4, r5, r6, r7}\n\t"
         "subs %2, %2, #16\n\t"
         "bne 1b\n\t"
        : "+r"(a0), "+r"(a1), "+r"(a2)
        :: "memory", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r12");
}

static __inline__ void stream_reset(void)
{
  dma_chunks_done = 0;
  packed_chunks = 0;
  stream->captured = 0;
  stream->overruns = 0;
  stream->backlog_max = 0;
  stream->m0_lag_max = 0;
  stream->dma_errors = 0;
  stream->adc_overflows = 0;
  adchs_epoch++;
}

static __inline__ uint32_t get_start_stop_adchs(void)
{
  return(start_adchs->cmd);
}

/* Acknowledge Start/Stop ADCHS by clearing the data */
static __inline__ void ack_start_stop_adchs(void)
{
  start_adchs->raw  = 0;
}

static __inline__ uint8_t get_samplerate(uint8_t *conf_number)
{
  *conf_number = set_samplerate->conf;
  return(set_samplerate->cmd);
}

/* Acknowledge set_samplerate by clearing the data */
static __inline__ void ack_samplerate(void)
{
  set_samplerate->raw = 0;
}

static __inline__ uint8_t get_packing(uint8_t *packing_state)
{
  *packing_state = set_packing->conf;
  return(set_packing->cmd);
}

static __inline__ void ack_packing(void)
{
  set_packing->raw = 0;
}

static __inline__ uint8_t get_framing(uint8_t *framing_state)
{
  *framing_state = set_framing->conf;
  return(set_framing->cmd);
}

static __inline__ void ack_framing(void)
{
  set_framing->raw = 0;
}

static __inline__ uint8_t get_sof(void)
{
  return(set_sof->cmd);
}

static __inline__ void ack_sof(void)
{
  set_sof->raw = 0;
}

static void publish_stream_format(void)
{
  uint32_t header = use_framing ? HYDRASDR_FRAME_HEADER_SIZE : 0;

  stream->header_bytes = header;
  stream->packing = use_packing;
  if(use_packing == HYDRASDR_PACKING_OFF)
  {
    stream->chunk_bytes = HYDRASDR_STREAM_CHUNK_BYTES_UNPACKED;
    stream->chunk_slots = 2;
    stream->ring_chunks = HYDRASDR_STREAM_NUM_SLOTS / 2;
    stream->chunk_samples = (HYDRASDR_STREAM_CHUNK_BYTES_UNPACKED - header) / 2;
  }
  else if(use_packing == HYDRASDR_PACKING_8BIT)
  {
    stream->chunk_bytes = HYDRASDR_STREAM_CHUNK_BYTES_8BIT;
    stream->chunk_slots = 1;
    stream->ring_chunks = HYDRASDR_STREAM_NUM_SLOTS;
    stream->chunk_samples = HYDRASDR_STREAM_CHUNK_BYTES_8BIT - header;
  }
  else
  {
    stream->chunk_bytes = HYDRASDR_STREAM_CHUNK_BYTES_PACKED;
    stream->chunk_slots = 1;
    stream->ring_chunks = HYDRASDR_STREAM_NUM_SLOTS;
    stream->chunk_samples = ((HYDRASDR_STREAM_CHUNK_BYTES_PACKED - header) / 3) * 2;
    if(use_packing == HYDRASDR_PACKING_TIMESTAMP_ON)
      stream->chunk_samples -= HYDRASDR_STREAM_TIMESTAMP_SAMPLES;
  }
}

void set_packing_state(uint8_t state)
{
  use_packing = (state > HYDRASDR_PACKING_8BIT) ? HYDRASDR_PACKING_ON : (hydrasdr_packing_type)state;
  publish_stream_format();
}

void set_framing_state(uint8_t state)
{
  use_framing = state ? 1 : 0;
  publish_stream_format();
}

static uint32_t adchs_sample_rate_hz;

static uint32_t timing_timer_clock_hz(void)
{
  const core_clock_pll1_hs_t* pll1 = &hydrasdr_conf->m4_init_conf.pll1_hs;
  uint32_t gp_clkin_hz = (hydrasdr_conf->conf_hw.hardware_type & HW_FEATURE_SI5351C) ? 20000000 : 24000000;
  return (gp_clkin_hz / (pll1->pll1_hs_nsel + 1)) * (pll1->pll1_hs_msel + 1);
}

void adchs_start(uint8_t chan_num)
{
  int i;
  uint32_t *dst;

  /* Disable IRQ globally */
  __asm__("cpsid i");

  if(first_start == 0)
  {
    cpu_clock_pll1_high_speed(&hydrasdr_conf->m4_init_conf.pll1_hs);
    first_start = 1;
  }

  /* Clear ADCHS Buffer */
  for(i=0; i<HYDRASDR_STREAM_NUM_PIECES; i++)
  {
    uint32_t k;
    dst = (uint32_t *)hydrasdr_stream_pieces[i];
    for(k=0; k<(HYDRASDR_STREAM_PIECE_SIZE/4); k++)
    {
      dst[k] = 0;
    }
  }
  stream_reset();

  ADCHS_init();
  ADCHS_desc_init(chan_num);
  ADCHS_DMA_init(use_packing, stream->header_bytes);

  led_on();
  timing_stream_start(adchs_sample_rate_hz, timing_timer_clock_hz());
  LPC_ADCHS->TRIGGER = 1;
  __asm("dsb");
  
  /* Enable IRQ globally */
  __asm__("cpsie i");
}

void adchs_stop(void)
{
  /* Disable IRQ globally */
  __asm__("cpsid i");

  ADCHS_deinit();
  timing_stream_stop();

//  cpu_clock_pll1_low_speed(&hydrasdr_conf->m4_init_conf.pll1_ls);

  led_off();

  /* Enable IRQ globally */
  __asm__("cpsie i");
}

__attribute__ ((always_inline)) static inline void dma_chunk_done(uint32_t adc_status)
{
  uint32_t produced = dma_chunks_done + 1;
  uint32_t queued = stream->queued;
  uint32_t consumed = stream->delivered + stream->lost;
  uint32_t backlog = produced - consumed;
  uint32_t overwritten = produced - stream->ring_chunks;

  if(use_packing == HYDRASDR_PACKING_TIMESTAMP_ON)
  {
    /* The word the DMA left free ahead of the samples packs into two 12-bit values: the chunk counter and the ADC status */
    uint32_t* ts = (uint32_t*)(hydrasdr_stream_slot(dma_chunks_done % HYDRASDR_STREAM_NUM_SLOTS) + stream->header_bytes);
    *ts = (dma_chunks_done & 4095) | (adc_status << 16);
  }

  dma_chunks_done = produced;

  if(queued != 0)
  {
    uint32_t m0_lag = produced - queued;

    if((int32_t)backlog < 0)
      backlog = 0;

    if(backlog > stream->backlog_max)
      stream->backlog_max = backlog;

    if(queued - consumed < HYDRASDR_STREAM_USB_POOL && (int32_t)m0_lag > 0 && m0_lag > stream->m0_lag_max)
      stream->m0_lag_max = m0_lag;

    if((int32_t)(overwritten - consumed) > 0 && (int32_t)(queued - overwritten) > 0)
      stream->overruns++;
  }

  if(use_packing == HYDRASDR_PACKING_OFF)
  {
    stream->captured = produced;
    signal_sev();
  }
}

void dma_isr(void) 
{
  uint32_t status;
  uint32_t adc_status;
  #define INTTC0  (1)

#ifdef DMA_ISR_DEBUG
  volatile uint32_t tmp_cycles;
  tmp_cycles = SCS_DWT_CYCCNT;

  stat_adchs.freq_dma_irq_cycles[stat_adchs.freq_dma_irq_cycles_idx] = tmp_cycles;
  stat_adchs.freq_dma_irq_cycles_idx++;
  if(stat_adchs.freq_dma_irq_cycles_idx == FREQ_DMA_IRQ_CYCLES_MAX)
    stat_adchs.freq_dma_irq_cycles_idx = 0;

  status = LPC_ADCHS->STATUS0;
  LPC_ADCHS->CLR_STAT0 = status;

  // ADCHS Error stat0

  /* FIFO was full; conversion sample is not stored and lost */
  if(status & STAT0_FIFO_OVERFLOW) 
    stat_adchs.adchs_fifo_ovf++;

  /* The ADC was not fully woken up when a sample was
     converted and the conversion results is unreliable */
  if(status & STAT0_DSCR_ERROR)
    stat_adchs.adchs_dscr_error++;

  /* Converted sample value was over range of the 12 bit output code. */
  if(status & STAT0_ADC_OVF)
    stat_adchs.adchs_adc_ovf++;

  /* Converted sample value was under range of the 12 bit output code. */
  if(status & STAT0_ADC_UNF)
    stat_adchs.adchs_adc_unf++;

  // DMA Error
  status = LPC_GPDMA->INTERRSTAT;
  if( status )
  {
    stat_adchs.dma_err_cnt++; // Count DMA Error
    LPC_GPDMA->INTERRCLR |= status;
  } 
#endif

  adc_status = LPC_ADCHS->STATUS0;
  if(adc_status & STAT0_FIFO_OVERFLOW)
    stream->adc_overflows++;
  /* The timestamp mode reports every status bit once per chunk */
  LPC_ADCHS->CLR_STAT0 = (use_packing == HYDRASDR_PACKING_TIMESTAMP_ON) ? adc_status : (adc_status & STAT0_FIFO_OVERFLOW);

  status = LPC_GPDMA->INTERRSTAT;
  if(status)
  {
    LPC_GPDMA->INTERRCLR = status;
    stream->dma_errors++;
  }

  status = LPC_GPDMA->INTTCSTAT;
  if( status & INTTC0 )
  {
    LPC_GPDMA->INTTCCLEAR |= INTTC0; /* Clear Chan0 */

    dma_chunk_done(adc_status);
  }

#ifdef DMA_ISR_DEBUG
  stat_adchs.dma_irq_cycles[stat_adchs.dma_irq_cycles_idx] = (SCS_DWT_CYCCNT - tmp_cycles);
  stat_adchs.dma_irq_cycles_idx++;
  if(stat_adchs.dma_irq_cycles_idx == DMA_IRQ_CYCLES_MAX)
    stat_adchs.dma_irq_cycles_idx = 0;
#endif
}

void m0core_isr(void)
{
  uint8_t adchs_conf;
  uint8_t adchs_start_stop_cmd;
  uint8_t samplerate_cmd;
  uint8_t packing_cmd;
  uint8_t packing_state;
  uint8_t framing_cmd;
  uint8_t framing_state;

  SLAVE_TXEV_QUIT();

  samplerate_cmd = get_samplerate(&adchs_conf);
  if(samplerate_cmd == SET_SAMPLERATE_CMD)
  {
    if((adchs_conf & HYDRASDR_SAMPLERATE_CONF_ALT) == HYDRASDR_SAMPLERATE_CONF_ALT)
    {
      adchs_conf = adchs_conf & (~HYDRASDR_SAMPLERATE_CONF_ALT);
      sys_clock_samplerate(&hydrasdr_conf->m0_m4_alt_conf[adchs_conf].hydrasdr_m4_conf);
      adchs_sample_rate_hz = hydrasdr_conf->m0_m4_alt_conf[adchs_conf].m0_conf.r82x_if_freq * 4;
    }else
    {
      sys_clock_samplerate(&hydrasdr_conf->m0_m4_conf[adchs_conf].hydrasdr_m4_conf);
      adchs_sample_rate_hz = hydrasdr_conf->m0_m4_conf[adchs_conf].m0_conf.r82x_if_freq * 4;
    }
    ADCHS_set_sample_rate(adchs_sample_rate_hz);
    ack_samplerate();
  }
  
  packing_cmd = get_packing(&packing_state);
  if(packing_cmd == SET_PACKING_CMD)
  {
    set_packing_state(packing_state);
    ack_packing();
  }

  framing_cmd = get_framing(&framing_state);
  if(framing_cmd == SET_FRAMING_CMD)
  {
    set_framing_state(framing_state);
    ack_framing();
  }

  if(get_sof() == SET_SOF_CMD)
  {
    timing_set_sof_divider(*set_sof_divider);
    ack_sof();
  }

  adchs_start_stop_cmd = get_start_stop_adchs();
  switch(adchs_start_stop_cmd)
  {
    case START_ADCHS_CMD:
      if(adchs_started == 0)
      {
        adchs_start(DEFAULT_ADCHS_CHAN);
        adchs_started = 1;
        adchs_stopped = 0;
      }
      ack_start_stop_adchs();
    break;

    case STOP_ADCHS_CMD:
      if(adchs_stopped == 0)
      {
        adchs_stop();
        adchs_stopped = 1;
        adchs_started = 0;
      }
      ack_start_stop_adchs();
    break;

    default:
    /* Invalid command do nothing */
    break;
  }
}

void m0_startup(void)
{
  uint32_t *src, *dest;

  /* Halt M0 core (in case it was running) */
  ipc_halt_m0();

  /* Copy M0 code from M4 embedded addr to final addr M0 */
  dest = &cm0_exec_baseaddr;
  for(src = (uint32_t *)&m0_bin[0]; src < (uint32_t *)(&m0_bin[0]+m0_bin_size); )
  {
    *dest++ = *src++;
  }

  ipc_start_m0( (uint32_t)(&cm0_exec_baseaddr) );
}

void m0s_startup(void)
{
  uint32_t *src, *dest;

  /* Halt M0 core (in case it was running) */
  ipc_halt_m0s();

  /* Copy M0 code from M4 embedded addr to final addr M0 */
  dest = &cm0s_exec_baseaddr;
  for(src = (uint32_t *)&m0s_bin[0]; src < (uint32_t *)(&m0s_bin[0]+m0s_bin_size); )
  {
    *dest++ = *src++;
  }

  ipc_start_m0s( (uint32_t)(&cm0s_exec_baseaddr) );
}

void scs_dwt_cycle_counter_enabled(void)
{
  SCS_DEMCR |= SCS_DEMCR_TRCENA;
  SCS_DWT_CTRL  |= SCS_DWT_CTRL_CYCCNTENA;
}

int main(void)
{
  SCB_VTOR = (uint32_t)&vector_table;

  watchdog_arm();

  scs_dwt_cycle_counter_enabled();
  pin_setup();
  sys_clock_init();

  nvic_set_priority(NVIC_DMA_IRQ, 255);
  nvic_set_priority(NVIC_M0CORE_IRQ, 1);

  stream_reset();

  nvic_enable_irq(NVIC_DMA_IRQ);
  nvic_enable_irq(NVIC_M0CORE_IRQ);

  adchs_sample_rate_hz = hydrasdr_conf->m0_m4_conf[0].m0_conf.r82x_if_freq * 4;
  ADCHS_set_sample_rate(adchs_sample_rate_hz);
  timing_init();
  HYDRASDR_DEBUG_MAILBOX->pending = 0; /* RAM is not cleared at boot */

  adchs_stop();
  adchs_stopped = 1;
  adchs_started = 0;
  
  set_packing_state(0);

  ack_start_stop_adchs();
  ack_samplerate();
  ack_packing();
  ack_framing();
  ack_sof();

  /* Start M0 */
  m0_startup();

#undef ENABLE_M0S
#ifdef ENABLE_M0S
  /* Start M0s */
  m0s_startup();
#else
  // Halt M0s
  ipc_halt_m0s();
  // Disable M0 Sub
  CCU1_CLK_PERIPH_CORE_CFG &= ~(1);
#endif

  while(true)
  {
    signal_wfe();

    if(HYDRASDR_DEBUG_MAILBOX->pending)
    {
      typedef uint32_t (*dbg_call_fn_t)(uint32_t, uint32_t, uint32_t, uint32_t);
      dbg_call_fn_t fn = (dbg_call_fn_t)(HYDRASDR_DEBUG_MAILBOX->address | 1);
      HYDRASDR_DEBUG_MAILBOX->result = fn(HYDRASDR_DEBUG_MAILBOX->args[0], HYDRASDR_DEBUG_MAILBOX->args[1], HYDRASDR_DEBUG_MAILBOX->args[2], HYDRASDR_DEBUG_MAILBOX->args[3]);
      HYDRASDR_DEBUG_MAILBOX->pending = 0;
    }

    if(use_packing)
    {
      /* Thanks to Pierre HB9FUF for the initial packing proof-of-concept */
      while((int32_t)(dma_chunks_done - packed_chunks) > 0)
      {
        uint32_t epoch = adchs_epoch;
        uint32_t chunk = packed_chunks;
        uint32_t* slot = (uint32_t*)(hydrasdr_stream_slot(chunk % HYDRASDR_STREAM_NUM_SLOTS) + stream->header_bytes);

        if(use_packing == HYDRASDR_PACKING_8BIT)
          pack8(slot, slot, stream->chunk_samples);
        else if(use_packing == HYDRASDR_PACKING_TIMESTAMP_ON)
          pack(slot, slot, stream->chunk_samples + HYDRASDR_STREAM_TIMESTAMP_SAMPLES);
        else
          pack(slot, slot, stream->chunk_samples);

        if(epoch != adchs_epoch)
          break;

        packed_chunks = chunk + 1;
        stream->captured = chunk + 1;
        signal_sev();
      }
    }
  }
}
