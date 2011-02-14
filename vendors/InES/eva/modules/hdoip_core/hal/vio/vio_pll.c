#include "vio_hal.h"


/** prototypes
 * 
 */

void vio_pll_write(void* p, int o, int v);
int  vio_pll_read(void* p, int o);
void vio_pll_counter(void* p, int cnt, int v);
void vio_pll_reconfig(void* p);
//void vio_pll_shift(void* p, int sel, int phase, int64_t fvco);
void vio_pll_drift(void* p, int ppm, int64_t fvco);

/** functions
 * 
 */


/** Write New Value to PLL (cached)
 * 
 * @param p base pointer to vio-registers
 * @param o type and param select
 * @param v new value
 */
void vio_pll_write(void* p, int o, int v)
{
    v = v & 0x1ff;
    
    // Set Type & Param
    HOI_WR32(p, VIO_OFF_PLL_TYPEPARAM, o);
    // Set Data
    HOI_WR32(p, VIO_OFF_PLL_DATA, v);
    
    // Write...
    HOI_WR32(p, VIO_OFF_PLL_CONFIG, VIO_PLL_CFG_WRITE);
    while (HOI_RD32(p, VIO_OFF_PLL_STATUS) & VIO_PLL_STA_BUSY);
}

/** Read Value from PLL (cached)
 * 
 * @param p base pointer to vio-registers
 * @param o type and param select
 * @return value
 */
int vio_pll_read(void* p, int o)
{
    // Set Type & Param
    HOI_WR32(p, VIO_OFF_PLL_TYPEPARAM, o);
    
    // Read...
    HOI_WR32(p, VIO_OFF_PLL_CONFIG, VIO_PLL_CFG_READ);
    while (HOI_RD32(p, VIO_OFF_PLL_STATUS) & VIO_PLL_STA_BUSY);
    
    // Get Data
    return HOI_RD32(p, VIO_OFF_PLL_DATA);    
}

/** Loads PLL counter with new value
 * 
 * Load PLL counter with a new 50% duty cycle value
 * 
 * @param p base pointer to vio-registers
 * @param cnt counter address to change
 * @param v new value
 */
void vio_pll_counter(void* p, int cnt, int v)
{
    if (v&1) {
        // odd counter value
        vio_pll_write(p, cnt|VIO_PLL_HIGHCOUNT, v/2+1);
        vio_pll_write(p, cnt|VIO_PLL_LOWCOUNT, v/2);
        vio_pll_write(p, cnt|VIO_PLL_MODE_ODD, 1);
        vio_pll_write(p, cnt|VIO_PLL_BYPASS, 0);
    } else {
        // even counter value
        vio_pll_write(p, cnt|VIO_PLL_HIGHCOUNT, v/2);
        vio_pll_write(p, cnt|VIO_PLL_LOWCOUNT, v/2);
        vio_pll_write(p, cnt|VIO_PLL_MODE_ODD, 0);
        vio_pll_write(p, cnt|VIO_PLL_BYPASS, 0);
    }
}

/** Loads PLL with cached values
 * 
 * @param p base pointer to vio-registers
 */
void vio_pll_reconfig(void* p)
{
    HOI_WR32(p, VIO_OFF_PLL_CONFIG, VIO_PLL_CFG_RECONFIG);
    while (HOI_RD32(p, VIO_OFF_PLL_STATUS) & VIO_PLL_STA_BUSY);
}

/** Shift PLL output
 * 
 * @param p base pointer to vio-registers
 * @param sel select clock output
 * @param n relative shift in [ps]
 */
/*void vio_pll_shift(void* p, int sel, int phase, int64_t fvco)
{
    uint32_t v;
    int n = (-(int32_t)(((int64_t)phase * fvco * 8 + (int64_t)5.0e11) / (int64_t)1.0e12));
    
    HOI_WR32(p, VIO_OFF_PLL_PCS, sel);
    
    if (n > 0) {
        v = VIO_PLL_SHIFT_UP | (n & VIO_PLL_SHIFT_MASK);
    } else if (n < 0) {
        v = VIO_PLL_SHIFT_DOWN | ((-n) & VIO_PLL_SHIFT_MASK);
    }

    HOI_WR32(p, VIO_OFF_PLL_SHIFT, v|VIO_PLL_SHIFT_EN);
    HOI_WR32(p, VIO_OFF_PLL_SHIFT, v);
}*/

/** Set the maximum pll frequency drift expressed in ppm.
 * 
 * ex.: 25MHz Output, 100ppm allows the controller
 *      to create a output frequence range from
 *      24'997'500 Hz to 25'002'500 Hz 
 * 
 * @param p base pointer to vio-registers
 * @param ppm parts per million the controller may change the vco
 * @param fvco VCO frequency in [Hz]
 */
void vio_pll_drift(void* p, int ppm, int64_t fvco)
{
    int nmax;
    
    nmax = (int)(((int64_t)SFREQ * (int64_t)1000000) / ((int64_t)8 * (int64_t)ppm * fvco));
    
    if (nmax < VIO_PLL_MIN_CYCLE) nmax = VIO_PLL_MIN_CYCLE;
    
    HOI_WR32(p, VIO_OFF_PHASESTEP, nmax);    
}

/** Select the input clock source
 *
 */
void vio_pll_sel(void* p, int sel)
{
    uint32_t status = vio_get_sta(p, VIO_STA_IN_CLOCK_EXT); 

    switch (sel) {
        case VIO_SEL_INPUT:
            if (!status) {
                // switch input
                vio_set_cfg(p, VIO_CFG_PLL_SW);
                msleep(10);
                vio_clr_cfg(p, VIO_CFG_PLL_SW);
                msleep(10);
            }
        break;
        case VIO_SEL_75MHZ:
            if (status) {
                // switch input
                vio_set_cfg(p, VIO_CFG_PLL_SW);
                msleep(10);
                vio_clr_cfg(p, VIO_CFG_PLL_SW);
                msleep(10);
            }
        break;
    }   
}

/** Update the Clock network/PLL with new values
 * 
 * @param p base pointer to vio-registers
 */
void vio_pll_update(void* p, t_vio_pll* pll)
{
    // select clock input
    vio_pll_sel(p, pll->insel);
    
    // configure pll control
    vio_pll_drift(p, pll->ppm, pll->fvco);
    
    // set new parameter
    vio_pll_counter(p, VIO_PLL_TYPE_N, pll->n);
    vio_pll_counter(p, VIO_PLL_TYPE_M, pll->m);
    vio_pll_counter(p, VIO_PLL_TYPE_C0, pll->c[0]);
    vio_pll_counter(p, VIO_PLL_TYPE_C1, pll->c[1]);
    vio_pll_counter(p, VIO_PLL_TYPE_C2, pll->c[2]);
    vio_pll_counter(p, VIO_PLL_TYPE_C3, pll->c[3]);
    
    vio_pll_reconfig(p);
    
    msleep(10);
}
