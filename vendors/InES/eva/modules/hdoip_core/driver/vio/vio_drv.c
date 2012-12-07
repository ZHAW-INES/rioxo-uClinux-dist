#include "vio_drv.h"
#include "adv212_str.h"
#include "vid_const.h"
#include "adv7441a_drv.h"
#include "gs2971_drv.h"
#include "hoi_msg.h"
#include "si598.h"

/** Sampling conversion
 */
#define S444TO444    (0)
#define S444TO422    (VIO_PP_DOWN)
#define S422TO444    (VIO_PP_UP)
#define S422TO422    (VIO_PP_UP|VIO_PP_DOWN)

// [input][output] sampling format conversion
static const int smtf[2][2] = {
        //  SM_444      SM_422
        {   S444TO444,  S444TO422   },  // SM_444
        {   S422TO444,  S422TO422   },  // SM_422
    };

// [advcnt-1] jvclk = vclk * jrel/2
static const int jrel[4] = {
        4, 2, 2, 1
    };

// [advcnt-1] adv color format
static const uint32_t advfmt[4] = {
        vio_format(CS_YUV, SM_422), // 1x ADV212
        vio_format(CS_YUV, SM_422), // 2x ADV212
        vio_format(CS_RGB, SM_444), // 3x ADV212
        vio_format(CS_YUV, SM_422)  // 4x ADV212
    };

/** convert bandwidth to size per image for CBR
 *
 * @param bandwidth bandwidth in byte/second
 * @param p_vt video timing struct
 * @return size in byte for an image
 */
static int vio_bandwidth_to_size(int64_t bandwidth, t_video_timing* p_vt)
{
    return (int32_t)(((int64_t)bandwidth * (int64_t)vid_clock_per_frame(p_vt)) / p_vt->pfreq);
}

/** Set a specific input format
 *
 * @param p vio hardware base address
 * @param p_vt video timing struct
 * @param f0 input format
 * @param f1 internal format
 * @return error code (0 on success)
 */
static int vio_set_input_format(void* p, t_video_timing* p_vt, t_video_format f0, t_video_format f1)
{
    void *tf = (void*)(coltf[vio_format_cs(f0)][vio_format_cs(f1)]);
	uint32_t cfg =  smtf[vio_format_sm(f0)][vio_format_sm(f1)];
    
	vio_set_transform(p, VIO_OFF_INPUT_PT, tf, cfg, p_vt->vpolarity, p_vt->hpolarity, false);
    
	VIO_REPORT_TRANSFORM(p + VIO_OFF_INPUT_PT, "input");

    return ERR_VIO_SUCCESS;    
}

/** Set a specific output format
 *
 * @param p vio hardware base address
 * @param p_vt video timing struct
 * @param f0 internal format
 * @param f1 output format
 * @return error code (0 on success)
 */
static int vio_set_output_format(void* p, t_video_timing* p_vt, t_video_format f0, t_video_format f1, bool invert_cb_cr)
{
    void *tf = (void*)(coltf[vio_format_cs(f0)][vio_format_cs(f1)]);
	uint32_t cfg =  smtf[vio_format_sm(f0)][vio_format_sm(f1)];

	vio_set_transform(p, VIO_OFF_OUTPUT_PT, tf, cfg, p_vt->vpolarity, p_vt->hpolarity, invert_cb_cr);

	VIO_REPORT_TRANSFORM(p + VIO_OFF_OUTPUT_PT, "output");

    return ERR_VIO_SUCCESS;    
}
 
/** Set as output format a black picture
 *
 * @param handle vio handle
 * @return error code (0 on success)
 */
int vio_drv_set_black_output(t_vio* handle) 
{
    void *tf = (void *) black_out_rgb; //_yuv;
    uint32_t cfg = S444TO444;

    vio_set_transform(handle->p_vio, VIO_OFF_OUTPUT_PT, tf, cfg, handle->timing.vpolarity, handle->timing.hpolarity, false);

    return ERR_VIO_SUCCESS;
}   

/** Restores the output format 
 *
 * @param handle vio handle
 * @return error code (0 on success)
 */
int vio_drv_clr_black_output(t_vio* handle)
{
    vio_set_output_format(handle->p_vio, &handle->timing, handle->format_proc, handle->format_out, false);

    return ERR_VIO_SUCCESS;
}

/** Initialize driver for vio
 * 
 * initializes the handle struct, which allows proper modifing
 * of a running system.
 * 
 * @param handle vio handle 
 * @param p_vio the vio hardware pointer
 * @param p_adv the adv212-hdata hardware pointer 
 */
int vio_drv_init(t_vio* handle, void* p_vio, void* p_adv, t_si598 *si598)
{
    PTR(handle); PTR(p_vio); PTR(p_adv);
    REPORT(INFO, ">> VIO-Driver: initialized");

    handle->p_vio = p_vio;
    handle->p_adv = p_adv;
    handle->si598 = si598;

    vio_drv_reset(handle, BDT_ID_HDMI_BOARD);
    
    VIO_REPORT_BASE(handle);

    return ERR_VIO_SUCCESS;
}

/** Halts driver for vio(keeps settings)
 *
 * @param handle vio handle
 */
int vio_drv_halt(t_vio* handle)
{
    vio_reset(handle->p_vio);
    adv212_drv_init(handle->p_adv);

    handle->active = 0;

    return ERR_VIO_SUCCESS;
}

/** Setup OSD
 *
 * Initializes Font used by OSD
 *
 * @param handle vio handle
 * @param font a bitmap font table
 */
int vio_drv_setup_osd(t_vio* handle, t_osd_font* font, uint32_t device)
{
	vio_osd_init(handle->p_vio, font, device);

    vio_enable_output_timing(handle->p_vio);

    handle->osd.x = 0;
    handle->osd.y = 0;

	VIO_REPORT_FONT(font);

	return ERR_VIO_SUCCESS;
}

