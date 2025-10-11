#ifndef WAYLAND_H_
#define WAYLAND_H_

#include "state.h"

void setup_registry(client_state* state);
void setup_surface(client_state* state);
void setup_seat(client_state* state, uint32_t name);
void setup_pointer(client_state* state);
void setup_keyboard(client_state* state);

#endif
