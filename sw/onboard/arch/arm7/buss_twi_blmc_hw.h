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
#ifndef BUSS_TWI_BLMC_HW_H
#define BUSS_TWI_BLMC_HW_H

#include "std.h"

#include "generated/settings.h"

extern uint8_t buss_twi_blmc_motor_power[MOTOR_NB];

void buss_twi_blmc_init(void);

void buss_twi_blmc_commit(void);

uint8_t buss_twi_blmc_get_num(void);

#endif /* BUSS_TWI_BLMC_HW_H */
