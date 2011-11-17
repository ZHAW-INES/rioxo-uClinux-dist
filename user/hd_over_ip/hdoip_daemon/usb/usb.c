/*
 * usb.c
 *
 *  Created on: 28.10.2011
 *      Author: buan
 */

#include "hdoipd.h"
#include "hdoipd_fsm.h"
#include "usb.h"

#include <linux/input.h>

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

/** Bind an USB device to USBIP
 *
 *  Device is been attached to USBIP and a message is sent via RSCP that a USB device is connected to host. 
 *
 */
void bind_usb_dev(char* s, int device_type)
{
    char tmp[256];
    t_rscp_client *client;
    char uri[40];
    
    // bind device to usbip
    report(INFO "Bind USB device: %s", s);
    sprintf(tmp, "usbip bind -b %s", s);
    system(tmp);

    // send message to remote uri
    if (hdoipd_rsc(RSC_ETH_LINK)) {
        sprintf(uri, "%s", reg_get("remote-uri"));
        client = rscp_client_open(hdoipd.client, 0, uri);
        if (client) {
            rscp_client_usb(client, s, uri, device_type);
            rscp_client_close(client);
        }
    }
}

/** Attach an USB device to USBIP 
 *
 *  Is used on device part of USBIP to connect with host
 *  On host part it is used to report connected devices is device part is restarted
 *
 */
void attach_usb_dev(t_usb_devices* old_values, char* ip, char* device, char* type)
{
    t_rscp_client *client;
    char tmp[256];
    char uri[40];
    char *mode;
    int i;

    mode = reg_get("usb-mode");

    // received message is request to report connected devices
    if (!strcmp(device, "request")) {

        // if host, send responds with connected devices
        if (!strcmp(mode, "host")) {
            sprintf(uri, "%s", reg_get("remote-uri"));
            for (i=0; i<old_values->device_count; i++) {

                // bind devices new to usbip
                sprintf(tmp, "usbip unbind -b %s", (old_values->device_list+(i*10)));
                system(tmp);
                sprintf(tmp, "usbip bind -b %s", (old_values->device_list+(i*10)));
                system(tmp);

                if (hdoipd_rsc(RSC_ETH_LINK)) {
                    client = rscp_client_open(hdoipd.client, 0, uri);
                    if (client) {
                        rscp_client_usb(client, (old_values->device_list+(i*10)), uri, (old_values->device_type[i]));
                        rscp_client_close(client);
                    }
                }
            }
        }
    // received message is connected device
    } else {
        if (!strcmp(mode, "device")) {
            report(INFO "Attach USB device: %s from %s", device, ip);
            // detach if a device already was attached (only for one device)
            sprintf(tmp, "usbip detach -p 0");
            system(tmp);
            // attach device
            sprintf(tmp, "usbip attach -h %s -b %s", ip, device);
            system(tmp);


            if (!strcmp(type, "mouse")) {
                report(INFO "connect mouse");
                // start with delay (wait until device is attached to USBIP)
                old_values->device_queue_mouse = USB_QUEUE_ADD;
            } else {
                if (!strcmp(type, "keyboard")) {
                    report(INFO "connect keyboard");
                    // start with delay (wait until device is attached to USBIP)
                    old_values->device_queue_keyboard = USB_QUEUE_ADD;
                } else {
                    report(INFO "connect unknown device");
                }
            }
            
        }
    }
}

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
        system("/bin/usbip-device");
    }

    if (!strcmp(mode, "off")) {
        report(INFO "Load no USB driver");
    }
}

/** Connect USB on Ethernet connect event
 *
 *  Connect devices to USBIP if an ethernet connect event occurs
 *
 */
