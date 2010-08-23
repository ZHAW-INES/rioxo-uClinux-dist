# +----------------------------------------------------------------------
# | FILES for Module
# +---------------------------------------------------------------------
  FILES += hdoip_core_main.o
INCLUDE += ../include
INCLUDE += src/stdinc
INCLUDE += src/stdinc
INCLUDE += src/hal
INCLUDE += src/hal/include
INCLUDE += src/hal/i2c
  FILES += src/hal/i2c/i2c_hal.o
INCLUDE += src/hal/vio
  FILES += src/hal/vio/vio_hal.o
  FILES += src/hal/vio/vio_osd.o
  FILES += src/hal/vio/vio_pll.o
INCLUDE += src/hal/vid
  FILES += src/hal/vid/stdvid.o
  FILES += src/hal/vid/vid_font0.o
  FILES += src/hal/vid/vid_color.o
  FILES += src/hal/vid/vid_timing.o
INCLUDE += src/hal/adv9889
INCLUDE += src/hal/adv212
  FILES += src/hal/adv212/adv212.o
INCLUDE += src/hal/adv212/sea
  FILES += src/hal/adv212/sea/adv212_decode_2_14_1.o
  FILES += src/hal/adv212/sea/adv212_encode_2_13_0.o
INCLUDE += src/driver
INCLUDE += src/driver/adv212
  FILES += src/driver/adv212/adv212_drv.o
  FILES += src/driver/adv212/adv212_str.o
INCLUDE += src/driver/i2c
  FILES += src/driver/i2c/i2c_drv.o
  FILES += src/driver/i2c/i2c_str.o
INCLUDE += src/driver/adv9889
  FILES += src/driver/adv9889/adv9889_drv.o
INCLUDE += src/driver/vio
  FILES += src/driver/vio/vio_drv.o
  FILES += src/driver/vio/vio_drv_pll.o
  FILES += src/driver/vio/vio_str.o


# +----------------------------------------------------------------------
# | Module Object
# +----------------------------------------------------------------------
obj-m = hdoip_core.o
hdoip_core-objs = $(FILES)


# +----------------------------------------------------------------------
# | Compiler Flags
# +----------------------------------------------------------------------
EXTRA_CFLAGS += -std=gnu99 -O0


# +----------------------------------------------------------------------
# | Do not change
# +----------------------------------------------------------------------
PWD := $(shell pwd)
KERNELDIR ?= /lib/modules/$(shell uname -r)/build

EXTRA_CFLAGS += -I$(src)
EXTRA_CFLAGS += $(addprefix -I$(src)/,$(INCLUDE))

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean

