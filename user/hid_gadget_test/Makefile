P = hid_gadget_test
O = hid_gadget_test.o

CFLAGS += -W -Wall

all: $(P)

$(P): $(O)
	@echo LD $@
	@$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	@echo CC $@
	@$(CC) $(CFLAGS) -c $< -o $@

romfs:
	$(ROMFSINST) /bin/$(P)

clean:
	@echo CLEAN
	@rm -f *.o *.gdb *.elf $(P)
