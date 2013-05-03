#ifndef TMR_REG_H_
#define TMR_REG_H_


/** register byte offset
 * 
 */
#define tmr_reg2off(slave_nr, base) (base + (4 * slave_nr))
#define TMR_OFF_CONFIG              (0x00)
#define TMR_OFF_DIVISION            (0x04)
#define TMR_OFF_MASTER              (0x08)
#define TMR_OFF_INCREMENT_0         (0x0C)
#define TMR_OFF_INCREMENT_1         (0x10)
#define TMR_OFF_SLAVE_0             (0x14)
#define TMR_OFF_SLAVE_1             (0x18)
#define TMR_OFF_DIFF_0              (0x1C)
#define TMR_OFF_DIFF_1              (0x20)
#define TMR_REG_INCREMENT           TMR_OFF_INCREMENT_0
#define TMR_REG_SLAVE               TMR_OFF_SLAVE_0
#define TMR_REG_DIFF                TMR_OFF_DIFF_0

/** register value
 * 
 */
#define TMR_INC_FRACTION_WIDTH      (20)

#endif /*TMR_REG_H_*/
