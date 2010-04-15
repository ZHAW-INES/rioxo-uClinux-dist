BINDIR = /bin

LDLIBS += -lortp -lpthread

all: rtpsend rtprecv

rtpsend: rtpsend.o
	$(CC) $(LDFLAGS) -o $@ $^ $(EXTRALIBS) $(LDLIBS)

rtprecv: rtprecv.o
	$(CC) $(LDFLAGS) -o $@ $^ $(EXTRALIBS) $(LDLIBS)

romfs:
	$(ROMFSINST) $(BINDIR)/rtpsend
	$(ROMFSINST) $(BINDIR)/rtprecv

clean:
	-rm -f *.o *.gdb *.elf rtpsend rtprecv
