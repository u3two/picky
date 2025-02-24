CC = gcc
export PKG_CONFIG ?= pkg-config

PKGS = wayland-client
PKG_CFLAGS = $(shell $(PKG_CONFIG) --cflags $(PKGS))
PKG_LIBS = $(shell $(PKG_CONFIG) --libs $(PKGS))

CFLAGS = -Wall -Wextra $(PKG_CFLAGS) -DDEBUG_ENABLE -Iprotocols/include
DEBUG_FLAGS = -fsanitize=address,undefined -g3

LD_FLAGS = -lm $(PKG_LIBS)

SRC = $(wildcard src/*.c) 
OBJ = $(SRC:.c=.o)

PROTOCOL_OBJ = $(addprefix protocols/glue/,xdg-shell-protocol.o)

TARGET = picky

build: protocols $(OBJ)
	$(CC) $(OBJ) $(PROTOCOL_OBJ) $(LD_FLAGS) -o $(TARGET)

debug: $(OBJ)
	$(CC) $(DEBUG_FLAGS) $(OBJ) $(PROTOCOL_OBJ) $(LD_FLAGS) -o $(TARGET)

protocols:
	$(MAKE) -C protocols

clean:
	$(MAKE) -C protocols clean
	rm $(OBJ) $(TARGET)

.PHONY: build debug protocols clean
