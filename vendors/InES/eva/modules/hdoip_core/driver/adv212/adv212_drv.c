#include "adv212_drv.h"
#include "adv212_str.h"
#include "adv212_decode_2_14_1.h"
#include "adv212_encode_2_13_0.h"

#define adv212_set(p, o, a, v) { uint32_t *_p_ = (p); uint32_t _o_ = 2*(o); \
                                 _p_[_o_] = (a); \
                                 _p_[_o_+1] = (v); \
                               }
#define adv212_param(a, b, c, d) (((a)<<24)|((b)<<16)|((c)<<8)|((d)<<0))
#define adv212_rc(a, b) (((a)<<24)|((b)<<0))

// [COUNT][INDEX] Components/Chip
static const int adv_comp[4][4] = {
        {3, 0, 0, 0},
        {1, 2, 0, 0},
        {1, 1, 1, 0},
        {1, 2, 1, 2}
    };

// [COUNT][INDEX] (4:double, 2:same, 1:half -horitontal pixel count)
static const int adv_scale[4][4] = {
        {4, 0, 0, 0},
        {2, 2, 0, 0}, 
        {2, 2, 2, 0},
        {1, 1, 1, 1}
    };

// [COUNT][INDEX] Pixel delay/Chip decode
static const int adv_decd[4][4] = {
        {12, 0, 0, 0},
        {11,11, 0, 0},
        {11,11,11, 0},
        {11,11,11,11}
    };

// [COUNT][INDEX] Pixel delay/Chip encode
static const int adv_encd[4][4] = {
        { 0, 0, 0, 0},
        { 0, 0, 0, 0}, 
        { 0, 0, 0, 0},
        { 0, 0, 0, 0}
    };
    
/** Set Default custom encode parameter
 * 
 * 8 Bit Color, 9x7 irreversible transform
 * rate control of size
 * 
 * @param p pointer to parameter set with 4 entrys (64 Byte)
 * @param size the initial compression size in byte/image
 * @param interlaced interlaced video enable
 */
void adv212_enc_custom_format(uint32_t* p, int size, int interlaced) 
{  
    int ADV212_CFG_CBSIZE;

    if (interlaced)
        ADV212_CFG_CBSIZE = ADV212_CFG_CBSIZE_I;
    else    
        ADV212_CFG_CBSIZE = ADV212_CFG_CBSIZE_P;

    adv212_set(p, 0, ADV212_PARAMETER_BASE+ 0, adv212_param(
        ADV212_VFORMAT_CUSTOM,
        ADV212_PREC_10BIT,
        ADV212_CFG_TLEVEL,
        ADV212_UNI_UNIPOLAR_Y | ADV212_UNI_UNIPOLAR_C ));

    adv212_set(p, 1, ADV212_PARAMETER_BASE+ 4, adv212_param(
        ADV212_CFG_CBSIZE,
        ADV212_CFG_WAVLET,
        ADV212_STALLPAR_NONE,
        ADV212_ATTRTYPE_NO ));

    adv212_set(p, 2, ADV212_PARAMETER_BASE+ 8, adv212_rc(
        ADV212_RCTYPE_IMGSIZE,
        size & 0xffffff ));

    adv212_set(p, 3, ADV212_PARAMETER_BASE+12, adv212_param(
        ADV212_J2KPROG_LRCP,
        ADV212_CFG_IF,
        0,
        ADV212_CFG_CODE ));
    
}


/** Set Default custom decode parameter
 * 
 * @param p pointer to parameter set with 2 entries (32 Byte)
 */
void adv212_dec_custom_format(uint32_t* p) 
{  
    adv212_set(p, 0, ADV212_PARAMETER_BASE+ 0, adv212_param(
        ADV212_VFORMAT_CUSTOM,
        ADV212_PREC_10BIT,
        0,
        ADV212_UNI_UNIPOLAR_Y | ADV212_UNI_UNIPOLAR_C ));
    adv212_set(p, 1, ADV212_PARAMETER_BASE+ 8, adv212_param(
        0,
        ADV212_CFG_IF,
        ADV212_DRES_KEEP,
        ADV212_CFG_CODE ));
    
} 

