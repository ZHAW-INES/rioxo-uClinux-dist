#ifndef VIO_DRV_CFG_H_
#define VIO_DRV_CFG_H_

#include "vid_const.h"
#include "vio_drv.h"


// sampling conversion [input][output]
uint32_t comp_ss[2][2] = {
    //      444,                422 output
        {   S444TO444,          S444TO422  }, // 444 input
        {   S422TO444,          S444TO444  }  // 422 input
    }; 
    
typedef t_color_transform t_color_transform_matrix[3][3]

// color space conversion [input][output]
const t_color_transform_matrix = {
    //  RGB         YUV lim     YUV ext (=output)
      { xyz_one,    rgb2yuv,    0       },  // RGB
      { yuv2rgb,    xyz_one,    0       },  // YUV limited
      { 0,          0,          xyz_one }   // YUV extended
    }



#endif /*VIO_DRV_CFG_H_*/
