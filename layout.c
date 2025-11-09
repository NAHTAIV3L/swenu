#include "layout.h"
#include "array.h"
#include "config.h"

void calculate_layout(client_state* state, rect_t* prompt, rect_t* input, rect_t* options, bool recalc_options, bool recalc_pages) {
	if (state->prompt && *state->prompt) {
		*prompt = (rect_t){ .x = 0, .y = state->line_height * state->lines, .dx = state->prompt_text_buffer.pixel_len + state->horizontal_spacing, .dy = state->line_height };
	}
	else {
		*prompt = (rect_t){ 0 };
	}
	// calculate input rect
	*input = (rect_t){ .x = prompt->dx, .y = state->line_height * state->lines, .dx = state->width - prompt->dx, .dy = state->line_height };
	if (state->lines == 0 && array_size(state->filtered_items) != 0) {
		input->dx = state->width / 3.0f;
	}

	// calculate options rect
	*options = (rect_t){0};
	if (state->lines) {
		options->dx = state->width;
		options->dy = input->y;
	} else {
		options->x = input->x + input->dx;
		options->dx = state->width - options->x;
		options->dy = state->line_height;
	}

	if (recalc_options || recalc_pages) {

		// calculate starting conditions
		rect_t cur;
		if (state->lines) {
			cur = (rect_t){ .x = options->x, .y = input->y - state->line_height, .dx = options->dx, .dy = state->line_height };
		} else {
			cur = (rect_t){ .x = options->x, .y = options->y, .dx = 0.0f, .dy = state->line_height };
		}

		// calculate base filtered item rects
		array_for_all(item_display_t, display, state->filtered_items) {
			if (!state->lines) {
				cur.dx = display->item->pixel_len + state->horizontal_spacing;
			}

			display->r = cur;

			if (state->lines) {
				cur.y -= state->line_height;
			} else {
				cur.x += cur.dx;
			}
		}

		// calculate page indices
		if (recalc_pages && !config.fancy_scroll) {
			state->current_page = 0;
			array_clear(state->page_indices);

			if (state->lines) {
				// this won't work if we add sized images and bullshit to the option list
				for (size_t i = 0; i < array_size(state->filtered_items); i += state->lines) {
					array_add(state->page_indices, i);
				}
			} else {
				array_add(state->page_indices, 0);
				float page_x = options->x;
				size_t i = 0;
				array_for_all(item_display_t, display, state->filtered_items) {
					// check if past the end of the page
					if (((display->r.x + display->r.dx) - page_x) / options->dx >= 1.f) {
						page_x = display->r.x;
						array_add(state->page_indices, i);
					}
					++i;
				}
			}
		}

		if (array_size(state->filtered_items) != 0) {
			if (config.fancy_scroll) {
				// calculate fancy scroll offset
				if (state->selected_filtered_item != -1) {
					if (state->lines) {
						// calc vertical scroll
						float selected_bot = state->scroll_offset + state->filtered_items[state->selected_filtered_item].r.y;
						float selected_top = selected_bot + state->filtered_items[state->selected_filtered_item].r.dy;
						if (selected_bot < options->y) {
							state->scroll_offset -= selected_bot;
						}
						float diff = selected_top - (options->y + options->dy);
						if (diff > 0) {
							state->scroll_offset -= diff;
						}
					} else {
						// calc horizontal scroll	
						item_display_t* selected = &state->filtered_items[state->selected_filtered_item];
						float selected_left = state->scroll_offset + selected->r.x;
						float selected_right = selected_left + selected->r.dx;
						float right_diff = selected_right - (options->x + options->dx);
						float left_diff = selected_left - options->x;

						if (selected->r.dx > options->dx) {
							state->scroll_offset -= left_diff;
						}
						else if (right_diff > 0) {
							state->scroll_offset -= right_diff;
						}
						else if (left_diff < 0) {
							state->scroll_offset -= left_diff;
						}
					}
				} else {
					state->scroll_offset = 0;
				}

				// display all filtered items (could be optimized)
				state->displayed_item_start = 0;
				state->displayed_item_end = array_size(state->filtered_items);

			} else {
				// calculate page scroll offset

				// determine what page the selected item is in
				size_t selected_page = 0;
				size_t selected_page_index = state->page_indices[selected_page];
				if (state->selected_filtered_item != -1) {
					for(size_t i = 0; i < array_size(state->page_indices); ++i) {
						if (state->selected_filtered_item < state->page_indices[i]) {
							break;
						}
						selected_page = i;
						selected_page_index = state->page_indices[i];
					}
				}

				// determine scroll to put that page at the start
				if (state->lines) {
					rect_t r = state->filtered_items[selected_page_index].r;
					state->scroll_offset = (options->y + options->dy) - (r.y + r.dy);
				} else {
					rect_t r = state->filtered_items[selected_page_index].r;
					state->scroll_offset = options->x - r.x;
				}
				
				// display filtered items on the page
				state->displayed_item_start = state->page_indices[selected_page];
				state->displayed_item_end = (selected_page + 1 < array_size(state->page_indices)) ? state->page_indices[selected_page + 1] : array_size(state->filtered_items);
			}

			// apply scroll offset
			array_for_all(item_display_t, display, state->filtered_items) {
				if (state->lines) {
					display->r.y += state->scroll_offset;
				} else {
					display->r.x += state->scroll_offset;
				}
			}
		}
		else {
			state->displayed_item_start = 0;
			state->displayed_item_end = 0;
		}
	}
}