/** Converts bit count to PMODE1 label
 *
 * @param bit number of bits per color component
 * @return the PMODE1 label corresponding to the color depth
 */
static uint32_t adv212_color_depth(int bit)
{
    uint32_t m = ADV212_PMODE1_PREC_8BIT;
    
    switch (bit) {
        case 8: m = ADV212_PMODE1_PREC_8BIT; break;
        case 10: m = ADV212_PMODE1_PREC_10BIT; break;
        case 12: m = ADV212_PMODE1_PREC_12BIT; break;
    }

    return m;
}


/** Init register set
 * 
 * Setup default register
 * 
 * @param p pointer to register set
 * @param o offset within register set
 * @return next unused offset in set
 */
int adv212_reg_init(uint32_t* p, int o)
{
    adv212_set(p, o+0, ADV212_PMODE2,           ADV212_PMODE2_VCLK_P | 
                                                ADV212_PMODE2_UNIPOLAR_Y | 
                                                ADV212_PMODE2_UNIPOLAR_C |
                                                ADV212_PMODE2_VSYNC_P | 
                                                ADV212_PMODE2_HSYNC_P | 
                                                ADV212_PMODE2_FIELD_N);
    adv212_set(p, o+1, ADV212_V0_REGION_ST,     0);
    adv212_set(p, o+2, ADV212_V1_REGION_ST,     0);
    adv212_set(p, o+3, ADV212_V0_REGION_END,    0xffff);
    adv212_set(p, o+4, ADV212_V1_REGION_END,    0xffff);
    adv212_set(p, o+5, ADV212_PIXEL_START_REF,  1);
    adv212_set(p, o+6, ADV212_PIXEL_END_REF,    0);
    return o+7;
}

/** Set Encoder HVF mode
 *
 * the register set needs enough free space to add this register.
 *
 * @param p base pointer to register set
 * @param o offset within register set
 * @return next free offset within register set
 */
int adv212_reg_enc_hvf(uint32_t* p, int o, int interlace_en)
{
    int regval;

    if (interlace_en)
        regval = ADV212_VMODE_ENCODE | ADV212_VMODE_HVF;
    else
        regval = ADV212_VMODE_ENCODE | ADV212_VMODE_HVF | ADV212_VMODE_PRGRSV_SCN;

    adv212_set(p, o, ADV212_VMODE, regval);

    return o+1;
}

/** Set Decoder HVF slave mode
 *
 * the register set needs enough free space to add this register.
 *
 * @param p base pointer to register set
 * @param o offset within register set
 * @return next free offset within register set
 */
int adv212_reg_dec_hvf_slave(uint32_t* p, int o, int interlace_en)
{
    int regval;

    if (interlace_en)
        regval = ADV212_VMODE_SLAVE | ADV212_VMODE_DECODE | ADV212_VMODE_HVF;
    else 
        regval = ADV212_VMODE_SLAVE | ADV212_VMODE_DECODE | ADV212_VMODE_HVF | ADV212_VMODE_PRGRSV_SCN;

    adv212_set(p, o, ADV212_VMODE, regval);

    return o+1; 
}

/** Set Code FIFO threshold
 *
 * @param p base pointer to register set
 * @param o offset within register set
 * @return next free offset within register set
 */
int adv212_reg_cfth(uint32_t* p, int o, int x)
{
    adv212_set(p, o, ADV212_FFTHRC,         x);
    return o+1; 
}

/** Set ADV212 mode
 *
 * the register set needs enough free space to add this register.
 *
 * @param p base pointer to register set
 * @param o offset within register set
 * @param bit color component bit count
 * @param cmp 1, 2 or 3 components to be encoded
 * @return next free offset within register set
 */
