#ifndef _HDOIP_USBIP_EVENT_H_
#define _HDOIP_USBIP_EVENT_H_

extern int hdoip_usbip_event_print_info(int efd);
extern int hdoip_usbip_event_check_supported(int efd, const char *type);
extern int hdoip_usbip_event_loop(int efd, int gfd, const char *type);

#endif /* _HDOIP_USBIP_EVENT_H_ */
