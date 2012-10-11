#ifndef ADV212_DRV_H_
#define ADV212_DRV_H_

#include "adv212.h"
#include "stdvid.h"
#include "adv212_drv_cfg.h"

// parameter / register
#define ADV212_REGISTER_COUNT       (19)
#define ADV212_PARAM_ENC_COUNT      (4)
#define ADV212_PARAM_DEC_COUNT      (2)

// Offset
#define ADV212_0_BASE(p)        (OFFSET(p, 0*64))
#define ADV212_1_BASE(p)        (OFFSET(p, 1*64))
#define ADV212_2_BASE(p)        (OFFSET(p, 2*64))
#define ADV212_3_BASE(p)        (OFFSET(p, 3*64))

typedef struct {
    uint32_t    cnt;        //!< Number of ADV212 to be used
    uint32_t    size;       //!< Total compressed frame size
    uint32_t    chroma;     //!< percent of chroma bandwidth
} t_adv212;


void adv212_drv_init(void* p);
void adv212_drv_rc_size(void* p, uint32_t size, t_adv212* p_adv, uint32_t chroma);
int adv212_drv_advcnt(t_video_timing* p_vt, int* adv_cnt);
int adv212_drv_detect_enc(t_video_timing* p_vt, t_adv212* p_adv);
int adv212_drv_detect_dec(t_video_timing* p_vt, t_adv212* p_adv);
int adv212_drv_boot_enc(void* p, t_video_timing* p_vt, t_adv212* p_adv, uint32_t device);
int adv212_drv_boot_dec(void* p, t_video_timing* p_vt, t_adv212* p_adv, uint32_t device);
int adv212_drv_boot_dec_sync(void* p, t_adv212* p_adv, void* p_vio);

#endif /*ADV212_DRV_H_*/
