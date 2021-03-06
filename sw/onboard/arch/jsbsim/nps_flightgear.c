#include <signal.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <math.h>

#include <glib.h>

#include "std.h"
#include "led.h"
#include "generated/settings.h"

#include "lib/spawn.h"

#include "nps_flightgear.h"
#include "nps_fdm.h"

#define MODEL_PREFIX    "/usr/share/games/FlightGear/"
#define MODEL_PATH      "Models/Aircraft/wasp/mikrokopter.xml"

/* THIS STRUCTURE IS TAKEN FROM THE FLIGHTGEAR SOURCE, AND MUST BE KEPT UP}
   TO DATE WITH FLIGHTGEAR, ALTHOUGH PRACTICALLY SPEAKING, IT HAS NOT BEEN
   CHANGED IN YEARS */
#define FG_NET_GUI_VERSION 7
#define FG_NET_GUI_MAX_TANKS 4
struct FGNetGUI {
    uint32_t version;           // increment when data values change

    // Positions
    double longitude;           // geodetic (radians)
    double latitude;            // geodetic (radians)
    float  altitude;            // above sea level (meters)
    float agl;                  // above ground level (meters)
    float phi;                  // roll (radians)
    float theta;                // pitch (radians)
    float psi;                  // yaw or true heading (radians)

    // Velocities
    float vcas;
    float climb_rate;           // feet per second

    // Consumables
    uint32_t num_tanks;         // Max number of fuel tanks
    float fuel_quantity[FG_NET_GUI_MAX_TANKS];

    // Environment
    uint32_t cur_time;          // current unix time
                                // FIXME: make this uint64_t before 2038
    uint32_t warp;              // offset in seconds to unix time
    float ground_elev;          // ground elev (meters)

    // Approach
    float tuned_freq;           // currently tuned frequency
    float nav_radial;           // target nav radial
    uint32_t in_range;          // tuned navaid is in range?
    float dist_nm;              // distance to tuned navaid in nautical miles
    float course_deviation_deg; // degrees off target course
    float gs_deviation_deg;     // degrees off target glide slope
};

static struct  {
    int disabled;
    int socket;
    struct sockaddr_in addr;
} flightgear;


void nps_flightgear_init(const char* host,  unsigned int port) {
    int so_reuseaddr;
    struct protoent *pte;

    flightgear.socket = -1;
    flightgear.disabled = FALSE;

    if (g_getenv("WASP_NO_FLIGHTGEAR")) {
        flightgear.disabled = TRUE;
        return;
    }


    /* if the host is 127.0.0.1 or localhost then also start flightgear */
    if ((g_strcmp0(host, "127.0.0.1") == 0) || (g_strcmp0(host, "localhost") == 0)) {
        char *cmd = NULL;
        char *model_command = NULL;

        if (g_file_test(MODEL_PREFIX MODEL_PATH, G_FILE_TEST_EXISTS))
            model_command = g_strdup_printf("--prop:/sim/model/path=" MODEL_PATH);

        cmd = g_strdup_printf(
                    "fgfs --fdm=null --native-gui=socket,in,%d,%s,%d,udp %s",
                    NPS_FLIGHTGEAR_UPDATE_FREQUENCY,
                    host,
                    port,
                    model_command);

        led_log("FLIGHTGEAR: %s\n", cmd);

        /* Setup the SIGCHILD handler, as flightgear is spawned occasionally */
        sigset(SIGCHLD, &spawn_sigchild_sighandler);
        /* Start flightgear */
        spawn_background_command(cmd,NULL,NULL,NULL);

        g_free(cmd);
    }

    so_reuseaddr = 1;
    pte = getprotobyname("UDP");

    flightgear.socket = socket( PF_INET, SOCK_DGRAM, pte->p_proto);
    setsockopt(flightgear.socket, SOL_SOCKET, SO_REUSEADDR, 
         &so_reuseaddr, sizeof(so_reuseaddr));
    flightgear.addr.sin_family = PF_INET;
    flightgear.addr.sin_port = htons(port);
    flightgear.addr.sin_addr.s_addr = inet_addr(host);
}

void nps_flightgear_send() {

    struct FGNetGUI gui;

    if (flightgear.disabled)
        return;

    gui.version = FG_NET_GUI_VERSION; 

    gui.latitude  = fdm.lla_pos.lat;
    gui.longitude = fdm.lla_pos.lon;
    gui.altitude  = fdm.lla_pos.alt;
    //printf("%f %f %f\n", gui.latitude, gui.longitude, gui.altitude);

    gui.agl = 1.111652;

    gui.phi = fdm.ltp_to_body_eulers.phi;
    gui.theta = fdm.ltp_to_body_eulers.theta;
    gui.psi = fdm.ltp_to_body_eulers.psi;

    gui.vcas = 0.;
    gui.climb_rate = 0.;

    gui.num_tanks = 1;
    gui.fuel_quantity[0] = 0.;

    //  gui.cur_time = 3198060679ul;
    gui.cur_time = 3198060679ul + rint(fdm.time);
    ///  gui.cur_time = 3198101679ul;
    gui.warp = 1122474394ul;

    gui.ground_elev = 0.;

    gui.tuned_freq = 125.65;
    gui.nav_radial = 90.;
    gui.in_range = 1;
    gui.dist_nm = 10.;
    gui.course_deviation_deg = 0.;
    gui.gs_deviation_deg = 0.;


    if (sendto(flightgear.socket, (char*)(&gui), sizeof(gui), 0,
             (struct sockaddr*)&flightgear.addr, sizeof(flightgear.addr)) == -1)
    {
        printf("error sending\n");
    }
}
