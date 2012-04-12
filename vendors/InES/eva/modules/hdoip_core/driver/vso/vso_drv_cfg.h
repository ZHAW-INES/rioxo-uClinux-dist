#ifndef VSO_DRV_CFG_H_
#define VSO_DRV_CFG_H_

#include "hoi_burst.h"

/* VSO configuration */

#define VSO_DRV_FIFO_SIZE			    (1024)
#define VSO_DRV_MTIMEOUT_MARGIN		    (0.6e-3) /* in seconds */
#define VSO_DRV_TIMESTAMP_TOLERANCE     (10) /* in percent */
#define VSO_DRV_CLEAR_TIME              (41)  /* in us */

#define VSO_DRV_DMA_ALMOST_FULL         (VSO_DRV_FIFO_SIZE - VSO_DRV_DMA_BURST_SIZE - 10)

/* status bit declaration */
#define VSO_DRV_STATUS                  (0x0000007F)
#define VSO_DRV_STATUS_ACTIV            (0x00000001)
#define VSO_DRV_STATUS_ST_ACTIVE        (0x00000002)
#define VSO_DRV_STATUS_CHOKED           (0x00000004)
#define VSO_DRV_STATUS_PAYLOAD_EMPTY    (0x00000008)
#define VSO_DRV_STATUS_PAYLOAD_MTIMEOUT (0x00000010)
#define VSO_DRV_STATUS_TIMESTAMP_ERROR  (0x00000020)
#define VSO_DRV_STATUS_TIMING_SET       (0x00000040)


typedef struct {
	uint32_t	status;
	void*		p_vso;
} t_vso;

#endif /* VSO_DRV_CFG_H_ */
