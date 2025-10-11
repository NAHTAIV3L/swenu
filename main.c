#include <EGL/egl.h>
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

	setup_registry(&state);
	setup_surface(&state);

	if (!init_gl(&state)) {
		fprintf(stderr, "Failed to Initalize EGL/OpenGL\n");
		return 1;
	}

	while (state.running) {
		wl_display_dispatch_pending(state.display);
		render_frame(&state);
	}
}
