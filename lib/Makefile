# Demo program compile process
CC = gcc
CFLAGS  = -g -static -Wall
TARGET = demo

demo: $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c

clean_demo:
	rm -f $(TARGET)

dump_demo:
	objdump -M intel -d demo > demo.asm