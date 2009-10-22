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
#ifndef BOOZ2_GUIDANCE_V
#define BOOZ2_GUIDANCE_V

#include "std.h"

#include "guidance.h"
#include "booz2_guidance_v_ref.h"
#include "booz2_guidance_v_adpt.h"

#define BOOZ2_GUIDANCE_V_MODE_KILL      0
#define BOOZ2_GUIDANCE_V_MODE_RC_DIRECT 1
#define BOOZ2_GUIDANCE_V_MODE_RC_CLIMB  2
#define BOOZ2_GUIDANCE_V_MODE_CLIMB     3
#define BOOZ2_GUIDANCE_V_MODE_HOVER     4
#define BOOZ2_GUIDANCE_V_MODE_NAV       5

extern int32_t booz2_guidance_v_z_sp;
extern int32_t booz2_guidance_v_zd_sp;
extern int32_t booz2_guidance_v_z_ref;
extern int32_t booz2_guidance_v_zd_ref;
extern int32_t booz2_guidance_v_zdd_ref;
extern int32_t booz2_guidance_v_z_sum_err;
extern int32_t booz2_guidance_v_ff_cmd;
extern int32_t booz2_guidance_v_fb_cmd;
extern int32_t booz2_guidance_v_delta_t;

extern int32_t booz2_guidance_v_kp;
extern int32_t booz2_guidance_v_kd;

extern void booz2_guidance_v_init(void);
extern void booz2_guidance_v_read_rc(void);
extern void booz2_guidance_v_mode_changed(uint8_t new_mode);
extern void booz2_guidance_v_notify_in_flight(bool_t in_flight);
extern void booz2_guidance_v_run(bool_t in_flight);

#endif /* BOOZ2_GUIDANCE_V */
