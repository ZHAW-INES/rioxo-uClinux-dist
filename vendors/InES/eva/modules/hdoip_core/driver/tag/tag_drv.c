/*
 * tag_drv.c
 *
 *  Created on: 14.09.2010
 *      Author: alda
 */

#include "tag_drv.h"


static inline void tag_lock(t_tag* handle) {
	i2c_drv_lock(handle->p_i2c);
}

static inline void tag_unlock(t_tag* handle) {
	i2c_drv_unlock(handle->p_i2c);
}

static inline void tag_write(t_tag* handle, uint8_t a, uint8_t x) {
	i2c_drv_wreg8(handle->p_i2c, handle->devaddr, a, x);
}

static inline int tag_read(t_tag* handle, uint8_t a) {
	return i2c_drv_rreg8(handle->p_i2c, handle->devaddr, a);
}

static inline int tag_write_buf(t_tag* handle, uint8_t a, uint8_t *x, size_t s) {
    return i2c_drv_wreg8b(handle->p_i2c, handle->devaddr, a, x, s);
}

static inline int tag_read_buf(t_tag* handle, uint8_t a, uint8_t *x, size_t s) {
    return i2c_drv_rreg8b(handle->p_i2c, handle->devaddr, a, x, s);
}


int tag_drv_init(t_tag* handle, t_i2c* p_i2c, uint32_t devaddr)
{
    REPORT(INFO, ">> tag-driver initialized");

    handle->p_i2c = p_i2c;
    handle->devaddr = devaddr;
    handle->available = false;
    memset(&handle->buf, 0, 256);

    return SUCCESS;
}

int tag_drv_inquire(t_tag* handle)
{
    uint32_t ret;

    tag_lock(handle);

	// read total size
    size_t size = tag_read(handle, 0) + 2;

    if ((ret = i2c_drv_get_error(handle->p_i2c))) {

        handle->available = false;

    } else {

        handle->available = true;

        // limit size
        if (size > 256) size = 256;

        // read payload
        ret = tag_read_buf(handle, 0, handle->buf, size);

    }

    tag_unlock(handle);

	return SUCCESS;
}

int tag_drv_write(t_tag* handle)
{
    uint32_t ret;

    tag_lock(handle);

    // read total size
    size_t size = (size_t)handle->buf[0] + 2;

    // limit size
    if (size > 256) size = 256;

    // write tag
    ret = tag_write_buf(handle, 0, handle->buf, size);

    tag_unlock(handle);

    return SUCCESS;
}


