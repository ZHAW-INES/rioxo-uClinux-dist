#include "vio_hal.h"
#include "vid_const.h"

/** Setup Video Timing Generator
 * 
 * @param p pointer to i/o base
 * @param p_vt pointer to timing struct
 * @param triggersource: 1=extern, 0=intern
 */
void vio_set_timing(void* p, t_video_timing* p_vt, int triggersource)
{
    int h, v;
    
    h = p_vt->hfront + p_vt->hpulse + p_vt->hback + p_vt->width;
    v = p_vt->vfront + p_vt->vpulse + p_vt->vback + p_vt->height;

    if (p_vt->interlaced) {
        v += p_vt->vfront_1 + p_vt->vpulse_1 + p_vt->vback_1 + p_vt->height_1 + 1;
    }

    if (p_vt->interlaced) {
        // use extern trigger source
        if (triggersource)
            vio_set_cfg(p, VIO_CFG_TG_VSEL);
        else
            vio_clr_cfg(p, VIO_CFG_TG_VSEL);
        vio_set_cfg(p, VIO_CFG_INTERLACE);
    }
    else {
        vio_clr_cfg(p, VIO_CFG_TG_VSEL);
        vio_clr_cfg(p, VIO_CFG_INTERLACE);
    }

    // H x V
    vio_set_rng(p, VIO_OFF_TG_SIZE,  h - 1, v - 1);

    // HSYNC pulse
    vio_set_rng(p, VIO_OFF_TG_HSYNC, 0, p_vt->hpulse);
    // Active Video (Horizontal)
    vio_set_rng(p, VIO_OFF_TG_HAVID, p_vt->hpulse + p_vt->hback, p_vt->hpulse + p_vt->hback + p_vt->width);

    // Field 0 : VSYNC
    vio_set_rng(p, VIO_OFF_TG_VSYNC, 0, p_vt->vpulse);
    // Field 0 : AVID Vertical
    vio_set_rng(p, VIO_OFF_TG_VAVID, p_vt->vpulse + p_vt->vback, p_vt->vpulse + p_vt->vback + p_vt->height);
    // Field 1 : VSYNC
    vio_set_rng(p, VIO_OFF_TG_VSYNC2, p_vt->vpulse + p_vt->vback + p_vt->height + p_vt->vfront_1, p_vt->vpulse + p_vt->vback + p_vt->height + p_vt->vfront_1 + p_vt->vpulse_1);        
    // Field 1 : AVID Vertical
    vio_set_rng(p, VIO_OFF_TG_VAVID2, p_vt->vpulse + p_vt->vback + p_vt->height + p_vt->vfront_1 + p_vt->vpulse_1 + p_vt->vback_1 + 1, p_vt->vpulse + p_vt->vback + p_vt->height + p_vt->vfront_1 + p_vt->vpulse_1 + p_vt->vback_1 + p_vt->height_1 + 1);
    // Field signal
    vio_set_rng(p, VIO_OFF_TG_FIELD, p_vt->vpulse + p_vt->vback + p_vt->height + p_vt->vfront_1 + p_vt->vpulse_1 + p_vt->vback_1 + p_vt->height_1 + (p_vt->vfront_1/2), p_vt->vpulse + p_vt->vback + p_vt->height + (p_vt->vfront/2));
    // Half horizontal pixel count
    vio_set_hsplit(p, (p_vt->width / 2) - 1);

    // Frame Length
    HOI_WR32(p, VIO_OFF_TG_DURATION, (h * v) / 2 - 1);
}

/** Read Input Video Frequency
 * 
 * @param p pointer to i/o base
 * @param p_vt pointer to timing struct to be filled
 */
void vio_get_input_frequency(void* p, t_video_timing* p_vt)
{
    p_vt->pfreq = vio_get_fin(p);
}

/** Read Input Video Timing
 * 
 * @param p pointer to i/o base
 * @param p_vt pointer to timing struct to be filled
 * @param wait 1 = wait until measured timings are ready
 */
