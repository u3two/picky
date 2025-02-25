#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include <wayland-client.h>

#include "args.h"
#include "macros.h"
#include "appstate.h"
#include "listeners.h"
#include "config.h"
#include "draw.h"

void wayland_init(AppState *state)
{
    state->wl_display = wl_display_connect(NULL);
    if (!state->wl_display)
	FATAL("wayland: failed to connect to display");
    state->wl_registry = wl_display_get_registry(state->wl_display);
    wl_registry_add_listener(state->wl_registry, &registry_listener, state);

    // fetch the globals
    wl_display_roundtrip(state->wl_display);

    if (!state->wl_shm || !state->wl_compositor || !state->wl_seat || !state->xdg_wm_base)
	FATAL("compositor does not support the necessary protocols");

    state->wl_surface = wl_compositor_create_surface(state->wl_compositor);

    state->xdg_surface = xdg_wm_base_get_xdg_surface(state->xdg_wm_base, state->wl_surface);
    xdg_surface_add_listener(state->xdg_surface, &xdg_surface_listener, state);

    state->xdg_toplevel = xdg_surface_get_toplevel(state->xdg_surface);
    xdg_toplevel_add_listener(state->xdg_toplevel, &xdg_toplevel_listener, state);

    xdg_toplevel_set_title(state->xdg_toplevel, PICKY_WINDOW_TITLE);
    xdg_toplevel_set_app_id(state->xdg_toplevel, PICKY_APP_ID);

    wl_surface_commit(state->wl_surface);
    // wait for the first configure event
    while (wl_display_dispatch(state->wl_display) && !state->configured);

    struct wl_callback *callback = wl_surface_frame(state->wl_surface);
    wl_callback_add_listener(callback, &frame_callback_listener, state);
}

void wayland_destroy(AppState *state)
{
    xdg_toplevel_destroy(state->xdg_toplevel);
    xdg_surface_destroy(state->xdg_surface);
    wl_surface_destroy(state->wl_surface);
}

void event_loop(AppState *state)
{
    // initial draw + commit
    draw_frame(state);
    wl_surface_commit(state->wl_surface);

    while (wl_display_dispatch(state->wl_display) != -1 && state->running);
}

int main(int argc, char **argv)
{
    Args args;
    args_parse(&args, argc, argv);

    AppState state;
    appstate_init(&state, &args);

    wayland_init(&state);
    event_loop(&state);
    wayland_destroy(&state);
}
