#include <string.h>

#include "input_field.h"
#include "array.h"
#include "graphics.h"
#include "layout.h"

int compare(const void *a, const void *b) {
	return strlen(((item_display_t*)a)->item->text) > strlen(((item_display_t*)b)->item->text);
}

int orderless_compare(const void *a, const void *b) {
	return ((item_display_t*)a)->perc < ((item_display_t*)b)->perc;
}

void refilter_items(client_state* state) {
	if (!state->items) return;

	// get null terminated input buffer
	size_t len = array_size(state->input_buffer);
	char input_buffer_string[len + 1];
	memcpy(input_buffer_string, state->input_buffer, len);
	input_buffer_string[len] = '\0';

	// add all strings with substring
	array_clear(state->filtered_items);
	if (state->orderless) {
		char** parts = array_new(char*, 0);
		char* part = NULL;
		for (part = strtok(input_buffer_string, " "); part; part = strtok(NULL, " ")) {
			array_add(parts, part);
		}
		array_for_all(item_t, i, state->items) {
			float total = strlen(i->text);
			float part = 0;
			bool add = true;
			array_for_all(char*, j, parts) {
				if (state->strstr(i->text, *j)) {
					part += strlen(*j);
				}
				else {
					add = false;
				}
			}
			if (add) {
				item_display_t tmp = {
					.item = i,
					.perc = part / total
				};
				array_add(state->filtered_items, tmp);
			}
		}
		if (array_size(state->input_buffer)) {
			qsort(state->filtered_items, array_size(state->filtered_items), sizeof(item_display_t), orderless_compare);
		}
		array_free(parts);
	}
	else {
		array_for_all(item_t, i, state->items) {
			if (state->strstr(i->text, input_buffer_string) != NULL) {
				array_add(state->filtered_items, (item_display_t){ .item = i });
			}
		}
		if (array_size(state->input_buffer)) {
			qsort(state->filtered_items, array_size(state->filtered_items), sizeof(item_display_t), compare);
		}
	}

	// automatically select item
	if (state->selected_filtered_item != -1 || state->forced_selected_zero) {
		if (array_size(state->filtered_items) == 0) {
			state->selected_filtered_item = -1;
			state->forced_selected_zero = true;
		} else {
			state->selected_filtered_item = 0;
		}
	}

	// update layout
	rect_t prompt, input, options;
	calculate_layout(state, &prompt, &input, &options, true, true);
}

void update_text_buffer(client_state* state) {
	refilter_items(state);
	destroy_text_buffer(&state->input_buffer_grafix);
	init_text_buffer(state, &state->input_buffer_grafix, state->input_buffer, array_size(state->input_buffer));
}

key_repeat_t execute_keypress(client_state* state, xkb_keysym_t keysym) {
	bool ctrl = xkb_state_mod_name_is_active(state->xkb_state, XKB_MOD_NAME_CTRL, XKB_STATE_MODS_EFFECTIVE);
	bool alt = xkb_state_mod_name_is_active(state->xkb_state, XKB_MOD_NAME_ALT, XKB_STATE_MODS_EFFECTIVE);

	if (ctrl && alt) {
		switch (keysym) {
			case XKB_KEY_i: return insert_selected(state);
			case XKB_KEY_d: return delete_word(state);
			case XKB_KEY_BackSpace: return delete_word_backward(state);
			case XKB_KEY_f: return forward_word(state);
			case XKB_KEY_b: return backward_word(state);
		}
		return KEY_NO_REPEAT;
	}
	else if (ctrl) {
		switch (keysym) {
			case XKB_KEY_c:
			case XKB_KEY_g:
				return quit(state);

			case XKB_KEY_n: return select_next(state);
			case XKB_KEY_p: return select_previous(state);

			case XKB_KEY_a: return goto_start(state);
			case XKB_KEY_e: return goto_end(state);

			case XKB_KEY_f: return forward_char(state);
			case XKB_KEY_b: return backward_char(state);

			case XKB_KEY_k: return kill_to_end(state);
			case XKB_KEY_d: return delete_char(state);

			case XKB_KEY_v: return paste_from_clipboard(state);

			case XKB_KEY_BackSpace: return kill_to_start(state);
			case XKB_KEY_Delete: return clear_input(state);
		}
		return KEY_NO_REPEAT;
	}
	else if (alt) {
		switch (keysym) {
			case XKB_KEY_d: return delete_word(state);
			case XKB_KEY_BackSpace: return delete_word_backward(state);
			case XKB_KEY_f: return forward_word(state);
			case XKB_KEY_b: return backward_word(state);
			case XKB_KEY_less: return select_first(state);
			case XKB_KEY_greater: return select_last(state);
		}
		return KEY_NO_REPEAT;
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
		case XKB_KEY_Down: return select_next(state);
		case XKB_KEY_Up: return select_previous(state);
		case XKB_KEY_Left: return backward_char_or_select_previous(state);
		case XKB_KEY_Right: return forward_char_or_select_next(state);
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
		return KEY_REPEAT;
	}

	return KEY_NO_REPEAT;
}

