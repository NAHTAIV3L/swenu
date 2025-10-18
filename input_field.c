#include <string.h>

#include "input_field.h"
#include "array.h"
#include "graphics.h"

void filter_items(client_state* state) {
	array_clear(state->filtered_items);

	// get null terminated input buffer
	size_t len = array_size(state->input_buffer);
	char input_buffer_string[len + 1];
	memcpy(input_buffer_string, state->input_buffer, len);
	input_buffer_string[len] = '\0';
	
	// add all strings with substring
	for (uint32_t i = 0; i < array_size(state->items); ++i) {
		if (strstr(state->items[i].text, input_buffer_string) != NULL) {
			array_add(state->filtered_items, i);
		}
	}
	
	// select item
	if (array_size(state->filtered_items) > 0) state->selected_filtered_item = 0;
	else state->selected_filtered_item = -1;
}

void update_text_buffer(client_state* state) {
	filter_items(state);
	destroy_text_buffer(&state->input_buffer_grafix);
	init_text_buffer(state, &state->input_buffer_grafix, state->input_buffer, array_size(state->input_buffer));
}

bool type_key(client_state* state, xkb_keysym_t keysym) {
	bool ctrl = xkb_state_mod_name_is_active(state->xkb_state, XKB_MOD_NAME_CTRL, XKB_STATE_MODS_EFFECTIVE);
	bool alt = xkb_state_mod_name_is_active(state->xkb_state, XKB_MOD_NAME_ALT, XKB_STATE_MODS_EFFECTIVE);

	// exit app
	if ((keysym == XKB_KEY_c && ctrl) ||
		(keysym == XKB_KEY_g && ctrl) ||
		(keysym == XKB_KEY_Escape)) {

		state->running = false;
		return false;
	}

	// complete
	if ((keysym == XKB_KEY_i && ctrl && alt) || keysym == XKB_KEY_Tab) {
		if (state->selected_filtered_item != -1) {
			char* selected = state->items[state->filtered_items[state->selected_filtered_item]].text;
			size_t len = strlen(selected);

			array_free(state->input_buffer);
			state->input_buffer = array_new(char, len);
			memcpy(state->input_buffer, selected, len);
			
			update_text_buffer(state);
		}
		return true;
	}

	// select next
	if ((keysym == XKB_KEY_n && ctrl) || keysym == XKB_KEY_Right || keysym == XKB_KEY_Down) {
		state->selected_filtered_item = MIN(state->selected_filtered_item + 1, array_size(state->filtered_items) - 1);
		return true;
	}

	// select previous
	if ((keysym == XKB_KEY_p && ctrl) || keysym == XKB_KEY_Left || keysym == XKB_KEY_Up) {
		state->selected_filtered_item = MAX(state->selected_filtered_item - 1, 0);
		return true;
	}

	// submit line
	if (keysym == XKB_KEY_Return || keysym == XKB_KEY_KP_Enter) {
		submit_line(state);
		update_text_buffer(state);
		return false;
	}

	// delete
	if (keysym == XKB_KEY_BackSpace || keysym == XKB_KEY_Delete) {
		if (ctrl) {
			array_clear(state->input_buffer);
		} else {
			array_pop(state->input_buffer);
		}
		update_text_buffer(state);
		return true;
	}

	// type char into buffer
	char buf[16] = {0};
	int len = xkb_keysym_to_utf8(keysym, buf, sizeof(buf));
	if (len > 0) {
		for (int i = 0; i < strlen(buf); i++) {
		array_add(state->input_buffer, buf[i]);
		}
		update_text_buffer(state);
		return true;
	}

	return false;
}

void submit_line(client_state* state) {
	if (state->selected_filtered_item == -1) {
		printf("%.*s\n", (int)array_size(state->input_buffer), state->input_buffer);
	} else {
		printf("%s\n", state->items[state->filtered_items[state->selected_filtered_item]].text);
	}
	state->running = false;
}
