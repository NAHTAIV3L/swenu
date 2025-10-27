#include "layout.h"
#include "array.h"
#include "config.h"

void init_layout(client_state* state) {

}

void calculate_layout(client_state* state, rect_t* input, rect_t* options, bool recalc_options) {

	// calculate input rect
	*input = (rect_t){ .x = 0, .y = state->line_height * state->lines, .dx = state->width, .dy = state->line_height };
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

	if (recalc_options) {

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

		// calculate fancy scroll
		if (fancy_scroll && array_size(state->filtered_items) != 0) {

			// calculate if we should adjust scrolling
			if (state->lines) {
				// calc vertical scroll
				float selected_bot = state->fancy_scroll + state->filtered_items[state->selected_filtered_item].r.y;
				float selected_top = selected_bot + state->filtered_items[state->selected_filtered_item].r.dy;
				if (selected_bot < options->y) {
					state->fancy_scroll -= selected_bot;
				}
				float diff = selected_top - (options->y + options->dy);
				if (diff > 0) {
					state->fancy_scroll -= diff;
				}
			} else {
				// calc horizontal scroll
				item_display_t* selected = &state->filtered_items[state->selected_filtered_item];
				float selected_left = state->fancy_scroll + selected->r.x;
				float selected_right = selected_left + selected->r.dx;
				float right_diff = selected_right - (options->x + options->dx);
				float left_diff = selected_left - options->x;

				if (selected->r.dx > options->dx) {
					state->fancy_scroll -= left_diff;
				}
				else if (right_diff > 0) {
					state->fancy_scroll -= right_diff;
				}
				else if (left_diff < 0) {
					state->fancy_scroll -= left_diff;
				}
			}

			// apply scroll offset
			array_for_all(item_display_t, display, state->filtered_items) {
				if (state->lines) {
					display->r.y += state->fancy_scroll;
				} else {
					display->r.x += state->fancy_scroll;
				}
			}

		}
	}
}
