#include "adv212.h"
#include "vio_hal.h"


/** Macros
 */
#define adv212_write16(p, o, v) HOI_WR32((p), (o), (v))
#define adv212_read16(p, o) HOI_RD32((p), (o))

// Indirect Access is 16 Bit Wide (we are using stage register for address)
// Autoincrementing the address (by 16 Bit) reduces overhead when writing to
// continuous addresses (Note 16 Bit Registers are 32 Bit aligned)
#define adv212_w16_stp16(p) adv212_write16(p, ADV212_MMODE, ADV212_IWIDTH_16 | ADV212_IAUTOSTP_16 | ADV212_IAUTOMOD_INC | ADV212_MAP_FULL);

// Indirect Access is 16 Bit Wide (we are using stage register for address)
// Autoincrementing the address (by 32 Bit) reduces overhead when writing to
// continuous addresses (Note 16 Bit Registers are 32 Bit aligned)
#define adv212_w16_stp32(p) adv212_write16(p, ADV212_MMODE, ADV212_IWIDTH_16 | ADV212_IAUTOSTP_32 | ADV212_IAUTOMOD_INC | ADV212_MAP_FULL);


/** Function Prototype
 */

// direct 16 & 32 Bit Read / Write
uint32_t adv212_read32(void* p, int o);
void adv212_write32(void* p, int o, uint32_t v);
// indirect Read / Write (for IWIDTH = 16 Bit; IAUTOSTOP = 16/32)
uint32_t adv212_iread16(void* p, int o);
uint32_t adv212_iread32(void* p, int o);
void adv212_iread16b(void* p, uint32_t o, uint16_t* buf, size_t size);
void adv212_iread32b(void* p, uint32_t o, uint32_t* buf, size_t size);
void adv212_iwrite16(void* p, int o, uint32_t v);
void adv212_iwrite32(void* p, int o, uint32_t v);
void adv212_iwrite16b(void* p, uint32_t o, uint16_t* buf, size_t size);
void adv212_iwrite32b(void* p, uint32_t o, uint32_t* buf, size_t size);


/** Function
 */

/** Read a 32 Bit direct register from ADV212
 *  
 * @param p base address of ADV212
 * @param o offset in bytes. Note: each register is 32 bit aligned (0, 4, 8 ... 60) 
 */
uint32_t adv212_read32(void* p, int o)
{
    uint32_t buf;
    buf = (adv212_read16(p, o) & 0xffff) << 16;
    buf = buf | adv212_read16(p, ADV212_STAGE);
    return buf;
}


/** Write a 32 Bit direct register to ADV212
 * 
 * @param p base address of ADV212
 * @param o offset in bytes. Note: each register is 32 bit aligned (0, 4, 8 ... 60)
 * @param v value to be written  
 */
void adv212_write32(void* p, int o, uint32_t v)
{
    adv212_write16(p, ADV212_STAGE, (v >> 16));
    adv212_write16(p, o, v & 0xffff);
}


/** Read 16 Bit indirect register/memory to ADV212
 *  
 * MMODE must be set to 16 Bit Width and 16/32 Bit Step.
 * 
 * @param p base address of ADV212
 * @param o address in bytes 
 */
uint32_t adv212_iread16(void* p, int o)
{
    adv212_write32(p, ADV212_IADDR, o);
    return adv212_read16(p, ADV212_IDATA);
}


/** Read 32 Bit indirect register/memory from ADV212
 * 
 * MMODE must be set to 16 Bit Width and 16 Bit Step
 *  
 * @param p base address of ADV212
 * @param o address in bytes 
 */
uint32_t adv212_iread32(void* p, int o)
{
    uint32_t buf;    
    adv212_write32(p, ADV212_IADDR, o);
    buf = (adv212_read16(p, ADV212_IDATA) & 0xffff);
    buf = buf | (adv212_read16(p, ADV212_IDATA) << 16);    
    return buf;
}


/** Read Burst 16 Bit indirect memory from ADV212
 *  
 * Reads size bytes from ADV212 Memory and copys
 * to a buffer. Reads a multiple of 16 Bit.
 * MMODE must be set to 16 Bit Width and 16/32 Bit Step
 * Step 16 Bit reads address 0,2,4,6...
 * Step 32 Bit reads address 0,4,8,12... 
 * 
 * @param p base address of ADV212
 * @param o address in bytes 
 * @param buf pointer to buffer. Needs to be able to store size bytes
 * @param size number of bytes to be read (multiple of 2)
 */
