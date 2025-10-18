#include <string.h>

#include "input_field.h"
#include "array.h"
#include "graphics.h"

void regenerate_text_buffer(client_state* state) {
	destroy_text_buffer(&state->input_buffer_grafix);
	init_text_buffer(state, &state->input_buffer_grafix, state->input_buffer, array_size(state->input_buffer));
}

bool type_key(client_state* state, xkb_keysym_t keysym) {
	bool ctrl = xkb_state_mod_name_is_active(state->xkb_state, XKB_MOD_NAME_CTRL, XKB_STATE_MODS_EFFECTIVE);

	// exit app
	if ((keysym == XKB_KEY_c && ctrl) ||
		(keysym == XKB_KEY_g && ctrl) ||
		(keysym == XKB_KEY_Escape)) {

		state->running = false;
		return false;
	}

	// submit line
	if (keysym == XKB_KEY_Return || keysym == XKB_KEY_KP_Enter) {
		submit_line(state);
		regenerate_text_buffer(state);
		return false;
	}

	// delete
	if (keysym == XKB_KEY_BackSpace || keysym == XKB_KEY_Delete) {
		if (ctrl) {
			array_clear(state->input_buffer);
		} else {
			array_pop(state->input_buffer);
		}
		regenerate_text_buffer(state);
		return true;
	}

	// type char into buffer
	char buf[16] = {0};
	int len = xkb_keysym_to_utf8(keysym, buf, sizeof(buf));
	if (len > 0) {
		for (int i = 0; i < strlen(buf); i++) {
		array_add(state->input_buffer, buf[i]);
		}
		regenerate_text_buffer(state);
		return true;
	}

	return false;
}

void submit_line(client_state* state) {
	if (state->input_buffer) {
		printf("%.*s\n", (int)array_size(state->input_buffer), state->input_buffer);
		state->running = false;
	}
}
