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
#ifndef BOOZ_AHRS_ALIGNER_H
#define BOOZ_AHRS_ALIGNER_H

#include "std.h"
#include "math/pprz_algebra_int.h"

#define BOOZ_AHRS_ALIGNER_UNINIT  0
#define BOOZ_AHRS_ALIGNER_RUNNING 1
#define BOOZ_AHRS_ALIGNER_LOCKED  2

struct BoozAhrsAligner {
  struct Int32Rates lp_gyro;
  struct Int32Vect3 lp_accel;
  struct Int32Vect3 lp_mag;
  int32_t           noise;
  int32_t           low_noise_cnt;
  uint8_t           status;
};

extern struct BoozAhrsAligner booz_ahrs_aligner;

extern void booz_ahrs_aligner_init(void);
extern void booz_ahrs_aligner_run(void);

#endif /* BOOZ_AHRS_ALIGNER_H */
