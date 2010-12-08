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

#include "bstmap.h"
#include "rtsp_include.h"
#include "hoi_cfg.h"

typedef void (f_task)(void* value);

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
    f_task*             fnc;
    void*               value;
} t_task;

typedef struct {
    t_bstmap*           registry;       // name=value
    t_bstmap*           reg_listener;   // listen for write
    t_node*             tasklet;        // will be executed
    int                 drv;            // used driver hdoipd
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
    t_hoi_cfg           config;
} t_hdoipd;


//------------------------------------------------------------------------------
//

extern t_hdoipd hdoipd;

#define lock()                  pthread_mutex_lock(&hdoipd.mutex)
#define unlock()                pthread_mutex_unlock(&hdoipd.mutex)

#define reg_print(n)            bstmap_print(hdoipd.registry, (n))
#define reg_set(n, v)           bstmap_set(&hdoipd.registry, (n), (v))
#define reg_get(n)              bstmap_get(hdoipd.registry, (n))
#define reg_get_all(a, i)       bstmap_get_all(hdoipd.registry, (a), (i))
#define reg_cnt(c)              bstmap_cnt_elements(hdoipd.registry, (c))
#define reg_listener(n, f)      bstmap_setp(&hdoipd.reg_listener, (n), (f))
#define reg_call(n, k) \
{ \
    f_task* f = bstmap_get(&hdoipd.reg_listener, (n)); \
    if (f) f(k); \
}

#define task_add(f, v) \
{ \
    t_task* t= malloc(sizeof(t_task)); \
    t->fnc = f; t->value = v; \
    queue_put(&hdoipd.tasklet, t); \
}


//------------------------------------------------------------------------------
//

extern FILE* report_fd;
extern FILE* rtsp_fd;

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
