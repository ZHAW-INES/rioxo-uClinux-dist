#include "stdvid.h"


/** get frames per second from the timing
 * 
 * @param p_vt the video timing struct
 * @return frames per second
 */
uint32_t vid_fps(t_video_timing* p_vt)
{
    uint32_t h, v, p;
    
    h = p_vt->hfront + p_vt->hpulse + p_vt->hback + p_vt->width;
    v = p_vt->vfront + p_vt->vpulse + p_vt->vback + p_vt->height;
    
    p = v * h;
      
    return (p_vt->pfreq + (p/2)) / p;
}

uint32_t vid_pixel_per_frame(t_video_timing* p_vt)
{
    return p_vt->width * p_vt->height;
}

uint32_t vid_clock_per_frame(t_video_timing* p_vt)
{
    uint32_t h, v;
    
    h = p_vt->hfront + p_vt->hpulse + p_vt->hback + p_vt->width;
    v = p_vt->vfront + p_vt->vpulse + p_vt->vback + p_vt->height;

    return h * v;
}

uint32_t vid_duration_in_us(t_video_timing* p_vt)
{
    uint64_t cc = (uint64_t)vid_clock_per_frame(p_vt);

    return (uint32_t)((cc * (uint64_t)1000000 + (uint64_t)(p_vt->pfreq >> 1)) / p_vt->pfreq);
}

uint32_t vid_duration_in_ns(t_video_timing* p_vt)
{
    uint64_t cc = (uint64_t)vid_clock_per_frame(p_vt);

    return (uint32_t)((cc * (uint64_t)1000000000 + (uint64_t)(p_vt->pfreq >> 1)) / p_vt->pfreq);
}