/** Reset driver/hardware for vio
 * 
 * reinitializes the handle struct, which allows proper modifing
 * of a running system.
 * 
 * @param handle vio handle 
 */
int vio_drv_reset(t_vio* handle, uint32_t device)
{
	handle->config 		= 0;
	handle->active		= false;

    switch (device) {
        case (BDT_ID_HDMI_BOARD):
            handle->format_in   = vio_format(CS_RGB, SM_444);
            handle->format_out  = vio_format(CS_RGB, SM_444);
            break;
        case (BDT_ID_SDI8_BOARD):
            // SDI can be 444 or 422, correct format should be set before video is starting
            handle->format_in   = vio_format(CS_YUV, SM_422);
            handle->format_out  = vio_format(CS_YUV, SM_422);
            break;
        default:
            REPORT(ERROR, "VIO-Driver: could not identify video board");
    }

    handle->format_proc = vio_format(CS_RGB, SM_444);
    handle->bandwidth   = 1000000;
    memset(&handle->adv, 0, sizeof(t_adv212));
    memset(&handle->osd, 0, sizeof(t_osd));
    
    // set OSD border
    vio_drv_osd_set_border(&handle->osd);

    // stop everything
    vio_reset(handle->p_vio);
    adv212_drv_init(handle->p_adv);

    // clear OSD
    vio_osd_clear_screen(handle->p_vio);
    handle->osd.x = 0;
    handle->osd.y = 0;
    
    return ERR_VIO_SUCCESS;
}




/** Activates the VIO/ADV212 for encoding operation
 * 
 * The hardware will start compressing beginning with 
 * the next frame received. Note that the following hardware (e.x.: VSI)
 * needs already to be started, to prevent the ADV212 from blocking.
 * 
 * @param handle vio handle 
 */
int vio_drv_encode(t_vio* handle, uint32_t device)
{
    int ret = ERR_VIO_SUCCESS;    

    REPORT(INFO, ">> VIO-Driver: adv212 encoding mode");
    
    // stop everything
    vio_reset(handle->p_vio);

    // setup timing generator
    vio_config_tg(handle, VIO_TG_CONFIG_ENCODE);

    // store config
    vio_drv_change_mode(handle, VIO_CONFIG_ENCODE);
    if (handle->config & VIO_CONFIG_VRP) {
        handle->adv.size = handle->bandwidth;
    } else {
        handle->adv.size = vio_bandwidth_to_size(handle->bandwidth, &handle->timing);
    }
    handle->adv.chroma = handle->chroma;

    // ... TODO: testing range
    
    // detect settings based on resolution
    if ((ret = adv212_drv_detect_enc(&handle->timing, &handle->adv))) {
        REPORT(ERROR, "adv212_drv_detect_enc failed: %s", adv212_str_err(ret));
        return ret;
    }
    
    // setup interface format
    handle->format_proc = advfmt[handle->adv.cnt-1];
    vio_set_output_format(handle->p_vio, &handle->timing, advfmt[handle->adv.cnt-1], handle->format_out, false);
    vio_set_input_format(handle->p_vio, &handle->timing, handle->format_in, advfmt[handle->adv.cnt-1]);
    
    // setup PLL
    vio_drv_pll_setup(handle->p_vio, &handle->pll, handle->si598, VIO_SEL_INPUT, 0, jrel[handle->adv.cnt-1], VIO_MUX_PLLC_FREE, device, handle->adv.cnt);    
    
    // setup osd 
    vio_osd_clear_screen(handle->p_vio);
    vio_osd_set_resolution(handle->p_vio, handle->timing.width, handle->timing.height);
    handle->osd.x = 0;
    handle->osd.y = 0;


    // setup muxes
    vio_set_stin(handle->p_vio, VIO_MUX_STIN_ADV212);
    vio_set_vout(handle->p_vio, VIO_MUX_VOUT_LOOP);
    vio_set_advcnt(handle->p_vio, handle->adv.cnt-1);
    vio_set_proc(handle->p_vio, ((handle->config & VIO_CONFIG_VRP) ? VIO_MUX_PROC_VRP : VIO_MUX_PROC_ETH));
    vio_set_cfg(handle->p_vio, VIO_CFG_VIN | VIO_CFG_VOUT | VIO_CFG_ADV_ENC);
    
    // START ...
    //////////////////////
    
    // 1.) start pll
    vio_pll_update(handle->p_vio, &handle->pll);

    // 2.) start the adv212
    if ((ret = adv212_drv_boot_enc(handle->p_adv, &handle->timing, &handle->adv, device))) {
        REPORT(ERROR, "adv212_drv_boot_enc failed: %s", adv212_str_err(ret));
        return ret;
    }    
    vio_set_cfg(handle->p_vio, VIO_CFG_SCOMM5);

    // 3.) start the data path
    vio_start(handle->p_vio);
    
    // 4.) Activate OSD for video loop?
    if (handle->config & VIO_CONFIG_OSD) {
    	vio_set_cfg(handle->p_vio, VIO_CFG_OVERLAY);
    }

    // 5.) Report
    VIO_REPORT_TIMING(&handle->timing);
    VIO_REPORT_OSD(handle->p_vio);
    VIO_REPORT_PLL(&handle->pll);
    VIO_REPORT_CONTROL(handle->p_vio, handle->pll.ppm);
    VIO_REPORT_MUX(handle->p_vio);

    // input is now running
    handle->active		= true;

    REPORT(INFO, "encoding is now running");
    
    return ret;
}

