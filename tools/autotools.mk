# Requirements:
# - set VER to the package directory
# - define romfs target
# Optional:
# - set CONF_OPTS
#
# Then just add to your package Makefile:
# include $(ROOTDIR)/tools/autotools.mk

all: build-$(VER)/Makefile $(AUTOTOOLS_ALL_DEPS)
	$(MAKE) pre-build
	$(MAKE) -C build-$(VER) install DESTDIR=$(STAGEDIR)
	$(MAKE) post-build

	$(ROOTDIR)/tools/cross-fix-root

pre-build::
post-build::

include $(ROOTDIR)/tools/download.mk

ifneq ($(findstring s,$(MAKEFLAGS)),)
echo-cmd = :
else
echo-cmd = printf
endif

if_changed = \
	settings=".build-$(3)$(VER).settings" ; \
	echo $(2) $(CFLAGS) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) > $$settings.new ; \
	if ! cmp -s $$settings.new $$settings ; then \
		$(echo-cmd) "%s\n" "$(cmd_$(1))" ; \
		( $(cmd_$(1)) ) || exit $$? ; \
	fi ; \
	mv $$settings.new $$settings

cmd_configure = \
	set -e ; \
	chmod a+rx $(VER)/configure ; \
	find $(VER) -type f -print0 | xargs -0 touch -r $(VER)/configure ; \
	rm -rf build-$(3)$(VER) ; \
	mkdir build-$(3)$(VER) ; \
	cd build-$(3)$(VER) ; \
	../$(VER)/configure $(2)
build-$(VER)/Makefile: build-host-$(VER)/Makefile FORCE
	@$(call if_changed,configure,$(CONFIGURE_OPTS) $(CONF_OPTS))

build-host-$(VER)/Makefile: FORCE
ifeq ($(AUTOTOOLS_BUILD_HOST),true)
	@export AR="" CC=$(HOSTCC) CXX="" LD="" RANLIB="" \
		CPPFLAGS="" CFLAGS="-O2 -g" CXXFLAGS="-O2 -g" LDFLAGS="" CONFIG_SITE="" \
	$(call if_changed,configure,$(BUILD_CONFIGURE_OPTS) $(BUILD_CONF_OPTS),host-)
	$(MAKE) host-build
endif

clean:
	rm -rf build* .build*

.PHONY: all clean pre-build post-build romfs FORCE

#
# Helper functions
#

# $(call USE_ENABLE,LIB_FFMPEG,video) => --enable-video if LIB_FFMPEG is set
USE_ENABLE = $(shell test "$(CONFIG_$(1))" = "y" && echo "--enable-$(2)" || echo "--disable-$(2)")

# $(call USE_WITH,LIB_FFMPEG,video) => --with-video if LIB_FFMPEG is set
USE_WITH = $(shell test "$(CONFIG_$(1))" = "y" && echo "--with-$(2)" || echo "--without-$(2)")
