/*
 * usb.c
 *
 *  Created on: 28.10.2011
 *      Author: buan
 */

#include <stdio.h>
#include "hdoipd.h"
#include "hdoipd_fsm.h"
#include "usb.h"
#include "usb_media.h"

#include <linux/input.h>

/** Load USB driver
 *
 *  Load used drivers due to function of host or device.
 *
 */
void usb_load_driver(char* mode)
{
    if (!strcmp(mode, "host")) {
        report(INFO "Load USB driver for host");
        system("/sbin/modprobe isp1763");
        system("/bin/usbip-host");
    }

    if (!strcmp(mode, "device")) {
        report(INFO "Load USB driver for device");
        system("/sbin/modprobe isp1763_udc");
        system("/sbin/modprobe g_hid");
        system("/bin/usbip-device");
    }

    if (!strcmp(mode, "off")) {
        report(INFO "Load no USB driver");
    }
}

/** Bind an USB device to USBIP
 *
 *  Device is been attached to USBIP and a message is sent via RSCP that an USB device is connected to host. 
 *
 */
void bind_usb_dev(char* s)
{
    char tmp[256];
    
    // bind device to usbip
    sprintf(tmp, "usbip bind -b %s", s);
    report(INFO "%s", tmp);
    system(tmp);
}

/** Get vendor and device name
 *
 *  This function reads out vendor and product ip and convert this to a string via usb.isd.
 *
 */
void usb_get_dev(char* s)
{
    FILE *fd;
    char vendor[50];
    char product[50];
    char tmp[256];
    char *line = 0;
    char *tmp_line = 0;
    size_t len = 0;
    char vendor_id[5];
    char product_id[5];
    bool vendor_found = false;
    int length;
    char *mode;

    mode = reg_get("usb-mode");

    // device can be read only on host
    if (!strcmp(mode, "host")) {

        strcpy(vendor, "unknown vendor");
        strcpy(product, "unknown product");

        // read Vendor ID
        sprintf(tmp, "/sys/bus/usb/devices/%s/idVendor", s);
        if (!(fd = fopen(tmp, "r"))) {
            strcpy(vendor, "no device");
            strcpy(product, "no device");
            memcpy(s,      vendor,  strlen(vendor));
            memcpy((s+50), product, strlen(product));
            return;
        }
        if (getline(&line, &len, fd) != -1) {
            memcpy(vendor_id, line, 4);
            vendor_id[4] = '\0';
        }
        fclose(fd);

        // read Product ID
        sprintf(tmp, "/sys/bus/usb/devices/%s/idProduct", s);
        if (!(fd = fopen(tmp, "r"))) {
            strcpy(vendor, "no device");
            strcpy(product, "no device");
            memcpy(s,      vendor,  strlen(vendor));
            memcpy((s+50), product, strlen(product));
            return;
        }
        if (getline(&line, &len, fd) != -1) {
            memcpy(product_id, line, 4);
            product_id[4] = '\0';
        }
        fclose(fd);

        // Compare with ID-list
        sprintf(tmp, "/usr/share/hwdata/usb.ids");
        if (!(fd = fopen(tmp, "r"))) {
            report(ERROR "Could not open %s", tmp);
        }

        while (getline(&line, &len, fd) != -1) {
            if (vendor_found){
                if (line[0] != ASCII_HORIZONTAL_TAB) {
                    //no product string found
                    break;
                }

                tmp_line = strstr(line, product_id);

                // product string found
                if (tmp_line != NULL) {
                    length = strlen(line);
                    if (length > 57) break;
                    memcpy(product, line+7, length-8);
                    product[length-8] = '\0';
                    break;
                }
            } else {
                if (!strncmp(line, vendor_id, 4)) {
                    length = strlen(line);
                    if (length > 56) break;
                    memcpy(vendor, line+6, length-7);
                    vendor[length-7] = '\0';
                    vendor_found = true;
                }
            }
        }
        fclose(fd);

        memcpy(s,      vendor,  strlen(vendor));
        memcpy((s+50), product, strlen(product));
    }
}

/** Check if device on "node" is a mouse or a keyboard
 *
 */
