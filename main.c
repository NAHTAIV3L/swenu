#include <stdio.h>
#include <poll.h>
#include <unistd.h>

#include "state.h"
#include "wayland.h"
#include "graphics.h"
#include "input_field.h"

void poll_events(client_state* state);

int main() {
	client_state state = {0};
	state.running = true;

	state.display = wl_display_connect(NULL);
	if (!state.display) {
		fprintf(stderr, "Failed to Connect to wayland display\n");
		return 0;
	}

	// bind globals and roundtrip
	setup_registry_and_globals(&state);
	wl_display_roundtrip(state.display);

	// set up other objects and roundtrip
	setup_seat(&state);
	create_surface(&state);
	wl_display_roundtrip(state.display);

	// set up graphics
	if (!init_gl(&state)) {
		fprintf(stderr, "Failed to Initalize EGL/OpenGL\n");
		return 1;
	}

	// event loop
	while (state.running) {
		poll_events(&state);
		render_frame(&state);
	}
}

void poll_events(client_state* state) {
    enum { DISPLAY_FD, KEYREPEAT_FD };
    struct pollfd fds[] = {
        [DISPLAY_FD] = { wl_display_get_fd(state->display), POLLIN },
        [KEYREPEAT_FD] = { state->key_repeat_timer_fd, POLLIN },
    };

    bool event = false;
    while (!event) {
        while (wl_display_prepare_read(state->display) != 0) {
            if (wl_display_dispatch_pending(state->display) > 0) {
                return;
            }
        }
        if (poll(fds, sizeof(fds) / sizeof(fds[0]), -1) == -1) {
            wl_display_cancel_read(state->display);
            return;
        }
        if (fds[DISPLAY_FD].revents & POLLIN) {
            wl_display_read_events(state->display);
            if (wl_display_dispatch_pending(state->display) > 0)
                event = true;
        }
        else {
            wl_display_cancel_read(state->display);
        }
        if (fds[KEYREPEAT_FD].revents & POLLIN) {
            uint64_t repeats;
            if (read(state->key_repeat_timer_fd, &repeats, sizeof(repeats)) == 8) {
                for (uint64_t i = 0; i < repeats; i++) {
					type_key(state, state->repeat_key);
                }
                event = true;
            }
        }
    }
}