int adv212_reg_mode(uint32_t* p, int o, int bit, int cmp)
{
    switch (cmp) {
        case 1: adv212_set(p, o, ADV212_PMODE1, ADV212_PMODE1_PFMT_V_SINGLE         | adv212_color_depth(bit)); break;
        case 2: adv212_set(p, o, ADV212_PMODE1, ADV212_PMODE1_PFMT_V_INTERLEAVED_C  | adv212_color_depth(bit)); break;
        case 3: adv212_set(p, o, ADV212_PMODE1, ADV212_PMODE1_PFMT_V_INTERLEAVED_CY | adv212_color_depth(bit)); break;
    }
    return o+1;
}

/** Set Horizontal timing
 *
 * Note: values are in pixels not clock cycles!
 * the register set needs enough free space to add this register.
 *
 * @param p base pointer to register set
 * @param o offset within register set
 * @param w visible width
 * @param fp front porch
 * @param hp horizontal pulse length
 * @param bp back porch
 * @param pd adv212 delay in clock cycles
 * @param s pixel/clock relation (1: half image, 1 or 2 component, 2: full image, 1 or 2 component, 4: full image, 3 components)
 * @return next free offset within register set
 */
int adv212_reg_hsamples(uint32_t* p, int o, uint32_t w, uint32_t fp, uint32_t hp, uint32_t bp, uint32_t pd, int s)
{
    // H starts with index '1' at first pixel after rising edge hsync
    adv212_set(p, o+0, ADV212_XTOT,         (fp + hp + bp + w) * s / 2);
    adv212_set(p, o+1, ADV212_PIXEL_START,  (hp + bp) * s / 2 + 1 - pd);
    adv212_set(p, o+2, ADV212_PIXEL_STOP,   (hp + bp + w) * s / 2 - pd);

    return o+3; 
}

/** Set Vertical timing
 *
 * Note: values are in line count
 * the register set needs enough free space to add this register.
 *
 * @param p base pointer to register set
 * @param o offset within register set
 * @param h visible height field 0
 * @param fp front porch field 0
 * @param vp vertical pulse length field 0
 * @param bp back porch field 0
*  @param h_1 visible height field 1
*  @param fp_1 front porch field 1
*  @param vp_1 vertical pulse length field 1
*  @param bp_1 back porch field 1
 * @return next free offset within register set
 */
int adv212_reg_vsamples(uint32_t* p, int o, uint32_t h, uint32_t fp, uint32_t vp, uint32_t bp, uint32_t h_1, uint32_t fp_1, uint32_t vp_1, uint32_t bp_1, uint32_t interlaced, int decoder)
{
    // V starts with index '1' at first line after falling edge vsync
    adv212_set(p, o+0, ADV212_V0_START,         bp + 1);
    adv212_set(p, o+1, ADV212_V0_END,           bp + h);

    if (interlaced) {
        adv212_set(p, o+3, ADV212_YTOT,         fp + vp + bp + h + fp_1 + vp_1 + bp_1 + h_1 + 1);    // (+ 1) because fp_1 and bp_1 are a half line longer
        if (decoder) {
            adv212_set(p, o+2, ADV212_F0_START, bp + h - vp/2);
            adv212_set(p, o+6, ADV212_F1_START, bp + h + fp_1 + vp_1 + bp_1 + h_1 + 1 - vp/2);
            // correct vertical offset of field 1 ((+1) why is this required?)
            adv212_set(p, o+4, ADV212_V1_START, bp + h + fp_1 + vp_1 + bp_1 + 2);
            adv212_set(p, o+5, ADV212_V1_END,   bp + h + fp_1 + vp_1 + bp_1 + h_1 + 1);
        } else {
            adv212_set(p, o+4, ADV212_V1_START, bp + h + fp_1 + vp_1 + bp_1 + 1);
            adv212_set(p, o+5, ADV212_V1_END,   bp + h + fp_1 + vp_1 + bp_1 + h_1);
            // mysterious cause but this improves performance for interlaced video
            adv212_set(p, o+2, ADV212_F0_START, 0);
            adv212_set(p, o+6, ADV212_F1_START, bp + h + fp_1 + vp_1);
        }
    }
    else {
        adv212_set(p, o+3, ADV212_YTOT,         fp + vp + bp + h);
        adv212_set(p, o+4, ADV212_V1_START,     0);
        adv212_set(p, o+5, ADV212_V1_END,       0);
        adv212_set(p, o+2, ADV212_F0_START,     0);
        adv212_set(p, o+6, ADV212_F1_START,     0);
    }

    return o+7;
}