void mouse_or_keyboard(t_usb_devices* old_values, char* node, int device_count)
{
    FILE *fd;
    char tmp[50];
    char *line = 0;
    char path[100];
    size_t len = 0;

    sprintf(path, "/sys/bus/usb/devices/%s", node);
    sprintf(tmp, "%s%s%s", path, ":1.0", "/bInterfaceProtocol");

    old_values->device_type[device_count] = USB_TYPE_UNKNOWN;

    if ((fd = fopen(tmp, "r"))) {
        if (getline(&line, &len, fd) != -1) {
            if (!strncmp(line, "02", 2)) {
                old_values->device_type[device_count] = USB_TYPE_MOUSE;
            }
            if (!strncmp(line, "01", 2)) {
                old_values->device_type[device_count] = USB_TYPE_KEYBOARD;
            }
        }
        fclose(fd);
    }
}

/** Search connected USB Devices
 *
 *  Supported are max 2 hubs and 10 devices
 *
 */
int detect_device(char* node_param)
{
    int i = 0;
    int j = 0;
    int device_count = 0;
    char hub[3];
    char hubhub[3];
    char node[6];
    char path[6+3+22-2];
    char tmp[6+3+3-2];
    FILE *fd1, *fd2, *fd3;

    // USB Host Port is always 1-1.2
    sprintf(node, "1-1.2");

    //search for connected device
    sprintf(path, "/sys/bus/usb/devices/%s", node);
    fd1 = fopen(path, "r");
    if (fd1) {
        memcpy(node_param, node, strlen(node)+1);
        device_count ++;
        //search for connected device on hub
        for (i=8; i>=0; i--) {
            sprintf(hub, ".%i",i);
            sprintf(path, "/sys/bus/usb/devices/%s%s", node, hub);

            fd2 = fopen(path, "r");
            if (fd2) {
                sprintf(tmp, "%s%s", node, hub);
                memcpy(node_param+(device_count*10), tmp, strlen(tmp)+1);
                device_count ++;
                // another hub
                for (j=8; j>=0; j--) {
                    sprintf(hubhub, ".%i",j);
                    sprintf(path, "/sys/bus/usb/devices/%s%s%s", node, hub, hubhub);

                    fd3 = fopen(path, "r");
                    if (fd3) {
                        sprintf(tmp, "%s%s%s", node, hub, hubhub);
                        memcpy(node_param+(device_count*10), tmp, strlen(tmp)+1);
                        device_count ++;
                        fclose(fd3);
                    }
                    // support max. 10 devices
                    if (device_count == 10) {
                        return 10;
                    }
                }
                fclose(fd2);
            }
        }
        fclose(fd1);
    }

    return device_count;
}

/** Send an RSCP Message that a new usb device has connected
 *
 */
int usb_connect_device(t_rscp_usb* usb)
{
    int ret;
    t_rscp_media* media;
    t_rscp_usb* usb_clone;

    media = &usb_media;

    usb_clone = (t_rscp_usb*) malloc(sizeof(t_rscp_usb));
    memcpy(usb_clone, usb, sizeof(t_rscp_usb));

    // start connection for usb if it doesnt exists
    if (!media->creator) {
        hdoipd_start_vrb_cb(media, 0);
    }

    ret = RSCP_SUCCESS;

    // do usb setup
    if (media->state == RSCP_INIT) {
        if (media->dosetup) {
            ret = media->dosetup(media, 0, 0);
        }
    }

    // do usb play
    if (ret == RSCP_SUCCESS) {
        if (media->doplay) {
            ret = media->doplay(media, usb_clone, 0);
        }
    }

    free(usb_clone);

    return ret;
}

/** Send an RSCP Message if there are connected devices
 *
 */
void usb_try_to_connect_device(t_usb_devices* old_values)
{
    t_rscp_media* media;
    t_rscp_usb* usb_clone;
    int i, ret;

    if (old_values->device_count > 0) {

        media = &usb_media;
        usb_clone = (t_rscp_usb*) malloc(sizeof(t_rscp_usb));

        sprintf(usb_clone->mouse,      "");
        sprintf(usb_clone->keyboard,   "");
        sprintf(usb_clone->storage,    "");

        // bind available devices to USBIP
        for (i=0; i<old_values->device_count; i++) {
            switch (old_values->device_type[i]) {
                case USB_TYPE_MOUSE:        report(INFO "connect USB mouse (%s)",old_values->device_list+(i*10));
                                            bind_usb_dev(old_values->device_list+(i*10));
                                            sprintf(usb_clone->mouse, "%s", old_values->device_list+(i*10));
                                            break;
                case USB_TYPE_KEYBOARD:     report(INFO "connect USB keyboard (%s)",old_values->device_list+(i*10));
                                            bind_usb_dev(old_values->device_list+(i*10));
                                            sprintf(usb_clone->keyboard, "%s", old_values->device_list+(i*10));
                                            break;
                default:                    report(INFO "dont connect unknown USB device (%s)",old_values->device_list+(i*10));
            }
        }

        // connect only if usb device is mouse, keyboard or storage
        if (strcmp(usb_clone->mouse, "") || strcmp(usb_clone->keyboard, "") || strcmp(usb_clone->storage, "")) {
            // start connection for usb if it doesnt exists
            if (!media->creator) {
                hdoipd_start_vrb_cb(media, 0);
            }

            ret = RSCP_SUCCESS;

            // do usb setup
            if (media->state == RSCP_INIT) {
                if (media->dosetup) {
                    ret = media->dosetup(media, 0, 0);
                }
            }

            // do usb play
            if (ret == RSCP_SUCCESS) {
                if (media->doplay) {
                    ret = media->doplay(media, usb_clone, 0);
                }
            }
        }

        free(usb_clone);
    }
}