void vio_get_timing(void* p, t_video_timing* p_vt)
{
    int h,v,hp,vp,ha,va,hb,vb;
    unsigned long time;            
    
    if (!p_vt) return;

    // wait until measured timings are valid (max 50ms)
    time = jiffies + (HZ/20); // 50ms
    HOI_WR32(p, VIO_OFF_TM_READY, 0);
    while(HOI_RD32(p, VIO_OFF_TM_READY) < 2) {
        if (time_after(jiffies, time)) {
            break;
        }
    }

    v = HOI_RD16(p, VIO_OFF_TM_V);
    h = HOI_RD16(p, VIO_OFF_TM_H);
    vp = HOI_RD16(p, VIO_OFF_TM_VP);
    hp = HOI_RD16(p, VIO_OFF_TM_HP);
    vb = HOI_RD16(p, VIO_OFF_TM_VB);
    hb = HOI_RD16(p, VIO_OFF_TM_HB);
    va = HOI_RD16(p, VIO_OFF_TM_VA);
    ha = HOI_RD16(p, VIO_OFF_TM_HA);

    p_vt->interlaced = 0;
    p_vt->vpulse_1   = 0;
    p_vt->vfront_1   = 0;
    p_vt->vback_1    = 0;
    p_vt->height_1   = 0;

    p_vt->pfreq = vio_get_fin(p);
        
    p_vt->hpolarity = (hp < (h >> 1)); 
    p_vt->vpolarity = (vp < (v >> 1)); 
    
    if (p_vt->hpolarity) {
        // Positive
        p_vt->width = ha;
        p_vt->hfront = hb - hp;
        p_vt->hpulse = hp;
        p_vt->hback = h - ha - hb;        
    } else {
        // Negative
        p_vt->width = ha;
        p_vt->hfront = hb;
        p_vt->hpulse = h - hp;
        p_vt->hback = hp - ha - hb;        
    }
    
    if (p_vt->vpolarity) {
        // Positive
        p_vt->height = va;
        p_vt->vfront = v - va - vb;
        p_vt->vpulse = vp;
        p_vt->vback = vb - vp;        
    } else {
        // Negative
        p_vt->height = va;
        p_vt->vfront = vp - vb - va;
        p_vt->vpulse = v - vp;
        p_vt->vback = vb;        
    }
}

/** Activates the pll control logic
 * 
 * the pll controll allows the pll to be adjusted
 * 
 * @param p pointer to i/o base
 * @param p_vt pointer to timing struct
 * @param ppm how accurate the timing should be followed in [parts per million]
 * @param sel select the control source
 */
void vio_set_control(void* p, t_video_timing* p_vt, int ppm, int sel)
{
	int32_t h, v, s, t;
    
    if (sel == VIO_MUX_PLLC_TG) {

        if (p_vt->interlaced)
            v = p_vt->vfront + p_vt->vpulse + p_vt->vback + p_vt->height + p_vt->vfront_1 + p_vt->vpulse_1 + p_vt->vback_1 + p_vt->height_1 + 1;
        else
            v = p_vt->vfront + p_vt->vpulse + p_vt->vback + p_vt->height;

        h = p_vt->hfront + p_vt->hpulse + p_vt->hback + p_vt->width;

        // select control source
        vio_set_pllc(p, sel);
        
        // Control tolerance (24 bit fractional)
        s = (int32_t)((int64_t)(1<<(24+VIO_PLL_CTRL_WIDTH)) / VIO_PLL_CTRL_DIV / ((int64_t)ppm * (int64_t)h * (int64_t)v / 1000000));
        // Threshold (pixel count)
        t = (int32_t)((int64_t)ppm * VIO_PLL_CTRL_MAX * (int64_t)h * (int64_t)v / 1000000);

        HOI_WR32(p, VIO_OFF_TG_THRESHOLD, t);
        HOI_WR32(p, VIO_OFF_TG_SCALE, s);
    }

    if (sel != VIO_MUX_PLLC_FREE) {
        // activate control logic
        vio_set_cfg(p, VIO_CFG_PLL_CTRL);
    }
    
}


/** Measures the video input frequency
 * 
 * This value is updated every 1 Million Clock cycle from sclk
 * ex.: sclk = 50 MHz -> update interval = 20ms
 * 
 * @param p pointer to i/o base
 * @return frequency in [Hz]
 */