/** Calculates the number of ADV212 required for a given video format
 * 
 * The algorithm tests for the given constraint on VCLK, tile size
 * and sampels per second.
 * 
 * @param p_vt the video timing format
 * @param adv_cnt the result is stored in this location
 * @return the error code (SUCCESS if no error)
 */
int adv212_drv_advcnt(t_video_timing* p_vt, int* adv_cnt)
{
    int adv_div, adv_siz, adv_min;
    int ret = ERR_ADV212_SUCCESS;
    int img_size = p_vt->width * p_vt->height;
    int width = p_vt->width + p_vt->hback + p_vt->hpulse + p_vt->hfront;
    int height = p_vt->height + p_vt->vback + p_vt->vpulse + p_vt->vfront;
    int sps = (int)((int64_t)p_vt->pfreq * 2 * (int64_t)img_size / ((int64_t)width * (int64_t)height));
    
    *adv_cnt = 0;

    // minimum adv count based on frequency constraint
    if ((2 * p_vt->pfreq) <= ADV212_MAX_VCLK) {
        adv_div = 1;
    } else if (p_vt->pfreq <= ADV212_MAX_VCLK) {
        adv_div = 2;
    } else if (p_vt->pfreq <= (2 * ADV212_MAX_VCLK)) {
        adv_div = 4;
    } else {
        return ERR_ADV212_PIXEL_CLOCK_TOOHIGH;
    }
    
    // minimum adv count based on tile size
    if ((2*img_size) <= ADV212_MAX_SIZE) {
        adv_siz = 1;
    } else if (img_size <= ADV212_MAX_SIZE) {
        adv_siz = 2;
    } else if ((img_size / 2) <= ADV212_MAX_SIZE) {
        adv_siz = 4;
    } else {
        return ERR_ADV212_PIXEL_COUNT_TOOHIGH;
    }
    
    // minimum adv count based on sample rate
    if (sps <= ADV212_MAX_SAMPLES) {
        adv_min = 1;
    } else if (sps <= (2 * ADV212_MAX_SAMPLES)) {
        adv_min = 2;
    } else if (sps <= (4 * ADV212_MAX_SAMPLES)) {
        adv_min = 4;
    } else {
        return ERR_ADV212_SAMPLE_RATE_TOOHIGH;
    }

    // maximum of all three constraint
    if (adv_min < adv_siz) adv_min = adv_siz;
    if (adv_min < adv_div) adv_min = adv_div;
    
    *adv_cnt = adv_min;
    
    return ret;
}

/** switch all ADV212 off
 * 
 * @param p hdata base ponter
 */
void adv212_drv_init(void* p)
{
    REPORT(INFO, "<adv212_drv_init()>");

    // Reset all ADV212
    adv212_pll_init(ADV212_0_BASE(p));
    adv212_pll_init(ADV212_1_BASE(p));
    adv212_pll_init(ADV212_2_BASE(p));
    adv212_pll_init(ADV212_3_BASE(p));
}

/** switch all ADV212 off
 *
 * @param p_adv adv212 configuration struct
 * @return size per adv
 */
int adv212_size_per_chip(t_adv212* p_adv)
{
    return p_adv->size / p_adv->cnt;
}

/** Split the available size between the ADV212 chips
 * 
 * @param size size in byte
 * @param p_adv
 */
