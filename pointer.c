#include "state.h"

struct wl_pointer_listener pointer_listener;

void setup_pointer(client_state* state) {
	state->pointer = wl_seat_get_pointer(state->seat);
	state->cursor_shape_device = wp_cursor_shape_manager_v1_get_pointer(
		state->cursor_shape_manager, state->pointer);
	wl_pointer_add_listener(state->pointer, &pointer_listener, state);
	printf("setup pointer\n");
}

// listeners

void enter(void *data, struct wl_pointer *pointer, uint32_t serial,
		   struct wl_surface *surface, wl_fixed_t fixed_surface_x, wl_fixed_t fixed_surface_y) {
	client_state* state = data;
	wp_cursor_shape_device_v1_set_shape(
		state->cursor_shape_device, serial,
		WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_DEFAULT);
}

void leave(void *data, struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface) {
}

void motion(void *data, struct wl_pointer *pointer, uint32_t time,
			wl_fixed_t fixed_surface_x, wl_fixed_t fixed_surface_y) {
}

void button(void *data, struct wl_pointer *pointer, uint32_t serial,
			uint32_t time, uint32_t button, uint32_t button_state) {
}

void axis(void *data, struct wl_pointer *pointer, uint32_t time,
		  uint32_t axis, wl_fixed_t fixed_value) {
}

struct wl_pointer_listener pointer_listener = {
	.enter = enter,
	.leave = leave,
	.motion = motion,
	.button = button,
	.axis = axis
};

