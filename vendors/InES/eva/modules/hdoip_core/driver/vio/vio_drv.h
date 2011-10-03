#ifndef VIO_DRV_H_
#define VIO_DRV_H_

#include "stdvid.h"
#include "vio_hal.h"
#include "vio_drv_pll.h"
#include "vio_drv_osd.h"
#include "adv212_drv.h"
#include "vio_str.h"

/** Handle
 */
typedef struct {
    bool            active;
    uint32_t		config;			//!< config mode
    
    void*           p_vio;          //!< pointer to vio hardware registers
    void*           p_adv;          //!< pointer to adv212 direct registers
    
    t_adv212        adv;            //!< ADV212 configuration
    t_vio_pll       pll;            //!< PLL configuration
    t_video_timing  timing;         //!< Video timing
    t_video_format  format_in;      //!< Video format input
    t_video_format  format_proc;    //!< Video format internal processing
    t_video_format  format_out;     //!< Video format output
    t_osd			osd;			//!< OSD
    
    uint32_t        bandwidth;      //!< when encoding this target bandwidth is used

    uint32_t        hw_cfg_old;
} t_vio;


/** config labels
 */
#define VIO_CONFIG_MODE 		(0x02f)
#define VIO_CONFIG_ENCODE		(0x001)
#define VIO_CONFIG_DECODE		(0x002)
#define VIO_CONFIG_DEBUG		(0x004)
#define VIO_CONFIG_PLAINOUT		(0x008)
#define VIO_CONFIG_PLAININ		(0x010)
#define VIO_CONFIG_LOOP         (0x020)

#define VIO_CONFIG_VRP  		(0x100)
#define VIO_CONFIG_OSD  		(0x200)

#define VIO_TG_CONFIG_ENCODE    (0x001)
#define VIO_TG_CONFIG_DECODE    (0x010)

/** inline functions
 */
static inline int vio_drv_osd_write(t_vio* handle, uint8_t* src, size_t size)
{
	return vio_drv_osd_write_(handle->p_vio, &handle->osd, src, size);
}

static inline void vio_drv_set_cfg(t_vio* handle, uint32_t cfg)
{
	handle->config = handle->config | cfg;
}

static inline void vio_drv_change_mode(t_vio* handle, uint32_t cfg)
{
    handle->config = (handle->config & ~VIO_CONFIG_MODE) | cfg;
}


static inline void vio_drv_clr_cfg(t_vio* handle, uint32_t cfg)
{
	handle->config = handle->config & ~cfg;
}


/** Set VIO Mode
 * 
 * advcnt 0 = autodetect (encode only)
 */
void vio_drv_handler(t_vio* handle, t_queue* event);
void vio_drv_irq_adv212(t_vio* handle, int nr, t_queue* event);
int vio_drv_init(t_vio* handle, void* p_vio, void* p_adv);
int vio_drv_setup_osd(t_vio* handle, t_osd_font* font);
int vio_drv_set_black_output(t_vio* handle);
int vio_drv_clr_black_output(t_vio* handle);
int vio_drv_reset(t_vio* handle, uint32_t device);
int vio_drv_encode(t_vio* handle);
int vio_drv_decode(t_vio* handle);
int vio_drv_decode_sync(t_vio* handle);
int vio_drv_plainout(t_vio* handle);
int vio_drv_plainin(t_vio* handle);
int vio_drv_debug(t_vio* handle);
int vio_drv_loop(t_vio* handle);

int vio_drv_encodex(t_vio* handle, int bandwidth, int advcnt);
int vio_drv_decodex(t_vio* handle, t_video_timing* p_vt, int advcnt);
int vio_drv_plainoutx(t_vio* handle, t_video_timing* p_vt);
int vio_drv_debugx(t_vio* handle, t_video_timing* p_vt);

int vio_drv_set_bandwidth(t_vio* handle, int bandwidth);
int vio_drv_set_format_in(t_vio* handle, t_video_format f);
int vio_drv_set_format_out(t_vio* handle, t_video_format f);
int vio_drv_set_format_proc(t_vio* handle, t_video_format f);
int vio_drv_set_osd(t_vio* handle, bool en);
int vio_drv_set_hpd(t_vio* handle, bool en);

int vio_drv_set_sync(t_vio* handle);

void vio_drv_get_timing(t_vio* handle, t_video_timing* p_vt);
void vio_drv_get_advcnt(t_vio* handle, uint32_t* advcnt);

void vio_copy_adv7441_timing(t_video_timing* timing, void* handle);
void vio_copy_gs2971_timing(t_video_timing* timing, void* handle);
void vio_config_tg(t_vio* handle, int config);
#endif /*VIO_DRV_H_*/
