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

#undef REPORT_PTHREAD
#undef REPORT_RSCP

#define DBGCONSOLE
#undef DBGCONSOLERSCP

#define EDID_WRITE_HEX_FILE

//------------------------------------------------------------------------------
//

extern FILE* report_fd;
extern FILE* rscp_fd;

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

#define UNUSED __attribute__((__unused__))

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

#ifdef REPORT_PTHREAD
    #define report2(...) report(__VA_ARGS__)
#else
    #define report2(...) {;}
#endif

#endif /* DEBUG_H_ */
