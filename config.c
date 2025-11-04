#include "config.h"

#include <stdlib.h>

config_t config = {
	.fancy_scroll = false,
	.min_width = 500,
	.font_size = 16,
	// colors
	.text_color = {0.85, 0.85, 0.85, 1.0},
	.highlight_color = {0.1, 0.3, 0.7, 1.0},
	.background_color = {0.1, 0.1, 0.1, 1.0},
	.cursor_color = {0.9, 0.9, 0.9, 0.5}
};


char* get_conf_file() {
	// get from xdg config home
	const char* xdg = getenv("XDG_CONFIG_HOME");
	if (xdg && *xdg) {
	}      

	// get from home
    const char* home = getenv("HOME");
    if (home && *home) {
    }
    
    return NULL;
}

void init_conf() {
	/* const char* conf_dir = get_conf_file(); */

	/* free(conf_dir); */
}
