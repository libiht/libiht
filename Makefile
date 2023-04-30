# Kernel module compile process
obj-m = libiht.o
KVERSION = $(shell uname -r)

all:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) clean

insert: all
	sudo insmod libiht.ko

remove:
	sudo rmmod libiht

# Demo program compile process
CC = gcc
CFLAGS  = -g -Wall
TARGET = demo

demo: $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c

clean_demo:
	rm -f $(TARGET)