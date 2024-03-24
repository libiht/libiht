CC = gcc
TARGET = lkm-demo
GDB_DEMO = gdb-demo
API = ../../lkm/src/lib_api.so

all:
		$(CC) -g -Wall -o $(TARGET) $(TARGET).c $(API)
		$(CC) -g -Wall -o $(GDB_DEMO) $(GDB_DEMO).c

clean:
		rm -f $(TARGET)
		rm -f $(GDB_DEMO)