#ifndef VIO_PLL_H_
#define VIO_PLL_H_

#include "stdhal.h"
#include "vio_reg.h"
#include "vio_cfg.h"

/** this struct stores the configuration for the pll
 */
typedef struct {
    uint32_t            fsys;       //!< System frequency [Hz]
    uint32_t            fvco;       //!< VCO frequency [Hz]
    uint32_t            fin[2];     //!< input frequency [Hz]
    uint32_t            insel;      //!< input selector
    uint32_t            fout[4];    //!< output frequency [Hz]
    //uint32_t            phase[4];   //!< output delay [ps]
    uint32_t            n, m, c[4]; //!< pll parameter [Hz]
    uint32_t            ppm;        //!< pll tolerance for control loop
    uint32_t            mode;       //!< control mode selector
} t_vio_pll;

/** defines (which pll input)
 */
#define VIO_SEL_75MHZ           (0x00)
#define VIO_SEL_INPUT           (0x01)

#define VIO_JVCLK_DOUBLE        (0x04)
#define VIO_JVCLK_FULL          (0x02)
#define VIO_JVCLK_HALF          (0x01)

/** vio pll prototypes
 */
void vio_pll_sel(void* p, int sel);
//void vio_pll_shift(void* p, int sel, int n, int64_t fvco);
void vio_pll_update(void* p, t_vio_pll* pll);

#endif /*VIO_PLL_H_*/
