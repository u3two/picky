#include <wayland-client.h>
#include <xdg-shell-protocol.h>

#include <string.h>
#include <linux/input-event-codes.h>

#include "appstate.h"
#include "args.h"
#include "color_conv.h"
#include "macros.h"
#include "draw.h"

void noop() {}

static void xdg_toplevel_configure(void *data, struct xdg_toplevel *xdg_toplevel,
				   int32_t width, int32_t height,
				   struct wl_array *states)
{
    (void) states, (void) xdg_toplevel;
    AppState *state = data;
    // as per protocol spec, width/height may be zero.
    // in that situation we should decide our own window dimensions.
    state->pending.win_width = width ? width : state->args->win_width;
    state->pending.win_height = height ? height : state->args->win_height;
    state->pending.win_dimensions = true;
}

static void xdg_toplevel_close(void *data,
		struct xdg_toplevel *xdg_toplevel)
{
    (void)xdg_toplevel;
    ((AppState*)data)->running = false;
    DEBUG("closing toplevel, bye!");
}

const struct xdg_toplevel_listener xdg_toplevel_listener = {
    .configure = xdg_toplevel_configure,
    .close = xdg_toplevel_close,
    .configure_bounds = noop,
    .wm_capabilities = noop,
};

static void xdg_surface_configure(void *data, struct xdg_surface *xdg_surface,
				  uint32_t serial)
{
    // this event marks the end of an xdg_surface configure sequence
    AppState *state = data;
    xdg_surface_ack_configure(xdg_surface, serial);
    
    if (state->pending.win_dimensions) {
	state->win_height = state->pending.win_height;
	state->win_width = state->pending.win_width;
	state->pending.win_dimensions = false;
    }

    wl_surface_commit(state->wl_surface);
    state->configured = true;
    state->redraw |= REDRAW_BACKGROUND;
}

const struct xdg_surface_listener xdg_surface_listener = {
    .configure = xdg_surface_configure
};

static void xdg_wm_base_ping(void *data, struct xdg_wm_base *xdg_wm_base,
			     uint32_t serial)
{
    (void)data;
    xdg_wm_base_pong(xdg_wm_base, serial);
}

const struct xdg_wm_base_listener xdg_wm_base_listener = {
    .ping = xdg_wm_base_ping
};

static void buffer_release(void *data, struct wl_buffer *buffer)
{
    (void)data;
    wl_buffer_destroy(buffer);
}

const struct wl_buffer_listener buffer_listener = {
    .release = buffer_release
};

const struct wl_callback_listener frame_callback_listener;

static void frame_callback_done(void *data, struct wl_callback *cb, uint32_t time)
{
    (void)time;
    AppState *state = data;

    cb = wl_surface_frame(state->wl_surface);
    wl_callback_add_listener(cb, &frame_callback_listener, state);

    if (state->redraw) {
	draw_frame(state);
	state->redraw = REDRAW_NONE;
    }

    wl_surface_commit(state->wl_surface);
}

const struct wl_callback_listener frame_callback_listener = {
    .done = frame_callback_done,
};

static void wl_pointer_axis(void *data, struct wl_pointer *pointer,
			    uint32_t time, uint32_t axis, wl_fixed_t value)
{
    (void) pointer, (void) time, (void) axis;

    AppState *state = data;

    if (state->args->lock_hue)
	return; 

    double as_double = wl_fixed_to_double(value);
    // note: I don't really know what determines value's actual value, but it seems like
    // the scroll direction is conveyed by the sign, and that's all we care for.
    if (as_double > 0) {
	state->hue += state->shift_held ? 15 : 1;
	if (state->hue > 360)
	    state->hue = 0;
    } else if (as_double < 0) {
	state->hue -= state->shift_held ? 15 : 1;
	if (state->hue < 0)
	    state->hue = 360;
    }

    state->redraw |= REDRAW_BACKGROUND;
    if (state->ctrl_held)
	state->redraw |= REDRAW_ZOOMED;
}

static void wl_pointer_motion(void *data, struct wl_pointer *pointer,
			      uint32_t time, wl_fixed_t fixed_x, wl_fixed_t fixed_y)
{
    (void) pointer, (void) time;

    AppState *state = data;
    int x = wl_fixed_to_int(fixed_x);
    int y = wl_fixed_to_int(fixed_y);

    state->mouse_x = x;
    state->mouse_y = y;

    if (state->ctrl_held)
	state->redraw |= REDRAW_ZOOMED;
}

