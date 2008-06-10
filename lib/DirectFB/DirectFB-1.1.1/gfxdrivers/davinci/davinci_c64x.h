/*
   TI Davinci driver - C64X+ DSP Library

   (c) Copyright 2007  Telio AG

   Written by Denis Oliver Kropp <dok@directfb.org>

   All rights reserved.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef __DAVINCI_C64X_H__
#define __DAVINCI_C64X_H__

#include <unistd.h>

#include <directfb.h>

#include <direct/messages.h>
#include <direct/trace.h>

#include <linux/c64x.h>


typedef struct {
     int              magic;

     int              fd;
     c64xTaskControl *ctl;
     void            *mem;

     c64xTask        *QueueL;
} DavinciC64x;


DFBResult davinci_c64x_open ( DavinciC64x *c64x );

DFBResult davinci_c64x_close( DavinciC64x *c64x );


/**********************************************************************************************************************/

static inline c64xTask *
c64x_get_task( DavinciC64x *c64x )
{
     c64xTaskControl *ctl  = c64x->ctl;
     int              idx  = ctl->QL_arm;
     c64xTask        *task = &c64x->QueueL[idx];
     int              next = (idx + 1) & 0x3fff;

     /* The entry should be free as we are... */
     D_ASSERT( !(task->c64x_flags & C64X_FLAG_TODO) );

     /* ...waiting for one spare entry between DSP head and ARM tail. */
     while (ctl->QL_dsp == next) {
          //direct_log_printf( NULL, "%s() sleeping... (dsp %d, arm %d)\n", __FUNCTION__, ctl->QL_dsp, ctl->QL_arm );

          /* FIXME: ioctl */
          usleep( 100 );
     }

     return task;
}

static inline void
c64x_submit_task( DavinciC64x *c64x )
{
     /* DSP head is at least two entries ahead, we can safely extend the ARM tail. */
     c64x->ctl->QL_arm = (c64x->ctl->QL_arm + 1) & 0x3fff;
}

/**********************************************************************************************************************/

static inline DFBResult
davinci_c64x_wait_low( DavinciC64x *c64x )
{
     c64xTaskControl *ctl = c64x->ctl;

     while (ctl->QL_dsp != ctl->QL_arm) {
          //direct_log_printf( NULL, "%s() sleeping... (dsp %d, arm %d)\n", __FUNCTION__, ctl->QL_dsp, ctl->QL_arm );

          /* FIXME: ioctl */
          usleep( 100 );
     }

     return DFB_OK;
}

static inline void
davinci_c64x_write_back_all( DavinciC64x *c64x )
{
     c64xTask *task = c64x_get_task( c64x );

     task->c64x_function = C64X_WRITE_BACK_ALL | C64X_FLAG_TODO;

     c64x_submit_task( c64x );
}

/**********************************************************************************************************************/

static inline void
davinci_c64x_load_block( DavinciC64x   *c64x,
                         unsigned long  words,
                         u32            num,
                         u32            flags )
{
     c64xTask *task = c64x_get_task( c64x );

     task->c64x_function = C64X_LOAD_BLOCK | C64X_FLAG_TODO;

     task->c64x_arg[0] = words;
     task->c64x_arg[1] = num;
     task->c64x_arg[2] = flags;

     c64x_submit_task( c64x );
}

static inline void
davinci_c64x_put_uyvy_16x16( DavinciC64x   *c64x,
                             unsigned long  dest,
                             u32            pitch,
                             unsigned long  source,
                             u32            flags )
{
     c64xTask *task = c64x_get_task( c64x );

     task->c64x_function = C64X_PUT_UYVY_16x16 | C64X_FLAG_TODO;

     task->c64x_arg[0] = dest;
     task->c64x_arg[1] = pitch;
     task->c64x_arg[2] = source;
     task->c64x_arg[3] = flags;

     c64x_submit_task( c64x );
}

/**********************************************************************************************************************/

static inline void
davinci_c64x_dither_argb( DavinciC64x   *c64x,
                          unsigned long  dst_rgb,
                          unsigned long  dst_alpha,
                          u32            dst_pitch,
                          unsigned long  source,
                          u32            src_pitch,
                          u32            width,
                          u32            height )
{
     c64xTask *task = c64x_get_task( c64x );

     task->c64x_function = C64X_DITHER_ARGB | C64X_FLAG_TODO;

     task->c64x_arg[0] = dst_rgb;
     task->c64x_arg[1] = dst_alpha;
     task->c64x_arg[2] = dst_pitch;
     task->c64x_arg[3] = source;
     task->c64x_arg[4] = src_pitch;
     task->c64x_arg[5] = width;
     task->c64x_arg[6] = height;

     c64x_submit_task( c64x );
}

