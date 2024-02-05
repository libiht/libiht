CC = gcc
TARGET = lkm-demo
GDB_DEMO = gdb-demo
LBR_API = ../../lkm/src/liblbr_api.so

all:
		$(CC) -g -Wall -o $(TARGET) $(TARGET).c $(LBR_API)
		$(CC) -g -Wall -o $(GDB_DEMO) $(GDB_DEMO).c

clean:
		rm -f $(TARGET)
		rm -f $(GDB_DEMO)