void adv212_drv_rc_size(void* p, uint32_t size, t_adv212* p_adv, uint32_t chroma)
{
    p_adv->size = size;
    size = adv212_size_per_chip(p_adv);

    switch (p_adv->cnt) {
        case 1: adv212_set_rc_size(ADV212_0_BASE(p), size); 
        break;
        case 2: adv212_set_rc_size(ADV212_0_BASE(p), size+(size/100*(100-chroma)));
                adv212_set_rc_size(ADV212_1_BASE(p), size/100*chroma); 
        break;
        case 3: adv212_set_rc_size(ADV212_0_BASE(p), size);
                adv212_set_rc_size(ADV212_1_BASE(p), size);
                adv212_set_rc_size(ADV212_2_BASE(p), size); 
        break;
        case 4: adv212_set_rc_size(ADV212_0_BASE(p), size+(size/100*(100-chroma)));
                adv212_set_rc_size(ADV212_1_BASE(p), size/100*chroma);
                adv212_set_rc_size(ADV212_2_BASE(p), size+(size/100*(100-chroma)));
                adv212_set_rc_size(ADV212_3_BASE(p), size/100*chroma);
        break;
    }
}

/** encoder: detect missing settings and test for validity
 *
 * @param p_vt the video timing struct
 * @param p_adv adv212 configuration struct
 * @return error code (0 on success)
 */
int adv212_drv_detect_enc(t_video_timing* p_vt, t_adv212* p_adv)
{
    int ret, adv_min;
    
    REPORT(INFO, "<adv212_drv_detect_enc()>");
    
    // autocalculate minimum adv number
    if ((ret = adv212_drv_advcnt(p_vt, &adv_min))) {
        // autodetection failed
        return ret;
    }
    
    if (p_adv->cnt < adv_min) {
        p_adv->cnt = adv_min;
    }
    
    if ((p_adv->cnt < 1) || (p_adv->cnt > 4)) {
        // out of range
        ret = ERR_ADV212_ADVCNT_OUT_OF_RANGE;
    }
        
    return ret;
}

/** decoder: detect missing settings and test for validity
 *
 * @param p_vt the video timing struct
 * @param p_adv adv212 configuration struct
 * @return error code (0 on success)
 */
int adv212_drv_detect_dec(t_video_timing* p_vt, t_adv212* p_adv)
{
    int ret, adv_min;
    
    REPORT(INFO, "<adv212_drv_detect_dec()>");
    
    // autocalculate minimum adv number
    if ((ret = adv212_drv_advcnt(p_vt, &adv_min))) {
        // autodetection failed
        return ret;
    }

    if ((p_adv->cnt < adv_min) || (p_adv->cnt > 4)) {
        // out of range
        ret = ERR_ADV212_ADVCNT_OUT_OF_RANGE;
    }
    
    return ret;
}

/** Boot the ADV212 for encode mode
 * 
 * @param p hdata base ponter
 * @param p_vt video timing struct
 * @param p_adv adv configuration struct
 * @return error code (0 on success)
 */
int adv212_drv_boot_enc(void* p, t_video_timing* p_vt, t_adv212* p_adv)
{
    int ret;
    uint32_t param[2*ADV212_PARAM_ENC_COUNT];
    uint32_t reg[2*ADV212_REGISTER_COUNT];    
    int size, o;

    REPORT(INFO, "<adv212_drv_boot_enc()>");
         
    size = adv212_size_per_chip(p_adv);
    
    // Initialize all ADV212 because of shared bus (HDATA)
    adv212_drv_init(p);
    
    // setup all needed ADV212    
    for (int i=0; i<p_adv->cnt; i++) {
        
        // Setup parameter
        adv212_enc_custom_format(param, size, p_vt->interlaced);    

        // Setup register
        o = 0;
        o = adv212_reg_init    (reg, o);
        o = adv212_reg_enc_hvf (reg, o, p_vt->interlaced);
        o = adv212_reg_mode    (reg, o, 10, adv_comp[p_adv->cnt-1][i]);
        o = adv212_reg_hsamples(reg, o, p_vt->width, p_vt->hfront, p_vt->hpulse, p_vt->hback, adv_encd[p_adv->cnt-1][i], adv_scale[p_adv->cnt-1][i]);
        o = adv212_reg_vsamples(reg, o, p_vt->height, p_vt->vfront, p_vt->vpulse, p_vt->vback, p_vt->height_1, p_vt->vfront_1, p_vt->vpulse_1, p_vt->vback_1, p_vt->interlaced, 0);
        
        // Boot ADV
        if ((ret = adv212_boot_jdata(OFFSET(p, i * ADV212_SIZE),
                (uint32_t*)adv212_encode_2_13_0, size_adv212_encode_2_13_0, ADV212_SWFLAG_ENCODE,
                param, ADV212_PARAM_ENC_COUNT,
                reg, ADV212_REGISTER_COUNT, (p_adv->cnt > 1)))) {
            ADV212_REPORT_BOOT(i, ret);
            return ret;
        }        

    }

    // sync all needed ADV212
    if (p_adv->cnt > 1) {
        for (int i=0; i<p_adv->cnt; i++) {
            if ((ret = adv212_boot_sync(OFFSET(p, i * ADV212_SIZE)))) {
                ADV212_REPORT_BOOT(i, ret);
                return ret;
            }
        }
    }
    
    return ret;
}

