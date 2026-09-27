/*
 * Copyright 2012 Jared Boone
 * Copyright 2015 Ian Gilmour <ian@sdrsharp.com>
 * Copyright 2013-2024 Benjamin Vernoux <bvernoux@hydrasdr.com>
 *
 * This file is part of HydraSDR (based on AirSpy/HackRF project).
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
#include <libopencm3/lpc43xx/i2c.h>
#include <libopencm3/lpc43xx/gpio.h>
#include <libopencm3/lpc43xx/m0/nvic.h>
#include <libopencm3/lpc43xx/creg.h>
#include <libopencm3/lpc43xx/rgu.h>
#include <libopencm3/cm3/cortex.h>
#include <libopencm3/cm3/cortex.h>

#include <core.h>
#include <si5351c.h>
#include <r82x.h>
#include <w25q80bv.h>
#include <rom_iap.h>
#include <signal_mcu.h>

#include "usb.h"
#include "usb_standard_request.h"

#include "usb_device.h"
#include "usb_endpoint.h"
#include "usb_descriptor.h"

#include "hydrasdr_conf.h"
#include "usb_req.h"
#include "hydrasdr_commands.h"
#include "rx.h"
#include "r82x.h"
#include "m0.h"
#include "hydrasdr_stream.h"
#include "watchdog.h"
#include "uart.h"
#include "m0.hdr"

extern uint32_t cm0_data_share; /* defined in linker script */

volatile unsigned int phase = 0;

volatile hydrasdr_stream_state_t * const stream = HYDRASDR_STREAM_STATE;
static volatile uint32_t stream_epoch = 0;

volatile mcore_t *start_adchs = (mcore_t *)(&cm0_data_share);
volatile mcore_t *set_samplerate = (mcore_t *)((&cm0_data_share)+1);
volatile mcore_t *set_packing = (mcore_t *)((&cm0_data_share)+2);
volatile mcore_t *set_framing = (mcore_t *)((&cm0_data_share)+3);
volatile mcore_t *set_sof = (mcore_t *)((&cm0_data_share)+4);
volatile uint32_t *set_sof_divider = (uint32_t *)((&cm0_data_share)+5);

#define MASTER_TXEV_FLAG  ((uint32_t *) 0x40043130)
#define MASTER_TXEV_QUIT()  { *MASTER_TXEV_FLAG = 0x0; }


const char version_string[] = " " HYDRASDR_FW_GIT_TAG " " HYDRASDR_FW_CHECKIN_DATE;

typedef struct {
  uint32_t freq_hz;
  uint32_t divider;
} set_sample_r_params_t;

set_sample_r_params_t set_sample_r_params;

__attribute__ ((always_inline)) static inline void start_stop_adchs_m4(uint8_t conf_num, uint8_t command)
{
  start_adchs->conf = conf_num;
  start_adchs->cmd = command;

  signal_sev();

  /* Wait until M4 have finished executing the command (it set the data to 0) */
  while(1)
  {
    if(start_adchs->raw == 0)
      break;
  }
}

void set_samplerate_m4(uint8_t conf_num)
{
  set_samplerate->conf = conf_num;
  set_samplerate->cmd = SET_SAMPLERATE_CMD;

  signal_sev();

  /* Wait until M4 have finished executing the command (it set the data to 0) */
  while(1)
  {
    if(set_samplerate->raw == 0)
      break;
  }
}

void set_packing_m4(uint8_t state)
{
  set_packing->conf = state;
  set_packing->cmd = SET_PACKING_CMD;
  
  signal_sev();
  
  while(1)
  {
    if(set_packing->raw == 0)
      break;
  }
}

void set_framing_m4(uint8_t state)
{
  set_framing->conf = state;
  set_framing->cmd = SET_FRAMING_CMD;

  signal_sev();

  while(1)
  {
    if(set_framing->raw == 0)
      break;
  }
}

void set_sof_divider_m4(uint32_t divider)
{
  *set_sof_divider = divider;
  set_sof->cmd = SET_SOF_CMD;

  signal_sev();

  while(1)
  {
    if(set_sof->raw == 0)
      break;
  }
}