/** Activates the VIO/ADV212 for decoding operation
 *
 * The hardware will start decompressing beginning with
 * the next sop received.
 *
 * @param handle vio handle
 */
int vio_drv_decode(t_vio* handle, uint32_t device)
{
    int ret = ERR_VIO_SUCCESS;    
    bool invert_cb_cr = false;

    REPORT(INFO, ">> VIO-Driver: adv212 decoding mode");

    vio_drv_change_mode(handle, VIO_CONFIG_DECODE);

    // stop everything
    vio_reset(handle->p_vio);
    
    // ... TODO: testing range

    // detect settings based on resolution
    if ((ret = adv212_drv_detect_dec(&handle->timing, &handle->adv))) {
        REPORT(ERROR, "adv212_drv_detect_dec failed: %s", adv212_str_err(ret));
        return ret;
    }

    // setup timing generator
    vio_config_tg(handle, VIO_TG_CONFIG_DECODE);

    // setup interface format

    if (device == BDT_ID_SDI8_BOARD) {
        invert_cb_cr = true;
    }
    handle->format_proc = advfmt[handle->adv.cnt-1];
    vio_set_output_format(handle->p_vio, &handle->timing, advfmt[handle->adv.cnt-1], handle->format_out, invert_cb_cr);    

    // setup PLL
    if (handle->config & VIO_CONFIG_VRP) {
    	// stream source is VRP
    	vio_drv_pll_setup(handle->p_vio, &handle->pll, handle->si598, VIO_SEL_75MHZ, handle->timing.pfreq, jrel[handle->adv.cnt-1], VIO_MUX_PLLC_FREE, device, handle->adv.cnt);
    } else {
    	// stream source is VSO
    	vio_drv_pll_setup(handle->p_vio, &handle->pll, handle->si598, VIO_SEL_75MHZ, handle->timing.pfreq, jrel[handle->adv.cnt-1], VIO_MUX_PLLC_TG, device, handle->adv.cnt);
    }

    // setup osd
    vio_osd_clear_screen(handle->p_vio);
    vio_osd_set_resolution(handle->p_vio, handle->timing.width, handle->timing.height);  
    handle->osd.x = 0;
    handle->osd.y = 0;

    // setup muxes
    vio_set_vout(handle->p_vio, VIO_MUX_VOUT_ADV212);
    vio_set_advcnt(handle->p_vio, handle->adv.cnt-1);
    vio_set_proc(handle->p_vio, ((handle->config & VIO_CONFIG_VRP) ? VIO_MUX_PROC_VRP : VIO_MUX_PROC_ETH));
    if (handle->config & VIO_CONFIG_VRP) {
    	// stream source is VRP
    	vio_set_cfg(handle->p_vio, VIO_CFG_VOUT | VIO_CFG_ADV_DEC);
    } else {
    	// stream source is VSO
    	vio_set_cfg(handle->p_vio, VIO_CFG_VOUT | VIO_CFG_ADV_DEC | VIO_CFG_WAITVS);// TODO: | VIO_CFG_SCOMM5_HW);
    }

    // START ...
    //////////////////////

    // 1.) start pll
    vio_pll_update(handle->p_vio, &handle->pll);
    vio_set_control(handle->p_vio, &handle->timing, handle->pll.ppm, handle->pll.mode);

    // 2.) start clock control
    if (device == BDT_ID_HDMI_BOARD) {
        vio_clock_control_start(handle);
    }
    if (device == BDT_ID_SDI8_BOARD) {
        si598_clock_control_activate(handle->si598);
    }

    // 3.) start timing generator
    vio_enable_output_timing(handle->p_vio);

    // 4.) start the adv212
    if ((ret = adv212_drv_boot_dec(handle->p_adv, &handle->timing, &handle->adv, device))) {
        REPORT(ERROR, "adv212_drv_boot_dec failed: %s", adv212_str_err(ret));
        return ret;
    }

    // 5.) start the i/o path
    vio_start(handle->p_vio);

    // 6.) Activate OSD?
    if (handle->config & VIO_CONFIG_OSD) {
    	vio_set_cfg(handle->p_vio, VIO_CFG_OVERLAY);
    }

    // 7.) Report
    VIO_REPORT_FORMAT(advfmt[handle->adv.cnt-1], "Process");
    VIO_REPORT_FORMAT(handle->format_out, "Output");
    VIO_REPORT_TIMING(&handle->timing);
    VIO_REPORT_OSD(handle->p_vio);
    VIO_REPORT_PLL(&handle->pll);
    VIO_REPORT_CONTROL(handle->p_vio, handle->pll.ppm);
    VIO_REPORT_MUX(handle->p_vio);

    // output is now running
    handle->active		= true;

    REPORT(INFO, "decoding is now running");

    return ret;
}

/** Synchronize output
 *
 * this function locks until in sync
 *
 * @param handle vio handle
 */
int vio_drv_decode_sync(t_vio* handle)
{
    int ret = SUCCESS;

    if (handle->adv.cnt > 1) {
        if (handle->config & VIO_CONFIG_DECODE) {
            if ((ret = adv212_drv_boot_dec_sync(handle->p_adv, &handle->adv, handle->p_vio))) {
                return ret;
            }
            vio_set_cfg(handle->p_vio, VIO_CFG_SCOMM5);
        }
    }

    return SUCCESS;
}

/** Activates the VIO for output operation
 *
 * The hardware will start outputing the stream directly
 *
 * @param handle vio handle
 */
