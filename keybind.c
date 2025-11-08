#include "keybind.h"
#include "array.h"
#include "input_field.h"
#include "layout.h"

key_repeat_t quit(client_state* state) {
	state->running = false;
	state->exit_code = EXIT_FAILURE;
	return KEY_NO_REPEAT;
}

key_repeat_t submit_line(client_state* state) {
	if (state->exact_match) {
		if (state->selected_filtered_item == -1) return false;
		printf("%s\n", state->filtered_items[state->selected_filtered_item].item->text);
	}
	else {
		if (state->selected_filtered_item == -1) printf("%.*s\n", (int)array_size(state->input_buffer), state->input_buffer);
		else printf("%s\n", state->filtered_items[state->selected_filtered_item].item->text);
	}
	state->running = false;
	state->exit_code = EXIT_SUCCESS;
	return KEY_NO_REPEAT;
}

key_repeat_t insert_selected(client_state* state) {
	if (state->selected_filtered_item != -1) {
		char* selected = state->filtered_items[state->selected_filtered_item].item->text;
		size_t len = strlen(selected);

		array_free(state->input_buffer);
		state->input_buffer = array_new(char, len);
		memcpy(state->input_buffer, selected, len);
		state->cursor_index = len;

		update_text_buffer(state);
	}
	return KEY_NO_REPEAT;
}

key_repeat_t select_next(client_state* state) {
	if (state->selected_filtered_item + 1 < array_size(state->filtered_items)) {
		state->selected_filtered_item = MIN(state->selected_filtered_item + 1, (int)array_size(state->filtered_items) - 1);
		rect_t prompt, input, options;
		calculate_layout(state, &prompt, &input, &options, true, false);
	}
	return KEY_REPEAT;
}

key_repeat_t select_previous(client_state* state) {
	if (state->selected_filtered_item != -1) {
		state->selected_filtered_item = MAX(state->selected_filtered_item - 1, state->exact_match ? 0 : -1);
		if (state->selected_filtered_item == -1) state->forced_selected_zero = false;
		rect_t prompt, input, options;
		calculate_layout(state, &prompt, &input, &options, true, false);
	}
	return KEY_REPEAT;
}

key_repeat_t select_first(client_state* state) {
	if (array_size(state->filtered_items) > 0) {
		state->selected_filtered_item = 0;
		rect_t prompt, input, options;
		calculate_layout(state, &prompt, &input, &options, true, false);
	}
	return KEY_NO_REPEAT;
}

key_repeat_t select_last(client_state* state) {
	if (array_size(state->filtered_items) > 0) {
		state->selected_filtered_item = array_size(state->filtered_items) - 1;
		rect_t prompt, input, options;
		calculate_layout(state, &prompt, &input, &options, true, false);
	}
	return KEY_NO_REPEAT;
}

key_repeat_t goto_end(client_state* state) {
	state->cursor_index = array_size(state->input_buffer);
	return KEY_NO_REPEAT;
}

key_repeat_t goto_start(client_state* state) {
	state->cursor_index = 0;
	return KEY_NO_REPEAT;
}

key_repeat_t forward_char_or_select_next(client_state* state) {
	if (state->cursor_index < array_size(state->input_buffer)) return forward_char(state);
	else return select_next(state);
}

key_repeat_t backward_char_or_select_previous(client_state* state) {
	if (state->cursor_index > 0) return backward_char(state);
	else return select_previous(state);
}

key_repeat_t forward_char(client_state* state) {
	if (state->cursor_index < array_size(state->input_buffer)) {
		state->cursor_index++;
	}
	return KEY_REPEAT;
}

key_repeat_t backward_char(client_state* state) {
	if (state->cursor_index > 0) {
		state->cursor_index--;
	}
	return KEY_REPEAT;
}

key_repeat_t forward_word(client_state* state) {
	if (state->cursor_index < array_size(state->input_buffer)) {
		bool hit_white = false;
		for (; state->cursor_index < array_size(state->input_buffer); state->cursor_index++) {
			if (state->input_buffer[state->cursor_index] == ' ') {
				hit_white = true;
			} else if (hit_white) {
				break;
			}
		}
	}
	return KEY_REPEAT;
}

key_repeat_t backward_word(client_state* state) {
	if (state->cursor_index > 0) {
		bool hit_non_white = false;
		state->cursor_index--;
		for (; state->cursor_index > 0; state->cursor_index--) {
			if (state->input_buffer[state->cursor_index] == ' ') {
				if (hit_non_white) {
					++state->cursor_index;
					break;
				}
			} else {
				hit_non_white = true;
			}
		}
	}
	return KEY_REPEAT;
}

key_repeat_t kill_to_start(client_state* state) {
	if (array_size(state->input_buffer) && state->cursor_index) {
		array_erase_range(state->input_buffer, 0, state->cursor_index - 1);
		state->cursor_index = 0;
		update_text_buffer(state);
	}

	return KEY_NO_REPEAT;
}

key_repeat_t kill_to_end(client_state* state) {
	array_resize(state->input_buffer, state->cursor_index);
	update_text_buffer(state);
	return KEY_NO_REPEAT;
}

key_repeat_t delete_word(client_state* state) {
	if (state->cursor_index != array_size(state->input_buffer)) {
		size_t endidx = state->cursor_index;
		bool hit_non_white = false;
		for (; endidx < array_size(state->input_buffer) - 1; endidx++) {
			if (state->input_buffer[endidx] == ' ') {
				if (hit_non_white) {
					endidx--;
					break;
				}
			} else {
				hit_non_white = true;
			}
		}
		array_erase_range(state->input_buffer, state->cursor_index, endidx);
		update_text_buffer(state);
	}
	return KEY_REPEAT;
}

key_repeat_t delete_word_backward(client_state* state) {
	if (state->cursor_index) {
		size_t startidx = state->cursor_index - 1;
		bool hit_non_white = false;
		for (; startidx > 0; startidx--) {
			if (state->input_buffer[startidx] == ' ') {
				if (hit_non_white) {
					++startidx;
					break;
				}
			} else {
				hit_non_white = true;
			}
		}
		array_erase_range(state->input_buffer, startidx, state->cursor_index - 1);
		state->cursor_index = startidx;
		update_text_buffer(state);
	}
	return KEY_REPEAT;
}

key_repeat_t delete_char(client_state* state) {
	if (state->cursor_index != array_size(state->input_buffer)) {
		array_erase(state->input_buffer, state->cursor_index);
		update_text_buffer(state);
		return KEY_REPEAT;
	}
	return KEY_NO_REPEAT;
}

key_repeat_t delete_char_backward(client_state* state) {
	if (state->cursor_index) {
		array_erase(state->input_buffer, state->cursor_index - 1);
		state->cursor_index--;
		update_text_buffer(state);
		return KEY_REPEAT;
	}
	return KEY_NO_REPEAT;
}

key_repeat_t clear_input(client_state* state) {
	array_clear(state->input_buffer);
	state->cursor_index = 0;
	update_text_buffer(state);
	return KEY_NO_REPEAT;
}

key_repeat_t paste_from_clipboard(client_state* state) {
	array_insert_many(state->input_buffer, state->cursor_index, state->clipboard, state->clipboard_size);
	state->cursor_index += state->clipboard_size;
	update_text_buffer(state);
	return KEY_REPEAT;
}
