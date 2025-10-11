#include <stdio.h>

#include "state.h"
#include "wayland.h"
#include "graphics.h"

int main() {
	client_state state = {0};
	state.running = true;

	state.display = wl_display_connect(NULL);
	if (!state.display) {
		fprintf(stderr, "Failed to Connect to wayland display\n");
		return 0;
	}

	// set up globals
	setup_registry_and_globals(&state);
	wl_display_roundtrip(state.display);

	// create non global objects
	create_surface(&state);

	// roundtrip to recieve the rest of the events
	wl_display_roundtrip(state.display);

	// set up graphics
	if (!init_gl(&state)) {
		fprintf(stderr, "Failed to Initalize EGL/OpenGL\n");
		return 1;
	}

	// event loop
	while (state.running) {
		wl_display_dispatch_pending(state.display);
		render_frame(&state);
	}
}
