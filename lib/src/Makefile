LIB_NAME = lbr_api.so
SRC_FILES = api.c
CFLAGS = -fPIC

all:
	gcc $(CFLAGS) -shared -o $(LIB_NAME) $(SRC_FILES) -Wl,-z,relro,-z,now

clean:
	rm -f $(LIB_NAME)