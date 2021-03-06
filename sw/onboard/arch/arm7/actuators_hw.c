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
#include "actuators.h"
#include "arm7/buss_twi_blmc_hw.h"
#include "arm7/servos_4017_hw.h"
#include "arm7/servos_direct_hw.h"

#if USE_SERVOS_4017 && USE_SERVOS_DIRECT
#error  only one sevos implementation allowed
#endif

void actuators_init( uint8_t bank )
{
    if ( bank & ACTUATOR_BANK_MOTORS )
        buss_twi_blmc_init();

    if ( bank & ACTUATOR_BANK_SERVOS ) {
#if USE_SERVOS_4017
        servos_4017_init();
#endif
#if USE_SERVOS_DIRECT
        servos_direct_init();
#endif
    }
}

void actuators_set( ActuatorID_t id, uint8_t value )
{
    /* mask out the bank */
    ActuatorID_t aid = id & 0x0F;

    if ( id & ACTUATOR_BANK_MOTORS )
        buss_twi_blmc_motor_power[aid] = value;

    if ( id & ACTUATOR_BANK_SERVOS ) {
#if USE_SERVOS_4017
        /*FIXME: HACK HACK BROKEN BROKEN. 
        THE FIRST TWO CHANNELS DO NOT SO SUBTRACT 2 FROM THE INDEX.
        !!!!!!
        THIS MEANS WE DO NOT HAVE TO CHANGE ANY SETTINGS LATER, BUT LIMITS
        US TO 6 CHANNELS
        !!!!!!
        NEED TO FIX THE TIMING IN SERVOS_4017_HW.c */
        if (aid >= 2)
            aid -= 2;
        servos_4017_set(aid, value);
#endif
#if USE_SERVOS_DIRECT
        servos_direct_set(aid, value);
#endif
    }

}

void actuators_commit( uint8_t bank )
{
    if ( bank & ACTUATOR_BANK_MOTORS )
        buss_twi_blmc_commit();

    if (bank & ACTUATOR_BANK_SERVOS) {
#if USE_SERVOS_4017
        servos_4017_commit();
#endif
#if USE_SERVOS_DIRECT
        servos_direct_commit();
#endif
    }
}

uint8_t actuators_get_num( uint8_t bank )
{
    if (bank == ACTUATOR_BANK_MOTORS)
        return buss_twi_blmc_get_num();

    if (bank == ACTUATOR_BANK_SERVOS) {
#if USE_SERVOS_4017
        return servos_4017_get_num();
#endif
#if USE_SERVOS_DIRECT
        return servos_direct_get_num();
#endif
    }
    return 0;
}

uint8_t actuators_get( ActuatorID_t id )
{
    /* mask out the bank */
    ActuatorID_t aid = id & 0x0F;

    if (id & ACTUATOR_BANK_MOTORS)
        return buss_twi_blmc_motor_power[aid];

    if (id & ACTUATOR_BANK_SERVOS) {
#if USE_SERVOS_4017
        return servos_4017_get(aid);
#endif
#if USE_SERVOS_DIRECT
        return servos_direct_get(aid);
#endif
    }
    return 0;
}

