LIB_NAME = lib_api.so
SRC_FILES = api.c
CFLAGS = -fPIC

all:
	gcc $(CFLAGS) -shared -o $(LIB_NAME) $(SRC_FILES)

clean:
	rm -f $(LIB_NAME)