void usb_ethernet_connect(t_usb_devices* old_values)
{
    t_rscp_client *client;
    char tmp[256];
    char uri[40];
    char *msg = "request";
    char *mode;
    int i;

    mode = reg_get("usb-mode");

    // if device, check if remote is host and if there are devices connected
    if (!strcmp(mode, "device")) {
        // send message to remote uri
        if (hdoipd_rsc(RSC_ETH_LINK)) {
            sprintf(uri, "%s", reg_get("remote-uri"));
            client = rscp_client_open(hdoipd.client, 0, uri);
            if (client) {
                rscp_client_usb(client, msg, uri, USB_TYPE_UNKNOWN);
                rscp_client_close(client);
            }
        }
    }

    if (!strcmp(mode, "host")) {
        sprintf(uri, "%s", reg_get("remote-uri"));
        for (i=0; i<old_values->device_count; i++) {

            // bind devices new to usbip
            sprintf(tmp, "usbip unbind -b %s", (old_values->device_list+(i*10)));
            system(tmp);
            sprintf(tmp, "usbip bind -b %s", (old_values->device_list+(i*10)));
            system(tmp);

            if (hdoipd_rsc(RSC_ETH_LINK)) {
                client = rscp_client_open(hdoipd.client, 0, uri);
                if (client) {
                    rscp_client_usb(client, (old_values->device_list+(i*10)), uri, (old_values->device_type[i]));
                    rscp_client_close(client);
                }
            }
        }
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
    if (fd1 = fopen(path, "r")) {
        memcpy(node_param, node, strlen(node)+1);
        device_count ++;
        //search for connected device on hub
        for (i=8; i>=0; i--) {
            sprintf(hub, ".%i",i);
            sprintf(path, "/sys/bus/usb/devices/%s%s", node, hub);

            if (fd2 = fopen(path, "r")) {
                sprintf(tmp, "%s%s", node, hub);
                memcpy(node_param+(device_count*10), tmp, strlen(tmp)+1);
                device_count ++;
                // another hub
                for (j=8; j>=0; j--) {
                    sprintf(hubhub, ".%i",j);
                    sprintf(path, "/sys/bus/usb/devices/%s%s%s", node, hub, hubhub);

                    if (fd3 = fopen(path, "r")) {
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

    if ((fd = fopen(tmp, "r"))) {
        if (getline(&line, &len, fd) != -1) {
            old_values->device_type[device_count] = USB_TYPE_UNKNOWN;
            if (!strncmp(line, "02", 2)) {
                old_values->device_type[device_count] = USB_TYPE_MOUSE;
                report("mouse detected");
            }
            if (!strncmp(line, "01", 2)) {
                old_values->device_type[device_count] = USB_TYPE_KEYBOARD;
                report("keyboard detected");
            }
        }
        fclose(fd);
    }
}

/** usb handler
 *
 * - load modules with a short delay
 * - check each 3 sec if a new device is attached to host
 *
 */
void usb_device_handler(t_usb_devices* old_values)
{
    char tmp[100];
    int device_count;
    int i, j;
    bool new_device[9] = {true, true, true, true ,true ,true , true, true, true, true};
    char zerostring[9] = "         ";
    char *s;
    bool active = false;

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

                // if some changes occured
                if (old_values->device_count != device_count) {

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
                                case USB_TYPE_MOUSE:        report(INFO "new USB mouse detected on %s",tmp+(i*10));
                                                            bind_usb_dev(tmp+(i*10), USB_TYPE_MOUSE);
                                                            break;
                                case USB_TYPE_KEYBOARD:     report(INFO "new USB keyboard detected on %s",tmp+(i*10));
                                                            bind_usb_dev(tmp+(i*10), USB_TYPE_KEYBOARD);
                                                            break;
                                default:                    report(INFO "new USB device detected on %s",tmp+(i*10));
                            }
                        }
                    }

                    // update device list
                    memcpy(old_values->device_list, tmp, 100);
                    for (i=device_count; i<10; i++) {
                        memcpy(old_values->device_list+(i*10), zerostring, 10);
                    }

                    old_values->device_count = device_count;
                }
            }

            if ((old_values->device_queue_mouse & USB_QUEUE_TEST) == USB_QUEUE_TEST) {
                //connect as mouse
                system("hdoip_usbipd mouse /dev/input/event0 /dev/hidg1 &");
            }
            if ((old_values->device_queue_keyboard & USB_QUEUE_TEST) == USB_QUEUE_TEST) {
                //connect as keyboard
                system("hdoip_usbipd keyboard /dev/input/event0 /dev/hidg0 &");
            }
            old_values->device_queue_mouse = old_values->device_queue_mouse << 1;
            old_values->device_queue_keyboard = old_values->device_queue_keyboard << 1;

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
