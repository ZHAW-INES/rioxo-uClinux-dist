# +----------------------------------------------------------------------
# | FILES for Module
# +---------------------------------------------------------------------
  FILES += hdoip_core_main.o
INCLUDE += ../include
INCLUDE += stdinc
INCLUDE += stdinc
INCLUDE += hal
INCLUDE += hal/include
INCLUDE += hal/rbf
  FILES += hal/rbf/stdrbf.o
INCLUDE += hal/eth
  FILES += hal/eth/stdeth.o
INCLUDE += hal/aud
  FILES += hal/aud/stdaud.o
INCLUDE += hal/vrp
INCLUDE += hal/vio
  FILES += hal/vio/vio_hal.o
  FILES += hal/vio/vio_osd.o
  FILES += hal/vio/vio_pll.o
INCLUDE += hal/vid
  FILES += hal/vid/stdvid.o
  FILES += hal/vid/vid_color.o
  FILES += hal/vid/vid_timing.o
  FILES += hal/vid/vid_font0.o
INCLUDE += hal/vsi
  FILES += hal/vsi/vsi_hal.o
INCLUDE += hal/vso
INCLUDE += hal/asi
  FILES += hal/asi/asi_hal.o
INCLUDE += hal/aso
  FILES += hal/aso/aso_hal.o
INCLUDE += hal/i2c
  FILES += hal/i2c/i2c_hal.o
INCLUDE += hal/adv9889
INCLUDE += hal/adv7441a
INCLUDE += hal/adv212
INCLUDE += hal/ext_pio
  FILES += hal/adv212/adv212.o
INCLUDE += hal/adv212/sea
  FILES += hal/adv212/sea/adv212_decode_2_14_1.o
  FILES += hal/adv212/sea/adv212_encode_2_13_0.o
INCLUDE += driver/i2c
  FILES += driver/i2c/i2c_drv.o
  FILES += driver/i2c/i2c_str.o
INCLUDE += driver
  FILES += driver/hoi_drv.o
  FILES += driver/hoi_msg.o
INCLUDE += driver/queue
  FILES += driver/queue/queue.o
INCLUDE += driver/tag
  FILES += driver/tag/tag_drv.o
INCLUDE += driver/adv212
  FILES += driver/adv212/adv212_drv.o
  FILES += driver/adv212/adv212_str.o
INCLUDE += driver/vrp
  FILES += driver/vrp/vrp_drv.o
INCLUDE += driver/vio
  FILES += driver/vio/vio_drv.o
  FILES += driver/vio/vio_drv_pll.o
  FILES += driver/vio/vio_drv_osd.o
  FILES += driver/vio/vio_str.o
INCLUDE += driver/vsi
  FILES += driver/vsi/vsi_drv.o
  FILES += driver/vsi/vsi_str.o
INCLUDE += driver/vso
  FILES += driver/vso/vso_drv.o
  FILES += driver/vso/vso_str.o
INCLUDE += driver/asi
  FILES += driver/asi/asi_drv.o
  FILES += driver/asi/asi_str.o
INCLUDE += driver/aso
  FILES += driver/aso/aso_drv.o
  FILES += driver/aso/aso_str.o
INCLUDE += driver/adv9889
  FILES += driver/adv9889/adv9889_drv.o
INCLUDE += driver/adv7441a
  FILES += driver/adv7441a/adv7441a_drv.o

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

