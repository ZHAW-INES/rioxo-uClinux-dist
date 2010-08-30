#
#	Makefile -- Build instructions for Altera NiosII with initramfs
#

KERNELZ = $(IMAGEDIR)/zImage
ROOT_TEMPLATE_DIR = rootfs

ROMFS_DIRS = bin sbin dev etc home lib mnt proc usr var sys tmp root \
	etc/init.d \
	home/httpd home/httpd/cgi-bin \
	mnt/config \
	usr/bin usr/sbin usr/share \
	var/lock var/log var/run var/tmp

EXTRAMODULES = hdoip_ether

all:
	dirs=$(DIRS) ; \
	for i in $$dirs ; do  make -C $$i || exit $? ; done

clean: vendor_clean
vendor_clean: vendor_modules_clean
	-dirs=$(DIRS) ; \
	for i in $$dirs; do [ ! -d $$i ] || make -C $$i clean; done

vendor_hwselect:
	[ -d $(ROMFSDIR) ] || mkdir -p $(ROMFSDIR)
	$(MAKEARCH_KERNEL) hwselect

romfs: vendor_modules
	for i in $(ROMFS_DIRS); do \
		[ -d $(ROMFSDIR)/$$i ] || mkdir -p $(ROMFSDIR)/$$i; \
	done
	$(ROMFSINST) -s /bin/init /init
	cd $(ROOT_TEMPLATE_DIR); $(ROMFSINST) -v . /
	echo "$(VERSIONSTR) -- " `date` > $(ROMFSDIR)/etc/version

image:
	[ -d $(IMAGEDIR) ] || mkdir -p $(IMAGEDIR)
	-chmod -R +x $(ROMFSDIR)/bin
	-rm $(LINUXDIR)/usr/initramfs_data.cpio.gz
	LDFLAGS="" $(MAKEARCH_KERNEL) $(LINUXTARGET)
	cp $(LINUXDIR)/arch/nios2/boot/zImage $(KERNELZ)

vendor_modules:
	for mod in $(EXTRAMODULES) ; do \
	  LDFLAGS="" $(MAKEARCH_KERNEL) SUBDIRS=`pwd`/modules/$$mod SRCROOT=`pwd`/modules/$$mod modules && \
	  LDFLAGS="" $(MAKEARCH_KERNEL) INSTALL_MOD_PATH=$(ROMFSDIR) DEPMOD=true SUBDIRS=`pwd`/modules/$$mod SRCROOT=`pwd`/modules/$$mod modules_install || exit 1;\
	done

vendor_modules_clean:
	for mod in $(EXTRAMODULES) ; do \
	  LDFLAGS="" $(MAKEARCH_KERNEL) SUBDIRS=`pwd`/modules/$$mod SRCROOT=`pwd`/modules/$$mod clean ;\
	done
