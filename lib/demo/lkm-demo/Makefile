CC = gcc
TARGET = lkm-demo
# LBR_API = -L../../lkm/src -llbr_api
LBR_API = ../../lkm/src/liblbr_api.so

all:
		$(CC) -g -Wall -o $(TARGET) $(TARGET).c $(LBR_API)

clean:
		rm -f $(TARGET)