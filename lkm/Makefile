# Kernel module compile process
KVERSION = $(shell uname -r)
MODULE = libiht_lkm
SRC_DIR = ./src/
# OBJS can add more helper files in it
OBJS = $(SRC_DIR)libiht_lkm.o

obj-m += $(MODULE).o
$(MODULE)-objs := $(OBJS)

all:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD)/lkm modules
clean:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD)/lkm clean

insert: all
	sudo insmod libiht_lkm.ko

remove:
	sudo rmmod libiht_lkm

# For test use
dump:
	objdump -M intel -d libiht_lkm.ko > libiht_lkm.asm