static inline void
davinci_c64x_fill_16( DavinciC64x   *c64x,
                      unsigned long  dest,
                      u32            pitch,
                      u32            width,
                      u32            height,
                      u32            value )
{
     c64xTask *task = c64x_get_task( c64x );

     task->c64x_function = C64X_FILL_16 | C64X_FLAG_TODO;

     task->c64x_arg[0] = dest;
     task->c64x_arg[1] = pitch;
     task->c64x_arg[2] = width;
     task->c64x_arg[3] = height;
     task->c64x_arg[4] = value;

     c64x_submit_task( c64x );
}

static inline void
davinci_c64x_fill_32( DavinciC64x   *c64x,
                      unsigned long  dest,
                      u32            pitch,
                      u32            width,
                      u32            height,
                      u32            value )
{
     c64xTask *task = c64x_get_task( c64x );

     task->c64x_function = C64X_FILL_32 | C64X_FLAG_TODO;

     task->c64x_arg[0] = dest;
     task->c64x_arg[1] = pitch;
     task->c64x_arg[2] = width;
     task->c64x_arg[3] = height;
     task->c64x_arg[4] = value;

     c64x_submit_task( c64x );
}

static inline void
davinci_c64x_blit_16( DavinciC64x   *c64x,
                      unsigned long  dest,
                      u32            dpitch,
                      unsigned long  src,
                      u32            spitch,
                      u32            width,
                      u32            height )
{
     c64xTask *task = c64x_get_task( c64x );

     task->c64x_function = C64X_COPY_16 | C64X_FLAG_TODO;

     task->c64x_arg[0] = dest;
     task->c64x_arg[1] = dpitch;
     task->c64x_arg[2] = src;
     task->c64x_arg[3] = spitch;
     task->c64x_arg[4] = width;
     task->c64x_arg[5] = height;

     c64x_submit_task( c64x );
}

static inline void
davinci_c64x_blit_32( DavinciC64x   *c64x,
                      unsigned long  dest,
                      u32            dpitch,
                      unsigned long  src,
                      u32            spitch,
                      u32            width,
                      u32            height )
{
     c64xTask *task = c64x_get_task( c64x );

     task->c64x_function = C64X_COPY_32 | C64X_FLAG_TODO;

     task->c64x_arg[0] = dest;
     task->c64x_arg[1] = dpitch;
     task->c64x_arg[2] = src;
     task->c64x_arg[3] = spitch;
     task->c64x_arg[4] = width;
     task->c64x_arg[5] = height;

     c64x_submit_task( c64x );
}

static inline void
davinci_c64x_blit_blend_32( DavinciC64x   *c64x,
                            unsigned long  dest,
                            u32            dpitch,
                            unsigned long  src,
                            u32            spitch,
                            u32            width,
                            u32            height,
                            u32            sub_func,
                            u8             alpha )
{
     c64xTask *task = c64x_get_task( c64x );

     task->c64x_function = (sub_func << 16) | C64X_BLEND_32 | C64X_FLAG_TODO;

     task->c64x_arg[0] = dest;
     task->c64x_arg[1] = dpitch;
     task->c64x_arg[2] = src;
     task->c64x_arg[3] = spitch;
     task->c64x_arg[4] = width;
     task->c64x_arg[5] = height;
     task->c64x_arg[6] = alpha;

     c64x_submit_task( c64x );
}

static inline void
davinci_c64x_blit_keyed_16( DavinciC64x   *c64x,
                            unsigned long  dest,
                            u32            dpitch,
                            unsigned long  src,
                            u32            spitch,
                            u32            width,
                            u32            height,
                            u32            key,
                            u32            mask )
{
     c64xTask *task = c64x_get_task( c64x );

     task->c64x_function = C64X_COPY_KEYED_16 | C64X_FLAG_TODO;

     task->c64x_arg[0] = dest;
     task->c64x_arg[1] = (dpitch << 16) | (spitch & 0xffff);
     task->c64x_arg[2] = src;
     task->c64x_arg[3] = width;
     task->c64x_arg[4] = height;
     task->c64x_arg[5] = key;
     task->c64x_arg[6] = mask;

     c64x_submit_task( c64x );
}

static inline void
davinci_c64x_blit_keyed_32( DavinciC64x   *c64x,
                            unsigned long  dest,
                            u32            dpitch,
                            unsigned long  src,
                            u32            spitch,
                            u32            width,
                            u32            height,
                            u32            key,
                            u32            mask )
{
     c64xTask *task = c64x_get_task( c64x );

     task->c64x_function = C64X_COPY_KEYED_32 | C64X_FLAG_TODO;

     task->c64x_arg[0] = dest;
     task->c64x_arg[1] = (dpitch << 16) | (spitch & 0xffff);
     task->c64x_arg[2] = src;
     task->c64x_arg[3] = width;
     task->c64x_arg[4] = height;
     task->c64x_arg[5] = key;
     task->c64x_arg[6] = mask;

     c64x_submit_task( c64x );
}

#endif