/** Boot the ADV212 for decode mode
 * 
 * @param p hdata base ponter
 * @param p_vt video timing struct
 * @param p_adv adv configuration struct
 * @return error code (0 on success)
 */
int adv212_drv_boot_dec(void* p, t_video_timing* p_vt, t_adv212* p_adv)
{
    int ret = SUCCESS;
    uint32_t param[2*ADV212_PARAM_DEC_COUNT];
    uint32_t reg[2*ADV212_REGISTER_COUNT];    
    int o;

    REPORT(INFO, "<adv212_drv_boot_dec()>");
            
    // Initialize all ADV212 because of shared bus (HDATA)
    adv212_drv_init(p);

    // setup all needed ADV212    
    for (int i=0; i<p_adv->cnt; i++) {
        
        // Setup parameter
        adv212_dec_custom_format(param);
        
        // Setup register
        o = 0;
        o = adv212_reg_init         (reg, o);
        o = adv212_reg_dec_hvf_slave(reg, o, p_vt->interlaced);
        o = adv212_reg_mode         (reg, o, 10, adv_comp[p_adv->cnt-1][i]);
        o = adv212_reg_hsamples     (reg, o, p_vt->width, p_vt->hfront, p_vt->hpulse, p_vt->hback, adv_decd[p_adv->cnt-1][i], adv_scale[p_adv->cnt-1][i]);  
        o = adv212_reg_vsamples     (reg, o, p_vt->height, p_vt->vfront, p_vt->vpulse, p_vt->vback, p_vt->height_1, p_vt->vfront_1, p_vt->vpulse_1, p_vt->vback_1, p_vt->interlaced, 1);


        // Boot ADV
        if ((ret = adv212_boot_jdata(OFFSET(p, i * ADV212_SIZE),
                (uint32_t*)adv212_decode_2_14_1, size_adv212_decode_2_14_1, ADV212_SWFLAG_DECODE,
                param, ADV212_PARAM_DEC_COUNT,
                reg, ADV212_REGISTER_COUNT, (p_adv->cnt > 1)))) {
            ADV212_REPORT_BOOT(i, ret);
            return ret;
        }

    }

    return ret;
}

/** Sync the decoding ADV212
 *
 * @param p hdata base ponter
 * @param p_adv adv configuration struct
 * @return error code (0 on success)
 */
int adv212_drv_boot_dec_sync(void* p, t_adv212* p_adv, void* p_vio)
{
    for (int i=0; i<p_adv->cnt; i++) {
        adv212_boot_sync_wait(OFFSET(p, i * ADV212_SIZE), p_vio);
    }

    for (int i=0; i<p_adv->cnt; i++) {
        // Ack SWIRQ0 IRQ
        adv212_boot_sync_ack(OFFSET(p, i * ADV212_SIZE));
    }

    return SUCCESS;
}

