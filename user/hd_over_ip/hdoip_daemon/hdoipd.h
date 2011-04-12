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

#include "debug.h"
#include "rscp_include.h"
#include "bstmap.h"

#define CFG_FILE            "/mnt/config/hdoipd.cfg"
#define CFG_RSP_TIMEOUT     20

typedef void (f_task)(void* value);

// state of box
enum {
    HOID_IDLE = 0x01,           // No drivers loaded
    HOID_READY = 0x02,          // Ready for operation
    HOID_SHOW_CANVAS = 0x04,    // Outputs an image from local memory
    HOID_LOOP = 0x08,           // Loops Input to Output
    HOID_VRB = 0x10,            // Configured as VRB (ethernet -> video out)
    HOID_VTB = 0x20             // Configure as VTB (video in -> ethernet)
};

// upstream state for video and audio
enum {
    VTB_OFF         = 0x01,			// no upstream
    VTB_VID_IDLE    = 0x02,			// video configured but not active
    VTB_AUD_IDLE    = 0x04,			// audio configured but not active
    VTB_VIDEO       = 0x08,         // video active
    VTB_AUDIO       = 0x10,         // audio active
    VTB_VID_OFF     = 0x20,			// goto video off (remove idle & active bit)
    VTB_AUD_OFF     = 0x40,			// goto audio off (remove idle & active bit)
    VTB_VID_MASK    = 0x2a,
    VTB_AUD_MASK    = 0x54,
    VTB_VALID       = 0x1f,
    VTB_ACTIVE      = 0x1e
};

// downstream state for audio backchannel
enum {
    VRB_OFF         = 0x01,
    VRB_IDLE        = 0x02,         // not streaming
    VRB_AUDIO       = 0x04,         // streaming audio back
    VRB_AUD_MASK    = 0x06
};

// what is
enum {
    RSC_AUDIO0_IN   = 0x000001,     // active audio input (from video board)
    RSC_AUDIO1_IN   = 0x000002,     // active audio input (from audio board)
    RSC_AUDIO_IN    = 0x000003,     // active audio input
    RSC_VIDEO_IN    = 0x000010,     // active video input
    RSC_VIDEO_SINK  = 0x000100,     // a video sink is connected
    RSC_ETH_LINK    = 0x000200,     // a ethernet link is on
    RSC_AUDIO_OUT   = 0x001000,     // active audio output
    RSC_VIDEO_OUT   = 0x002000,     // active video output
    RSC_OSD         = 0x004000,     // osd output is active
    RSC_VIDEO_SYNC  = 0x010000,     // sync running on video
    RSC_AUDIO_SYNC  = 0x020000,     // sync running on audio
    RSC_SYNC        = 0x030000,     // sync running
    RSC_EVI         = 0x100000,     // ethernet video input
    RSC_EAI         = 0x200000,     // ethernet audio input
    RSC_EVO         = 0x400000,     // ethernet video output
    RSC_EAO         = 0x800000      // ethernet audio output
};

