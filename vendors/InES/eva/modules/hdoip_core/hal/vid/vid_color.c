#include "stdvid.h"

#define FRAC_M3x4(m1,m2,m3,m4,m5,m6,m7,m8,m9,m10,m11,m12) \
	  {{to_frac16(m1),to_frac16(m2),to_frac16(m3),to_frac16(m4)}, \
	   {to_frac16(m5),to_frac16(m6),to_frac16(m7),to_frac16(m8)}, \
	   {to_frac16(m9),to_frac16(m10),to_frac16(m11),to_frac16(m12)}}

// null transform
const t_color_transform xyz_one = FRAC_M3x4(
		1.0,     0.0,     0.0,     0.0,
		0.0,     1.0,     0.0,     0.0,
		0.0,     0.0,     1.0,     0.0
    );

// black picture transform
const t_color_transform black_out = FRAC_M3x4(
		0.0,     0.0,     0.0,     0.0,
		0.0,     0.0,     0.0,     0.0,
		0.0,     0.0,     0.0,     0.0
    );
    
// yuv range y:[16..236] uv:[16..240]
const t_color_transform rgb2yuv = FRAC_M3x4(
		0.2578,  0.5039,  0.0977,  0.0625,
	   -0.1484, -0.2891,  0.4375,  0.5000,
		0.4375, -0.3672, -0.0703,  0.5000
    );

// rgb range after clip y:[0..255] uv:[0..255]
const t_color_transform yuv2rgb = FRAC_M3x4(
		1.1563,  0.0   ,  1.6042, -0.8741,
		1.1563, -0.3889, -0.8133,  0.5288,
		1.1563,  2.0286,  0.0067, -1.0899
    );
    
// gray range y:[16..236]
const t_color_transform rgb2y = FRAC_M3x4(
		0.2578,  0.5039,  0.0977,  0.0625,
		0.0,     0.0,     0.0,     0.5000,
		0.0,     0.0,     0.0,     0.5000
    );
    
const t_color_transform x2rgb = FRAC_M3x4(
        1.0,  0.0,  0.0,  0.0,
        1.0,  0.0,  0.0,  0.0,
        1.0,  0.0,  0.0,  0.0
    );
    
const t_color_transform y2rgb = FRAC_M3x4(
        0.0,  1.0,  0.0,  0.0,
        0.0,  1.0,  0.0,  0.0,
        0.0,  1.0,  0.0,  0.0
    );
    
const t_color_transform z2rgb = FRAC_M3x4(
        0.0,  0.0,  1.0,  0.0,
	  	0.0,  0.0,  1.0,  0.0,
	  	0.0,  0.0,  1.0,  0.0
	);

const t_color_transform x2y = FRAC_M3x4(
        1.0,  0.0,  0.0,  0.0,
        0.0,  0.0,  0.0,  0.5,
        0.0,  0.0,  0.0,  0.5
    );

const t_color_transform y2y = FRAC_M3x4(
        0.0,  1.0,  0.0,  0.0,
        0.0,  0.0,  0.0,  0.5,
        0.0,  0.0,  0.0,  0.5
    );

const t_color_transform z2y = FRAC_M3x4(
        0.0,  0.0,  1.0,  0.0,
        0.0,  0.0,  0.0,  0.5,
        0.0,  0.0,  0.0,  0.5
    );

// [input][output] color format transformation matrix
const void* coltf[3][6] = {
        //  RGB         YUV         YUV_ER      x       y       z
        {   xyz_one,    rgb2yuv,    0,          x2rgb,  y2rgb,  z2rgb   },  // RGB in
        {   yuv2rgb,    xyz_one,    0,          x2rgb,  y2rgb,  z2rgb   },  // YUV in
        {   0,          0,          xyz_one,    x2rgb,  y2rgb,  z2rgb   },  // YUV_ER in
    };
