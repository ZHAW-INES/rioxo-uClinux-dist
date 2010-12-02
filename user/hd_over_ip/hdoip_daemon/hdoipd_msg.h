/*
 * hdoipd_msg.h
 *
 *  Created on: 03.11.2010
 *      Author: alda
 */

#ifndef HDOIPD_MSG_H_
#define HDOIPD_MSG_H_

#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <linux/types.h>
#include <stdint.h>
#include <string.h>


//------------------------------------------------------------------------------
// Commands

#define hoic_group(x)           ((x)&0x0f000000)
#define hoic_command(x)         ((x)&0x0000ffff)

// commands
#define HOIC_CANVAS             (0x31000001)
#define HOIC_LOAD               (0x31000002)
#define HOIC_CAPTURE            (0x31000003)
#define HOIC_LOOP               (0x31000004)
#define HOIC_VTB                (0x31000005)
#define HOIC_VRB_SETUP          (0x31000006)
#define HOIC_VRB_PLAY           (0x31000007)

// Switches
#define HOIC_OSD_ON             (0x32000005)
#define HOIC_OSD_OFF            (0x32000006)

// Singel Parameters
#define HOIC_FMT_IN             (0x34000007)
#define HOIC_FMT_OUT            (0x34000008)
#define HOIC_FMT_PROC           (0x34000009)


//------------------------------------------------------------------------------
//

// compress format
#define HOIC_RAW                (0x00)
#define HOIC_JP2K               (0x01)


//------------------------------------------------------------------------------
// Macros

#define hoic_setup(t, l) \
    t tmp; tmp.msgid = l; tmp.msgsize = sizeof(t)
#define hoic_setupx(t, l, s) \
    t tmp; tmp.msgid = l; tmp.msgsize = sizeof(t) + (s)
#define hoic_write(t) \
    write(fd, &tmp, sizeof(t))


//------------------------------------------------------------------------------
// Plain commands

typedef struct {
    uint32_t            msgid, msgsize;
} __attribute__ ((__packed__)) t_hoic_cmd;

static inline void hoic_sw(int fd, uint32_t id)
{
    hoic_setup(t_hoic_cmd, id);
    hoic_write(t_hoic_cmd);
    fflush(fd);
}

#define hoic_loop(fd) hoic_sw(fd, HOIC_LOOP)
#define hoic_osd_on(fd) hoic_sw(fd, HOIC_OSD_ON)
#define hoic_osd_off(fd) hoic_sw(fd, HOIC_OSD_OFF)
#define hoic_vtb(fd) hoic_sw(fd, HOIC_VTB)
#define hoic_vrb_play(fd) hoic_sw(fd, HOIC_VRB_PLAY)


//------------------------------------------------------------------------------
// attribute/parameter commands

typedef struct {
    uint32_t            msgid, msgsize;
    uint32_t            value;
} __attribute__ ((__packed__)) t_hoic_param;

static inline void hoic_wr(int fd, uint32_t a, uint32_t v)
{
    hoic_setup(t_hoic_param, a);
    tmp.value = v;
    hoic_write(t_hoic_param);
    fflush(fd);
}

#define hoic_fmt_in(fd, v) hoic_wr(fd, HOIC_FMT_IN, v)
#define hoic_fmt_out(fd, v) hoic_wr(fd, HOIC_FMT_OUT, v)
#define hoic_fmt_proc(fd, v) hoic_wr(fd, HOIC_FMT_PROC, v)


//------------------------------------------------------------------------------
// canvas

typedef struct {
    uint32_t            msgid, msgsize;
    uint32_t            width, height, fps;
} __attribute__ ((__packed__)) t_hoic_canvas;

static inline void hoic_canvas(int fd, uint32_t width, uint32_t height, uint32_t fps)
{
    hoic_setup(t_hoic_canvas, HOIC_CANVAS);
    tmp.width = width; tmp.height = height; tmp.fps = fps;
    hoic_write(t_hoic_canvas);
    fflush(fd);
}


//------------------------------------------------------------------------------
// load

typedef struct {
    uint32_t            msgid, msgsize;
    char                filename[];
} __attribute__ ((__packed__)) t_hoic_load;

static inline void hoic_load(int fd, char* filename)
{
    size_t length = strlen(filename) + 1;
    hoic_setupx(t_hoic_load, HOIC_LOAD, length);
    hoic_write(t_hoic_load);
    write(fd, filename, length);
    fflush(fd);
}

static inline void hoic_vrb_setup(int fd, char* uri)
{
    size_t length = strlen(uri) + 1;
    hoic_setupx(t_hoic_load, HOIC_VRB_SETUP, length);
    hoic_write(t_hoic_load);
    write(fd, uri, length);
    fflush(fd);
}


//------------------------------------------------------------------------------
// capture

typedef struct {
    uint32_t            msgid, msgsize;
    uint32_t            compress, size;
    char                filename[];
} __attribute__ ((__packed__)) t_hoic_capture;


static inline void hoic_capture(int fd, uint32_t compress, uint32_t size, char* filename)
{
    size_t length = strlen(filename) + 1;
    hoic_setupx(t_hoic_capture, HOIC_CAPTURE, length);
    tmp.compress = compress;
    tmp.size = size;
    hoic_write(t_hoic_capture);
    write(fd, filename, length);
    fflush(fd);
}


#endif /* HDOIPD_MSG_H_ */
