/*
 * debug.h
 *
 *  Created on: 04.01.2011
 *      Author: alda
 */

#ifndef DEBUG_H_
#define DEBUG_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "hdoip_log.h"

//------------------------------------------------------------------------------
// Debug Switches

#define VID_IN_PATH
#define VID_OUT_PATH
#define AUD_IN_PATH
#define AUD_OUT_PATH
#define ETI_PATH

#define USE_OSD_TIMER
#define USE_LAUNCHER
#define USE_SYS_TICK



// main log file
#define MAIN_LOG
#undef REPORT_RSCP
#undef REPORT_RSCP_CLIENT
#undef REPORT_RSCP_SERVER
#undef REPORT_RSCP_HELLO
#undef REPORT_RSCP_RX
#undef REPORT_PTHREAD
#undef REPORT_RSCP_UPDATE
#undef REPORT_ALIVE_HELLO

// RSCP log file
#undef RSCP_LOG
#undef REPORT_RSCP_PACKETS

#define DBGCONSOLE          // x
#define DBGCONSOLERSCP     // x

//#undef DBGCONSOLE          // x
//#undef DBGCONSOLERSCP     // x

#define EDID_WRITE_HEX_FILE
#undef REPORT_EDID

//------------------------------------------------------------------------------
//

#define STATE       " § "
#define ERROR       " ? "
#define NEW         " + "
#define DEL         " - "
#define INFO        " i "
#define INCOMING    " < "
#define OUTGOING    " > "
#define EVENT       " ← "
#define CHANGE      " → "
#define CONT        "     "
#define DEBUG       "------------------------------------------------------------- \n d "
// #define VTB_METHOD  " M "
#define VTB_METHOD  "------------------------------------------------------------- \n M "
// #define VRB_METHOD  " M "
#define VRB_METHOD  "------------------------------------------------------------- \n M "

#define UNUSED __attribute__((__unused__))
/*
#define report(...) { \
    hdoip_report(&hdoipd.main_log, __VA_ARGS__); \
}
*/
/*
#define report(...) { \
    fprintf(report_fd, __VA_ARGS__); \
    fprintf(report_fd, "\n"); \
    fflush(report_fd); \
}

#define reportn(...) { \
    fprintf(report_fd, __VA_ARGS__); \
}

#define perrno(...) { \
    fprintf(report_fd, "[error] %s (%d): %s\n", __FILE__, __LINE__, strerror(errno)); \
    fprintf(report_fd, __VA_ARGS__); \
    fprintf(report_fd, "\n"); \
    fflush(report_fd); \
}
*/

#ifdef REPORT_PTHREAD
    #define report2(...) report(__VA_ARGS__)
#else
    #define report2(...) {;}
#endif

#endif /* DEBUG_H_ */
