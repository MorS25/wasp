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
#include "booz_ahrs_aligner.h"

#include <stdlib.h> /* for abs() */
#include "led.h"
#include "imu.h"

#include "generated/settings.h"

struct BoozAhrsAligner booz_ahrs_aligner;

#define SAMPLES_NB 512
static struct Int32Rates gyro_sum;
static struct Int32Vect3 accel_sum;
static struct Int32Vect3 mag_sum;
static int32_t ref_sensor_samples[SAMPLES_NB];
static uint32_t samples_idx;

void booz_ahrs_aligner_init(void) {
  
  booz_ahrs_aligner.status = BOOZ_AHRS_ALIGNER_RUNNING;
  INT_RATES_ZERO(gyro_sum);
  INT_VECT3_ZERO(accel_sum);
  INT_VECT3_ZERO(mag_sum);
  samples_idx = 0;
  booz_ahrs_aligner.noise = 0;
  booz_ahrs_aligner.low_noise_cnt = 0;

}

#define LOW_NOISE_THRESHOLD 90000
#define LOW_NOISE_TIME          5

void booz_ahrs_aligner_run(void) {

  RATES_ADD(gyro_sum,  booz_imu.gyro);
  VECT3_ADD(accel_sum, booz_imu.accel);
  VECT3_ADD(mag_sum,   booz_imu.mag);

  ref_sensor_samples[samples_idx] = booz_imu.accel.z;
  samples_idx++;

#if LED_AHRS
  RunOnceEvery(50, {led_toggle(LED_AHRS);});
#endif

  if (samples_idx >= SAMPLES_NB) {
    int32_t avg_ref_sensor = accel_sum.z;
    if ( avg_ref_sensor >= 0)
      avg_ref_sensor += SAMPLES_NB / 2;
    else
      avg_ref_sensor -= SAMPLES_NB / 2;
    avg_ref_sensor /= SAMPLES_NB;
    
    booz_ahrs_aligner.noise = 0;
    int i;
    for (i=0; i<SAMPLES_NB; i++) {
      int32_t diff = ref_sensor_samples[i] - avg_ref_sensor;
      booz_ahrs_aligner.noise += abs(diff);
    }

    RATES_SDIV(booz_ahrs_aligner.lp_gyro,  gyro_sum,  SAMPLES_NB);
    VECT3_SDIV(booz_ahrs_aligner.lp_accel, accel_sum, SAMPLES_NB);
    VECT3_SDIV(booz_ahrs_aligner.lp_mag,   mag_sum,   SAMPLES_NB);

    INT_RATES_ZERO(gyro_sum);
    INT_VECT3_ZERO(accel_sum);
    INT_VECT3_ZERO(mag_sum);
    samples_idx = 0;

    if (booz_ahrs_aligner.noise < LOW_NOISE_THRESHOLD)
      booz_ahrs_aligner.low_noise_cnt++;
    else
      if ( booz_ahrs_aligner.low_noise_cnt > 0)
	booz_ahrs_aligner.low_noise_cnt--;
    
    if (booz_ahrs_aligner.low_noise_cnt > LOW_NOISE_TIME) {
      booz_ahrs_aligner.status = BOOZ_AHRS_ALIGNER_LOCKED;
#if LED_AHRS
      led_on(LED_AHRS);
#endif
    }
  }

}

