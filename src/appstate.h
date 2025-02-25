#ifndef PICKY_APPSTATE_H
#define PICKY_APPSTATE_H

#include <wayland-client.h>
#include <stdbool.h>

// NOTE: this represents a bit field, the next value should be 4!
typedef enum {
    REDRAW_NONE = 0,
    REDRAW_BACKGROUND = 1,
    REDRAW_ZOOMED = 2,
} Redraw;

// NOTE: when adding a new field to this struct, always assign a default value
// in the constructor (appstate.c)!
typedef struct {
    bool running, configured;
    int32_t win_width, win_height;
    Redraw redraw;

    // colors
    int32_t hue;

    // input
    bool shift_held, ctrl_held;
    int32_t mouse_x, mouse_y;

    // wayland, being an atomic protocol, often sends sequences of events ending
    // with a final apply/commit event.
    struct {
	bool win_dimensions;
	int32_t win_width, win_height;
    } pending;

    // wayland::globals
    struct wl_display *wl_display;
    struct wl_registry *wl_registry;
    struct wl_compositor *wl_compositor;
    struct wl_shm *wl_shm;
    struct wl_seat *wl_seat;
    // wayland::objects
    struct wl_surface *wl_surface;
    struct wl_keyboard *wl_keyboard;
    struct wl_pointer *wl_pointer;
    // wayland::xdg
    struct xdg_wm_base *xdg_wm_base;
    struct xdg_surface *xdg_surface;
    struct xdg_toplevel *xdg_toplevel;
} AppState;

void appstate_init(AppState *state);

#endif