/** Send an RSCP Message that an usb device is lost
 *
 */
int usb_teardown_device(void)
{
    int ret = RSCP_NULL_POINTER;
    t_rscp_media* media;

    media = &usb_media;

    if (media->doteardown) {
        ret = media->doteardown(media, 0, 0);
    }

    return ret;
}

/** Attach an usb device to usbip
 *
 */
void usb_attach_device(t_usb_devices* old_values, char* ip, char* device, char* type)
{
    char tmp[256];
    char *mode;
    mode = reg_get("usb-mode");

    if (!strcmp(mode, "device")) {
        // attach device
        sprintf(tmp, "usbip attach -h %s -b %s", ip, device);
        report(INFO "%s", tmp);
        system(tmp);
        if (!strcmp(type, "mouse")) {
            report(INFO "connect mouse");
            // start with delay (wait until device is attached to USBIP)
            old_values->device_queue_mouse = USB_QUEUE_TEST*2;
        } else {
            if (!strcmp(type, "keyboard")) {
                report(INFO "connect keyboard");
                // start with delay (wait until device is attached to USBIP)
                old_values->device_queue_keyboard = USB_QUEUE_TEST;
            } else {
                report(INFO "connect unknown device");
            }
        }        
    }
}


/** Detach an usb device from usbip
 *
 */
void usb_detach_device(int vhci_port)
{
    char tmp[256];
    sprintf(tmp, "usbip detach -p %d", vhci_port);
    report(INFO "%s", tmp);
    system(tmp);
}

/** usb handler
 *
 * - load modules with a short delay
 * - check each 3 sec if a new device is attached to host
 *
 */
