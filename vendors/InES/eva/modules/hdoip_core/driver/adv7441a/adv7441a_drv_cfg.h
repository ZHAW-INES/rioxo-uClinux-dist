#ifndef ADV7441A_DRV_CFG_H_
#define ADV7441A_DRV_CFG_H_

#include "std.h"
#include "i2c_drv.h"


/* timing parameters */
//#define ADV7441A_TIMER_INTERVAL		(20 * HZ / 1000) /* 20ms */
#define ADV7441A_RESET_WAIT		(5  * HZ / 1000) /* 5ms */

/* status bit declaration */
#define ADV7441A_STATUS			(0x0000000F)
#define ADV7441A_STATUS_ACTIVE		(0x00000001)
#define ADV7441A_STATUS_CONNECTION	(0x00000002)
#define ADV7441A_STATUS_ENCRYPTED	(0x00000004)
#define ADV7441A_STATUS_AUDIO		(0x00000008)

typedef struct {
	uint32_t		fs;
	uint8_t			bit_width;
	uint8_t			channel_cnt;
	uint8_t			mute;
} t_adv7441a_aud_st;

typedef struct {
	uint8_t			vsync_pol;
	uint8_t			hsync_pol;
	uint32_t		line_width;
	uint32_t		field0;
	uint32_t		field1;
	uint16_t		pixel_clk;
} t_adv7441a_vid_st;

typedef struct {
	uint32_t		status;		
	t_i2c    		*p_i2c;       	//!< I2C driver
	t_adv7441a_vid_st	vid_st;		//!< Video status
	t_adv7441a_aud_st	aud_st;		//!< Audio status	
		
} t_adv7441a;

#endif /* ADV7441A_DRV_CFG_H_ */
