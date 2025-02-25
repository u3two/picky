#include "draw.h"

#include <unistd.h>

#include "macros.h"
#include "appstate.h"
#include "listeners.h"
#include "config.h"
#include "shm.h"
#include "color_conv.h"

static void draw_background(AppState *state, int32_t stride, uint8_t *shm_pool_data)
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

static void draw_zoomed(AppState *state, int32_t stride, uint8_t *shm_pool_data)
{
    static struct {
	int32_t prev_mouse_x, prev_mouse_y;
    } zoomed_state = { 0 };

    // first, redraw the part of the screen that was covered by the zoomed view
    // in the previous frame
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

    // now we can draw the new zoomed area
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

    if (state->redraw & REDRAW_BACKGROUND)
	draw_background(state, stride, shm_pool_data);

    if (state->redraw & REDRAW_ZOOMED)
	draw_zoomed(state, stride, shm_pool_data);
}
