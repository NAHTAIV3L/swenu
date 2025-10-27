#include <string.h>

#include "input_field.h"
#include "array.h"
#include "graphics.h"
#include "layout.h"

int compare(const void *a, const void *b) {
	return strlen(((item_display_t*)a)->item->text) > strlen(((item_display_t*)b)->item->text);
}

void update_layout(client_state* state) {
	rect_t input, options;
	calculate_layout(state, &input, &options, true);
}

void filter_items(client_state* state) {
	if (!state->items) return;

	// get null terminated input buffer
	size_t len = array_size(state->input_buffer);
	char input_buffer_string[len + 1];
	memcpy(input_buffer_string, state->input_buffer, len);
	input_buffer_string[len] = '\0';

	// add all strings with substring
	array_clear(state->filtered_items);
	array_for_all(item_t, i, state->items) {
		if (strstr(i->text, input_buffer_string) != NULL) {
			array_add(state->filtered_items, (item_display_t){ .item = i });
		}
	}
	if (array_size(state->input_buffer)) qsort(state->filtered_items, array_size(state->filtered_items), sizeof(item_display_t), compare);

	// select item
	if (array_size(state->filtered_items) > 0) state->selected_filtered_item = 0;
	else state->selected_filtered_item = -1;

	// update layout
	update_layout(state);
}

void update_text_buffer(client_state* state) {
	filter_items(state);
	destroy_text_buffer(&state->input_buffer_grafix);
	init_text_buffer(state, &state->input_buffer_grafix, state->input_buffer, array_size(state->input_buffer));
}

bool type_key(client_state* state, xkb_keysym_t keysym) {
	bool ctrl = xkb_state_mod_name_is_active(state->xkb_state, XKB_MOD_NAME_CTRL, XKB_STATE_MODS_EFFECTIVE);
	bool alt = xkb_state_mod_name_is_active(state->xkb_state, XKB_MOD_NAME_ALT, XKB_STATE_MODS_EFFECTIVE);

	if (ctrl && alt) {
		switch (keysym) {
			case XKB_KEY_i: return insert_selected(state);
			case XKB_KEY_d: return delete_word(state);
		}
	}
	else if (ctrl) {
		switch (keysym) {
			case XKB_KEY_c:
			case XKB_KEY_g:
				return quit(state);

			case XKB_KEY_n: return select_next(state);
			case XKB_KEY_p: return select_previous(state);

			case XKB_KEY_e: return goto_end(state);
			case XKB_KEY_a: return goto_begining(state);

			case XKB_KEY_f: return forward_char(state);
			case XKB_KEY_b: return backward_char(state);

			case XKB_KEY_k: return kill_to_end(state);
			case XKB_KEY_d: return delete_char(state);
			case XKB_KEY_BackSpace:
			case XKB_KEY_Delete:
				return clear_input(state);
		}
	}
	else if (alt) {
		switch (keysym) {
			case XKB_KEY_d: return delete_word(state);
		}
	}
	switch (keysym) {
		case XKB_KEY_Tab: return insert_selected(state);
		case XKB_KEY_Escape: return quit(state);
		case XKB_KEY_Return:
		case XKB_KEY_KP_Enter:
			return submit_line(state);
		case XKB_KEY_BackSpace:
		case XKB_KEY_Delete:
			return delete_char_backward(state);
	}

	// type char into buffer
	char buf[16] = {0};
	int len = xkb_keysym_to_utf8(keysym, buf, sizeof(buf));
	if (len > 0) {
		for (int i = 0; i < strlen(buf); i++) {
			array_insert(state->input_buffer, state->cursor_index, buf[i]);
			state->cursor_index++;
		}
		update_text_buffer(state);
		return true;
	}

	return false;
}

