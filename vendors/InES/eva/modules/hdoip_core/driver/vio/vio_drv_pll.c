#include "vio_drv.h"
#include "vio_drv_pll.h"
#include "hoi_msg.h"

/*
// Constants
// vio_phase[in][out]
static const int vio_phase[2][4] = {
        //    c[0]    c[1]    c[2]    c[3]
        {       0,      0,      0,      0},   // 75 MHz internal
        {       0,  -4000,  -4000,      0}    // external
    };
*/

int vio_drv_pll_setup(void* p, t_vio_pll* pll, t_si598* si598, int in, int out, int rel, int mode, uint32_t device, int advcnt)
{
    const uint64_t fmin = 600000000;
    const uint64_t fmax = 1100000000;     // should be 1300000000 (see manual), but if its bigger than 1,1GHz, output frequency is not right
    int64_t cmin, cmax;
    // PLL config set
    bool b;
    int64_t n, m; // n:div m:mul
    int64_t c0, c1;    
    int64_t fin, fout, fvco, ferr;
    // temporary
    int64_t kn, km, kc, ki;
    int64_t kfvco, tfvco, kferr;
    

    // output frequency 
    fout = fin;
    if (device == BDT_ID_HDMI_BOARD) {
        if (out != 0) {
            fout = out;
        }
    } else {

        if ((out > 147000000) && (out < 150000000)) {
            fout = 148500000;
        }

        if ((out > 73000000) && (out < 76000000)) {
            fout = 74250000;
        }

        if ((out > 12000000) && (out < 15000000)) {
            fout = 13500000;
        }
    }

    pll->fout[0] = fout;
    pll->fout[1] = fout * rel / 2;
    pll->fout[2] = fout * rel / 2;
    pll->fout[3] = fout;


    // input frequency for HDMI
    if (device == BDT_ID_HDMI_BOARD) {
        pll->insel      = in;
        pll->fin[0]     = BFREQ;
        pll->fin[1]     = vio_get_fin(p);
        fin = pll->fin[in]; 
        vio_set_input_clockmux(p, advcnt, true);

    // input frequency for SDI
    } else {
        pll->insel      = in;
        pll->fin[1]     = vio_get_fin(p);

        // fin = fout / set SI598 output frequency
        if (fout == 148500000) {
            set_si598_output_frequency (si598, 148);
            pll->fin[0]     = 148500000;
            vio_set_input_clockmux(p, advcnt, false);
        }
        if (fout == 74250000) {

            if (advcnt == 4) {
                set_si598_output_frequency (si598, 148);
            } else {
                set_si598_output_frequency (si598, 74);
            }

            pll->fin[0]     = 74250000;
            vio_set_input_clockmux(p, advcnt, false);
        }
        if (fout == 13500000) {
            set_si598_output_frequency (si598, 27);
            pll->fin[0]     = 13500000;
            vio_set_input_clockmux(p, advcnt, false);
        }
        fin = pll->fin[in];
    } 
    
    // VCO is between 600 and 1100 MHz
    cmin = fmin / fout + 1;
    cmax = fmax / fout;
    if (cmax >= 512) cmax = 511;
    if (cmin >= 512) cmin = 511;
    
    // c needs to be multiple of 2
    if (rel == VIO_JVCLK_DOUBLE) {
        cmin = cmin + (cmin&1);
        cmax = cmax & 0x1fe;
        ki = 2;
    } else {
        ki = 1;
    }

    b = false;
    for (kc = cmin; kc < cmax; kc += ki) {
        // target vco
        tfvco = kc * fout;

        // search for factors;        
        for (kn=2;kn<512;kn++) {

            // target vco = fin * m / n
            km = (tfvco * kn + fin/2) / fin;

            if (km && (km < 512)) {

                // real vco = fin * m / n
                kfvco = fin * km / kn;
                
                // get error
                kferr = (kfvco / kc) - fout;
                kferr = (kferr>0) ? kferr : -kferr;
                
                if ((kfvco > fmin) && (kfvco < fmax)) {
                    if (!b || (kferr < ferr)) {
                        b = true;
                        n = kn;
                        m = km;
                        c0 = kc;
                        ferr = kferr;
                        fvco = kfvco;
                    }                    
                }
                
            }
            
        }
        
    }

    if ((n >= 512) || (m >= 512) || !b) {
        return ERR_VIO_NO_PLL_CONFIG;
    } 
    
    if ((fvco>fmax)||(fvco<fmin)) {
        return ERR_VIO_VCO_OUT_OF_RANGE;
    }
    
    c1 = c0 * 2 / rel;

    // output set
    pll->fsys       = SFREQ;
    pll->fvco       = fvco;
    /*pll->phase[0]   = vio_phase[in][0];
    pll->phase[1]   = vio_phase[in][1];
    pll->phase[2]   = vio_phase[in][2];
    pll->phase[3]   = vio_phase[in][3];*/
    pll->n          = n;
    pll->m          = m;
    pll->c[0]       = c0;
    pll->c[1]       = c1;
    pll->c[2]       = c1;
    pll->c[3]       = c0;
    pll->ppm        = (int)(1000000*ferr/fout) + (int32_t)(SYS_TOL*1.0e6);
    pll->mode       = mode;

    return ERR_VIO_SUCCESS;
}


