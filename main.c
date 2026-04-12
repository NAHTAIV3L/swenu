#include <locale.h>
#include <langinfo.h>
#include <stdio.h>
#include <poll.h>
#include <unistd.h>

#include "config.h"
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
		if (len) {
			if (line[len - 1] == '\n') {
				line[len - 1] = '\0';
				len -= 1;
			}
		}
		if (len != 0) {
			array_add(state->items, (item_t){0});
			array_last(state->items).text = line;
		} else {
			free(line);
		}
		line = NULL;
	}
	free(line);
}

int main(int argc, char* argv[]) {
	client_state state = {0};
	state.running = true;
	state.items = array_new(item_t, 0);
	state.input_buffer = array_new(char, 0);
	state.filtered_items = array_new(item_display_t, 0);
	state.page_indices = array_new(size_t, 0);
	state.strstr2 = strstr;

	// check locale
	setlocale(LC_ALL, "");
	const char *encoding = nl_langinfo(CODESET);
	if (strcmp(encoding, "UTF-8") != 0) {
		fprintf(stderr, "Cannot continue because encoding (%s), is not UTF-8.", encoding);
		return EXIT_FAILURE;
	}

	// read input
	init_conf();
	read_stdin(&state);
	parse_args(&state, argc, argv);

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
		return EXIT_FAILURE;
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
		return EXIT_FAILURE;
	}

	// create font altas textures
	atlas_create_texture(&state);

	// create text buffers
	if (state.prompt && *state.prompt) {
		init_text_buffer(&state, &state.prompt_text_buffer, state.prompt, strlen(state.prompt));
	}
	init_text_buffer(&state, &state.input_buffer_grafix, state.input_buffer, array_size(state.input_buffer));
	array_for_all(item_t, item, state.items) {
		init_text_buffer(&state, &item->text_buffer, item->text, strlen(item->text));
	}
	refilter_items(&state);

	// event loop
	while (state.running) {
		poll_events(&state);
		render_frame(&state);
		wl_display_dispatch(state.display);
	}

	// cleanup (of course)
	if (state.items) {
		array_for_all(item_t, item, state.items) {
			free(item->text);
		}
		array_free(state.items);
	}
	array_free(state.input_buffer);
	array_free(state.filtered_items);
	array_free(state.page_indices);
	if (state.clipboard) free(state.clipboard);
	FT_Done_Face(state.ft_face);
	FT_Done_FreeType(state.ft_library);

	xkb_state_unref(state.xkb_state);
	xkb_keymap_unref(state.xkb_keymap);
	xkb_context_unref(state.xkb_context);

	return state.exit_code;
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
					if (execute_keypress(state, state->repeat_key) == KEY_NO_REPEAT) {
						// stop key repeat if we shouldn't repeat anymore
						struct itimerspec timer = {0};
						timerfd_settime(state->key_repeat_timer_fd, 0, &timer, NULL);
						break;
					}
				}
				event = true;
			}
		}
	}
}
