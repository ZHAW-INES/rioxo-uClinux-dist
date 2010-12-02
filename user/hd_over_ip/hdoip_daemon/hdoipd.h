/*
 * hdoipd.h
 *
 *  Created on: 03.11.2010
 *      Author: alda
 */

#ifndef HDOIPD_H_
#define HDOIPD_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "rtsp_include.h"

enum {
    HOID_IDLE,              // No drivers loaded
    HOID_READY,             // Ready for operation
    HOID_SHOW_CANVAS,       // Outputs an image from local memory
    HOID_LOOP,              // Loops Input to Output
    HOID_VRB,               // Configured as VRB (ethernet -> video out)
    HOID_VTB                // Configure as VTB (video in -> ethernet)
};

enum {
    VTB_OFF,
    VTB_IDLE,               // nothing active
    VTB_VIDEO,              // streaming video only
    VTB_AUDIO,              // streaming audio only
    VTB_AV,                 // streaming audio and video
};

enum {
    VRB_OFF,
    VRB_IDLE,               // nothing active
    VRB_AUDIO               // streaming audio back
};

// what is
#define RSC_AUDIO_IN        0x01        // active audio input
#define RSC_VIDEO_IN        0x02        // active video input
#define RSC_VIDEO_SINK      0x04        // a video sink is connected
#define RSC_AUDIO_OUT       0x08        // active audio output
#define RSC_VIDEO_OUT       0x10        // active video otuput
#define RSC_OSD             0x20        // osd is active

typedef struct {
    uint32_t            address;        // remote ip address
    uint8_t             mac[6];         // mac address
    uint32_t            vid_port;
    uint32_t            aud_port;
} t_hdoip_eth;

typedef struct {
    int                 drv;            // used driver handle
    int                 capabilities;   // reported capabilities
    int                 state;          // daemon state
    int                 rsc_state;      // bitmap: resource input states
    int                 vtb_state;      // vtb send state (idle, video, audio, av)
    int                 vrb_state;      // vrb send state (idle, audio)
    pthread_mutex_t     mutex;

    void*               canvas;
    uint32_t            drivers;
    t_rtsp_listener     listener;
    t_rtsp_client       client;
    int                 fd;
    t_hdoip_eth         local;
} t_hdoipd;

extern int report_fd;

#define report(...) { \
    fprintf(report_fd, __VA_ARGS__); \
    fprintf(report_fd, "\n"); \
    fflush(report_fd); \
}

#define perrno(...) { \
    fprintf(report_fd, "[error] %s (%d): %s\n", __FILE__, __LINE__, strerror(errno)); \
    fprintf(report_fd, __VA_ARGS__); \
    fprintf(report_fd, "\n"); \
    fflush(report_fd); \
}

#endif /* HDOIPD_H_ */
