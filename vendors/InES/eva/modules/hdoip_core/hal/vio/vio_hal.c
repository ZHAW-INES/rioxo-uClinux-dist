#include "vio_hal.h"
#include "vid_const.h"

/** Enable Timing Generator
 * 
 * @param p pointer to i/o base
 */
void vio_enable_output_timing(void* p)
{
    vio_clr_cfg_2(p, VIO_CFG_TG_OUTPUT_ON);
    vio_set_cfg_2(p, VIO_CFG_TG_OUTPUT_ON);
    vio_clr_cfg_2(p, VIO_CFG_TG_OUTPUT_ON);
}

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
int vio_get_timing(void* p, t_video_timing* timing)
{
    unsigned long time;            
    t_video_timing p_vt;
    uint32_t hz;

    if (!timing) return ERR_VIO_NO_INPUT;

    time = jiffies + (HZ/20); // wait max 50ms
    HOI_WR32(p, VIO_OFF_TM_READY, 2);   // reset timing measurement flags
    HOI_WR32(p, VIO_OFF_TM_READY, 0);
    while(HOI_RD32(p, VIO_OFF_TM_READY) != 1) {  // wait until timing is valid
        if (time_after(jiffies, time)) {
            REPORT(INFO, "Timeout: vio_get_timing()");
            return ERR_VIO_NO_INPUT;
        }
    }

    p_vt.id         = timing->id;
    p_vt.hpolarity  = timing->hpolarity;
    p_vt.vpolarity  = timing->vpolarity;
    p_vt.fpolarity  = timing->fpolarity;

    p_vt.interlaced = 0;
    p_vt.vpulse_1   = 0;
    p_vt.vfront_1   = 0;
    p_vt.vback_1    = 0;
    p_vt.height_1   = 0;

    p_vt.pfreq      = vio_get_fin(p);
    
    p_vt.width      = HOI_RD16(p, VIO_OFF_TM_HA);
    p_vt.hfront     = HOI_RD16(p, VIO_OFF_TM_H);
    p_vt.hpulse     = HOI_RD16(p, VIO_OFF_TM_HP);
    p_vt.hback      = HOI_RD16(p, VIO_OFF_TM_HB);
    p_vt.height     = HOI_RD16(p, VIO_OFF_TM_VA);
    p_vt.vfront     = HOI_RD16(p, VIO_OFF_TM_V);
    p_vt.vpulse     = HOI_RD16(p, VIO_OFF_TM_VP);
    p_vt.vback      = HOI_RD16(p, VIO_OFF_TM_VB);

    // check if timing is valid 
    hz = vid_fps(&p_vt);
    if ((p_vt.height < 100) || (p_vt.width < 100) || (hz < 20) || (hz > 85)) {
        REPORT(INFO, "vio_get_timing(): timing not valid (%iHz)", hz);
        return ERR_VIO_NO_INPUT;
    }

    // return measured timing only if it is ok
    memcpy(timing, &p_vt, sizeof(t_video_timing));
 
    return ERR_VIO_SUCCESS;
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

/** Set the Video Clock input multiplexer
 * 
 * @param p pointer to i/o base
 * @param advcnt number of used ADV212
 * @param hdmi_active HDMI or SDI (HDMI = true)
 */
void vio_set_input_clockmux(void* p, int advcnt, bool hdmi_active)
{
    uint32_t temp;

    temp = HOI_RD32(p, VIO_OFF_CONFIG_2) & ~(VIO_CFG_INPUT_MUX_0 | VIO_CFG_INPUT_MUX_1 | VIO_CFG_INPUT_MUX_2);

    if (hdmi_active) {
        temp |= VIO_CFG_INPUT_MUX_0 + VIO_CFG_INPUT_MUX_1 + VIO_CFG_INPUT_MUX_2;    // advclk = inclk     | vidclk = inclk      | Source = PLL
    } else {
        switch (advcnt) {
            case 1:     temp |= VIO_CFG_INPUT_MUX_1;                                // advclk = inclk     | vidclk = inclk / 2  | Source = SI598
                        break;
            case 2:
            case 3:     temp |= VIO_CFG_INPUT_MUX_0 + VIO_CFG_INPUT_MUX_1;          // advclk = inclk     | vidclk = inclk      | Source = SI598
                        break;
            case 4:     temp |= VIO_CFG_INPUT_MUX_0;                                // advclk = inclk / 2 | vidclk = inclk      | Source = SI598
                        break;
            default:    break;
        }
    }

    HOI_WR32(p, VIO_OFF_CONFIG_2, temp);
}

