#include "config.h"
#include "ini.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

config_t config = {
	// default config
	.fancy_scroll = false,
	.min_width = 500,
	.font_size = 16,
	.text_color = {0.85, 0.85, 0.85, 1.0},
	.highlight_color = {0.1, 0.3, 0.7, 1.0},
	.background_color = {0.1, 0.1, 0.1, 1.0},
	.cursor_color = {0.9, 0.9, 0.9, 0.5}
};

// get config file
const char* conf_file_name = "/swenu/conf.ini";
char* get_conf_path() {
	// get from xdg config home
	const char* xdg = getenv("XDG_CONFIG_HOME");
	if (xdg && *xdg) {
		char* conf_file = NULL;
		asprintf(&conf_file, "%s%s", xdg, conf_file_name);
		return conf_file;
	}      

	// get from home
    const char* home = getenv("HOME");
    if (home && *home) {
		char* conf_file = NULL;
		asprintf(&conf_file, "%s/.config%s", home, conf_file_name);
		return conf_file;
    }
    
    return NULL;
}

// bool parser
bool parse_bool(const char* value) {
	if (strcasecmp(value, "true") == 0 || strcasecmp(value, "t") == 0 || strcasecmp(value, "yes") == 0) {
		return true;
	} else {
		return false;
	}
}

// color parser (TODO - support hex codes)
regex_t color_regex;
const char *color_regex_pattern = "^\\([[:space:]]*([[:digit:].]*)[[:space:]]*,[[:space:]]*([[:digit:].]*)[[:space:]]*,[[:space:]]*([[:digit:].]*)[[:space:]]*,[[:space:]]*([[:digit:].]*)[[:space:]]*\\)$"; // this is fucking evil
void parse_color(const char* value, color_t* color) {
    regmatch_t matches[5];  // full match + 4 capture groups
    if (regexec(&color_regex, value, 5, matches, 0) == 0) {
        for (int i = 1; i <= 4; i++) {
            size_t len = matches[i].rm_eo - matches[i].rm_so;
			char match[len + 1];
			memcpy(match, value + matches[i].rm_so, len);
			match[len] = '\0';
			*((float*)color + i - 1) = atof(match);
        }
    }
}

static int our_ini_handler(void* user, const char* section, const char* name, const char* value)
{
	config_t* config = (config_t*)user;

	#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
	if (MATCH("", "fancy_scroll")) {
		config->fancy_scroll = parse_bool(value);
	} else if (MATCH("", "min_width")) {
		config->min_width = atoi(value);
	} else if (MATCH("", "font_size")) {
		config->font_size = atoi(value);
	} else if (MATCH("colors", "text_color")) {
		parse_color(value, &config->text_color);
	} else if (MATCH("colors", "highlight_color")) {
		parse_color(value, &config->highlight_color);
	} else if (MATCH("colors", "background_color")) {
		parse_color(value, &config->background_color);
	} else if (MATCH("colors", "cursor_color")) {
		parse_color(value, &config->cursor_color);
	} else {
		return 0;  /* unknown section/name, error */
	}
	return 1;
}

void init_conf() {
	// compile color regex
    if (regcomp(&color_regex, color_regex_pattern, REG_EXTENDED) != 0) {
        fprintf(stderr, "Color regex compile failed\n");
    }
	
	// get config dir
	char* conf_path = get_conf_path();
	if (!conf_path) return;

	// parse config file
	int ini_res = ini_parse(conf_path, our_ini_handler, &config);
	if (ini_res > 0) {
		fprintf(stderr, "Error parsing config file (%s), on line %d\n", conf_path, ini_res);
    }

	free(conf_path);
    regfree(&color_regex);
}
