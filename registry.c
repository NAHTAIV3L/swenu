#include <stdio.h>
#include <string.h>

#include "./state.h"

extern struct wl_seat_listener seat_listener;

void global(void *data, struct wl_registry *wl_registry, uint32_t name, const char *interface, uint32_t version) {
	client_state* state = data;

	printf("iface: %s v%d\n", interface, version);

	if (!strcmp(interface, wl_compositor_interface.name)) {
		state->compositor = wl_registry_bind(wl_registry, name, &wl_compositor_interface, 1);
	}
	else if (!strcmp(interface, wl_seat_interface.name)) {
		state->seat = wl_registry_bind(wl_registry, name, &wl_seat_interface, 1);
		wl_seat_add_listener(state->seat, &seat_listener, state);
	}
	else if (!strcmp(interface, zwlr_layer_shell_v1_interface.name)) {
		state->layer_shell = wl_registry_bind(wl_registry, name, &zwlr_layer_shell_v1_interface, version);
	}
	else if (!strcmp(interface, wp_cursor_shape_manager_v1_interface.name)) {
		state->cursor_shape_manager = wl_registry_bind(
			wl_registry, name, &wp_cursor_shape_manager_v1_interface, version);
	}
	else {
	}

}

void global_remove(void *data, struct wl_registry *wl_registry, uint32_t name) {

}

struct wl_registry_listener registry_listener = {
	.global = global,
	.global_remove = global_remove,
};