int vio_drv_plainout(t_vio* handle, uint32_t device)
{
    int ret = ERR_VIO_SUCCESS;

    REPORT(INFO, ">> VIO-Driver: plainout mode");

    vio_drv_change_mode(handle, VIO_CONFIG_PLAINOUT);

    // stop everything
    vio_reset(handle->p_vio);

    // ... TODO: testing range

    // setup timing generator
    vio_set_timing(handle->p_vio, &handle->timing, 0);

    // setup interface format
    vio_set_output_format(handle->p_vio, &handle->timing, vio_format(CS_RGB, SM_444), handle->format_out, false);

    // setup PLL
    if (handle->config & VIO_CONFIG_VRP) {
        // stream source is VRP
        vio_drv_pll_setup(handle->p_vio, &handle->pll, handle->si598, VIO_SEL_75MHZ, handle->timing.pfreq, 1, VIO_MUX_PLLC_FREE, device, 2);
    } else {
        // stream source is VSO
        vio_drv_pll_setup(handle->p_vio, &handle->pll, handle->si598, VIO_SEL_75MHZ, handle->timing.pfreq, 1, VIO_MUX_PLLC_TG, device, 2);
    }

    // setup osd
    vio_osd_clear_screen(handle->p_vio);
    vio_osd_set_resolution(handle->p_vio, handle->timing.width, handle->timing.height);
    handle->osd.x = 0;
    handle->osd.y = 0;

    // setup muxes
    vio_set_vout(handle->p_vio, VIO_MUX_VOUT_PLAIN);
   	vio_set_proc(handle->p_vio, ((handle->config & VIO_CONFIG_VRP) ? VIO_MUX_PROC_VRP : VIO_MUX_PROC_ETH));
    if (handle->config & VIO_CONFIG_VRP) {
        // stream source is VRP
        vio_set_cfg(handle->p_vio, VIO_CFG_VOUT | VIO_CFG_PLAIN_DEC);
    } else {
        // stream source is VSO
        vio_set_cfg(handle->p_vio, VIO_CFG_VOUT | VIO_CFG_PLAIN_DEC | VIO_CFG_WAITVS);
    }

    // START ...
    //////////////////////

    // 1.) start pll
    vio_pll_update(handle->p_vio, &handle->pll);
    vio_set_control(handle->p_vio, &handle->timing, handle->pll.ppm, handle->pll.mode);

    // 2.) start the i/o path
    vio_start(handle->p_vio);

    // 3.) Activate OSD?
    if (handle->config & VIO_CONFIG_OSD) {
    	vio_set_cfg(handle->p_vio, VIO_CFG_OVERLAY);
    }

    // 4.) Report
    VIO_REPORT_TIMING(&handle->timing);
    VIO_REPORT_OSD(handle->p_vio);
    VIO_REPORT_PLL(&handle->pll);
    VIO_REPORT_CONTROL(handle->p_vio, handle->pll.ppm);
    VIO_REPORT_MUX(handle->p_vio);

    // output is now running
    handle->active		= true;

    REPORT(INFO, "plainout is now running");

    return ret;
}

/** Activates the VIO for input operation
 *
 * The hardware will start reading the stream
 *
 * @param handle vio handle
 */
int vio_drv_plainin(t_vio* handle, uint32_t device)
{
    int ret = ERR_VIO_SUCCESS;

    REPORT(INFO, ">> VIO-Driver: plainin mode");

    // store config
    vio_drv_change_mode(handle, VIO_CONFIG_PLAININ);

    // stop everything
    vio_reset(handle->p_vio);

    // Read current video format and test for validity
    vio_get_timing(handle->p_vio, &handle->timing);
    // ... TODO: testing range

    // setup interface format
    vio_set_output_format(handle->p_vio, &handle->timing, vio_format(CS_RGB, SM_444), handle->format_out, false);
    vio_set_input_format(handle->p_vio, &handle->timing, handle->format_in, vio_format(CS_RGB, SM_444));

    // setup PLL
    vio_drv_pll_setup(handle->p_vio, &handle->pll, handle->si598, VIO_SEL_INPUT, 0, 1, VIO_MUX_PLLC_FREE, device, 2);

    // setup osd
    vio_osd_clear_screen(handle->p_vio);
    vio_osd_set_resolution(handle->p_vio, handle->timing.width, handle->timing.height);
    handle->osd.x = 0;
    handle->osd.y = 0;

    // setup frame size in words
    vio_set_size(handle->p_vio, vid_pixel_per_frame(&handle->timing)*3/4);



    // setup muxes
    vio_set_stin(handle->p_vio, VIO_MUX_STIN_PLAIN);
    vio_set_vout(handle->p_vio, VIO_MUX_VOUT_LOOP);
    vio_set_proc(handle->p_vio, ((handle->config & VIO_CONFIG_VRP) ? VIO_MUX_PROC_VRP : VIO_MUX_PROC_ETH));
    vio_set_cfg(handle->p_vio, VIO_CFG_VIN | VIO_CFG_VOUT | VIO_CFG_PLAIN_ENC);

    // START ...
    //////////////////////

    // 1.) start pll
    vio_pll_update(handle->p_vio, &handle->pll);

    // 2.) start the data path
    vio_start(handle->p_vio);

    // 3.) Activate OSD for video loop?
    if (handle->config & VIO_CONFIG_OSD) {
        vio_set_cfg(handle->p_vio, VIO_CFG_OVERLAY);
    }



    // 4.) Report
    VIO_REPORT_TIMING(&handle->timing);
    VIO_REPORT_OSD(handle->p_vio);
    VIO_REPORT_PLL(&handle->pll);
    VIO_REPORT_CONTROL(handle->p_vio, handle->pll.ppm);
    VIO_REPORT_MUX(handle->p_vio);

    // input is now running
    handle->active      = true;

    REPORT(INFO, "plainin is now running");

    return ret;
}

/** Activates the VIO debug output (OSD only)
 *
 * Only an OSD will be output
 *
 * @param handle vio handle
 */