uint32_t vio_get_fin(void* p)
{
    uint64_t tmp = HOI_RD32(p, VIO_OFF_FIN);
    
    tmp = (tmp * SFREQ) / 1000000;
    
    return (uint32_t)tmp;
}


/** Measures the video output frequency
 * 
 * This value is updated every 1 Million Clock cycle from sclk
 * ex.: sclk = 50 MHz -> update interval = 20ms
 * 
 * @param p pointer to i/o base
 * @return frequency in [Hz]
 */
uint32_t vio_get_fout(void* p)
{
    uint64_t tmp = HOI_RD32(p, VIO_OFF_FOUT);
    
    tmp = (tmp * SFREQ) / 1000000;
    
    return (uint32_t)tmp;
}


/** Measures the clock error
 * 
 * This value is updated every Video frame.
 * The value represents the difference from actual vsync pulse, 
 * to the required vsync pulse
 * 
 * @param p pointer to i/o base
 * @return captured error
 */
int32_t vio_get_tg_error(void* p)
{
    int32_t tmp = HOI_RD32(p, VIO_OFF_TG_CAPTURE);
    
    // sign expansion
    if (tmp & 0x00800000) {
        tmp = tmp | 0xff000000;
    }
     
    return tmp;
}


/** Measures the PLL frequency shift
 * 
 * This value is updated every Video frame.
 * The value represents the shift in frequecy from the pll
 * 
 * @param p pointer to i/o base
 * @return captured shift
 */
int32_t vio_get_pll_error(void* p)
{
    int32_t tmp = HOI_RD32(p, VIO_OFF_PLL_STATUS);
    if (tmp & VIO_PLL_STA_ACTIVE) {
        if (tmp & VIO_PLL_STA_DRIFT_SIGN) {
            tmp = ((tmp & VIO_PLL_STA_DRIFT_MASK) >> VIO_PLL_STA_DRIFT_SHIFT) | ~(int32_t)(VIO_PLL_STA_DRIFT_SIGN-1);
        } else {
            tmp = (tmp & VIO_PLL_STA_DRIFT_MASK) >> VIO_PLL_STA_DRIFT_SHIFT;
        }
    } else {
        tmp = 0;
    }
    
    return tmp;
}


/** Init the Video Pixelprocessing
 * 
 * @param p pointer to i/o base
 * @param o pixel processing offset
 * @param m 3x4 transformation matrix
 */
void vio_set_transform(void* p, uint32_t o, t_color_transform m, uint32_t cfg, bool vpol, bool hpol, bool invert_cb_cr)
{
    if (!m) { m = (void*)xyz_one; }
    
    p = OFFSET(p, o);
    
    // transform:
    HOI_WR16(p, VIO_OFF_PP_M11, (m[0][0]));
    HOI_WR16(p, VIO_OFF_PP_M12, (m[0][1]));
    HOI_WR16(p, VIO_OFF_PP_M13, (m[0][2]));
    HOI_WR16(p, VIO_OFF_PP_M21, (m[1][0]));
    HOI_WR16(p, VIO_OFF_PP_M22, (m[1][1]));
    HOI_WR16(p, VIO_OFF_PP_M23, (m[1][2]));
    HOI_WR16(p, VIO_OFF_PP_M31, (m[2][0]));
    HOI_WR16(p, VIO_OFF_PP_M32, (m[2][1]));
    HOI_WR16(p, VIO_OFF_PP_M33, (m[2][2]));
    // offset
    HOI_WR16(p, VIO_OFF_PP_V1, (m[0][3]));
    HOI_WR16(p, VIO_OFF_PP_V2, (m[1][3]));
    HOI_WR16(p, VIO_OFF_PP_V3, (m[2][3]));

    cfg = cfg | (vpol ? VIO_PP_VPOL_P : VIO_PP_VPOL_N);
    cfg = cfg | (hpol ? VIO_PP_HPOL_P : VIO_PP_HPOL_N);

    // invert cb and cr of chroma channel
    // if (invert_cb_cr) {
    //     cfg = cfg | VIO_PP_DFIRST;
    // }

    HOI_WR32(p, VIO_OFF_PP_CFG, cfg);
}

