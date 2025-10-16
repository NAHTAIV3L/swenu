#ifndef GRAPHICS_H_
#define GRAPHICS_H_

#include "state.h"

bool init_gl(client_state* state);
void render_frame(client_state* state);

void initTextBuffer(client_state* state, text_buffer_t* buffer, char* text, size_t text_len);

#endif