int vio_drv_debug(t_vio* handle, uint32_t device, bool vtb, t_gs2972 *sdi_tx, t_adv7441a* hdmi_in)
{
    int ret = ERR_VIO_SUCCESS;    
    int no_input = 1;

    REPORT(INFO, ">> VIO-Driver: debug mode");

    vio_drv_change_mode(handle, VIO_CONFIG_DEBUG|VIO_CONFIG_OSD);

    // stop everything
    vio_reset(handle->p_vio);

    // get input video timing if transmitter and input is active
    if (vtb) {
        if (device == BDT_ID_HDMI_BOARD) {
            if (adv7441a_poll_active_resolution(hdmi_in, handle->p_vio) == 2) {
                no_input = vio_get_timing(handle->p_vio, &handle->timing);
            }
        } else {
            no_input = vio_get_timing(handle->p_vio, &handle->timing);
        }
    }

    // if SDI, timing of h and v is inverted
    if (device == BDT_ID_SDI8_BOARD) {
        handle->timing.vpolarity = 0;
        handle->timing.hpolarity = 0;
    }

    // setup timing generator
    vio_set_timing(handle->p_vio, &handle->timing, 0);

    // if sdi, set output data rate
    if (device == BDT_ID_SDI8_BOARD) {
        gs2972_driver_set_data_rate(sdi_tx, handle->timing.pfreq);
    }

    // setup interface format
    vio_set_output_format(handle->p_vio, &handle->timing, handle->format_proc, handle->format_out, false);
    vio_set_input_format(handle->p_vio, &handle->timing, handle->format_in, handle->format_proc);

    // setup PLL
    if (no_input) {
        vio_drv_pll_setup(handle->p_vio, &handle->pll, handle->si598, VIO_SEL_75MHZ, handle->timing.pfreq, 1, VIO_MUX_PLLC_FREE, device, 2);
    } else {
        vio_drv_pll_setup(handle->p_vio, &handle->pll, handle->si598, VIO_SEL_INPUT, handle->timing.pfreq, 1, VIO_MUX_PLLC_FREE, device, 2);
    }

    // setup osd
    vio_osd_clear_screen(handle->p_vio);
    vio_osd_set_resolution(handle->p_vio, handle->timing.width, handle->timing.height);
    handle->osd.x = 0;
    handle->osd.y = 0;
    
    // setup muxes
    if (no_input) {
        vio_set_vout(handle->p_vio, VIO_MUX_VOUT_DEBUG);
        vio_set_cfg(handle->p_vio, VIO_CFG_VOUT | VIO_CFG_OVERLAY);
    } else {
        vio_set_vout(handle->p_vio, VIO_MUX_VOUT_LOOP);
        vio_set_cfg(handle->p_vio, VIO_CFG_VIN | VIO_CFG_VOUT | VIO_CFG_OVERLAY);
    }
    
    // START ...
    //////////////////////
    
    // 1.) start pll
    vio_pll_update(handle->p_vio, &handle->pll);
    
    // 2.) start the i/o path
    vio_start(handle->p_vio);
    
    // 3.) report
    VIO_REPORT_TIMING(&handle->timing);
    VIO_REPORT_OSD(handle->p_vio);
    VIO_REPORT_PLL(&handle->pll);
    VIO_REPORT_CONTROL(handle->p_vio, handle->pll.ppm);
    VIO_REPORT_MUX(handle->p_vio);
    
    // output is now running with osd active
    handle->active		= true;
    
    return ret;    
}

/** Activates the VIO for loop operation
 *
 * Loop input to output
 *
 * @param handle vio handle
 */
int vio_drv_loop(t_vio* handle, uint32_t device)
{
    int ret = ERR_VIO_SUCCESS;

    REPORT(INFO, ">> VIO-Driver: loop mode");

    // store config
    vio_drv_change_mode(handle, VIO_CONFIG_LOOP);

    // stop everything
    vio_reset(handle->p_vio);

    // Read current video format and test for validity
    vio_get_timing(handle->p_vio, &handle->timing);
    // ... TODO: testing range

    // setup interface format
    vio_set_output_format(handle->p_vio, &handle->timing, handle->format_proc, handle->format_out, false);
    vio_set_input_format(handle->p_vio, &handle->timing, handle->format_in, handle->format_proc);

    // setup PLL
    vio_drv_pll_setup(handle->p_vio, &handle->pll, handle->si598, VIO_SEL_INPUT, 0, 1, VIO_MUX_PLLC_FREE, device, 2);

    // setup osd
    vio_osd_clear_screen(handle->p_vio);
    vio_osd_set_resolution(handle->p_vio, handle->timing.width, handle->timing.height);
    handle->osd.x = 0;
    handle->osd.y = 0;

    // setup muxes
    vio_set_stin(handle->p_vio, VIO_MUX_STIN_LOOP);
    vio_set_vout(handle->p_vio, VIO_MUX_VOUT_LOOP);
    vio_set_proc(handle->p_vio, ((handle->config & VIO_CONFIG_VRP) ? VIO_MUX_PROC_VRP : VIO_MUX_PROC_ETH));
    vio_set_cfg(handle->p_vio, VIO_CFG_VIN | VIO_CFG_VOUT | VIO_CFG_PLAIN_ENC);

    // START ...
    //////////////////////

    // 1.) start pll
    vio_pll_update(handle->p_vio, &handle->pll);

    // 2.) start the data path
    vio_start(handle->p_vio);

    // 3.) Activate OSD for video loop?
    if (handle->config & VIO_CONFIG_OSD) {
        vio_set_cfg(handle->p_vio, VIO_CFG_OVERLAY);
    }

    // 4.) Report
    VIO_REPORT_TIMING(&handle->timing);
    VIO_REPORT_OSD(handle->p_vio);
    VIO_REPORT_PLL(&handle->pll);
    VIO_REPORT_CONTROL(handle->p_vio, handle->pll.ppm);
    VIO_REPORT_MUX(handle->p_vio);

    // input is now running
    handle->active      = true;

    REPORT(INFO, "loop is now running");

    return ret;
}


