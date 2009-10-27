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
#define B2_GUIDANCE_V_C
#define B2_GUIDANCE_V_USE_REF
#include "booz2_guidance_v.h"

#include "rc.h"
#include "ins.h"
#include "config/airframe.h"
#include "stabilization/booz2_stabilization.h"
#include "fms/booz2_fms.h"
#include "booz2_navigation.h"

#include "booz_geometry_mixed.h"

uint8_t booz2_guidance_v_mode;
int32_t booz2_guidance_v_ff_cmd;
int32_t booz2_guidance_v_fb_cmd;
/* command output                                 */
int32_t booz2_guidance_v_delta_t;

/* direct throttle from radio control             */
/* range 0:200                                    */
int32_t booz2_guidance_v_rc_delta_t;
/* vertical speed setpoint from radio control     */
/* Q12.19 : accuracy 0.0000019, range +/-4096     */
int32_t booz2_guidance_v_rc_zd_sp;
/* altitude setpoint in meter (input)             */
/* Q23.8 : accuracy 0.0039, range 8388km          */
int32_t booz2_guidance_v_z_sp;
/* vertical speed setpoint in meter/s (input)     */
/* Q12.19 : accuracy 0.0000019, range +/-4096     */
int32_t booz2_guidance_v_zd_sp;
#define BOOZ2_GUIDANCE_V_ZD_SP_FRAC ISPEED_RES

/* altitude reference in meter                    */
/* Q23.8 : accuracy 0.0039, range 8388km          */
int32_t booz2_guidance_v_z_ref;
/* vertical speed reference in meter/s            */
/* Q12.19 : accuracy 0.0000038, range 4096        */
int32_t booz2_guidance_v_zd_ref;
/* vertical acceleration reference in meter/s^2   */
/* Q21.10 : accuracy 0.0009766, range 2097152     */
int32_t booz2_guidance_v_zdd_ref;

int32_t booz2_guidance_v_kp;
int32_t booz2_guidance_v_kd;

int32_t booz2_guidance_v_z_sum_err;


#define Booz2GuidanceVSetRef(_pos, _speed, _accel) { \
    b2_gv_set_ref(_pos, _speed, _accel);	     \
    booz2_guidance_v_z_ref = _pos;		     \
    booz2_guidance_v_zd_ref = _speed;		     \
    booz2_guidance_v_zdd_ref = _accel;		     \
  }


static inline void run_hover_loop(bool_t in_flight);


void booz2_guidance_v_init(void) {

  booz2_guidance_v_mode = BOOZ2_GUIDANCE_V_MODE_KILL;

  booz2_guidance_v_kp = BOOZ2_GUIDANCE_V_HOVER_KP;
  booz2_guidance_v_kd = BOOZ2_GUIDANCE_V_HOVER_KD;

  booz2_guidance_v_z_sum_err = 0;

  b2_gv_adapt_init();
}


void booz2_guidance_v_read_rc(void) {

  // used in RC_DIRECT directly and as saturation in CLIMB and HOVER
  booz2_guidance_v_rc_delta_t = (int32_t)rc_values[RADIO_THROTTLE] * 200 / MAX_PPRZ;
  // used in RC_CLIMB
  booz2_guidance_v_rc_zd_sp   = ((MAX_PPRZ/2) - (int32_t)rc_values[RADIO_THROTTLE]) * 
                                BOOZ2_GUIDANCE_V_RC_CLIMB_COEF;
  DeadBand(booz2_guidance_v_rc_zd_sp, BOOZ2_GUIDANCE_V_RC_CLIMB_DEAD_BAND);
  
}

void booz2_guidance_v_mode_changed(uint8_t new_mode) {
  
  if (new_mode == booz2_guidance_v_mode)
    return;

  //  switch ( booz2_guidance_v_mode ) {
  //  
  //  }
   
  switch (new_mode) {

  case BOOZ2_GUIDANCE_V_MODE_RC_CLIMB:
  case BOOZ2_GUIDANCE_V_MODE_CLIMB:
  case BOOZ2_GUIDANCE_V_MODE_HOVER:
    booz2_guidance_v_z_sum_err = 0;
    Booz2GuidanceVSetRef(ins.ltp_pos.z, ins.ltp_speed.z, 0);
    break;
  case BOOZ2_GUIDANCE_V_MODE_NAV:
    break;

  }

  
  booz2_guidance_v_mode = new_mode;

}

void booz2_guidance_v_notify_in_flight( bool_t in_flight) {
  if (in_flight)
    b2_gv_adapt_init();
}


