VER = usbip-current

FLTFLAGS += -s 8192

CONF_OPTS = \
	--sbindir=/sbin \
	--libexecdir=/lib \
	--libdir=/lib \
	$(CONFIGURE_SHARED_ENABLE)

all: build-$(VER)/Makefile
	$(MAKE) -C build-$(VER)

build-$(VER)/Makefile:
	find $(VER) -type f -print0 | xargs -0 touch -r $(VER)/configure
	set -e ; \
	rm -rf build-$(VER) ; \
	mkdir build-$(VER) ; \
	cd build-$(VER) ; \
	../$(VER)/configure $(CONFIGURE_OPTS) $(CONF_OPTS)

clean:
	rm -rf build*

romfs:
	$(ROMFSINST) build-$(VER)/src/usbip /bin/usbip
	$(ROMFSINST) build-$(VER)/src/usbipd /bin/usbipd
	$(ROMFSINST) usbip-device /bin/usbip-device
	$(ROMFSINST) usbip-host /bin/usbip-host
