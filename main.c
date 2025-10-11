#include <EGL/egl.h>
#include <stdio.h>

#include "state.h"
#include "graphics.h"

#include "wlr-layer-shell-unstable-v1.h"

extern struct wl_registry_listener registry_listener;
extern struct zwlr_layer_surface_v1_listener layer_surface_listener;

int main() {
	client_state state = {0};

	state.running = true;

	state.display = wl_display_connect(NULL);
	if (!state.display) {
		fprintf(stderr, "Failed to Connect to wayland display\n");
		return 0;
	}

	state.registry = wl_display_get_registry(state.display);
	wl_registry_add_listener(state.registry, &registry_listener, &state);
	wl_display_roundtrip(state.display);

	state.surface = wl_compositor_create_surface(state.compositor);
	state.layer_surface = zwlr_layer_shell_v1_get_layer_surface(
		state.layer_shell, state.surface, NULL,
		ZWLR_LAYER_SHELL_V1_LAYER_TOP, "swenu");
	zwlr_layer_surface_v1_add_listener(state.layer_surface, &layer_surface_listener, &state);
	zwlr_layer_surface_v1_set_size(state.layer_surface, 600, 400);
	zwlr_layer_surface_v1_set_anchor(state.layer_surface, ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP |
								  ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM |
								  ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT |
								  ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT);
	zwlr_layer_surface_v1_set_exclusive_zone(state.layer_surface, -1);
	zwlr_layer_surface_v1_set_margin(state.layer_surface, 0, 0, 0, 0);
	zwlr_layer_surface_v1_set_keyboard_interactivity(state.layer_surface, ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_EXCLUSIVE);
	wl_surface_commit(state.surface);
	wl_display_roundtrip(state.display);

	if (!init_gl(&state)) {
		fprintf(stderr, "Failed to Initalize EGL/OpenGL\n");
		return 1;
	}

	while (state.running) {
		wl_display_dispatch_pending(state.display);
		render_frame(&state);
	}
}
