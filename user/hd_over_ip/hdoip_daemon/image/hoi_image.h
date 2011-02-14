/*
 * hoi_image.h
 *
 *  Created on: 20.10.2010
 *      Author: alda
 */

#ifndef HOI_IMAGE_H_
#define HOI_IMAGE_H_

#include "hoi_drv_user.h"

uint8_t* hoi_image_canvas_create(t_video_timing* timing);
void hoi_image_capture_jpeg2000( FILE* out, size_t size);
void hoi_image_capture(FILE* out);
uint8_t* hoi_image_load(FILE* in);
void hoi_image_free(uint8_t* pimage);

#endif /* HOI_IMAGE_H_ */
