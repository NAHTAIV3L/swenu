#ifndef FONT_H_
#define FONT_H_
#include "state.h"


char* get_font(const char* font_name);
bool freetype_init(client_state* state, const char* font);
void atlas_init(client_state* state);
void atlas_create_texture(client_state* state);

int atlas_get_strwidth(client_state* state, const char* str);

#endif // FONT_H_
