/*
 * sta_reg.h
 *
 *  Created on: 05.10.2010
 *      Author: alda
 */

#ifndef STA_REG_H_
#define STA_REG_H_

/*
 *  Register Offset
 */
#define STA_OFF_STATUS              (0x0000)
#define STA_OFF_DELAY               (0x0004)
#define STA_OFF_MISS_CNT            (0x0008)


/* Register Value
 *
 */
#define STA_STA_MORE                (1<<0)
#define STA_STA_FULL                (1<<1)


#endif /* STA_REG_H_ */
