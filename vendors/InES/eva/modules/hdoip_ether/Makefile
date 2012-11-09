EXTRA_CFLAGS += -O0

FILES 	+= hdoip_ether_main.o

INCLUDE += ../hdoip_core/stdinc
INCLUDE += ../hdoip_core/hal/include
INCLUDE	+= ../hdoip_core/hal/ext_pio

INCLUDE += ../hdoip_core/driver/queue
FILES	+= ../hdoip_core/driver/queue/queue.o
	
INCLUDE += ../hdoip_core/hal/rbf
FILES 	+= ../hdoip_core/hal/rbf/stdrbf.o

INCLUDE += ../hdoip_core/hal/vid
FILES   += ../hdoip_core/hal/vid/stdvid.o

INCLUDE += ../hdoip_core/hal/eto
FILES 	+= ../hdoip_core/hal/eto/eto_hal.o
INCLUDE += ../hdoip_core/driver/eto
FILES	+= ../hdoip_core/driver/eto/eto_drv.o

INCLUDE += ../hdoip_core/hal/eti
INCLUDE += ../hdoip_core/driver
INCLUDE += ../hdoip_core/driver/eti
FILES	+= ../hdoip_core/driver/eti/eti_drv.o

INCLUDE += ../hdoip_core/hal/esc
FILES 	+= ../hdoip_core/hal/esc/esc_hal.o
INCLUDE += ../hdoip_core/driver/esc
FILES	+= ../hdoip_core/driver/esc/esc_drv.o


obj-m += hdoip_ether.o

hdoip_ether-objs = $(FILES)
EXTRA_CFLAGS += $(addprefix -I$(src)/,$(INCLUDE))

clean:
	rm -f *.o *.ko *.mod.c Module.symvers modules.order .*.cmd $(FILES)
