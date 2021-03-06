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
    int  device_type[10];
    int  device_queue_mouse;
    int  device_queue_keyboard;
} t_usb_devices;

#define USB_TYPE_MOUSE      2
#define USB_TYPE_KEYBOARD   1
#define USB_TYPE_UNKNOWN    0

#define USB_QUEUE_ADD     0x00000004
#define USB_QUEUE_TEST    0x00000001

#define ASCII_HORIZONTAL_TAB    0x09


void usb_get_dev(char* s);
void usb_attach_device(t_usb_devices* old_values, char* ip, char* device, char* type);
void usb_detach_device(int vhci_port);
void usb_try_to_connect_device(t_usb_devices* old_values);
void usb_device_handler(t_usb_devices* old_values);
void usb_handler_init(t_usb_devices* handle);

#endif /* USB_H_ */
