#include "./state.h"
#include "config.h"

#include "font.h"

struct zwlr_layer_surface_v1_listener layer_surface_listener;

void create_surface(client_state* state) {
	// figure out anchor
	uint32_t anchor = ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP |
		ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT |
		ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT;
	if (state->center) {
		anchor |= ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM;
	}

	// figure out size
	uint32_t desired_width = 0;
	uint32_t desired_height = state->line_height;
	if (state->center) {
		desired_width = MAX(state->required_width, config.min_width);
		if (!state->lines) {
			desired_width += state->prompt ? atlas_get_strwidth(state, state->prompt) + state->horizontal_spacing : 0.0f;
		}
	}
	if (state->lines) {
		desired_height = (state->lines + 1) * state->line_height;
	}

	// create surface
	state->surface = wl_compositor_create_surface(state->compositor);
	state->layer_surface = zwlr_layer_shell_v1_get_layer_surface(
		state->layer_shell, state->surface, NULL,
		ZWLR_LAYER_SHELL_V1_LAYER_TOP, "swenu");
	zwlr_layer_surface_v1_add_listener(state->layer_surface, &layer_surface_listener, state);
	zwlr_layer_surface_v1_set_size(state->layer_surface, desired_width, desired_height);
	zwlr_layer_surface_v1_set_anchor(state->layer_surface, anchor);
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
	state->exit_code = EXIT_FAILURE;
}

struct zwlr_layer_surface_v1_listener layer_surface_listener = {
	.configure = configure,
	.closed = closed,
};
