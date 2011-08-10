/*
 * asi_drv_cfg.h
 *
 * Created on: 25.10.2010
 *     Author: stth
 */

#ifndef ASI_DRV_CFG_H_
#define ASI_DRV_CFG_H_

#include "std.h"
#include "stdeth.h"
#include "stdaud.h"
#include "hoi_burst.h"

/* config values */
#define ASI_DRV_MIN_FRAME_SIZE          (40)    // bytes
#define ASI_DRV_MAX_FRAME_SIZE          (1520)  // bytes
#define ASI_DRV_MIN_CH_CNT              (1)     /* minimum amount of audio channels */
#define ASI_DRV_MAX_CH_CNT              (8)     /* maximum amount of audio channels */

#define ASI_DRV_ETH_HEADER_LEN          (5)     /* in 32 bit words */
#define ASI_DRV_IPV4_HEADER_LEN         (5)     /* in 32 bit words */
#define ASI_DRV_UDP_HEADER_LEN          (2)     /* in 32 bit words */
#define ASI_DRV_RTP_HEADER_LEN          (4)     /* in 32 bit words */
#define ASI_DRV_AES_HEADER_LEN          (2)     /* in 32 bit words */

/* status bits */
#define ASI_DRV_STATUS_ACTIV            (0x00000001)
#define ASI_DRV_STATUS_ETH_PARAMS_SET   (0x00000002)
#define ASI_DRV_STATUS_AUD_PARAMS_SET   (0x00000004)
#define ASI_DRV_STATUS_RBF_ERROR        (0x00000008)

typedef struct {
    uint32_t                    status;
    uint32_t                    fs;
    uint32_t                    sampling_rate;
    uint32_t                    sampling_rate_old;
    void*                       p_asi;
    struct hdoip_eth_params     eth_params;
} t_asi;


#endif /* ASI_DRV_CFG_H_ */
