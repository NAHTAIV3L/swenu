#include "wayland.h"
#include <unistd.h>

struct wl_data_device_listener data_device_listener;

void setup_data_device(client_state* state) {
	state->data_device = wl_data_device_manager_get_data_device(state->data_device_manager, state->seat);
	wl_data_device_add_listener(state->data_device, &data_device_listener, state);
}

static void data_offer_handle_offer(void *data, struct wl_data_offer *offer,
									const char *mime_type) {
}

static const struct wl_data_offer_listener data_offer_listener = {
	.offer = data_offer_handle_offer,
};

static void data_device_handle_data_offer(void *data,
										  struct wl_data_device *data_device, struct wl_data_offer *offer) {
	// An application has created a new data source
	wl_data_offer_add_listener(offer, &data_offer_listener, NULL);
}

static void data_device_handle_selection(void *data,
										 struct wl_data_device *data_device, struct wl_data_offer *offer) {

	client_state *state = data;
	state->data_offer = offer;

	// free old clipboard
	free(state->clipboard);

	// no clipboard contents
	if (offer == NULL) {
		state->clipboard = "";
		return;
	}

	// get contents in fd
	int fds[2];
	pipe(fds);
	wl_data_offer_receive(offer, "text/plain", fds[1]);
	close(fds[1]);
	wl_display_roundtrip(state->display);

	// read fd
	state->clipboard_size = 64;
	state->clipboard = malloc(state->clipboard_size);
	size_t current_clip = 0;
	while (true) {
		ssize_t n = read(fds[0], state->clipboard + current_clip, state->clipboard_size - current_clip);
		if (n <= 0) {
			break;
		}
		current_clip += n;
		if (current_clip == state->clipboard_size) {
			state->clipboard_size *= 2;
			state->clipboard = realloc(state->clipboard, state->clipboard_size);
		}
	}
	close(fds[0]);

	state->clipboard_size = current_clip; // we don't need the ACTUAL length of the buffer anymore, just the string :D
	// remove newlines
	int i, j;
	for (i = 0, j = 0; i < state->clipboard_size; i++) {
		if (state->clipboard[i] != '\n' && state->clipboard[i] != '\r') {
			state->clipboard[j++] = state->clipboard[i];
		}
	}
	state->clipboard_size = j;

	wl_data_offer_destroy(offer);
}

struct wl_data_device_listener data_device_listener = {
	.data_offer = data_device_handle_data_offer,
	.selection = data_device_handle_selection,
};
