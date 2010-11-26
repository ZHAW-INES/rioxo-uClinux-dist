#ifndef STDVID_H_
#define STDVID_H_

#include <linux/types.h>

/** Video Format Information (packed)
 */
typedef struct {
    uint32_t    id;                 //!< resolution id
    
    uint32_t    width;              //!< visible screen size
    uint32_t    height;
    uint32_t    pfreq;              //!< pixel frequency in 1 [Hz]
    
    uint32_t    hfront;             //!< horizontal front porch
    uint32_t    hpulse;             //!< horizontal sync pulse
    uint32_t    hback;              //!< horizontal back porch
    uint32_t    hpolarity;          //!< 1=active high, 0=active low
    
    uint32_t    vfront;             //!< vertical front porch
    uint32_t    vpulse;             //!< vertical sync pulse
    uint32_t    vback;              //!< vertical back porch
    uint32_t    vpolarity;          //!< 1=active high, 0=active low
    
    uint32_t    interlace;          //!< 1=interlace, 0=progressive
} __attribute__((__packed__)) t_video_timing;


/** 4x3 Matrix for Color Transformation
 * 
 * The Matrix is split into a 3x3 Matrix M and a 3 Vector O:
 * [ M11 M12 M13 O1 ]
 * [ M21 M22 M23 O2 ]
 * [ M31 M22 M33 O3 ]
 * 
 * Pixel are transformed using the following formula:
 *  x: input pixel (3 component vector)
 *  y: output pixel (3 component vector)
 * y = x * M + O  
 */
typedef int16_t t_color_transform[3][4];

#define to_frac16(x) 	( (int16_t)((x) * 8192.0) )


/** Bitmap font
 */
typedef struct {
    uint32_t    width;
    uint32_t    height;
    uint32_t    start;
    uint32_t    length;
    uint16_t*   bitmap;
} t_osd_font;


/** Video format
 * 
 *  - color space
 *  - sampling mode
 */
typedef uint32_t t_video_format;

// Color Space
#define CS_RGB      0x00        // RGB
#define CS_YUV      0x01        // YUV limited range
#define CS_YUV_ER   0x02        // YUV extended range
#define CS_X2RGB    0x03        // Test RGB-GRAY from X
#define CS_Y2RGB    0x04        // Test RGB-GRAY from Y
#define CS_Z2RGB    0x05        // Test RGB-GRAY from Z

// Sampling Mode
#define SM_444      0x00
#define SM_422      0x01

/** Input/Output mode [Color Space:8 | Sampling Mode:8]
 */
#define vid_cs(c)               (vio_format(c, 0))
#define vid_sm(s)               (vio_format(0, s))
#define vio_format(c, s)        (((c&0xff)<<8)|((s&0xff)<<0)) 
#define vio_format_cs(x)        (((x)>>8)&0xff)
#define vio_format_sm(x)        (((x)>>0)&0xff)


/** defines 
 */
#define VID_POL_HIGH                (1)
#define VID_POL_LOW                 (0)
#define VID_INTERLACED              (1)
#define VID_PROGRESSIVE             (0)


/** Functions
 */
uint32_t vid_fps(t_video_timing* p_vt);
uint32_t vid_pixel_per_frame(t_video_timing* p_vt);
uint32_t vid_clock_per_frame(t_video_timing* p_vt);
uint32_t vid_duration_in_us(t_video_timing* p_vt);
uint32_t vid_duration_in_ns(t_video_timing* p_vt);

#endif /*STDVID_H_*/
