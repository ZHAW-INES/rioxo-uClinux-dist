#include "vio_hal.h"


inline uint16_t reverse16(uint16_t x)
{
    x = ((x >> 8) & 0x00ff) | ((x << 8) & 0xff00);
    x = ((x >> 4) & 0x0f0f) | ((x << 4) & 0xf0f0);
    x = ((x >> 2) & 0x3333) | ((x << 2) & 0xcccc);
    x = ((x >> 1) & 0x5555) | ((x << 1) & 0xaaaa);
    return x;
}

/** Initialize the OSD
 * 
 * @param p pointer to i/o base
 * @param font the font used
 */
void vio_osd_init(void* p, t_osd_font* font)
{
    uint16_t *tab;

    HOI_WR16(p, VIO_OFF_OSD_CHAR_WIDTH, font->width - 1);
    HOI_WR16(p, VIO_OFF_OSD_CHAR_HEIGHT, font->height - 1);
    
    memset(OFFSET(p, VIO_OFF_OSD_SCREEN), ' ', 0x4000);
    memset(OFFSET(p, VIO_OFF_OSD_CHAR), 0, 0x2000);
    
    tab = &((uint16_t*)OFFSET(p, VIO_OFF_OSD_CHAR))[font->start*16];
    
    // copy <num> 32 Byte tabel entrys (16x16 Bit)
    memcpy(tab, font->bitmap, font->length * 32);

    HOI_WR32(p, VIO_OFF_OSD_CONFIG, VIO_OSD_CFG_ACTIVE);
}

/** Initialize the OSD resolution
 * 
 * @param p pointer to i/o base
 * @param width screen width in pixel
 * @param height screen height in pixel
 */
void vio_osd_set_resolution(void* p, uint32_t width, uint32_t height)
{
    uint16_t w, h, cw, ch;
    
    cw = HOI_RD16(p, VIO_OFF_OSD_CHAR_WIDTH) + 1;
    ch = HOI_RD16(p, VIO_OFF_OSD_CHAR_HEIGHT) + 1;
    
    w = (width + cw - 1) / cw;
    h = (height + ch - 1) / ch;

    HOI_WR16(p, VIO_OFF_OSD_SCREEN_WIDTH, w);
    HOI_WR16(p, VIO_OFF_OSD_SCREEN_HEIGHT, h);
}

/** Clear OSD
 * 
 * @param p pointer to i/o base
 */
void vio_osd_clear_screen(void* p)
{
    memset(OFFSET(p, VIO_OFF_OSD_SCREEN), ' ', 0x4000);
}


/** Scroll OSD
 * 
 * moves screen y lines up. top lines will get lost.
 * new lines will be empty (zero)
 * 
 * @param p_overlay the t_vco_overlay struct
 * @param y number of lines to move
 */
void vio_osd_move_screen(void* p, int y)
{
    int width = HOI_RD16(p, VIO_OFF_OSD_SCREEN_WIDTH);
    int height = HOI_RD16(p, VIO_OFF_OSD_SCREEN_HEIGHT);
    
    // line count to move
    int lc = (height - y);
    uint8_t* p_src = OFFSET(p, VIO_OFF_OSD_SCREEN + y * width);
    uint8_t* p_dst = OFFSET(p, VIO_OFF_OSD_SCREEN);
    uint8_t* p_end = OFFSET(p, VIO_OFF_OSD_SCREEN + lc * width);
    
    memmove(p_dst, p_src, lc * width);

    memset(p_end, ' ', y * width);
}


