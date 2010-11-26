#ifndef ADV7441A_DRV_I2C_H_
#define ADV7441A_DRV_I2C_H_

#include "std.h"
#include "i2c_drv.h"
#include "adv7441a_drv_cfg.h"

/* Inline functions */ 
static inline void adv7441a_lock(t_adv7441a* handle) 
{
	i2c_drv_lock(handle->p_i2c);
}

static inline void adv7441a_unlock(t_adv7441a* handle) 
{
	i2c_drv_unlock(handle->p_i2c);
}

static inline void adv7441a_usr_map_write(t_adv7441a* handle, uint8_t a, uint8_t x) 
{
	i2c_drv_wreg8(handle->p_i2c, ADV7441A_I2C_USR_MAP, a, x);
}

static inline int adv7441a_usr_map_read(t_adv7441a* handle, uint8_t a) 
{
	return i2c_drv_rreg8(handle->p_i2c, ADV7441A_I2C_USR_MAP, a);
}

static inline void adv7441a_usr_map1_write(t_adv7441a* handle,uint8_t a, uint8_t x) 
{
	i2c_drv_wreg8(handle->p_i2c, ADV7441A_I2C_USR_MAP1, a, x);
}

static inline int adv7441a_usr_map1_read(t_adv7441a* handle,uint8_t a) 
{
	return i2c_drv_rreg8(handle->p_i2c, ADV7441A_I2C_USR_MAP1, a);
}

static inline void adv7441a_usr_map2_write(t_adv7441a* handle,uint8_t a, uint8_t x) 
{
	i2c_drv_wreg8(handle->p_i2c, ADV7441A_I2C_USR_MAP2, a, x);
}

static inline int adv7441a_usr_map2_read(t_adv7441a* handle,uint8_t a) 
{
	return i2c_drv_rreg8(handle->p_i2c, ADV7441A_I2C_USR_MAP2, a);
}

static inline void adv7441a_usr_map3_write(t_adv7441a* handle,uint8_t a, uint8_t x) 
{
	i2c_drv_wreg8(handle->p_i2c, ADV7441A_I2C_USR_MAP3, a, x);
}

static inline int adv7441a_usr_map3_read(t_adv7441a* handle,uint8_t a) 
{
	return i2c_drv_rreg8(handle->p_i2c, ADV7441A_I2C_USR_MAP3, a);
}

static inline void adv7441a_hdmi_map_write(t_adv7441a* handle,uint8_t a, uint8_t x) 
{
	i2c_drv_wreg8(handle->p_i2c, ADV7441A_I2C_HDMI_MAP, a, x);
}

static inline int adv7441a_hdmi_map_read(t_adv7441a* handle,uint8_t a) 
{
	return i2c_drv_rreg8(handle->p_i2c, ADV7441A_I2C_HDMI_MAP, a);
}

static inline void adv7441a_ksv_map_write(t_adv7441a* handle,uint8_t a, uint8_t x) 
{
	i2c_drv_wreg8(handle->p_i2c, ADV7441A_I2C_KSV_MAP, a, x);
}

static inline int adv7441a_ksv_map_read(t_adv7441a* handle, uint8_t a) 
{
	return i2c_drv_rreg8(handle->p_i2c, ADV7441A_I2C_KSV_MAP, a);
}

static inline void adv7441a_edid_map_write(t_adv7441a* handle, uint8_t a, uint8_t x) 
{
	i2c_drv_wreg8(handle->p_i2c, ADV7441A_I2C_EDID_MAP, a, x);
}

static inline int adv7441a_edid_map_read(t_adv7441a* handle, uint8_t a) 
{
	return i2c_drv_rreg8(handle->p_i2c, ADV7441A_I2C_EDID_MAP,a);
}

#endif /* ADV7441A_DRV_I2C_H_ */
