#ifndef ADV7441A_DRV_CFG_H_
#define ADV7441A_DRV_CFG_H_

#include "std.h"
#include "i2c_drv.h"
#include "adv7441a_drv_vid_std.h"

/* timing parameters */
//#define ADV7441A_TIMER_INTERVAL		(20 * HZ / 1000) /* 20ms */
#define ADV7441A_RESET_WAIT         (5  * HZ / 1000) /* 5ms */
#define ADV7441A_XTAL_FREQ          (28636300)

/* status bit declaration */
#define ADV7441A_STATUS             (0x0000000F)
#define ADV7441A_STATUS_ACTIVE      (0x00000001)
#define ADV7441A_STATUS_CONNECTION  (0x00000002)
#define ADV7441A_STATUS_ENCRYPTED   (0x00000004)
#define ADV7441A_STATUS_AUDIO       (0x00000008)

typedef struct {
	uint32_t		    status;
	t_i2c    		    *p_i2c;     //!< I2C driver
	t_vio               *p_vio;     //!< Vio driver
	t_adv7441a_vid_st	vid_st;		//!< Video status
	t_adv7441a_aud_st	aud_st;		//!< Audio status	

} t_adv7441a;

#endif /* ADV7441A_DRV_CFG_H_ */
