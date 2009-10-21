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
#ifndef BOOZ_AHRS_H
#define BOOZ_AHRS_H

#include "std.h"
#include "pprz_algebra_int.h"

#define BOOZ_AHRS_UNINIT  0
#define BOOZ_AHRS_RUNNING 1

struct BoozAhrs {
  struct Int32Eulers ltp_to_body_euler;
  struct Int32Quat   ltp_to_body_quat;
  struct Int32RMat   ltp_to_body_rmat;
  struct Int32Eulers ltp_to_imu_euler;
  struct Int32Quat   ltp_to_imu_quat;
  struct Int32RMat   ltp_to_imu_rmat;
  struct Int32Rates  imu_rate;
  struct Int32Rates  body_rate;  
  uint8_t status;
};

extern struct BoozAhrs booz_ahrs;

extern void booz_ahrs_init(void);
extern void booz_ahrs_align(void);
extern void booz_ahrs_propagate(void);
extern void booz_ahrs_update(void);


#endif /* BOOZ_AHRS_H */
