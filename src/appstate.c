#include "appstate.h"

void appstate_init(AppState *state, Args *args)
{
    *state = (AppState){
	.running = true,
	.configured = false,
	.win_width = args->win_width,
	.win_height = args->win_height,
	.redraw = REDRAW_NONE,

	// runtime args
	.args = args,

	// colors
	.hue = args->initial_hue,

	// input
	.shift_held = false,
	.ctrl_held = false,
	.mouse_x = 0,
	.mouse_y = 0,

	.pending = { 0 },

	// wayland::globals
	.wl_display = NULL,
	.wl_registry = NULL,
	.wl_compositor = NULL,
	.wl_shm = NULL,
	.wl_seat = NULL,
	// wayland::objects
	.wl_surface = NULL,
	.wl_keyboard = NULL,
	.wl_pointer = NULL,
	// wayland::xdg
	.xdg_wm_base = NULL,
	.xdg_surface = NULL,
	.xdg_toplevel = NULL,
    };
}