void adv212_iread16b(void* p, uint32_t o, uint16_t* buf, size_t size)
{
    // size in beats(16 Bit)
    size = (size + 1) / 2;
    adv212_write32(p, ADV212_IADDR, o);
    for (int n=0;n<size;n++) {       
        buf[n] = adv212_read16(p, ADV212_IDATA);
    }
}


/** Read Burst 32 Bit indirect memory from ADV212
 *  
 * Reads size bytes from ADV212 Memory and copys
 * to a buffer. Reads a multiple of 32 Bit.
 * MMODE must be set to 16 Bit Width and 16 Bit Step. 
 * 
 * @param p base address of ADV212
 * @param o address in bytes 
 * @param buf pointer to buffer. Needs to be able to store size bytes
 * @param size number of bytes to be read (multiple of 4)
 */
void adv212_iread32b(void* p, uint32_t o, uint32_t* buf, size_t size)
{
    // size in beats(32 Bit)
    size = (size + 3) / 4;
    adv212_write32(p, ADV212_IADDR, o);
    for (int n=0;n<size;n++) {
        buf[n] = (adv212_read16(p, ADV212_IDATA) & 0xffff);
        buf[n] = buf[n] | (adv212_read16(p, ADV212_IDATA) << 16);    
    }
}


/** Write 16 Bit indirect register/memory to ADV212
 *  
 * @param p base address of ADV212
 * @param o address in bytes
 * @param v value to be written  
 */
void adv212_iwrite16(void* p, int o, uint32_t v)
{
    adv212_write32(p, ADV212_IADDR, o);
    adv212_write16(p, ADV212_IDATA, v);
}


/** Write 32 Bit indirect register/memory to ADV212
 *  
 * MMODE must be set to 16 Bit Width and 16 Bit Step
 * 
 * @param p base address of ADV212
 * @param o address in bytes
 * @param v value to be written  
 */
void adv212_iwrite32(void* p, int o, uint32_t v)
{
    adv212_write32(p, ADV212_IADDR, o);
    adv212_write16(p, ADV212_IDATA, (v >> 16));
    adv212_write16(p, ADV212_IDATA, (v & 0xffff));
}


/** Write Burst 16 Bit indirect memory to ADV212
 * 
 * Writes size bytes to ADV212 Memory from buffer.
 * Writes a multiple of 16 Bit.
 * MMODE must be set to 16 Bit Width and 16/32 Bit Step 
 * Step 16 Bit writes address 0,2,4,6...
 * Step 32 Bit writes address 0,4,8,12... 
 * 
 * @param p base address of ADV212
 * @param o address in bytes 
 * @param buf pointer to buffer.
 * @param size number of bytes to be written (multiple of 2)
 */
void adv212_iwrite16b(void* p, uint32_t o, uint16_t* buf, size_t size)
{
    // size in beats(16 Bit)
    size = (size + 1) / 2;
    adv212_write32(p, ADV212_IADDR, o);
    for (int n=0;n<size;n++) {
        adv212_write16(p, ADV212_IDATA, buf[n]);
    }
}


/** Write Burst 32 Bit indirect memory to ADV212
 *  
 * Writes size bytes to ADV212 Memory from buffer.
 * Writes a multiple of 32 Bit. 
 * MMODE must be set to 16 Bit Width and 16 Bit Step 
 * 
 * @param p base address of ADV212
 * @param o address in bytes 
 * @param buf pointer to buffer.
 * @param size number of bytes to be written (multiple of 4)
 */
void adv212_iwrite32b(void* p, uint32_t o, uint32_t* buf, size_t size)
{
    // size in beats(32 Bit)
    size = (size + 3) / 4;
    adv212_write32(p, ADV212_IADDR, o);
    for (int n=0;n<size;n++) {
        adv212_write16(p, ADV212_IDATA, (buf[n] >> 16));
        adv212_write16(p, ADV212_IDATA, (buf[n] & 0xffff));
    }
}


/** ADV212 PLL Boot up for JDATA-Mode
 *
 * Boot Up routine according to "ADV212 JPEG2000 Programming Guide" 
 * rev 2.0 page 29. Frequency(JCLK) is set to 150 MHz and MCLK is expected 
 * to be 37.5 MHz. HCLK is 75 MHz. 
 * Sets the ACK pin as Open Drain
 *
* @param p base address of the ADV212   
 */
