#include <stdio.h>
#include <poll.h>
#include <unistd.h>

#include "array.h"
#include "state.h"
#include "wayland.h"
#include "graphics.h"
#include "input_field.h"
#include "font.h"
#include "array.h"
#include "args.h"

void poll_events(client_state* state);

void read_stdin(client_state* state) {
	ssize_t len = 0;
	size_t a;
	char* line = NULL;
	while((len = getline(&line, &a, stdin)) != -1) {
		if (line[len - 1] == '\n') {
			line[len - 1] = '\0';
			len -= 1;
		}
		if (len != 0) {
			array_add(state->items, (item_t){0});
			array_last(state->items).text = line;
		}
		line = NULL;
	}
	free(line);
}

int main(int argc, char* argv[]) {
	client_state state = {0};
	state.running = true;
	state.input_buffer = array_new(char, 0);

	parse_args(&state, argc, argv);
	read_stdin(&state);

	// find font
	char* font = get_font("Monospace");
	if (!freetype_init(&state, font)) {
		fprintf(stderr, "Failed to Initalize FreeType\n");
	}
	free(font);

	// create font atlas
	atlas_init(&state);

	atlas_calc_item_widths(&state);

	// start wayland
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

	// create font altas textures
	atlas_create_texture(&state);

	init_text_buffer(&state, &state.input_buffer_text, "", 0);

	// event loop
	while (state.running) {
		poll_events(&state);
		render_frame(&state);
	}

	if (state.items) {
		array_free(state.items);
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
