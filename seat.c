#include "state.h"

#include "wayland.h"

struct wl_seat_listener seat_listener;

void setup_seat(client_state* state, uint32_t name) {
	state->seat = wl_registry_bind(state->registry, name, &wl_seat_interface, 1);
	wl_seat_add_listener(state->seat, &seat_listener, state);
}

void capabilities(void *data, struct wl_seat *wl_seat, uint32_t capabilities) {
    client_state* state = data;

    if (capabilities & WL_SEAT_CAPABILITY_KEYBOARD && !state->keyboard) {
		setup_keyboard(state);
    }
    if (capabilities & WL_SEAT_CAPABILITY_POINTER && !state->pointer) {
		setup_pointer(state);
    }
}

void name(void *data, struct wl_seat *wl_seat, const char *name) {
}

struct wl_seat_listener seat_listener = {
	.capabilities = capabilities,
	.name = name,
};
