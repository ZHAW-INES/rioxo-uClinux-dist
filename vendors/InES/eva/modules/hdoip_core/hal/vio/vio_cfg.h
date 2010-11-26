#ifndef VIO_CFG_H_
#define VIO_CFG_H_

// minimum clock cycles between pll shift operation
#define VIO_PLL_MIN_CYCLE       (10)

// number of bits for the control loop input
#define VIO_PLL_CTRL_WIDTH      (5)
// divide error by this value before feeding to the pll control
#define VIO_PLL_CTRL_DIV        (16)
// how much the pll control may drift before hard reset (relative to tolerance)
#define VIO_PLL_CTRL_MAX        (20)

#endif /*VIO_CFG_H_*/