static void stream_write_header(uint8_t* chunk, uint32_t chunk_index, uint32_t chunk_samples)
{
  hydrasdr_frame_header_t* h = (hydrasdr_frame_header_t*)chunk;
  uint32_t i;

  h->sample_index = (uint64_t)chunk_index * chunk_samples;
  h->magic = HYDRASDR_FRAME_MAGIC;
  h->chunk_index = chunk_index;
  h->sample_count = chunk_samples;
  h->flags = (stream->packing == HYDRASDR_PACKING_ON) ? HYDRASDR_FRAME_FLAG_PACKED :
             (stream->packing == HYDRASDR_PACKING_TIMESTAMP_ON) ? (HYDRASDR_FRAME_FLAG_PACKED | HYDRASDR_FRAME_FLAG_TIMESTAMP) :
             (stream->packing == HYDRASDR_PACKING_8BIT) ? HYDRASDR_FRAME_FLAG_8BIT : 0;
  h->lost_chunks = stream->lost;
  h->overrun_chunks = stream->overruns;
  h->freq_hz = (uint32_t)set_freq_params.freq_hz;
  {
    uint32_t s1, s2;
    uint32_t tries = 16;
    do
    {
      s1 = stream->pps_seq;
      h->pps_sample_index_lo = stream->pps_index_lo;
      h->pps_sample_index_hi = stream->pps_index_hi;
      h->pps_fraction = stream->pps_fraction;
      h->pps_count = stream->pps_count;
      s2 = stream->pps_seq;
    } while(((s1 != s2) || (s1 & 1)) && --tries);
  }
  {
    uint32_t s1, s2;
    uint32_t tries = 16;
    do
    {
      s1 = stream->sof_seq;
      h->sof_sample_index_lo = stream->sof_index_lo;
      h->sof_sample_index_hi = stream->sof_index_hi;
      h->sof_fraction = stream->sof_fraction;
      h->sof_frame = stream->sof_frame;
      h->sof_count = stream->sof_count;
      s2 = stream->sof_seq;
    } while(((s1 != s2) || (s1 & 1)) && --tries);
  }
  h->uart_len = (uint8_t)hydrasdr_uart_read(h->uart_data, HYDRASDR_FRAME_UART_BYTES);
  for(i = 0; i < sizeof(h->reserved) / sizeof(h->reserved[0]); i++)
    h->reserved[i] = 0;
}

void usb_configuration_changed(usb_device_t* const device)
{
  if( device->configuration->number )
  {
    /* RECEIVER ON */
    set_receiver_mode(get_receiver_mode());
  } else
  {
    /* RECEIVER OFF */
    /* Configuration number equal 0 means usb bus reset. */
    set_receiver_mode(RECEIVER_MODE_OFF);
  }
}

void ADCHS_start(uint8_t conf_num)
{
  hydrasdr_uart_flush_rx();
  stream->delivered = 0;
  stream->lost = 0;
  stream->queued = 0;

  start_stop_adchs_m4(conf_num, START_ADCHS_CMD);

  //enable_r82x_power();

  /* Re-Init I2C0 & I2C1 after PLL1 frequency is modified (for I2C1 also because PowerOn on R82X) */
  i2c0_init(hydrasdr_conf->i2c_conf.i2c0_pll1_ls_hs_conf_val); /* Si5351C I2C peripheral */
  i2c1_init(hydrasdr_conf->i2c_conf.i2c1_pll1_hs_conf_val); /* R82X I2C peripheral */

  if((conf_num & HYDRASDR_SAMPLERATE_CONF_ALT) == HYDRASDR_SAMPLERATE_CONF_ALT)
  {
    conf_num = conf_num & (~HYDRASDR_SAMPLERATE_CONF_ALT);
    r82x_init(&hydrasdr_conf->r82x_conf_rw, hydrasdr_conf->m0_m4_alt_conf[conf_num].m0_conf.r82x_if_freq);
    r82x_set_if_bandwidth(&hydrasdr_conf->r82x_conf_rw, hydrasdr_conf->m0_m4_alt_conf[conf_num].m0_conf.r82x_if_bw);
  }else
  {
    r82x_init(&hydrasdr_conf->r82x_conf_rw, hydrasdr_conf->m0_m4_conf[conf_num].m0_conf.r82x_if_freq);
    r82x_set_if_bandwidth(&hydrasdr_conf->r82x_conf_rw, hydrasdr_conf->m0_m4_conf[conf_num].m0_conf.r82x_if_bw);
  }
  phase = 1;
  stream_epoch++;
}

void ADCHS_stop(uint8_t conf_num)
{
  r82x_standby();
  start_stop_adchs_m4(conf_num, STOP_ADCHS_CMD);
  set_framing_m4(0);
  set_sof_divider_m4(0);

  /* Re-Init I2C0 & I2C1 after PLL1 frequency is modified */
  i2c0_init(hydrasdr_conf->i2c_conf.i2c0_pll1_ls_hs_conf_val); /* Si5351C I2C peripheral */
  i2c1_init(hydrasdr_conf->i2c_conf.i2c1_pll1_ls_conf_val); /* R82X I2C peripheral */
  stream_epoch++;
}

