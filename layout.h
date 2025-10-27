#ifndef LAYOUT_H_
#define LAYOUT_H_

#include "state.h"

void init_layout(client_state* state);
void calculate_layout(client_state* state, rect_t* prompt, rect_t* input, rect_t* options, bool recalc_options);

#endif
