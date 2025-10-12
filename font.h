#ifndef FONT_H_
#define FONT_H_
#include "state.h"


char* get_font(const char* font_name);
bool init_freetype(client_state* state, const char* font);
bool init_atlas(client_state* state);

#endif // FONT_H_
