#ifndef INPUT_FIELD_H_
#define INPUT_FIELD_H_

#include "state.h"

// returns true if the key should repeat
bool type_key(client_state* state, xkb_keysym_t keysym);
void submit_line(client_state* state);

#endif
