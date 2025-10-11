#include "./state.h"

struct zwlr_layer_surface_v1_listener layer_surface_listener;

void create_surface(client_state* state) {
	state->surface = wl_compositor_create_surface(state->compositor);
	state->layer_surface = zwlr_layer_shell_v1_get_layer_surface(
		state->layer_shell, state->surface, NULL,
		ZWLR_LAYER_SHELL_V1_LAYER_TOP, "swenu");
	zwlr_layer_surface_v1_add_listener(state->layer_surface, &layer_surface_listener, state);
	zwlr_layer_surface_v1_set_size(state->layer_surface, 600, 400);
	zwlr_layer_surface_v1_set_anchor(state->layer_surface, ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP |
								  ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM |
								  ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT |
								  ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT);
	zwlr_layer_surface_v1_set_exclusive_zone(state->layer_surface, -1);
	zwlr_layer_surface_v1_set_margin(state->layer_surface, 0, 0, 0, 0);
	zwlr_layer_surface_v1_set_keyboard_interactivity(state->layer_surface, ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_EXCLUSIVE);
	wl_surface_commit(state->surface);
}

// listeners

void configure(void *data, struct zwlr_layer_surface_v1 *zwlr_layer_surface_v1, uint32_t serial, uint32_t width, uint32_t height) {
	client_state* state = data;
	state->width = width;
	state->height = height;
	zwlr_layer_surface_v1_ack_configure(zwlr_layer_surface_v1, serial);
}

void closed(void *data, struct zwlr_layer_surface_v1 *zwlr_layer_surface_v1) {
	client_state* state = data;
	state->running = false;
}

struct zwlr_layer_surface_v1_listener layer_surface_listener = {
	.configure = configure,
	.closed = closed,
};