/** Update bandwidth of the generated data stream
 *
 * The update is active for the start of the next frame
 *
 * @param handle vio handle
 * @param bandwidth bandwidth in byte/second
 */
int vio_drv_set_bandwidth(t_vio* handle, int bandwidth, int chroma_percent)
{
    handle->bandwidth = bandwidth;
    handle->chroma = chroma_percent;

    if (handle->active && ((handle->config & VIO_CONFIG_MODE) == VIO_CONFIG_ENCODE)) {
        adv212_drv_rc_size(handle->p_adv, vio_bandwidth_to_size(handle->bandwidth, &handle->timing), &handle->adv, handle->chroma);
    }
    return ERR_VIO_SUCCESS;
}


/** Activates the VIO/ADV212 for encoding operation
 *
 * The hardware will start compressing beginning with
 * the next frame received. Note that the following hardware (e.x.: VSI)
 * needs already to be started, to prevent the ADV212 from blocking.
 *
 * @param handle vio handle
 * @param bandwidth the initial bandwidth in [bytes/s]
 * @param advcnt numbers of adv212 to be used for encoding(0: autodetect)
 */
int vio_drv_encodex(t_vio* handle, int bandwidth, uint32_t chroma, int advcnt, uint32_t device)
{
    PTR(handle); PTR(handle->p_vio); PTR(handle->p_adv);
    handle->bandwidth = bandwidth;
    handle->chroma = chroma;
    handle->adv.cnt = advcnt;
    return vio_drv_encode(handle, device);
}


/** Activates the VIO/ADV212 for decoding operation
 *
 * The hardware will start decompressing beginning with
 * the next marker frame received.
 *
 * @param handle vio handle
 * @param p_vt video timing struct
 * @param advcnt the numbers of adv212 used for encoding
 */
int vio_drv_decodex(t_vio* handle, t_video_timing* p_vt, int advcnt, uint32_t device)
{
    PTR(handle); PTR(handle->p_vio); PTR(handle->p_adv); PTR(p_vt);
    handle->adv.cnt = advcnt;
    memcpy(&handle->timing, p_vt, sizeof(t_video_timing));
    return vio_drv_decode(handle, device);
}

/** Activates the VIO/ADV212 for output operation
 *
 * The hardware will start outputing the stream directly
 *
 * @param handle vio handle
 */
int vio_drv_plainoutx(t_vio* handle, t_video_timing* p_vt, uint32_t device)
{
    PTR(handle); PTR(handle->p_vio); PTR(handle->p_adv); PTR(p_vt);
    memcpy(&handle->timing, p_vt, sizeof(t_video_timing));
    return vio_drv_plainout(handle, device);
}


/** Activates the VIO Debug output
 *
 * @param handle vio handle
 * @param p_vt video timing struct
 */
int vio_drv_debugx(t_vio* handle, t_video_timing* p_vt, bool vtb, uint32_t device, t_gs2972 *sdi_tx, t_adv7441a* hdmi_in)
{
    PTR(handle); PTR(handle->p_vio); PTR(handle->p_adv); PTR(p_vt);
    memcpy(&handle->timing, p_vt, sizeof(t_video_timing));
    return vio_drv_debug(handle, device, vtb, sdi_tx, hdmi_in);
}

int vio_drv_set_format_in(t_vio* handle, t_video_format f)
{
    handle->format_in = f;
    // TODO: update when already running?
    if (handle->active) {
        vio_set_input_format(handle->p_vio, &handle->timing, handle->format_in, handle->format_proc);
    }
    return ERR_VIO_SUCCESS;
}    

int vio_drv_set_format_out(t_vio* handle, t_video_format f)
{
    handle->format_out = f;
    // TODO: update when already running?
    if (handle->active) {
        vio_set_output_format(handle->p_vio, &handle->timing, handle->format_proc, handle->format_out, false);
    }
    return ERR_VIO_SUCCESS;
}

int vio_drv_set_format_proc(t_vio* handle, t_video_format f)
{
    handle->format_proc = f;
    // TODO: update when already running?
    if (handle->active) {
        vio_set_input_format(handle->p_vio, &handle->timing, handle->format_in, handle->format_proc);

        vio_set_output_format(handle->p_vio, &handle->timing, handle->format_proc, handle->format_out, false);
    }
    return ERR_VIO_SUCCESS;
}

int vio_drv_set_osd(t_vio* handle, bool en)
{
	if (en) {
		vio_drv_set_cfg(handle, VIO_CONFIG_OSD);
		if (handle->active) {
			vio_set_cfg(handle->p_vio, VIO_CFG_OVERLAY);
		}
        // set OSD border
        vio_drv_osd_set_border(&handle->osd);
	} else {
		vio_drv_clr_cfg(handle, VIO_CONFIG_OSD);
		if (handle->active) {
			vio_clr_cfg(handle->p_vio, VIO_CFG_OVERLAY);
		}
	}
	return ERR_VIO_SUCCESS;
}

int vio_drv_set_hpd(t_vio* handle, bool en)
{
    if (en) {
        vio_clr_cfg(handle->p_vio, VIO_CFG_HPD);
    } else {
        vio_set_cfg(handle->p_vio, VIO_CFG_HPD);
    }
    return ERR_VIO_SUCCESS;
}


