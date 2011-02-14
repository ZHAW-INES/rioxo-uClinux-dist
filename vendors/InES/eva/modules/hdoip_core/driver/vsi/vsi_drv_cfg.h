#ifndef VSI_DRV_CFG_H_
#define VSI_DRV_CFG_H_

#include "std.h"
#include "stdeth.h"
#include "hoi_burst.h"


/* VSO configuration */ 
#define VSI_DRV_MIN_PACKET_LEN      (64)    // in bytes
#define VSI_DRV_MAX_PACKET_LEN      (1520)   // in bytes

/* status bit declaration */
#define VSI_DRV_STATUS                      (0x0000000F)
#define VSI_DRV_STATUS_ACTIV                (0x00000001)
#define VSI_DRV_STATUS_FIFO2_FULL           (0x00000002)
#define VSI_DRV_STATUS_CDFIFO_FULL          (0x00000004)
#define VSI_DRV_STATUS_ETH_PARAMS_SET       (0x00000008)

typedef struct {
	uint32_t	                status;
    struct hdoip_eth_params	    eth_params;
	void*		                p_vsi;
} t_vsi;


#endif /* VSI_DRV_CFG_H_ */