void booz2_guidance_v_run(bool_t in_flight) {

  // FIXME... SATURATIONS NOT TAKEN INTO ACCOUNT
  // AKA SUPERVISION and co
  if (in_flight) {
    // we should use something after the supervision!!! fuck!!!
    int32_t cmd_hack = Chop(booz2_stabilization_cmd[COMMAND_THRUST], 1, 200);
    b2_gv_adapt_run(ins.ltp_accel.z, cmd_hack);
  }
		
  switch (booz2_guidance_v_mode) {

  case BOOZ2_GUIDANCE_V_MODE_RC_DIRECT:
    booz2_guidance_v_z_sp = ins.ltp_pos.z;  // not sure why we do that
    Booz2GuidanceVSetRef(ins.ltp_pos.z, 0, 0); // or that - mode enter should take care of it ?
    booz2_stabilization_cmd[COMMAND_THRUST] = booz2_guidance_v_rc_delta_t;
    break;

  case BOOZ2_GUIDANCE_V_MODE_RC_CLIMB:
    booz2_guidance_v_zd_sp = booz2_guidance_v_rc_zd_sp;
    b2_gv_update_ref_from_zd_sp(booz2_guidance_v_zd_sp);
    run_hover_loop(in_flight);
    booz2_stabilization_cmd[COMMAND_THRUST] = booz2_guidance_v_delta_t;
    break;

  case BOOZ2_GUIDANCE_V_MODE_CLIMB:
    if (booz_fms_on && booz_fms_input.v_mode == BOOZ2_GUIDANCE_V_MODE_CLIMB)
      booz2_guidance_v_zd_sp = booz_fms_input.v_sp.climb;
    b2_gv_update_ref_from_zd_sp(booz2_guidance_v_zd_sp);
    run_hover_loop(in_flight);
    // saturate max authority with RC stick
    booz2_stabilization_cmd[COMMAND_THRUST] = Min( booz2_guidance_v_rc_delta_t, booz2_guidance_v_delta_t);
    break;

  case BOOZ2_GUIDANCE_V_MODE_HOVER:
    if (booz_fms_on && booz_fms_input.v_mode == BOOZ2_GUIDANCE_V_MODE_HOVER)
      booz2_guidance_v_z_sp = booz_fms_input.v_sp.height;
    b2_gv_update_ref_from_z_sp(booz2_guidance_v_z_sp);
    run_hover_loop(in_flight);
    // saturate max authority with RC stick
    booz2_stabilization_cmd[COMMAND_THRUST] = Min( booz2_guidance_v_rc_delta_t, booz2_guidance_v_delta_t);
    break;

  case BOOZ2_GUIDANCE_V_MODE_NAV:
    //booz2_guidance_v_z_sp = -nav_altitude;
    //b2_gv_update_ref_from_z_sp(booz2_guidance_v_z_sp);
    //run_hover_loop(in_flight);
    // saturate max authority with RC stick
    //booz2_stabilization_cmd[COMMAND_THRUST] = Min( booz2_guidance_v_rc_delta_t, booz2_guidance_v_delta_t);
    break;
  }
}


#define FF_CMD_FRAC 18

static inline void run_hover_loop(bool_t in_flight) {

  /* convert our reference to generic representation */
  int64_t tmp  = b2_gv_z_ref>>(B2_GV_Z_REF_FRAC - IPOS_FRAC);
  booz2_guidance_v_z_ref = (int32_t)tmp;
  booz2_guidance_v_zd_ref = b2_gv_zd_ref<<(ISPEED_RES - B2_GV_ZD_REF_FRAC);
  booz2_guidance_v_zdd_ref = b2_gv_zdd_ref<<(IACCEL_RES - B2_GV_ZDD_REF_FRAC);
  /* compute the error to our reference */
  int32_t err_z  =  ins.ltp_pos.z - booz2_guidance_v_z_ref;
  Bound(err_z, BOOZ2_GUIDANCE_V_MIN_ERR_Z, BOOZ2_GUIDANCE_V_MAX_ERR_Z);
  int32_t err_zd =  ins.ltp_speed.z - booz2_guidance_v_zd_ref;
  Bound(err_zd, BOOZ2_GUIDANCE_V_MIN_ERR_ZD, BOOZ2_GUIDANCE_V_MAX_ERR_ZD);

  if (in_flight)
    booz2_guidance_v_z_sum_err += err_z;
  else
    booz2_guidance_v_z_sum_err = 0;

  /* our nominal command : (g + zdd)*m   */
  //const int32_t inv_m = BOOZ_INT_OF_FLOAT(0.140, IACCEL_RES);
  const int32_t inv_m =  b2_gv_adapt_X>>(B2_GV_ADAPT_X_FRAC - FF_CMD_FRAC);
  const int32_t g_m_zdd = (int32_t)BOOZ_INT_OF_FLOAT(9.81, FF_CMD_FRAC) - 
                          (booz2_guidance_v_zdd_ref<<(FF_CMD_FRAC - IACCEL_RES));
  if (g_m_zdd > 0) 
    booz2_guidance_v_ff_cmd = ( g_m_zdd + (inv_m>>1)) / inv_m;
  else
    booz2_guidance_v_ff_cmd = ( g_m_zdd - (inv_m>>1)) / inv_m;
  //  booz2_guidance_v_ff_cmd = BOOZ2_GUIDANCE_V_HOVER_POWER;
  
  /* our error command                   */
  booz2_guidance_v_fb_cmd = ((-booz2_guidance_v_kp * err_z) >> 12) + 
                            ((-booz2_guidance_v_kd * err_zd) >> 21);

  booz2_guidance_v_delta_t = booz2_guidance_v_ff_cmd + booz2_guidance_v_fb_cmd;


}

