# Kernel module compile process
obj-m = src/libiht_lkm.o
KVERSION = $(shell uname -r)

all:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) clean

# For test use
dump:
	objdump -M intel -d src/libiht_lkm.ko > libiht_lkm.asm