#ifndef LAYOUT_H_
#define LAYOUT_H_

#include "state.h"

void calculate_layout(client_state* state, rect_t* prompt, rect_t* input, rect_t* options, bool recalc_options, bool recalc_pages);

#endif