static void stream_queue_chunks(void);
static volatile uint32_t stream_queued;
static volatile uint32_t stream_run_epoch;

void usb_bulk_in_transfer_complete(usb_endpoint_t* const endpoint)
{
  uint32_t before = usb_queue_active_count(endpoint);
  usb_queue_transfer_complete(endpoint);
  stream->delivered += before - usb_queue_active_count(endpoint);
  stream->usb_errors = usb_queue_transfer_errors;
  watchdog_feed();
  if(stream_run_epoch == stream_epoch && get_receiver_mode() == RECEIVER_MODE_RX)
    stream_queue_chunks();
  signal_sev();
}

static void stream_queue_chunks(void)
{
  uint32_t queued = stream_queued;
  uint32_t captured = stream->captured;
  uint32_t ring_chunks = stream->ring_chunks;
  uint32_t chunk_slots = stream->chunk_slots;
  uint32_t chunk_bytes = stream->chunk_bytes;
  uint32_t header_bytes = stream->header_bytes;
  uint32_t chunk_samples = stream->chunk_samples;
  int32_t pending = (int32_t)(captured - queued);

  if(pending < 0 || ring_chunks == 0)
    return;

  if((uint32_t)pending > ring_chunks - 1)
  {
    uint32_t skip = (uint32_t)pending - (ring_chunks - 1);
    stream->lost += skip;
    queued += skip;
  }

  while((int32_t)(captured - queued) > 0)
  {
    uint8_t* chunk = hydrasdr_stream_slot((queued % ring_chunks) * chunk_slots);
    if(header_bytes)
      stream_write_header(chunk, queued, chunk_samples);
    if(usb_transfer_schedule(&usb_endpoint_bulk_in, chunk, chunk_bytes) != 0)
      break;
    queued++;
  }

  stream->queued = queued;
  stream_queued = queued;
}

/* adchs_isr managed by M4 */
void m4core_isr(void)
{
  MASTER_TXEV_QUIT();
}

/*
M0 Core Manage USB 
*/
int main(void)
{
  iap_cmd_res_t iap_cmd_res;
  usb_descriptor_serial_number_t serial_number;
  usb_req_init();

  watchdog_init();
  hydrasdr_uart_init(HYDRASDR_UART_DEFAULT_BAUD);

  /* R82X Startup */
  r82x_startup(&hydrasdr_conf->r82x_conf_rw);

  usb_set_configuration_changed_cb(usb_configuration_changed);
  usb_peripheral_reset();

  usb_device_init(0, &usb_device);

  usb_queue_init(&usb_endpoint_control_out_queue);
  usb_queue_init(&usb_endpoint_control_in_queue);
  usb_queue_init(&usb_endpoint_bulk_out_queue);
  usb_queue_init(&usb_endpoint_bulk_in_queue);

  usb_endpoint_init(&usb_endpoint_control_out);
  usb_endpoint_init(&usb_endpoint_control_in);

  /* Read IAP Serial Number Identification */
  iap_cmd_res.cmd_param.command_code = IAP_CMD_READ_SERIAL_NO;
  iap_cmd_call(&iap_cmd_res);
  if(iap_cmd_res.status_res.status_ret == CMD_SUCCESS)
  {
    /* Only retrieve 2 last 32bits for Serial Number */
    serial_number.sn_32b[0] = iap_cmd_res.status_res.iap_result[2];
    serial_number.sn_32b[1] = iap_cmd_res.status_res.iap_result[3];
    usb_descriptor_fill_string_serial_number(serial_number);
  }

  nvic_set_priority(NVIC_USB0_IRQ, 255);
  
  nvic_set_priority(NVIC_M4CORE_IRQ, 1);
  nvic_enable_irq(NVIC_M4CORE_IRQ);

  usb_run(&usb_device);

  uint32_t epoch = stream_epoch;
  stream_queued = 0;
  stream_run_epoch = epoch;
  while(true)
  {
    signal_wfe();
    watchdog_heartbeat();
    deferred_job_run();
    if(epoch != stream_epoch)
    {
      cm_disable_interrupts();
      epoch = stream_epoch;
      stream_queued = 0;
      if(get_receiver_mode() == RECEIVER_MODE_RX)
    {
        usb_endpoint_flush(&usb_endpoint_bulk_in);
        stream_queued = stream->captured;
        stream->delivered = stream_queued;
        stream->lost = 0;
        stream->queued = stream_queued;
      }
      stream_run_epoch = epoch;
      cm_enable_interrupts();
      continue;
    }
    if(get_receiver_mode() != RECEIVER_MODE_RX)
      continue;
    cm_disable_interrupts();
    stream_queue_chunks();
    cm_enable_interrupts();
  }
}
