#ifndef TMR_REG_H_
#define TMR_REG_H_


/** register byte offset
 * 
 */
#define TMR_OFF_CONFIG              (0x00)
#define TMR_OFF_DIVISION            (0x04)
#define TMR_OFF_INCREMENT           (0x08)
#define TMR_OFF_MASTER              (0x0c)
#define TMR_OFF_SLAVE               (0x10)
#define TMR_OFF_AVIN                (0x14)
#define TMR_OFF_AVOUT               (0x18)
#define TMR_OFF_ETH                 (0x0C)
#define TMR_OFF_DIFF                (0x1C)
 

/** register value
 * 
 */
#define TMR_CFG_MUX_MASK            (0x07)
#define TMR_CFG_AVIN_MASTER         (0x00)
#define TMR_CFG_AVIN_SLAVE          (0x01)
#define TMR_CFG_AVOUT_MASTER        (0x00)
#define TMR_CFG_AVOUT_SLAVE         (0x02)
#define TMR_CFG_ETH_MASTER          (0x00)
#define TMR_CFG_ETH_SLAVE           (0x04)

#define TMR_INC_FRACTION_WIDTH      (20)

#endif /*TMR_REG_H_*/
