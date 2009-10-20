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
#ifndef BOOZ2_STABILIZATION_ATTITUDE_H
#define BOOZ2_STABILIZATION_ATTITUDE_H

#include "booz_geometry_int.h"

extern void booz2_stabilization_attitude_init(void);
extern void booz2_stabilization_attitude_read_rc(bool_t in_flight);
extern void booz2_stabilization_attitude_enter(void);
extern void booz2_stabilization_attitude_run(bool_t  in_flight);


//extern struct booz_ieuler booz_stabilization_att_sp;
//extern struct booz_ieuler booz_stabilization_att_ref;
//extern struct booz_ivect  booz_stabilization_rate_ref;
//extern struct booz_ivect  booz_stabilization_accel_ref;
#include "booz2_stabilization_attitude_ref_traj_euler.h"


extern struct booz_ivect  booz_stabilization_igain;
extern struct booz_ivect  booz_stabilization_pgain;
extern struct booz_ivect  booz_stabilization_dgain;
extern struct booz_ivect  booz_stabilization_ddgain;
extern struct booz_ieuler booz_stabilization_att_sum_err;

#endif /* BOOZ2_STABILIZATION_ATTITUDE_H */
