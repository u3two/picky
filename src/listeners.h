#ifndef PICKY_LISTENERS_H
#define PICKY_LISTENERS_H

#include "wayland-client-protocol.h"
#include <wayland-client.h>
#include <xdg-shell-protocol.h>

extern const struct wl_registry_listener registry_listener;
extern const struct wl_buffer_listener buffer_listener;
extern const struct wl_callback_listener frame_callback_listener;

extern const struct xdg_surface_listener xdg_surface_listener;
extern const struct xdg_toplevel_listener xdg_toplevel_listener;

#endif /* PICKY_LISTENERS_H */
