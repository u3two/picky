#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include <wayland-client.h>

#include "macros.h"
#include "appstate.h"
#include "listeners.h"
#include "config.h"
#include "shm.h"
#include "color_conv.h"

void xy_to_rgb(AppState *state, int x, int y, int *red, int *green, int *blue)
{
    double saturation = (double)x/state->win_width;
    double value = 1 - (double)y/state->win_height;
    hsl_to_rgb(state->hue, saturation, value, red, green, blue);
}

void draw_background(AppState *state, int32_t stride, uint8_t *shm_pool_data)
{
    for (int x = 0; x < state->win_width; x++) {
	for(int y = 0; y < state->win_height; y++) {
	    int idx = stride * y + x * 4;

	    int red, green, blue;
	    xy_to_rgb(state, x, y, &red, &green, &blue);
	    // reversed because little endian
	    shm_pool_data[idx] = blue;
	    shm_pool_data[idx + 1] = green;
	    shm_pool_data[idx + 2] = red;
	    shm_pool_data[idx + 3] = 255;
	}
    }

    wl_surface_damage_buffer(state->wl_surface, 0, 0, state->win_width, state->win_height);
}

void draw_zoomed(AppState *state, int32_t stride, uint8_t *shm_pool_data)
{
    static struct {
	int32_t prev_mouse_x, prev_mouse_y;
    } zoomed_state = { 0 };

    // first, redraw the part of the screen that was covered by the zoomed view
    // in the previous frame.
    int x_start = MAX(0, zoomed_state.prev_mouse_x - PICKY_REGION_SIZE);
    int x_end = MIN(state->win_width, zoomed_state.prev_mouse_x + PICKY_REGION_SIZE);

    int y_start = MAX(0, zoomed_state.prev_mouse_y - PICKY_REGION_SIZE);
    int y_end = MIN(state->win_height, zoomed_state.prev_mouse_y + PICKY_REGION_SIZE);

    int red, green, blue;

    for (int x = x_start; x < x_end; x++) {
	for (int y = y_start; y < y_end; y++) {
	    int idx = stride * y + x * 4;
		
	    xy_to_rgb(state, x, y, &red, &green, &blue);
	    // reversed because little endian
	    shm_pool_data[idx] = blue;
	    shm_pool_data[idx + 1] = green;
	    shm_pool_data[idx + 2] = red;
	    shm_pool_data[idx + 3] = 255;
	}
    }

    wl_surface_damage_buffer(state->wl_surface, x_start, y_start, x_end - x_start, y_end - y_start);

    // now we can draw the new zoomed area.
    x_start = MAX(0, state->mouse_x - PICKY_REGION_SIZE);
    x_end = MIN(state->win_width, state->mouse_x + PICKY_REGION_SIZE);

    y_start = MAX(0, state->mouse_y - PICKY_REGION_SIZE);
    y_end = MIN(state->win_height, state->mouse_y + PICKY_REGION_SIZE);

    xy_to_rgb(state, state->mouse_x, state->mouse_y, &red, &green, &blue);

    for (int x = x_start; x < x_end; x++) {
	for (int y = y_start; y < y_end; y++) {
	    int idx = stride * y + x * 4;

	    if (abs(y - y_start) < PICKY_BORDER_SIZE || abs(x - x_start) < PICKY_BORDER_SIZE ||
		abs(y - y_end) < PICKY_BORDER_SIZE   || abs(x - x_end) < PICKY_BORDER_SIZE)
	    {
		shm_pool_data[idx] = 255;
		shm_pool_data[idx + 1] = 255;
		shm_pool_data[idx + 2] = 255;
		shm_pool_data[idx + 3] = 255;
	    } else {
		shm_pool_data[idx] = blue;
		shm_pool_data[idx + 1] = green;
		shm_pool_data[idx + 2] = red;
		shm_pool_data[idx + 3] = 255;
	    }
	}
    }

    wl_surface_damage_buffer(state->wl_surface, x_start, y_start, x_end - x_start, y_end - y_start);

    zoomed_state.prev_mouse_x = state->mouse_x;
    zoomed_state.prev_mouse_y = state->mouse_y;
}

void draw_frame(AppState *state)
{
    const int stride = state->win_width * 4; // 4 bytes per pixel
    const int shm_pool_size = state->win_height * stride;

    int fd = allocate_shm_file(shm_pool_size);
    if (fd < 0)
	FATAL("failed to allocate shm file: %m");

    uint8_t *shm_pool_data = mmap(NULL, shm_pool_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shm_pool_data == MAP_FAILED)
	FATAL("failed to mmap: %m");

    // signal the compositor we have created the pool
    struct wl_shm_pool *pool = wl_shm_create_pool(state->wl_shm, fd, shm_pool_size);
    // signal the compositor that we are designating a buffer area in that pool
    struct wl_buffer *buffer = wl_shm_pool_create_buffer(pool, 0,
							 state->win_width, state->win_height,
							 stride, WL_SHM_FORMAT_ARGB8888);
    wl_surface_attach(state->wl_surface, buffer, 0, 0);
    wl_buffer_add_listener(buffer, &buffer_listener, state);
    wl_shm_pool_destroy(pool);
    close(fd);

    if (state->ctrl_held)
	draw_zoomed(state, stride, shm_pool_data);
    else
	draw_background(state, stride, shm_pool_data);
}

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
}

void wayland_destroy(AppState *state)
{
    xdg_toplevel_destroy(state->xdg_toplevel);
    xdg_surface_destroy(state->xdg_surface);
    wl_surface_destroy(state->wl_surface);
}

void event_loop(AppState *state)
{
    do {
	if (state->redraw) {
	    draw_frame(state);
	    state->redraw = false;
	}
	wl_surface_commit(state->wl_surface);
    } while (wl_display_dispatch(state->wl_display) != -1 && state->running);
}

int main(int argc, char **argv)
{
    (void) argc, (void) argv;

    AppState state;
    appstate_init(&state);

    // TODO: parse args

    wayland_init(&state);

    event_loop(&state);

    wayland_destroy(&state);
}
