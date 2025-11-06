#ifndef INPUT_FIELD_H_
#define INPUT_FIELD_H_

#include "state.h"
#include "keybind.h"

// returns true if the key should repeat
key_repeat_t execute_keypress(client_state* state, xkb_keysym_t keysym);
void refilter_items(client_state* state);
void update_text_buffer(client_state* state);

#endif
