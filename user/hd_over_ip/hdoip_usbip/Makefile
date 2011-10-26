APP = hdoip_usbip
APP_OBJS = hdoip_usbip.o hdoip_usbip_monitor.o
DAEMON = hdoip_usbipd
DAEMON_OBJS = hdoip_usbipd.o hdoip_usbip_event.o

CFLAGS += -W -Wall

BINDIR = /bin

all: $(APP) $(DAEMON)

$(APP): $(APP_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(APP_OBJS) $(EXTRALIBS) $(LDLIBS)

$(DAEMON): $(DAEMON_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(DAEMON_OBJS) $(EXTRALIBS) $(LDLIBS)

romfs:
	$(ROMFSINST) $(BINDIR)/$(APP)
	$(ROMFSINST) $(BINDIR)/$(DAEMON)

clean:
	-rm -f *.o *.gdb *.elf $(APP) $(DAEMON) $(APP_OBJS) $(DAEMON_OBJS)
