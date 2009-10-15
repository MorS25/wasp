/*
 * $Id$
 *  
 * Copyright (C) 2008  Antoine Drouin
 *
 * This file is part of paparazzi.
 *
 * paparazzi is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * paparazzi is distributed in the hope that it will be useful,
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
#include "altimeter.h"
#include "arm7/altimeter_analog_baro_hw.h"

#include "generated/settings.h"

SystemStatus_t altimeter_system_status;

uint16_t booz2_analog_baro_offset;
uint16_t booz2_analog_baro_value;
uint16_t booz2_analog_baro_value_filtered;
bool_t   booz2_analog_baro_data_available;


void altimeter_init( void ) {

  altimeter_system_status = STATUS_UNINITIAIZED;

  booz2_analog_baro_offset = 1023;
  Booz2AnalogSetDAC(booz2_analog_baro_offset);

  booz2_analog_baro_value = 0;
  booz2_analog_baro_value_filtered = 0;
  booz2_analog_baro_data_available = FALSE;
#ifdef BOOZ2_ANALOG_BARO_LED
  LED_OFF(BOOZ2_ANALOG_BARO_LED);
#endif
}

uint8_t
altimeter_event_task(void)
{
    uint8_t ret = FALSE;
    if (booz2_analog_baro_data_available) 
    {
      ret = TRUE;
      booz2_analog_baro_data_available = FALSE;
    }
    return ret;
}

int32_t
altimeter_get_altitude(void)
{ 
    return (booz2_analog_baro_value * INS_BARO_SENS_NUM)/INS_BARO_SENS_DEN;
}

