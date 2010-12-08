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
#include <err.h>
#include <errno.h>


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

// Switches
#define HOIC_PARAM_GET          (0x3800000a)
#define HOIC_PARAM_SET          (0x3800000b)



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
#define hoic_response(t) \
    t rsp; hoic_rdpipe(fdr, (void*)&rsp, sizeof(t))

//------------------------------------------------------------------------------
// read

static inline ssize_t hoic_rdpipe(int rsp, uint8_t *buf, size_t size)
{
    ssize_t ret;
    size_t offset = 0;

    while ((offset<size)&&rsp) {
        ret = read(rsp, buf+offset, size-offset);
        if (ret <= 0) {
            return ret;
        }
        offset = offset + ret;
    };

    return ret;
}


//------------------------------------------------------------------------------
// Plain commands

typedef struct {
    uint32_t            msgid, msgsize;
} __attribute__ ((__packed__)) t_hoic_cmd;

static inline void hoic_sw(int fd, uint32_t id)
{
    hoic_setup(t_hoic_cmd, id);
    hoic_write(t_hoic_cmd);
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
}

static inline void hoic_vrb_setup(int fd, char* uri)
{
    size_t length = strlen(uri) + 1;
    hoic_setupx(t_hoic_load, HOIC_VRB_SETUP, length);
    hoic_write(t_hoic_load);
    write(fd, uri, length);
}



//------------------------------------------------------------------------------
// parameter

typedef struct {
    uint32_t            msgid, msgsize;
    size_t              offset;
    char                str[];
} __attribute__ ((__packed__)) t_hoic_kvparam;

static inline void hoic_set_param(int fd, char* name, char* value)
{
    size_t length1 = strlen(name) + 1;
    size_t length2 = strlen(value) + 1;
    hoic_setupx(t_hoic_kvparam, HOIC_PARAM_SET, length1+length2);
    tmp.offset = length1;
    hoic_write(t_hoic_kvparam);
    write(fd, name, length1);
    write(fd, value, length2);
}

static inline char* hoic_get_param(int fd, int fdr, char* name)
{
    size_t length1 = strlen(name) + 1;
    hoic_setupx(t_hoic_kvparam, HOIC_PARAM_GET, length1);
    tmp.offset = length1;
    hoic_write(t_hoic_kvparam);
    write(fd, name, length1);

    hoic_response(t_hoic_kvparam);
    char* ret = malloc(rsp.offset);
    hoic_rdpipe(fdr, ret, rsp.offset);
    return ret;
}

static inline void hoic_ret_param(int fd, char* value)
{
    size_t length1 = strlen(value) + 1;
    hoic_setupx(t_hoic_kvparam, HOIC_PARAM_GET, length1);
    tmp.offset = length1;
    hoic_write(t_hoic_kvparam);
    write(fd, value, length1);
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
}


#endif /* HDOIPD_MSG_H_ */