void adv212_pll_init(void* p)
{
    // Hard Reset (10ms)
    // Boot Mode should also be reset?
    adv212_write16(p, ADV212_BOOT, ADV212_HARDRST);
    msleep(100);
    
    // mclk is 37.5 MHz
    // set pll: Open-drain ACK_N, HCLK = JCLK / 2 (because HCLK max is 108 MHZ)
    // JCLK = 150 MHz, HCLK = 75 MHz
    // JCLK and MCLK are at maximum speed for JDATA Mode
    adv212_write16(p, ADV212_PLL_HI, ADV212_PLL_ACK_OD | ADV212_PLL_HCLKD);
    adv212_write16(p, ADV212_PLL_LO, (4 << ADV212_PLL_MUL_SHIFT));
    
    // wait at least 20us
    udelay(20);
}


/** ADV212 Boot up in JDATA-Mode
 *
 * Boot Up routine according to "ADV212 JPEG2000 Programming Guide"
 * rev 2.0 page 29
 * PLL needs to be initialized for all ADV212 sharing the HDATA, as
 * the ACK pin needs to be set to Open Drain prior to calling this 
 * boot up routine
 * 
 * @see adv212_pll_init for configuring pll
 * @param p base address of the ADV212
 * @param firmware pointer to the firmware to be loaded
 * @param size size of the firmware
 * @param swflag compare value for Encode/Decode
 * @param param_set pointer to the param sets (array of [address|value]) 
 * @param param_count number of param sets 
 * @param register_set pointer to the register sets (array of [address|value]) 
 * @param register_count number of register sets
 */
int adv212_boot_jdata(void* p, uint32_t* firmware, size_t size, uint16_t swflag,
                      uint32_t* param_set, uint32_t param_count, 
                      uint32_t* register_set, uint32_t register_count, bool mc)
{
    unsigned long time;

    if (!param_set || !register_set || !firmware) {
        return ERR_ADV212_NULL_POINTER;
    }
    
    // Enter No-Boot Host Mode 
    adv212_write16(p, ADV212_BOOT, ADV212_SOFTRST | ADV212_NO_BOOT_HOST);
    
    // Set Bus Mode / MMode
    // We are using 16 Bit Host Data-Width / 16 Bit DMA 
    adv212_write16(p, ADV212_BUSMODE, ADV212_HWIDTH_16 | ADV212_DWIDTH_16);
    
    // Access for Memory
    adv212_w16_stp16(p);

    // Load Firmware
    adv212_iwrite16b(p, ADV212_FIRMWARE_BASE, (uint16_t*)firmware, size);

    // Soft reset
    adv212_write16(p, ADV212_BOOT, ADV212_SOFTRST | ADV212_COPROCESSOR_BOOT);

    // Set Bus Mode / MMode again after soft-reset
    // We are using 16 Bit Host Data-Width  
    // and JDATA Mode, No DMA Width may be set in JDATA Mode
    adv212_write16(p, ADV212_BUSMODE, ADV212_HWIDTH_16 | ADV212_BCFG_JDATA);

    // Access for Parameter
    adv212_w16_stp16(p);
    
    // Load Parameters
    for (int i=0; i<param_count; i++) {
        adv212_iwrite32(p, param_set[2*i+0], param_set[2*i+1]);
    }

    // Access for Register (32 Bit Aligned)
    adv212_w16_stp32(p);
    
    // Set Registers
    for (int i=0; i<register_count; i++) {
        adv212_iwrite16(p, register_set[2*i+0], register_set[2*i+1]);
    }
   
    // Set EDMOD: Compressed Stream Output, JDATA interface, Active LOW Valid/Hold
    adv212_iwrite16(p, ADV212_EDMOD0, ADV212_EDMOD0_DMSEL0_COMPRESSED |
                                      ADV212_EDMOD0_DMMOD0_JDATA |
                                      ADV212_EDMOD0_DR0POL_N |
                                      ADV212_EDMOD0_DA0POL_N); 
    
    // Enable SWIRQ0
    adv212_write16(p, ADV212_EIRQIE, ADV212_EIRQ_SWIRQ0);

    // wait for irq assertion (max. 1s)
    time = jiffies + HZ; // 1 second
    while(!(adv212_read16(p, ADV212_EIRQFLG) & ADV212_EIRQ_SWIRQ0)) {
        // add timeout? -> return ADV212_BOOT_TIMEOUT
        if (time_after(jiffies, time)) {
            return ERR_ADV212_BOOT_TIMEOUT;
        }
    }

    // Set EDMOD: Compressed Stream Output, JDATA interface, Active LOW Valid/Hold
    // Enable
    adv212_iwrite16(p, ADV212_EDMOD0, ADV212_EDMOD0_DMSEL0_COMPRESSED |
                                      ADV212_EDMOD0_DMMOD0_JDATA |
                                      ADV212_EDMOD0_DR0POL_N |
                                      ADV212_EDMOD0_DA0POL_N | 
                                      ADV212_EDMOD0_DMEN0); 
    
    if (adv212_read16(p, ADV212_SWFLAG) != swflag) {
        return ERR_ADV212_BOOT_FAIL;
    }

    // Ack SWIRQ0 IRQ
    adv212_write16(p, ADV212_EIRQFLG, ADV212_EIRQ_SWIRQ0);

    // IRQ Enable (CFTH/CFERR seems to be not valid in JDATA mode)
    if (mc) {
        adv212_write16(p, ADV212_EIRQIE, ADV212_EIRQ_SWIRQ1);
    } else {
        adv212_write16(p, ADV212_EIRQIE, 0);
    }

    // Set Bus Mode / MMode again after soft-reset
    // We are using 16 Bit Host Data-Width  
    // and JDATA Mode, No DMA Width may be set in JDATA Mode
    adv212_write16(p, ADV212_BUSMODE, ADV212_HWIDTH_16 | ADV212_BCFG_JDATA);
    
    // Access for Parameter
    adv212_w16_stp16(p);
    

    return ERR_ADV212_SUCCESS;
}


