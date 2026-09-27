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

#ifndef __HYDRASDR_WATCHDOG_H
#define __HYDRASDR_WATCHDOG_H

#include <stdint.h>
#include "hydrasdr_commands.h"

/* LPC4370 windowed watchdog, always armed */

/* M4: arm as early as possible; nothing else to do on the M4 */
void watchdog_arm(void);

void watchdog_init(void); /* adopt the watchdog the M4 armed, enable the warning interrupt */
void watchdog_heartbeat(void);     /* main loop: called on every iteration */
void watchdog_feed(void); /* direct feed, for the host request and long flash waits */
void watchdog_host_contact(void); /* an application started a stream or fed explicitly */
void watchdog_get_status(hydrasdr_watchdog_status_t* status);

#endif /* __HYDRASDR_WATCHDOG_H */
