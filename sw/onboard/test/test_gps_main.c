/*
 * Copyright (C) 2008 Antoine Drouin
 * Copyright (C) 2009 John Stowers
 *
 * This file is part of wasp, some code taken from paparazzi (GPL)
 *
 * wasp is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * wasp is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with paparazzi; see the file COPYING.  If not, write to
 * the Free Software Foundation, 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */
#include "std.h"
#include "init.h"
#include "sys_time.h"
#include "led.h"
#include "comm.h"
#include "comm_autopilot.h"
#include "gps.h"
#include "generated/messages.h"
#include "generated/settings.h"

static inline void main_init( void );
static inline void main_periodic_task( void );
static inline void main_event_task( void );

int main( void ) {
  main_init();
  while(1) {
    if (sys_time_periodic())
      main_periodic_task();
    main_event_task();
  }
  return 0;
}

static inline void main_init( void ) {
  hw_init();
  sys_time_init();
  led_init();

  comm_init(COMM_TELEMETRY);

  gps_init();

  int_enable();
}

static inline void main_periodic_task( void ) {
    comm_periodic_task(COMM_TELEMETRY);
    led_periodic_task();

    RunOnceEvery(250, {
        comm_autopilot_message_send (COMM_TELEMETRY, MESSAGE_ID_GPS_LLH);
        comm_autopilot_message_send (COMM_TELEMETRY, MESSAGE_ID_GPS_STATUS);
        comm_autopilot_message_send (COMM_TELEMETRY, MESSAGE_ID_STATUS_LOWLEVEL);
    });

}

static inline void main_event_task( void ) {
    comm_event_task(COMM_TELEMETRY);

    if (gps_event_task()) {
        if (gps_state.fix == GPS_FIX_3D)
            led_on(LED_GPS);
        else
            led_toggle(LED_GPS);
    }
}
