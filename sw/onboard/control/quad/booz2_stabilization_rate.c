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
#include "booz2_stabilization.h"
#include "booz2_stabilization_rate.h"

#include "rc.h"
#include "imu.h"
#include "ahrs/booz_ahrs_aligner.h"

#include "generated/settings.h"

struct Int32Rates booz2_stabilization_rate_measure;
struct Int32Rates booz2_stabilization_rate_sp;
struct Int32Rates booz2_stabilization_rate_gain;

void booz2_stabilization_rate_init(void) {

  INT_RATES_ZERO(booz2_stabilization_rate_sp);

  RATES_ASSIGN(booz2_stabilization_rate_gain,
	       BOOZ_STABILIZATION_RATE_GAIN_P,
	       BOOZ_STABILIZATION_RATE_GAIN_Q,
	       BOOZ_STABILIZATION_RATE_GAIN_R);
  
}


void booz2_stabilization_rate_read_rc( void ) {

  RATES_ASSIGN(booz2_stabilization_rate_sp,
	       (int32_t)-rc_values[RADIO_ROLL]  * BOOZ_STABILIZATION_RATE_SP_MAX_P / MAX_PPRZ,
	       (int32_t) rc_values[RADIO_PITCH] * BOOZ_STABILIZATION_RATE_SP_MAX_Q / MAX_PPRZ,
	       (int32_t)-rc_values[RADIO_YAW]   * BOOZ_STABILIZATION_RATE_SP_MAX_R / MAX_PPRZ);

}


void booz2_stabilization_rate_run(void) {

  /* low pass */
  RATES_ADD(booz2_stabilization_rate_measure, booz_imu.gyro);
  if (booz_ahrs_aligner.status == BOOZ_AHRS_ALIGNER_LOCKED)
    RATES_SUB(booz2_stabilization_rate_measure, booz_ahrs_aligner.lp_gyro);
  RATES_SDIV(booz2_stabilization_rate_measure, booz2_stabilization_rate_measure, 2);


  struct Int32Rates _error;
  RATES_DIFF(_error, booz2_stabilization_rate_measure, booz2_stabilization_rate_sp);
  struct Int32Rates _cmd;
  RATES_EWMULT_RSHIFT(_cmd, _error, booz2_stabilization_rate_gain, 16);

  booz2_stabilization_cmd[COMMAND_ROLL]  = _cmd.p;
  booz2_stabilization_cmd[COMMAND_PITCH] = _cmd.q;
  booz2_stabilization_cmd[COMMAND_YAW]   = _cmd.r;
 
}

