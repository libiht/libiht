# Kernel module compile process
obj-m = src/libiht_lkm.o
KVERSION = $(shell uname -r)

all:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) clean

insert: all
	sudo insmod src/libiht_lkm.ko

remove:
	sudo rmmod libiht_lkm

# For test use
dump:
	objdump -M intel -d src/libiht_lkm.ko > libiht_lkm.asm