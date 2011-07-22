VER = usbip-current

CONF_OPTS = \
	--sbindir=/sbin \
	--libexecdir=/lib \
	--libdir=/lib \
	$(CONFIGURE_SHARED_ENABLE)

include $(ROOTDIR)/tools/autotools.mk

romfs:
	$(ROMFSINST) $(STAGEDIR)/sbin/usbip /bin/usbip
	$(ROMFSINST) $(STAGEDIR)/sbin/usbipd /bin/usbipd
	$(ROMFSINST) -e CONFIG_FMT_USE_FDPIC_ELF $(STAGEDIR)/lib/libusbip.so.0.0.1 /lib/libusbip.so.0
	$(ROMFSINST) -d $(STAGEDIR)/usr/share/usbip /usr/share/usbip
	$(ROMFSINST) usbip-client /bin/usbip-client
	$(ROMFSINST) usbip-server /bin/usbip-server
