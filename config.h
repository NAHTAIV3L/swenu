#ifndef CONFIG_H_
#define CONFIG_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct {
	float r,g,b,a;
} color_t;

typedef struct {
	bool fancy_scroll;
	uint32_t min_width;
	uint32_t font_size;
	color_t text_color;
	color_t highlight_color;
	color_t background_color;
	color_t cursor_color;
} config_t;

extern config_t config;

void init_conf();

#endif