// events
enum {
    EVENT_AUDIO_IN0_ON  = 0x00000001,   // Audio input[0] activated
    EVENT_AUDIO_IN0_OFF = 0x00000002,   // Audio input[0] deactivated
    EVENT_AUDIO_IN1_ON  = 0x00000004,   // Audio input[1] activated
    EVENT_AUDIO_IN1_OFF = 0x00000008,   // Audio input[1] deactivated
    EVENT_VIDEO_IN_ON   = 0x00000010,   // Video input activated
    EVENT_VIDEO_IN_OFF  = 0x00000020,   // Video input deactivated
    EVENT_VIDEO_SINK_ON = 0x00000100,   // Video input activated
    EVENT_VIDEO_SINK_OFF= 0x00000200,   // Video input deactivated
    EVENT_VIDEO_STIN_OFF= 0x00001000,   // Ethernet Video Stream Input stoped
    EVENT_AUDIO_STIN_OFF= 0x00002000,   // Ethernet Audio Stream Input stoped
    EVENT_TICK          = 0x10000000    // a tick event
};

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
    int                 drv;            // used driver hdoipd
    t_bstmap*           registry;       // name=value
    t_bstmap*           verify;         //
    t_bstmap*           set_listener;   // listen for write
    t_bstmap*           get_listener;   // listen for read

    uint32_t            drivers;
    int                 capabilities;   // reported capabilities
    int                 state;          // daemon state
    int                 rsc_state;      // resource input states
    int                 vtb_state;      // vtb state
    int                 vrb_state;      // vrb state
    int                 update;         // pending updates
    pthread_mutex_t     mutex;

    void*               canvas;
    t_rscp_listener     listener;
    t_node*             client;
    int                 fd;
    t_hdoip_eth         local;
    int                 osd_timeout;
    uint64_t            tick;
    int                 eth_alive;
    int                 eth_timeout;
    int			hdcp_hdmi_forced;
    int			hdcp_extern_forced;
} t_hdoipd;


//------------------------------------------------------------------------------
//

extern t_hdoipd                 hdoipd;


static inline bool tick_delay(uint64_t x)
{
    return (hdoipd.tick - x);
}


//------------------------------------------------------------------------------
//

#define reg_set(n, v)           bstmap_set(&hdoipd.registry, (n), (v))
#define reg_get(n)              bstmap_get(hdoipd.registry, (n))
#define reg_get_int(n)          atoi(reg_get(n))
#define reg_test(n, s)          (strcmp(reg_get(n), s) == 0)
#define reg_del(n)              bstmap_remove(&hdoipd.registry, (n))

#define set_listener(n, f)      bstmap_setp(&hdoipd.set_listener, (n), (f))
#define get_listener(n, f)      bstmap_setp(&hdoipd.get_listener, (n), (f))
#define verify_listener(n, f)   bstmap_setp(&hdoipd.verify, (n), (f))

static inline void reg_verify_set(char* n, char* k)
{
    f_task* f = (f_task*)bstmap_get(hdoipd.verify, (n));
    if (f) f(&k);
    reg_set(n, k);
}

static inline void set_call(char* n, char* k)
{
    f_task* f = (f_task*)bstmap_get(hdoipd.set_listener, (n));
    if (f) {
        if (!reg_test(n, k)) {
            reg_set(n, k);
            f(k);
        }
    } else {
        reg_set(n, k);
    }
}

static inline void get_call(char* n, char** k)
{
    f_task* f = (f_task*)bstmap_get(hdoipd.get_listener, (n));
    if (f) f(k); else *k = reg_get(n);
}


//------------------------------------------------------------------------------
//


#define pthread(th, f, d) \
{ \
    int ret = pthread_create(&th, 0, f, d); \
    if (ret) { \
        report(ERROR #f ".pthread_create: failed %d", ret); \
    } else { \
        report(INFO #f ".pthread_create successful"); \
    } \
}

#define pthreada(th, a, f, d) \
{ \
    int ret = pthread_create(&th, a, f, d); \
    if (ret) { \
        report(ERROR #f ".pthread_create: failed %d", ret); \
    } else { \
        report(INFO #f ".pthread_create successful"); \
    } \
}

static inline void lock(const char* s)
{
    if (pthread_mutex_lock(&hdoipd.mutex)) perrno("hdoipd:pthread_mutex_lock() failed");
    report2("hdoipd:pthread_mutex_lock(%d, %s)", pthread_self(), s);
}

static inline void unlock(const char* s)
{
    report2("hdoipd:pthread_mutex_unlock(%d, %s)", pthread_self(), s);
    if (pthread_mutex_unlock(&hdoipd.mutex)) perrno("hdoipd:pthread_mutex_unlock() failed");
}


#endif /* HDOIPD_H_ */
