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

#include "watchdog.h"

#include <libopencm3/cm3/cortex.h>
#include <libopencm3/lpc43xx/ccu.h>
#include <libopencm3/lpc43xx/wwdt.h>

#define WWDT_MOD_WDEN     (1 << 0)
#define WWDT_MOD_WDRESET  (1 << 1) /* time-out resets the chip */
#define WWDT_MOD_WDTOF    (1 << 2) /* time-out happened; cleared by writing 0 */
#define WWDT_MOD_WDINT    (1 << 3)

#define WATCHDOG_COUNTS_PER_MS (3000)
#define WATCHDOG_TIMEOUT_COUNTS (0xFFFFFF)
#define WATCHDOG_WARNING_COUNTS (0x3FF)

void watchdog_feed(void)
{
  cm_disable_interrupts();
  WWDT_FEED = 0xAA;
  WWDT_FEED = 0x55;
  cm_enable_interrupts();
}

static void watchdog_start(void)
{
  int i;

  CCU1_CLK_M4_WWDT_CFG |= 1;
  while((CCU1_CLK_M4_WWDT_STAT & 1) == 0);

  for(i = 0; i < 16; i++)
  {
    WWDT_TC = WATCHDOG_TIMEOUT_COUNTS;
    if(WWDT_TC == WATCHDOG_TIMEOUT_COUNTS)
      break;
  }
  WWDT_MOD = WWDT_MOD_WDEN | WWDT_MOD_WDRESET | (WWDT_MOD & WWDT_MOD_WDTOF); /* WDTOF kept for reporting */
  watchdog_feed(); /* starts the countdown from TC */
}

#if defined(LPC43XX_M4)

void watchdog_arm(void)
{
  watchdog_start();
}

#elif defined(LPC43XX_M0)

#include <libopencm3/lpc43xx/m0/nvic.h>
#include "hydrasdr_stream.h"

receiver_mode_t get_receiver_mode(void); /* m0/rx.c */

static uint32_t watchdog_flags = 0;
static volatile uint32_t heartbeat = 0;
static uint32_t heartbeat_seen = 0;
static uint32_t captured_seen = 0;
#ifdef HYDRASDR_HOST_WATCHDOG
static volatile uint8_t host_contact = 0;
static volatile uint8_t self_feeds_left = HYDRASDR_WATCHDOG_SELF_FEEDS;
#endif

void watchdog_heartbeat(void)
{
  heartbeat++;
}

void watchdog_host_contact(void)
{
#ifdef HYDRASDR_HOST_WATCHDOG
  host_contact = 1;
#endif
}

void ritimer_or_wwdt_isr(void)
{
  uint32_t beat = heartbeat;
  uint32_t captured = HYDRASDR_STREAM_STATE->captured;
  uint8_t healthy = (beat != heartbeat_seen);

  WWDT_MOD = (WWDT_MOD & ~WWDT_MOD_WDTOF) | WWDT_MOD_WDINT;

  if(get_receiver_mode() == RECEIVER_MODE_RX && captured == captured_seen)
    healthy = 0;
  heartbeat_seen = beat;
  captured_seen = captured;

  if(!healthy)
    return; /* let it expire */

#ifdef HYDRASDR_HOST_WATCHDOG
  if(host_contact == 0 && self_feeds_left > 0)
  {
    self_feeds_left--;
    watchdog_feed();
  }
#else
  watchdog_feed();
#endif
}

void watchdog_init(void)
{
  if((WWDT_MOD & WWDT_MOD_WDEN) == 0)
  {
    watchdog_start();
  }

  watchdog_flags = 0;
  if(WWDT_MOD & WWDT_MOD_WDEN)
  {
    watchdog_flags |= HYDRASDR_WATCHDOG_FLAG_ARMED;
  }
#ifdef HYDRASDR_HOST_WATCHDOG
  watchdog_flags |= HYDRASDR_WATCHDOG_FLAG_STRICT;
#endif
  if(WWDT_MOD & WWDT_MOD_WDTOF)
  {
    watchdog_flags |= HYDRASDR_WATCHDOG_FLAG_RESET_BY_WATCHDOG;
  }

  heartbeat_seen = heartbeat - 1;
  captured_seen = HYDRASDR_STREAM_STATE->captured - 1;

  WWDT_WARNINT = WATCHDOG_WARNING_COUNTS;
  WWDT_MOD = (WWDT_MOD & ~WWDT_MOD_WDTOF) | WWDT_MOD_WDINT; /* clear WDTOF and any stale warning */
  nvic_set_priority(NVIC_RITIMER_OR_WWDT_IRQ, 0);
  nvic_enable_irq(NVIC_RITIMER_OR_WWDT_IRQ);
}

void watchdog_get_status(hydrasdr_watchdog_status_t* status)
{
  status->flags = watchdog_flags;
  status->timeout_ms = WATCHDOG_TIMEOUT_COUNTS / WATCHDOG_COUNTS_PER_MS;
  status->remaining_ms = WWDT_TV / WATCHDOG_COUNTS_PER_MS;
#ifdef HYDRASDR_HOST_WATCHDOG
  if(host_contact)
    status->flags |= HYDRASDR_WATCHDOG_FLAG_HOST_OWNED;
  status->self_feeds_left = self_feeds_left;
#else
  status->self_feeds_left = 0xFFFFFFFF; /* unlimited */
#endif
}

#endif
