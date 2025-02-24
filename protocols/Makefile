CC = gcc
CFLAGS = -Wall -Wextra
PKG_CONFIG = pkg-config

WAYLAND_SCANNER = $(shell $(PKG_CONFIG) --variable=wayland_scanner wayland-scanner)
WAYLAND_PROTOCOLS_DIR = $(shell $(PKG_CONFIG) wayland-protocols --variable=pkgdatadir)

build: xdg-shell layer-shell

debug: build

XDG_SHELL_PROTOCOL = $(WAYLAND_PROTOCOLS_DIR)/stable/xdg-shell/xdg-shell.xml

xdg-shell: $(addprefix glue/xdg-shell-protocol,.c .o) include/xdg-shell-protocol.h
glue/xdg-shell-protocol.c:
	$(WAYLAND_SCANNER) private-code $(XDG_SHELL_PROTOCOL) $@
include/xdg-shell-protocol.h:
	$(WAYLAND_SCANNER) client-header $(XDG_SHELL_PROTOCOL) $@

clean:
	rm include/* glue/*

.PHONY: build xdg-shell layer-shell clean
