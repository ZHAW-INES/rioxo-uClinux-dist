#ifndef ADV7441A_REG_H_
#define ADV7441A_REG_H_

#include "adv7441a_user_map_reg.h"
#include "adv7441a_user_1_map_reg.h"
#include "adv7441a_user_2_map_reg.h"
#include "adv7441a_user_3_map_reg.h"
#include "adv7441a_hdmi_map_reg.h"
#include "adv7441a_repeater_map_reg.h"


// I2C Address	(ALSB = high)
#define ADV7441A_I2C_USR_MAP		(0x42)
#define ADV7441A_I2C_USR_MAP1		(0x46)
#define ADV7441A_I2C_USR_MAP2		(0x62)
#define ADV7441A_I2C_USR_MAP3		(0x4A)
#define ADV7441A_I2C_HDMI_MAP		(0x6A)
#define ADV7441A_I2C_KSV_MAP		(0x66)
#define ADV7441A_I2C_EDID_MAP		(0x6E)

#endif /*ADV7441A_REG_H_*/
