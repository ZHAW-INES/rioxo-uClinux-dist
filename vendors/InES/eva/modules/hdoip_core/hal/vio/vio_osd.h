#ifndef VIO_OSD_H_
#define VIO_OSD_H_

#include "stdhal.h"
#include "stdvid.h"
#include "vio_reg.h"
#include "vio_cfg.h"

/** 
 */
#define vio_osd_put(p, o, c)        HOI_WR8(p, OFFSET(VIO_OFF_OSD_SCREEN, o), c)
#define vio_osd_get(p, o)           HOI_RD8(p, OFFSET(VIO_OFF_OSD_SCREEN, o))
#define vio_osd_screen(p)           OFFSET(p, VIO_OFF_OSD_SCREEN)

#define vio_osd_get_width(p)		(HOI_RD16(p, VIO_OFF_OSD_SCREEN_WIDTH))
#define vio_osd_get_height(p)		(HOI_RD16(p, VIO_OFF_OSD_SCREEN_HEIGHT))

void vio_osd_init(void* p, t_osd_font* font);
void vio_osd_set_resolution(void* p, uint32_t width, uint32_t height);
void vio_osd_clear_screen(void* p);
void vio_osd_move_screen(void* p, int y);

#endif /*VIO_OSD_H_*/
