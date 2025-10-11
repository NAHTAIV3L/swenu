#include "state.h"
#include <cursor-shape-v1.h>

extern struct wl_pointer_listener pointer_listener;
extern struct wl_keyboard_listener keyboard_listener;

void capabilities(void *data, struct wl_seat *wl_seat, uint32_t capabilities) {
    client_state* state = data;

    if (capabilities & WL_SEAT_CAPABILITY_KEYBOARD && !state->keyboard) {
        state->keyboard = wl_seat_get_keyboard(wl_seat);
        wl_keyboard_add_listener(state->keyboard, &keyboard_listener, state);
    }
    if (capabilities & WL_SEAT_CAPABILITY_POINTER && !state->pointer) {
        state->pointer = wl_seat_get_pointer(wl_seat);
        state->cursor_shape_device = wp_cursor_shape_manager_v1_get_pointer(
            state->cursor_shape_manager, state->pointer);
		wl_pointer_add_listener(state->pointer, &pointer_listener, state);
    }

}

void name(void *data, struct wl_seat *wl_seat, const char *name) {
}

struct wl_seat_listener seat_listener = {
	.capabilities = capabilities,
	.name = name,
};
