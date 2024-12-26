CC = gcc
CFLAGS = -shared -fPIC
LDFLAGS = -ldl

# Source files
MAIN_SRC = main.c
FREEBLOCKS_SRC = freeblocks.c
MCKUSICK_SRC = mckusick.c
HEADER = library.h

# Output binaries
FREEBLOCKS_LIB = libfreeblocks.so
MCKUSICK_LIB = libmckusick.so
MAIN_BIN = main

.PHONY: all clean

all: $(FREEBLOCKS_LIB) $(MCKUSICK_LIB) $(MAIN_BIN)

$(FREEBLOCKS_LIB): $(FREEBLOCKS_SRC) $(HEADER)
	$(CC) $(CFLAGS) -o $@ $(FREEBLOCKS_SRC)

$(MCKUSICK_LIB): $(MCKUSICK_SRC) $(HEADER)
	$(CC) $(CFLAGS) -o $@ $(MCKUSICK_SRC)

$(MAIN_BIN): $(MAIN_SRC) $(HEADER)
	$(CC) -o $@ $(MAIN_SRC) $(LDFLAGS)

clean:
	rm -f $(FREEBLOCKS_LIB) $(MCKUSICK_LIB) $(MAIN_BIN)
