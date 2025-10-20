#include <string.h>

#include "input_field.h"
#include "array.h"
#include "graphics.h"

int compare(const void *a, const void *b) {
    return strlen(((item_display_t*)a)->item->text) > strlen(((item_display_t*)b)->item->text);
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

	// lay out items
	float offset = 0;
	array_for_all(item_display_t, display, state->filtered_items) {
		display->offset = offset;
		if (state->lines > 0) offset += state->line_height;
		else offset += display->item->pixel_len + state->horizontal_spacing;
	}
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
		state->exit_code = EXIT_FAILURE;
		return false;
	}

	// complete
	if ((keysym == XKB_KEY_i && ctrl && alt) || keysym == XKB_KEY_Tab) {
		if (state->selected_filtered_item != -1) {
			char* selected = state->filtered_items[state->selected_filtered_item].item->text;
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
		if (state->selected_filtered_item != -1) {
			state->selected_filtered_item = MIN(state->selected_filtered_item + 1, (int)array_size(state->filtered_items) - 1);
		}
		return true;
	}

	// select previous
	if ((keysym == XKB_KEY_p && ctrl) || keysym == XKB_KEY_Left || keysym == XKB_KEY_Up) {
		if (state->selected_filtered_item != -1) {
			state->selected_filtered_item = MAX(state->selected_filtered_item - 1, 0);
		}
		return true;
	}

	// paste
	if (keysym == XKB_KEY_v && ctrl) {
		FILE* fp = popen("wl-paste --no-newline --type text/plain 2>/dev/null", "r");
		if (!fp) return false;

		char* buffer = NULL;
		size_t size = 0;
		ssize_t len = getdelim(&buffer, &size, '\0', fp);
		int status = pclose(fp);

		if (status != 0 || len <= 0) {
			free(buffer);
			return false;
		}

		for (size_t i = 0; i < len; ++i) {
			array_add(state->input_buffer, buffer[i]);
		}

		update_text_buffer(state);

		free(buffer);
		return false;
	}

	// submit line
	if (keysym == XKB_KEY_Return || keysym == XKB_KEY_KP_Enter) {
		submit_line(state);
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
	if (state->exact_match) {
		if (state->selected_filtered_item == -1) return;
		printf("%s\n", state->filtered_items[state->selected_filtered_item].item->text);
	}
	else {
		if (state->selected_filtered_item == -1) printf("%.*s\n", (int)array_size(state->input_buffer), state->input_buffer);
		else printf("%s\n", state->filtered_items[state->selected_filtered_item].item->text);
	}
	state->running = false;
	state->exit_code = EXIT_SUCCESS;
}
