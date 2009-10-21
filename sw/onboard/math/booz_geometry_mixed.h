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
#ifndef BOOZ_GEOMETRY_MIXED_H
#define BOOZ_GEOMETRY_MIXED_H

#include "booz_geometry_int.h"
#include "booz_geometry_float.h"

#define BOOZ_INT_OF_FLOAT(f,r) ((f)*(float)(1<<(r)))
#define BOOZ_FLOAT_OF_INT(i,r) ((float)(i)/(float)(1<<(r)))

#define BOOZ_FQUAT_OF_IQUAT(fq, iq) {                                   \
    fq.qi = BOOZ_FLOAT_OF_INT(iq.qi, IQUAT_RES);			\
    fq.qx = BOOZ_FLOAT_OF_INT(iq.qx, IQUAT_RES);			\
    fq.qy = BOOZ_FLOAT_OF_INT(iq.qy, IQUAT_RES);			\
    fq.qz = BOOZ_FLOAT_OF_INT(iq.qz, IQUAT_RES);			\
  }

#define BOOZ_IQUAT_OF_FQUAT(iq, fq) {                                   \
    iq.qi = BOOZ_INT_OF_FLOAT(fq.qi, IQUAT_RES);			\
    iq.qx = BOOZ_INT_OF_FLOAT(fq.qx, IQUAT_RES);			\
    iq.qy = BOOZ_INT_OF_FLOAT(fq.qy, IQUAT_RES);			\
    iq.qz = BOOZ_INT_OF_FLOAT(fq.qz, IQUAT_RES);			\
  }


#define BOOZ_RATE_F_OF_I(i)  BOOZ_FLOAT_OF_INT(i,IRATE_RES)
#define BOOZ_RATE_I_OF_F(f)  BOOZ_INT_OF_FLOAT(f,IRATE_RES)

#define BOOZ_VRATE_F_OF_I(f, i) {		\
    f.x = BOOZ_RATE_F_OF_I(i.x);		\
    f.y = BOOZ_RATE_F_OF_I(i.y);		\
    f.z = BOOZ_RATE_F_OF_I(i.z);		\
  }

#define BOOZ_VRATE_I_OF_F(i, f) {		\
    i.x = BOOZ_RATE_I_OF_F(f.x);		\
    i.y = BOOZ_RATE_I_OF_F(f.y);		\
    i.z = BOOZ_RATE_I_OF_F(f.z);		\
  }

#define BOOZ_ANGLE_F_OF_I(i)  BOOZ_FLOAT_OF_INT(i,IANGLE_RES)
#define BOOZ_ANGLE_I_OF_F(f)  BOOZ_INT_OF_FLOAT(f,IANGLE_RES)

#define BOOZ_TRIG_F_OF_I(i)  BOOZ_FLOAT_OF_INT(i,ITRIG_RES)
#define BOOZ_TRIG_I_OF_F(f)  BOOZ_INT_OF_FLOAT(f,ITRIG_RES)

#define BOOZ_IEULER_OF_FEULER(_ie, _fe) {	\
    _ie.phi   = BOOZ_ANGLE_I_OF_F(_fe.phi);	\
    _ie.theta = BOOZ_ANGLE_I_OF_F(_fe.theta);	\
    _ie.psi   = BOOZ_ANGLE_I_OF_F(_fe.psi);	\
  }

#define BOOZ_ACCEL_I_OF_F(f)  BOOZ_INT_OF_FLOAT(f,IACCEL_RES)
#define BOOZ_ACCEL_F_OF_I(i)  BOOZ_FLOAT_OF_INT(i,IACCEL_RES)

#define BOOZ_SPEED_I_OF_F(f)  BOOZ_INT_OF_FLOAT(f,ISPEED_RES)
#define BOOZ_SPEED_F_OF_I(i)  BOOZ_FLOAT_OF_INT(i,ISPEED_RES)

#define BOOZ_POS_I_OF_F(f)  BOOZ_INT_OF_FLOAT(f,IPOS_FRAC)
#define BOOZ_POS_F_OF_I(i)  BOOZ_FLOAT_OF_INT(i,IPOS_FRAC)

#endif /* BOOZ_GEOMETRY_MIXED_H */

