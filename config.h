#ifndef CONFIG_H_
#define CONFIG_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct {
	float r,g,b,a;
} color_t;

static const bool fancy_scroll = true;
static const uint32_t min_width = 500;
static const uint32_t font_size = 16;
static const color_t text_color = {0.85, 0.85, 0.85, 1.0};
static const color_t highlight_color = {0.1, 0.3, 0.7, 1.0};
static const color_t background_color = {0.1, 0.1, 0.1, 1.0};
static const color_t cursor_color = {0.9, 0.9, 0.9, 0.5};

#endif
