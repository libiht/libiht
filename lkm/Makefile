# Makefile for lkm directory

# Path to the Linux Kernel build system
KERNEL_DIR ?= /lib/modules/$(shell uname -r)/build
# Path to the kernel module
SRC_DIR ?= ./src
# Path to the include directory
INC_DIR ?= ./include
# Path to the common directory
COMMON_DIR ?= ../commons

# Module name
MODULE_NAME := libiht_lkm
obj-m += $(MODULE_NAME).o

# Source files
libiht_lkm-objs := $(SRC_DIR)/libiht_lkm.o $(SRC_DIR)/xplat_lkm.o $(COMMON_DIR)/cpu.o $(COMMON_DIR)/debug.o $(COMMON_DIR)/lbr.o

# Include paths
ccflags-y := -I$(INC_DIR) -I$(COMMON_DIR)

# Targets for make
all:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean
	$(MAKE) -C $(COMMON_DIR) clean
