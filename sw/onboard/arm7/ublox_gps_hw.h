#ifndef BOOZ2_GPS_H
#define BOOZ2_GPS_H

#include "std.h"
#include "config/config.h"
#include "pprz_geodetic_int.h"

struct Booz_gps_state {
  struct EcefCoor_i ecef_pos;    /* pos ECEF in cm        */
  struct EcefCoor_i ecef_speed;  /* speed ECEF in cm/s    */
  uint32_t pacc;                 /* position accuracy     */
  uint32_t sacc;                 /* speed accuracy        */
  uint16_t pdop;                 /* dilution of precision */
  uint8_t  num_sv;               /* number of sat in fix  */
  uint8_t  fix;                  /* status of fix         */
};

extern struct Booz_gps_state booz_gps_state;

/* UBX NAV POSLLH */
extern int32_t  booz2_gps_lon;
extern int32_t  booz2_gps_lat;
extern int32_t  booz2_gps_hmsl;
extern uint32_t booz2_gps_hacc;
extern uint32_t booz2_gps_vacc;

/* UBX NAV SOL */
#define  BOOZ2_GPS_FIX_NONE 0x00
#define  BOOZ2_GPS_FIX_3D   0x03

/* UBX NAV VELNED */
extern int32_t  booz2_gps_vel_n;
extern int32_t  booz2_gps_vel_e;

#include "ubx_protocol.h"

#define Booz2GpsEvent(_sol_available_callback) {			\
    if (GpsBuffer()) {							\
      ReadGpsBuffer();							\
    }									\
    if (ubx_msg_available) {						\
      booz2_gps_read_ubx_message();					\
      if (ubx_class == UBX_NAV_ID && ubx_id == UBX_NAV_VELNED_ID) {	\
	_sol_available_callback();					\
      }									\
      ubx_msg_available = FALSE;					\
    }									\
  }

extern void booz2_gps_init(void);
extern void booz2_gps_read_ubx_message(void);


#define __GpsLink(dev, _x) dev##_x
#define _GpsLink(dev, _x)  __GpsLink(dev, _x)
#define GpsLink(_x) _GpsLink(GPS_LINK, _x)

#define GpsBuffer() GpsLink(ChAvailable())
#define ReadGpsBuffer() {				\
    while (GpsLink(ChAvailable())&&!ubx_msg_available)	\
      ubx_parse(GpsLink(Getch()));			\
  }

/* UBX parsing - copied from gps_ubx.c */

extern bool_t  ubx_msg_available;
extern uint8_t ubx_msg_buf[];
extern uint8_t ubx_id;
extern uint8_t ubx_class;

extern void ubx_parse( uint8_t c );
extern void ubx_init(void);




#endif /* BOOZ2_GPS_H */


