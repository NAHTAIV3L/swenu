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
	if (strcasecmp(value, "true") == 0 || strcasecmp(value, "t") == 0 || strcasecmp(value, "yes") == 0 || strcasecmp(value, "on") == 0) {
		return true;
	} else {
		return false;
	}
}

// color parser (TODO - support hex codes)
regex_t color_regex;
const char *color_regex_pattern = "^\\([[:space:]]*([[:digit:].]*)[[:space:]]*,[[:space:]]*([[:digit:].]*)[[:space:]]*,[[:space:]]*([[:digit:].]*)[[:space:]]*,[[:space:]]*([[:digit:].]*)[[:space:]]*\\)$"; // this is fucking evil
bool parse_color(const char* value, color_t* color) {
	if (!strlen(value)) {
		return false;
	}

	if (value[0] == '(') {
		// (r,g,b,a) format
		regmatch_t matches[5];  // full match + 4 capture groups
		if (regexec(&color_regex, value, 5, matches, 0) == 0) {
			for (int i = 1; i <= 4; i++) {
				size_t len = matches[i].rm_eo - matches[i].rm_so;
				char match[len + 1];
				memcpy(match, value + matches[i].rm_so, len);
				match[len] = '\0';
				*((float*)color + i - 1) = atof(match);
			}
			return true;
		}
	} else {
		// hex format
		const char* start = value;
		if (*start == '#') ++start;

		// scan
		int code_len = strlen(start);
		if (code_len == 6) {
			unsigned char r,g,b;
			sscanf(start, "%2hhx%2hhx%2hhx", &r, &g, &b);
			printf("%d, %d, %d\n", r, g, b);
			color->r = r / 255.0f;
			color->g = g / 255.0f;
			color->b = b / 255.0f;
			color->a = 1.0f;
		} else if (code_len == 8) {
			unsigned char r,g,b,a;
			sscanf(start, "%2hhx%2hhx%2hhx%2hhx", &r, &g, &b, &a);
			color->r = r / 255.0f;
			color->g = g / 255.0f;
			color->b = b / 255.0f;
			color->a = a / 255.0f;
		} else {
			return false;
		}

		return true;
	}

	return false;
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
	} else if (strcmp(section, "colors") == 0) {
		color_t color;
		if (!parse_color(value, &color)) {
			return 0;
		}

		if (strcmp(name, "text_color") == 0) {
			config->text_color = color;
		} else if (strcmp(name, "highlight_color") == 0) {
			config->highlight_color = color;
		} else if (strcmp(name, "background_color") == 0) {
			config->background_color = color;
		} else if (strcmp(name, "cursor_color") == 0) {
			config->cursor_color = color;
		} else {
			return 0;
		}
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
