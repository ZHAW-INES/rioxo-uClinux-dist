#ifndef ADV212_H_
#define ADV212_H_

#include "stdhal.h"
#include "adv212_reg.h"

// Timeout = 0.25s
#define ADV212_BOOT_TIMEOUT         25000000

/* !!! ADV212 is Big Endian !!!
 * 
 * HDATA-BUS:
 * Bus:    | 31:24 | 23:16 | 15: 8 |  7: 0 | 
 * Byte:   |  MSB  |       |       |  LSB  |
 * Offset: |   0   |   1   |   2   |   3   | (ADV212)
 * Offset: |   3   |   2   |   1   |   0   | (NIOS II) 
 * 
 * NIOS II also outputs MSB on [31:24]. No Problem for 32 Bit Access.
 *
 * Swap firmware: Byte 1<->0 per 16 Bit / 2 Byte
 *  firmware is loaded in 16 bit mode
 *  can be done prior to storing to flash. firmware is stored big endian
 * Swap when 16 bit array: swap offset 0<->1, 2<->3, 4<->5 etc...
 *  offset_le = offset_be xor (offset_be & 2)
 * 
 */

// Board specific boot up routine for jdata mode
void adv212_pll_init(void* p);
int adv212_boot_jdata(void* p, uint32_t* firmware, size_t size, uint16_t swflag,
                      uint32_t* param_set, uint32_t param_count, 
                      uint32_t* register_set, uint32_t register_count, bool mc);
void adv212_set_rc_size(void* p, uint32_t size);
uint32_t adv212_get_irq(void* p);

int adv212_boot_sync(void* p);
int adv212_boot_sync_wait(void* p, void* vio);
int adv212_boot_sync_ack(void* p);


#endif /*ADV212_H_*/
