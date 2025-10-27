#ifndef KEYBIND_H_
#define KEYBIND_H_
#include <stdbool.h>

#include "state.h"

typedef bool key_repeat_t;

#define KEY_REPEAT 1
#define KEY_NO_REPEAT 0

key_repeat_t quit(client_state* state);

key_repeat_t submit_line(client_state* state);

key_repeat_t insert_selected(client_state* state);

key_repeat_t select_next(client_state* state);
key_repeat_t select_previous(client_state* state);

key_repeat_t goto_end(client_state* state);
key_repeat_t goto_begining(client_state* state);

key_repeat_t forward_char(client_state* state);
key_repeat_t backward_char(client_state* state);

key_repeat_t kill_to_end(client_state* state);

key_repeat_t delete_word(client_state* state);
key_repeat_t delete_char(client_state* state);
key_repeat_t delete_char_backward(client_state* state);

key_repeat_t clear_input(client_state* state);

key_repeat_t paste_from_clipboard(client_state* state);
#endif // KEYBIND_H_
