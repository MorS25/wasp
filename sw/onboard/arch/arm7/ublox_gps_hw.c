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
#include "gps.h"

#include "arm7/config.h"
#include "arm7/uart_hw.h"
#include "arm7/led_hw.h"
#include "lib/ubx_protocol.h"

struct Booz_gps_state booz_gps_state;
SystemStatus_t gps_system_status = STATUS_UNINITIAIZED;

/* misc */
volatile bool_t  booz2_gps_msg_received;
volatile uint8_t booz2_gps_nb_ovrn;

/* UBX parsing */
#define  UBX_FIX_NONE 0x00
#define  UBX_FIX_2D   0x02
#define  UBX_FIX_3D   0x03

static bool_t  ubx_msg_available;

#define UBX_MAX_PAYLOAD 255
static uint8_t ubx_msg_buf[UBX_MAX_PAYLOAD] __attribute__ ((aligned));
static uint8_t ubx_id;
static uint8_t ubx_class;

static void ubx_init(void);
static void ubx_parse( uint8_t c );

#define UNINIT        0
#define GOT_SYNC1     1
#define GOT_SYNC2     2
#define GOT_CLASS     3
#define GOT_ID        4
#define GOT_LEN1      5
#define GOT_LEN2      6
#define GOT_PAYLOAD   7
#define GOT_CHECKSUM1 8

static uint8_t  ubx_status;
static uint16_t ubx_len;
static uint8_t  ubx_msg_idx;
static uint8_t  ck_a, ck_b;

void gps_init(void) 
{
  booz_gps_state.fix = GPS_FIX_NONE;
  uart0_init_tx();
  ubx_init();
  gps_system_status = STATUS_INITIALIZED;
}

bool_t
gps_event_task(void) 
{
    uint8_t r = FALSE;    

    if (Uart0ChAvailable()) {
        while ( Uart0ChAvailable() && !ubx_msg_available )
            ubx_parse(Uart0Getch());
    }
    if (ubx_msg_available) {
        if (ubx_class == UBX_NAV_ID) {

            gps_system_status = STATUS_ALIVE;

            if (ubx_id == UBX_NAV_POSLLH_ID) {
                booz_gps_state.booz2_gps_lon = UBX_NAV_POSLLH_LON(ubx_msg_buf);
                booz_gps_state.booz2_gps_lat = UBX_NAV_POSLLH_LAT(ubx_msg_buf);
                booz_gps_state.booz2_gps_hmsl = UBX_NAV_POSLLH_HMSL(ubx_msg_buf);
                booz_gps_state.booz2_gps_hacc = UBX_NAV_POSLLH_Hacc(ubx_msg_buf);
                booz_gps_state.booz2_gps_vacc = UBX_NAV_POSLLH_Vacc(ubx_msg_buf);
            }
            else if (ubx_id == UBX_NAV_SOL_ID) {
                uint8_t fix = UBX_NAV_SOL_GPSfix(ubx_msg_buf);
                if ( fix == UBX_FIX_3D)
                    booz_gps_state.fix = GPS_FIX_3D;
                else if ( fix == UBX_FIX_2D )
                    booz_gps_state.fix = GPS_FIX_2D;
                else
                    booz_gps_state.fix = GPS_FIX_NONE;
                booz_gps_state.ecef_pos.x   = UBX_NAV_SOL_ECEF_X(ubx_msg_buf);
                booz_gps_state.ecef_pos.y   = UBX_NAV_SOL_ECEF_Y(ubx_msg_buf);
                booz_gps_state.ecef_pos.z   = UBX_NAV_SOL_ECEF_Z(ubx_msg_buf);
                booz_gps_state.pacc         = UBX_NAV_SOL_Pacc(ubx_msg_buf);
                booz_gps_state.ecef_speed.x = UBX_NAV_SOL_ECEFVX(ubx_msg_buf);
                booz_gps_state.ecef_speed.y = UBX_NAV_SOL_ECEFVY(ubx_msg_buf);
                booz_gps_state.ecef_speed.z = UBX_NAV_SOL_ECEFVZ(ubx_msg_buf);
                booz_gps_state.sacc         = UBX_NAV_SOL_Sacc(ubx_msg_buf);
                booz_gps_state.pdop         = UBX_NAV_SOL_PDOP(ubx_msg_buf);
                booz_gps_state.num_sv       = UBX_NAV_SOL_numSV(ubx_msg_buf);
            }
            else if (ubx_id == UBX_NAV_VELNED_ID) {
                booz_gps_state.booz2_gps_vel_n = UBX_NAV_VELNED_VEL_N(ubx_msg_buf);
                booz_gps_state.booz2_gps_vel_e = UBX_NAV_VELNED_VEL_E(ubx_msg_buf);
            }
        }

        r = TRUE;
        ubx_msg_available = FALSE;
    }
    return r;    
}

static void ubx_init(void) {
  ubx_status = UNINIT;
  ubx_msg_available = FALSE;
}

static void ubx_parse( uint8_t c ) {
  if (ubx_status < GOT_PAYLOAD) {
    ck_a += c;
    ck_b += ck_a;
  }
  switch (ubx_status) {
  case UNINIT:
    if (c == UBX_SYNC1)
      ubx_status++;
    break;
  case GOT_SYNC1:
    if (c != UBX_SYNC2)
      goto error;
    ck_a = 0;
    ck_b = 0;
    ubx_status++;
    break;
  case GOT_SYNC2:
    if (ubx_msg_available) {
      /* Previous message has not yet been parsed: discard this one */
      booz2_gps_nb_ovrn++;
      goto error;
    }
    ubx_class = c;
    ubx_status++;
    break;
  case GOT_CLASS:
    ubx_id = c;
    ubx_status++;
    break;    
  case GOT_ID:
    ubx_len = c;
    ubx_status++;
    break;
  case GOT_LEN1:
    ubx_len |= (c<<8);
    if (ubx_len > UBX_MAX_PAYLOAD)
      goto error;
    ubx_msg_idx = 0;
    ubx_status++;
    break;
  case GOT_LEN2:
    ubx_msg_buf[ubx_msg_idx] = c;
    ubx_msg_idx++;
    if (ubx_msg_idx >= ubx_len) {
      ubx_status++;
    }
    break;
  case GOT_PAYLOAD:
    if (c != ck_a)
      goto error;
    ubx_status++;
    break;
  case GOT_CHECKSUM1:
    if (c != ck_b)
      goto error;
    ubx_msg_available = TRUE;
    goto restart;
    break;
  }
  return;
 error:  
 restart:
  ubx_status = UNINIT;
  return;
}