/** wait for SWIRQ1
 *
 * @param p base address of the ADV212
 */

int adv212_boot_sync(void* p)
{
    unsigned long time;

    // wait for irq assertion (max. 1s)
    time = jiffies + HZ; // 1 second
    while(!(adv212_read16(p, ADV212_EIRQFLG) & ADV212_EIRQ_SWIRQ1)) {
        // add timeout? -> return ADV212_BOOT_TIMEOUT
        if (time_after(jiffies, time)) {
            return ERR_ADV212_BOOT_TIMEOUT;
        }
    }

    // Ack SWIRQ0 IRQ
    adv212_write16(p, ADV212_EIRQFLG, ADV212_EIRQ_SWIRQ1);

    return ERR_ADV212_SUCCESS;
}

int adv212_boot_sync_wait(void* p, void* vio)
{
    uint32_t time, dt;


    dt = 0;
    time = vio_get_timer(vio);

    while(!(adv212_read16(p, ADV212_EIRQFLG) & ADV212_EIRQ_SWIRQ1)) {
        // add timeout? -> return ADV212_BOOT_TIMEOUT
        dt = vio_get_timer(vio) - time;
        if (dt > ADV212_BOOT_TIMEOUT) {
            return ERR_ADV212_BOOT_TIMEOUT;
        }
    }

    return ERR_ADV212_SUCCESS;
}

int adv212_boot_sync_ack(void* p)
{
    // Ack SWIRQ0 IRQ
    adv212_write16(p, ADV212_EIRQFLG, ADV212_EIRQ_SWIRQ1);

    return ERR_ADV212_SUCCESS;
}


/** Set Rate Control to Size per (Image/Tile/Component)
 * 
 * the ADV212 can archiev a output size from (-100% ... +5%)
 * 
 * @param p base address of the ADV212
 * @param size new size of Image/Tile/Component 
 */
void adv212_set_rc_size(void* p, uint32_t size)
{
    adv212_iwrite32(p, ADV212_PARAMETER_BASE+ADV212_ENC_RCVAL, 
                       (ADV212_RCTYPE_IMGSIZE<<24) | (size & 0x00ffffff));   
}


uint32_t adv212_get_irq(void* p)
{
    uint32_t irq = adv212_read16(p, ADV212_EIRQFLG);
    adv212_write16(p, ADV212_EIRQFLG, ADV212_EIRQ_ALL);
    return irq;
}

