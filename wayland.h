#ifndef WAYLAND_H_
#define WAYLAND_H_

#include "state.h"

void setup_registry_and_globals(client_state* state);
void create_surface(client_state* state);
void setup_seat(client_state* state);
void setup_pointer(client_state* state);
void setup_keyboard(client_state* state);

#endif
