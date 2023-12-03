# Makefile for lkm directory

# Path to the Linux Kernel build system
KERNEL_DIR ?= /lib/modules/$(shell uname -r)/build

# Module name
obj-m += libiht_lkm.o

# Source files
libiht_lkm-objs := src/libiht_lkm.o src/xplat_lkm.o ../commons/cpu.o ../commons/debug.o ../commons/lbr.o

# Include paths
ccflags-y := -I$(PWD)/include -I$(PWD)/../commons

# Targets for make
all:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean
	$(MAKE) -C $(PWD)/../commons clean
