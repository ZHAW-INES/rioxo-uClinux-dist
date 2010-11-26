/*
 * i2c_str.h
 *
 *  Created on: 20.08.2010
 *      Author: alda
 */

#ifndef I2C_STR_H_
#define I2C_STR_H_

#define I2C_REPORT_ERROR(err, dev, adr) { \
	REPORT(WARNING, "[%02x:%02x] i2c drv error: %s", (int)dev, (int)adr, i2c_str_err(err)); \
}

#define I2C_REPORT(p) { \
	REPORT(INFO, "<i2c controller> [%08x]", (uint32_t)p); \
	REPORT(INFO, "  prescale = %d", i2c_get_prescale(p)); \
	REPORT(INFO, "  ctr = %02x", HOI_RD32(p, I2C_OFF_CTR)); \
}

char* i2c_str_err(int err);

#endif /* I2C_STR_H_ */