int vio_drv_set_sync(t_vio* handle)
{
	// set scomm5 for multichip decoding
	vio_set_cfg(handle->p_vio, VIO_CFG_SCOMM5);
	return 0;
}

void vio_drv_get_timing(t_vio* handle, t_video_timing* p_vt)
{
    if (handle->active) {
        memcpy(p_vt, &handle->timing, sizeof(t_video_timing));
    } else {
        vio_get_timing(handle->p_vio, p_vt);
    }
}

void vio_drv_get_advcnt(t_vio* handle, uint32_t* advcnt)
{
    if (handle->active) {
        *advcnt = handle->adv.cnt;
    } else {
        *advcnt = 0;
    }
}



void vio_drv_handler(t_vio* handle, t_queue* event)
{
    uint32_t cfg, pcfg, ncfg;

    PTR(handle); PTR(handle->p_vio); PTR(handle->p_adv); PTR(event);
    cfg = vio_get_sta(handle->p_vio, 0xffffffff);

    pcfg = cfg & ~handle->hw_cfg_old;
    ncfg = ~cfg & handle->hw_cfg_old;

    if (pcfg) {
        if (pcfg & VIO_STA_PLL_LOST) {
            queue_put(event, E_VIO_PLL_OOS);
        }

        if (pcfg & VIO_STA_JDATA0_SYNC_LOST) {
            queue_put(event, E_VIO_JD0ENC_OOS);
        }
        if (pcfg & VIO_STA_JDATA1_SYNC_LOST) {
            queue_put(event, E_VIO_JD1ENC_OOS);
        }
        if (pcfg & VIO_STA_JDATA2_SYNC_LOST) {
            queue_put(event, E_VIO_JD2ENC_OOS);
        }
        if (pcfg & VIO_STA_JDATA3_SYNC_LOST) {
            queue_put(event, E_VIO_JD3ENC_OOS);
        }
    }

    if (ncfg) {
        if (ncfg & VIO_STA_PLL_LOST) {
            queue_put(event, E_VIO_PLL_SNC);
        }
    }

    handle->hw_cfg_old = cfg;
}


void vio_drv_irq_adv212(t_vio* handle, int nr, t_queue* event)
{
    uint32_t irq;
    if (handle->active) {
        irq = adv212_get_irq(OFFSET(handle->p_adv, nr * ADV212_SIZE));

        if (irq & ADV212_EIRQ_CFTH) {
//            REPORT(INFO, "adv212[%d] cfth", nr);
        }
        if (irq & ADV212_EIRQ_CFERR) {
//            REPORT(INFO, "adv212[%d] cferr", nr);
            //queue_put(event, E_VIO_ADV212_CFERR+nr);
            //vio_drv_halt(handle);
        }
    }
}


void vio_copy_adv7441_timing(t_video_timing* timing, void* handle)
{
    t_adv7441a* handle_adv = (t_adv7441a*) handle;

    timing->width        = handle_adv->vid_st.h_line_width;
    timing->height       = handle_adv->vid_st.f0_height;
    timing->height_1     = handle_adv->vid_st.f1_height;
    timing->hfront       = handle_adv->vid_st.h_front_porch_width;
    timing->hpulse       = handle_adv->vid_st.h_sync_width;
    timing->hback        = handle_adv->vid_st.h_back_porch_width;
    timing->vfront       = handle_adv->vid_st.f0_front_porch_width;
    timing->vpulse       = handle_adv->vid_st.f0_vs_pulse_width;
    timing->vback        = handle_adv->vid_st.f0_back_porch_width;
    timing->vfront_1     = handle_adv->vid_st.f1_front_porch_width;
    timing->vpulse_1     = handle_adv->vid_st.f1_vs_pulse_width;
    timing->vback_1      = handle_adv->vid_st.f1_back_porch_width;
    timing->interlaced   = handle_adv->vid_st.interlaced;
    timing->vpolarity    = handle_adv->vid_st.vsync_pol;
    timing->hpolarity    = handle_adv->vid_st.hsync_pol;
    timing->fpolarity    = handle_adv->vid_st.field_pol;
} 


void vio_copy_gs2971_timing(t_video_timing* timing, void* handle)
{
    t_gs2971* handle_gs = (t_gs2971*) handle;

    timing->width        = handle_gs->vid_st.h_line_width;
    timing->height       = handle_gs->vid_st.f0_height;
    timing->height_1     = handle_gs->vid_st.f1_height;
    timing->hfront       = handle_gs->vid_st.h_front_porch_width;
    timing->hpulse       = handle_gs->vid_st.h_sync_width;
    timing->hback        = handle_gs->vid_st.h_back_porch_width;
    timing->vfront       = handle_gs->vid_st.f0_front_porch_width;
    timing->vpulse       = handle_gs->vid_st.f0_vs_pulse_width;
    timing->vback        = handle_gs->vid_st.f0_back_porch_width;
    timing->vfront_1     = handle_gs->vid_st.f1_front_porch_width;
    timing->vpulse_1     = handle_gs->vid_st.f1_vs_pulse_width;
    timing->vback_1      = handle_gs->vid_st.f1_back_porch_width;
    timing->interlaced   = handle_gs->vid_st.interlaced;
    timing->vpolarity    = handle_gs->vid_st.vsync_pol;
    timing->hpolarity    = handle_gs->vid_st.hsync_pol;
    timing->fpolarity    = handle_gs->vid_st.field_pol;
} 


