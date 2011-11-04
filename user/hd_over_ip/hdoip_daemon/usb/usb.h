/*
 * usb.h
 *
 *  Created on: 28.10.2011
 *      Author: buan
 */

#ifndef USB_H_
#define USB_H_

typedef struct {
    bool init_done;
    int  service_count;
    int  boot_count;
    int  device_count;
    char device_list[100];      /* 10 devices x 10 characters (9 + '\0') */
} t_usb_devices;


void usb_get_dev(char* s);
void bind_usb_dev(char* s);
void attach_usb_dev(t_usb_devices* old_values, char* ip, char* device);
void usb_load_driver(char* mode);
int detect_device(char* node_param);
void usb_device_handler(t_usb_devices* old_values);
void usb_handler_init(t_usb_devices* handle);
void usb_ethernet_connect(t_usb_devices* old_values);

#endif /* USB_H_ */