static void wl_pointer_button(void *data, struct wl_pointer *pointer,
			      uint32_t serial, uint32_t time,
			      uint32_t button, uint32_t state)
{
    (void) pointer, (void) time, (void) serial, (void) state;
    AppState *app_state = data;

    if (button == BTN_LEFT) {
	int32_t r,g,b;
	xy_to_rgb(app_state, app_state->mouse_x, app_state->mouse_y, &r, &g, &b);

	switch (app_state->args->output_format) {
	case OUTFORMAT_HEX:
	    printf("%02X%02X%02X", r, g, b);
	    break;
	case OUTFORMAT_RGB:
	    printf("rgb(%d, %d, %d)", r, g, b);
	    break;
	case OUTFORMAT_HSV:
	    printf("hsv(%ddeg, %d%%, %d%%)",
		   app_state->hue,
		   (int)((double)app_state->mouse_x/app_state->win_width * 100),
		   (int)((1 - (double)app_state->mouse_y/app_state->win_height) * 100));
	    break;
	case OUTFORMAT_INVALID: __builtin_unreachable();
	}

	app_state->running = false;
    }
}

static const struct wl_pointer_listener pointer_listener = {
    .axis = wl_pointer_axis,
    .axis_discrete = noop,
    .axis_relative_direction = noop,
    .axis_source = noop,
    .axis_stop = noop,
    .axis_value120 = noop,
    .button = wl_pointer_button,
    .enter = noop,
    .leave = noop,
    .frame = noop,
    .motion = wl_pointer_motion,
};

static void wl_keyboard_key(void *data, struct wl_keyboard *keyboard, uint32_t serial,
			    uint32_t time, uint32_t key, enum wl_keyboard_key_state state)
{
    (void) keyboard, (void) time, (void) serial;
    AppState *app_state = data;

    if (key == KEY_C && state == WL_KEYBOARD_KEY_STATE_PRESSED) {
	// TODO: copy to clipboard
    }

    if (key == KEY_LEFTSHIFT) {
	if (state == WL_KEYBOARD_KEY_STATE_PRESSED)
	    app_state->shift_held = true;
	else if (state == WL_KEYBOARD_KEY_STATE_RELEASED)
	    app_state->shift_held = false;
    }

    if (key == KEY_LEFTCTRL) {
	if (state == WL_KEYBOARD_KEY_STATE_PRESSED)
	    app_state->ctrl_held = true;
	else if (state == WL_KEYBOARD_KEY_STATE_RELEASED) {
	    app_state->ctrl_held = false;
	    app_state->redraw |= REDRAW_BACKGROUND;
	}
    }
}

const struct wl_keyboard_listener keyboard_listener = {
    .enter = noop,
    .leave = noop,
    .key = wl_keyboard_key,
    .keymap = noop,
    .modifiers = noop,
    .repeat_info = noop
};

static void wl_seat_capabilities(void *data, struct wl_seat *seat, uint32_t caps)
{
    AppState *state = data;

    if (!(caps & WL_SEAT_CAPABILITY_KEYBOARD))
	FATAL("wayland seat missing keyboard capability");

    if (!(caps & WL_SEAT_CAPABILITY_POINTER))
	FATAL("wayland seat missing pointer capability");

    state->wl_keyboard = wl_seat_get_keyboard(seat);
    wl_keyboard_add_listener(state->wl_keyboard, &keyboard_listener, state);

    state->wl_pointer = wl_seat_get_pointer(seat);
    wl_pointer_add_listener(state->wl_pointer, &pointer_listener, state);
}

const struct wl_seat_listener seat_listener = {
    .name = noop,
    .capabilities = wl_seat_capabilities,
};

static void registry_global(void *data, struct wl_registry *registry,
			    uint32_t name, const char *interface, uint32_t version)
{
    (void)version;
    AppState *state = data;
    if (!strcmp(interface, wl_compositor_interface.name)) {
	state->wl_compositor = wl_registry_bind(registry, name, &wl_compositor_interface, 4);
    } else if (!strcmp(interface, wl_shm_interface.name)) {
	state->wl_shm = wl_registry_bind(registry, name, &wl_shm_interface, 1);
    } else if (!strcmp(interface, wl_seat_interface.name)) {
	state->wl_seat = wl_registry_bind(registry, name, &wl_seat_interface, 5);
	wl_seat_add_listener(state->wl_seat, &seat_listener, state);
    } else if (!strcmp(interface, xdg_wm_base_interface.name)) {
	state->xdg_wm_base = wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
	xdg_wm_base_add_listener(state->xdg_wm_base, &xdg_wm_base_listener, data);
    } else {
	return;
    }
    DEBUG("global bind: %s", interface);
}

const struct wl_registry_listener registry_listener = {
    .global = registry_global,
    .global_remove = noop
};
