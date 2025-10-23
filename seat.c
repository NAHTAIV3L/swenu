#include "state.h"

#include "wayland.h"

struct wl_seat_listener seat_listener;

void setup_seat(client_state* state) {
	wl_seat_add_listener(state->seat, &seat_listener, state);
}

// listeners

void capabilities(void *data, struct wl_seat *wl_seat, uint32_t capabilities) {
    client_state* state = data;
	setup_data_device(state);
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
