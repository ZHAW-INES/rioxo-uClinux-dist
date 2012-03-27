/*
 * box_sys.h
 *
 *  Created on: 14.12.2010
 *      Author: alda
 */

#ifndef USB_MEDIA_H_
#define USB_MEDIA_H_

#include "rscp_include.h"

extern t_rscp_media usb_media;
extern t_rscp_usb   usb_list;
extern int          mouse_vhci_port;
extern int          keyboard_vhci_port;
extern int          storage_vhci_port;

int usb_doplay(t_rscp_media *media, t_rscp_usb* m, void* UNUSED rsp);

#endif /* USB_MEDIA_H_ */
