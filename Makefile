.PHONY: all clean insert remove mesg

all:
	@$(MAKE) -C lib
	@$(MAKE) -C lkm

clean:
	@$(MAKE) -C lib clean
	@$(MAKE) -C lkm clean

insert:
	@$(MAKE) -C lkm insert

remove:
	@$(MAKE) -C lkm remove

mesg:
	sudo dmesg -wH