.PHONY: all clean insert remove mesg

all:
	@$(MAKE) -C lib
	@$(MAKE) -C lkm

clean:
	@$(MAKE) -C lib clean
	@$(MAKE) -C lkm clean

insert: all
	sudo insmod lkm/src/libiht.ko

remove:
	sudo rmmod libiht

mesg:
	sudo dmesg -wH