void vio_config_tg(t_vio* handle, int config)
{

    switch (config) {
        case (VIO_TG_CONFIG_ENCODE):

            // Timing Generator outputs only if input trigger is active
            vio_set_cfg(handle->p_vio, VIO_CFG_TG_FREE_RUN);

            // Set noninverted polarity of timing generator output signals
            vio_clr_cfg(handle->p_vio, VIO_CFG_VIN_TM_POL_FIELD);
            vio_clr_cfg(handle->p_vio, VIO_CFG_VIN_TM_POL_VSYNC);
            vio_clr_cfg(handle->p_vio, VIO_CFG_VIN_TM_POL_HSYNC);
            vio_clr_cfg(handle->p_vio, VIO_CFG_VIN_TM_POL_AVID);
            vio_clr_cfg(handle->p_vio, VIO_CFG_VIN_TM_POL_TRIG);

            // use external timing signals
            vio_clr_cfg(handle->p_vio, VIO_CFG_VIN_TIMING_FIELD);
            vio_clr_cfg(handle->p_vio, VIO_CFG_VIN_TIMING_HSYNC);
            vio_clr_cfg(handle->p_vio, VIO_CFG_VIN_TIMING_AVID);

            if(handle->timing.interlaced == 0) {
                // override timing with measured timing
                vio_get_timing(handle->p_vio, &handle->timing);
                // use external vsync
                vio_clr_cfg(handle->p_vio, VIO_CFG_VIN_TIMING_VSYNC);
            } else {
                // enable vsync output in timing generator
                vio_clr_cfg(handle->p_vio, VIO_CFG_TG_VSYNC_RESET);
                // use only vsync of timing generator
                vio_set_cfg(handle->p_vio, VIO_CFG_VIN_TIMING_VSYNC);
                // measure input video frequency
                vio_get_input_frequency(handle->p_vio, &handle->timing);

                // invert timing of vsync if necessary
                if (handle->timing.vpolarity == 0) {
                    vio_set_cfg(handle->p_vio, VIO_CFG_VIN_TM_POL_VSYNC);
                }

                // invert timing of hsync if necessary
                if (handle->timing.hpolarity == 0) {
                    vio_set_cfg(handle->p_vio, VIO_CFG_VIN_TM_POL_HSYNC);
                }

                // invert timing of field if necessary
                if (handle->timing.fpolarity == 0) {
                    vio_set_cfg(handle->p_vio, VIO_CFG_VIN_TM_POL_FIELD);
                }
            }

            // set timing generator registers
            vio_set_timing(handle->p_vio, &handle->timing, 1);
            break;

        case (VIO_TG_CONFIG_DECODE):

            vio_clr_cfg(handle->p_vio, VIO_CFG_TG_VSYNC_RESET);

            // Timing Generator is free running
            vio_clr_cfg(handle->p_vio, VIO_CFG_TG_FREE_RUN);

            vio_clr_cfg(handle->p_vio, VIO_CFG_VIN_TIMING_FIELD);
            vio_clr_cfg(handle->p_vio, VIO_CFG_VIN_TIMING_VSYNC);
            vio_clr_cfg(handle->p_vio, VIO_CFG_VIN_TIMING_HSYNC);
            vio_clr_cfg(handle->p_vio, VIO_CFG_VIN_TIMING_AVID);
            vio_clr_cfg(handle->p_vio, VIO_CFG_VIN_TM_POL_TRIG);

            vio_set_timing(handle->p_vio, &handle->timing, 0);
            break;

    }
}

void vio_clock_control_reset(t_vio* handle)
{
    handle->mean_1 = 0;
    handle->mean_2 = 0;
    handle->mean_cnt = 0;
    handle->clock_control_active = false;
    // Set output frequency to original value
    HOI_WR32((handle->p_vio), VIO_OFF_PLL_PCS, 100000);
}

void vio_clock_control_start(t_vio* handle)
{
    handle->clock_control_active = true;
}

void vio_clock_control(t_vio* handle)
{
    uint32_t phase_offset;  // small value = big frequency change (y=f(1/x))
    uint32_t direction;     // 1 = faster / 0 = slower
    uint32_t image_offset;

    if (handle->clock_control_active) {

        image_offset = HOI_RD32((handle->p_vio), VIO_OFF_TG_CAPTURE);
        // convert 24bit signed number into sign flag and value
        if (image_offset & 0x00800000) {
            direction = 1;
            image_offset = 0x01000000 - image_offset;
        } else {
            direction = 0;
        }
        // low pass filter
        handle->mean_1 = (handle->mean_1 * 4/5) + (((uint64_t)image_offset<<32) * 1/5);  //32.32 fractional
        handle->mean_2 = (handle->mean_2 * 49/50) + (handle->mean_1 * 1/50);

        // prevent division through 0
        if (handle->mean_1 == 0) {
            handle->mean_1 = (uint64_t)1<<32;
        }

        if (handle->mean_2 == 0) {
            handle->mean_2 = (uint64_t)1<<32;
        }

        // switch between two filters
        if (handle->mean_cnt < 33) { // after 1s
            phase_offset = 1000000/((uint32_t)(handle->mean_1>>32));
            handle->mean_cnt++;
        } else {
            phase_offset = 1000000/((uint32_t)(handle->mean_2>>32));
        }
        // write control value to gateware
        HOI_WR32((handle->p_vio), VIO_OFF_PLL_PCS, ((direction << 31) | (phase_offset & 0x7FFFFFFF)));

   //     if (direction == 1) {
   //         printk("\ni: -%06i, m1: %06i, m2: %06i, p: %06i, c: %06i", image_offset, (uint32_t)(handle->mean_1>>32), (uint32_t)(handle->mean_2>>32), phase_offset, (uint32_t)handle->mean_cnt);
   //     } else {
   //        printk("\ni:  %06i, m1: %06i, m2: %06i, p: %06i, c: %06i", image_offset, (uint32_t)(handle->mean_1>>32), (uint32_t)(handle->mean_2>>32), phase_offset, (uint32_t)handle->mean_cnt);
   //     }
    }
}