void usb_device_handler(t_usb_devices* old_values)
{
    char tmp[255];
    int device_count;
    int i, j;
    bool new_device[] = {true, true, true, true ,true ,true , true, true, true, true};
    bool old_device[] = {true, true, true, true ,true ,true , true, true, true, true};
    char zerostring[] = "         ";
    char *s;
    bool active = false;
    t_rscp_usb usb;

    // load drivers and start usbip on first call
    // if usbip-device is started to early, there are some problems with vhci driver
    if (old_values->init_done == false) {
        old_values->init_done = true;
        s = reg_get("usb-mode");
        usb_load_driver(s);
    }

    // call this handler all 3 sec (3 * OSD Timer duration)
    if (old_values->service_count > 1 ) {
        old_values->service_count = 0;
        active = true;
    } else {
        old_values->service_count ++;
    }

    // after boot, wait some time until usbip is ready
    if (old_values->boot_count < 7) {
        old_values->boot_count ++;
    } else {
        if (active) {

            if (reg_test("usb-mode", "host")) {

                // check connected devices
                device_count = detect_device(tmp);

                // if new device
                if (old_values->device_count < device_count) {

                    sprintf(usb.mouse,      "");
                    sprintf(usb.keyboard,   "");
                    sprintf(usb.storage,    "");

                    // check if devices are already in list
                    for (i=0; i<device_count; i++) {
                        for (j=0; j<old_values->device_count; j++) {
                            // if device is already in list
                            if (strncmp((tmp+(i*10)), (old_values->device_list+(j*10)), 10) == 0) {
                                new_device[i] = false;
                            }
                        }
                    }

                    // bind new devices to USBIP
                    for (i=0; i<device_count; i++) {
                        if (new_device[i] == true) {
                            mouse_or_keyboard(old_values, tmp+(i*10), i);
                            switch (old_values->device_type[i]) {
                                case USB_TYPE_MOUSE:        report(INFO "new USB mouse (%s) detected",tmp+(i*10));
                                                            bind_usb_dev(tmp+(i*10));
                                                            sprintf(usb.mouse, "%s", tmp+(i*10));
                                                            break;
                                case USB_TYPE_KEYBOARD:     report(INFO "new USB keyboard (%s) detected",tmp+(i*10));
                                                            bind_usb_dev(tmp+(i*10));
                                                            sprintf(usb.keyboard, "%s", tmp+(i*10));
                                                            break;
                                default:                    report(INFO "new USB device (%s) detected",tmp+(i*10));
                            }
                        }
                    }

                    // update device list
                    memcpy(old_values->device_list, tmp, 100);
                    for (i=device_count; i<10; i++) {
                        memcpy(old_values->device_list+(i*10), zerostring, 10);
                    }

                    // connect only if usb device is mouse, keyboard or storage
                    if (strcmp(usb.mouse, "") || strcmp(usb.keyboard, "") || strcmp(usb.storage, "")) {
                        usb_connect_device(&usb);
                    }

                    old_values->device_count = device_count;
                }

                // if device is lost
                if (old_values->device_count > device_count) {

                    sprintf(usb.mouse,      "");
                    sprintf(usb.keyboard,   "");
                    sprintf(usb.storage,    "");

                   // check if devices is missing in list
                    for (i=0; i<device_count; i++) {
                        for (j=0; j<old_values->device_count; j++) {
                            // if device is already in list
                            if (strncmp((tmp+(i*10)), (old_values->device_list+(j*10)), 10) == 0) {
                                old_device[j] = false;
                            }
                        }
                    }

                    for (j=0; j<old_values->device_count; j++) {
                        if (old_device[j]) {
                            switch (old_values->device_type[j]) {
                                case USB_TYPE_MOUSE:        report(INFO "USB mouse (%s) is lost", old_values->device_list+(j*10));
                                                            sprintf(usb.mouse, "%s", old_values->device_list+(j*10));
                                                            break;
                                case USB_TYPE_KEYBOARD:     report(INFO "USB keyboard (%s) is lost", old_values->device_list+(j*10));
                                                            sprintf(usb.keyboard, "%s", old_values->device_list+(j*10));
                                                            break;
                                default:                    report(INFO "unknown USB device (%s) is lost", old_values->device_list+(j*10));
                            }
                        }
                    }

                    // update device list
                    memcpy(old_values->device_list, tmp, 100);
                    for (i=device_count; i<10; i++) {
                        memcpy(old_values->device_list+(i*10), zerostring, 10);
                    }

                    old_values->device_count = device_count;

                    // check if no device is available
                    for (i=0; i<old_values->device_count; i++) {
                        mouse_or_keyboard(old_values, tmp+(i*10), i);
                        if (((old_values->device_type[i]) == USB_TYPE_MOUSE) || ((old_values->device_type[i]) == USB_TYPE_KEYBOARD)) {
                            return;
                        }
                    }

                    usb_teardown_device();
                }  
            }

            if (reg_test("usb-mode", "device")) {
                if ((old_values->device_queue_mouse & USB_QUEUE_TEST) == USB_QUEUE_TEST) {
                    // mouse is attached to event0 or event1, but we dont know to which event, so we try both
                    sprintf(tmp, "hdoip_usbipd xxxxxxxxx /dev/input/event0 xxxxxxxxxx &");
                    report(INFO "%s", tmp);
                    system(tmp);

                    sprintf(tmp, "hdoip_usbipd xxxxxxxxx /dev/input/event1 xxxxxxxxx &");
                    report(INFO "%s", tmp);
                    system(tmp);
                }
                if ((old_values->device_queue_keyboard & USB_QUEUE_TEST) == USB_QUEUE_TEST) {
                    // keyboard is attached to event0 or event1, but we dont know to which event, so we try both
                    sprintf(tmp, "hdoip_usbipd xxxxxxxxx /dev/input/event0 xxxxxxxxxx &");
                    report(INFO "%s", tmp);
                    system(tmp);

                    sprintf(tmp, "hdoip_usbipd xxxxxxxxx /dev/input/event1 xxxxxxxxx &");
                    report(INFO "%s", tmp);
                    system(tmp);
                }
                old_values->device_queue_mouse = old_values->device_queue_mouse >> 1;
                old_values->device_queue_keyboard = old_values->device_queue_keyboard >> 1;
            }   
        }
    }
}


/** Initialisation routine
 *
 */
void usb_handler_init(t_usb_devices* handle)
{
    handle->init_done = false;
    handle->service_count = 0;
    handle->device_count = 0;
    handle->boot_count = 0;
    handle->device_queue_mouse = 0;
    handle->device_queue_keyboard = 0;
}
