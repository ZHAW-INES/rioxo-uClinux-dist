/*
 * hdoip_lib.h
 *
 *  Created on: 05.11.2010
 *      Author: alda
 */

#ifndef HDOIP_LIB_H_
#define HDOIP_LIB_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "stdvid.h"

int hdoip_lib_color_format(char* str);
int hdoip_lib_res(char* s, uint32_t* width, uint32_t* height, uint32_t* fps);

#endif /* HDOIP_LIB_H_ */
