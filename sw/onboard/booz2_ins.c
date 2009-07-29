#include "booz2_ins.h"

#include "imu.h"
#include "altimeter.h"
#include "gps.h"

#include "booz_geometry_mixed.h"

#if USE_VFF
#include "booz2_vf_float.h"
#endif

#include "booz_ahrs.h"
#include "booz2_hf_float.h"

#include "pprz_geodetic_int.h"


/* gps transformed to LTP-NED  */
struct LtpDef_i  booz_ins_ltp_def;
         bool_t  booz_ins_ltp_initialised;
struct NedCoor_i booz_ins_gps_pos_cm_ned;
struct NedCoor_i booz_ins_gps_speed_cm_s_ned;

/* barometer                   */
#if USE_VFF
int32_t booz_ins_qfe;
bool_t  booz_ins_baro_initialised;
int32_t booz_ins_baro_alt;
bool_t  booz_ins_vff_realign; 
#endif

/* output                      */
struct NedCoor_i booz_ins_ltp_pos;
struct NedCoor_i booz_ins_ltp_speed;
struct NedCoor_i booz_ins_ltp_accel;
struct EnuCoor_i booz_ins_enu_pos;
struct EnuCoor_i booz_ins_enu_speed;
struct EnuCoor_i booz_ins_enu_accel;


void booz_ins_init() {
#if USE_VFF
  booz_ins_ltp_initialised  = FALSE;
  booz_ins_baro_initialised = FALSE;
  booz_ins_vff_realign = FALSE;
  b2_vff_init(0., 0., 0.);
#endif
  b2ins_init();
  INT32_VECT3_ZERO(booz_ins_enu_pos);
  INT32_VECT3_ZERO(booz_ins_enu_speed);
  INT32_VECT3_ZERO(booz_ins_enu_accel);
}

void booz_ins_propagate() {

#if USE_VFF
  if (altimeter_status == STATUS_INITIALIZED && booz_ins_baro_initialised) {
    float accel_float = BOOZ_ACCEL_F_OF_I(booz_imu.accel.z);
    b2_vff_propagate(accel_float);
    booz_ins_ltp_accel.z = BOOZ_ACCEL_I_OF_F(b2_vff_zdotdot);
    booz_ins_ltp_speed.z = BOOZ_SPEED_I_OF_F(b2_vff_zdot);
    booz_ins_ltp_pos.z = BOOZ_POS_I_OF_F(b2_vff_z);
    booz_ins_enu_pos.z = -booz_ins_ltp_pos.z;
    booz_ins_enu_speed.z = -booz_ins_ltp_speed.z;
    booz_ins_enu_accel.z = -booz_ins_ltp_accel.z;
  }
#endif
#ifdef USE_HFF
  if (booz_ahrs.status == BOOZ_AHRS_RUNNING &&
      booz_gps_state.fix == BOOZ2_GPS_FIX_3D && booz_ins_ltp_initialised )
    b2ins_propagate();
#endif
}

void booz_ins_update_baro() {

#if USE_VFF
  if (altimeter_status == STATUS_INITIALIZED) {
    uint32_t alt = altimeter_get_altitude();
    if (!booz_ins_baro_initialised) {
      booz_ins_qfe = alt;
      booz_ins_baro_initialised = TRUE;
    }
    booz_ins_baro_alt = alt - booz_ins_qfe;
    float alt_float = BOOZ_POS_F_OF_I(booz_ins_baro_alt);
    if (booz_ins_vff_realign) {
      booz_ins_vff_realign = FALSE;
      booz_ins_qfe = alt;
      b2_vff_realign(0.);
    }
    b2_vff_update(alt_float);
  }
#endif
}


void booz_ins_update_gps(void) {

  if (booz_gps_state.fix == GPS_FIX_3D) {
    if (!booz_ins_ltp_initialised) {
      ltp_def_from_ecef_i(&booz_ins_ltp_def, &booz_gps_state.ecef_pos);
      booz_ins_ltp_initialised = TRUE;
    }
    ned_of_ecef_point_i(&booz_ins_gps_pos_cm_ned, &booz_ins_ltp_def, &booz_gps_state.ecef_pos);
    ned_of_ecef_vect_i(&booz_ins_gps_speed_cm_s_ned, &booz_ins_ltp_def, &booz_gps_state.ecef_speed);
#ifdef USE_HFF
    b2ins_update_gps();
    VECT2_SDIV(booz_ins_ltp_pos, (1<<(B2INS_POS_LTP_FRAC-IPOS_FRAC)), b2ins_pos_ltp);
    VECT2_SDIV(booz_ins_ltp_speed, (1<<(B2INS_SPEED_LTP_FRAC-ISPEED_RES)), b2ins_speed_ltp);
#else
    VECT3_COPY(booz_ins_ltp_pos,   b2ins_meas_gps_pos_ned);
    VECT3_COPY(booz_ins_ltp_speed, b2ins_meas_gps_speed_ned);
#endif
    INT32_VECT3_ENU_OF_NED(booz_ins_enu_pos, booz_ins_ltp_pos);
    INT32_VECT3_ENU_OF_NED(booz_ins_enu_speed, booz_ins_ltp_speed);
    INT32_VECT3_ENU_OF_NED(booz_ins_enu_accel, booz_ins_ltp_accel);
  }

}


