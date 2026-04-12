#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

#include "args.h"
#include "array.h"

void parse_args(client_state* state, int argc, char* argv[]) {
	static struct option long_opts[] = {
		{"lines", required_argument, 0, 'l'},
		{"center", no_argument, 0, 'c'},
		{"exact-match", no_argument, 0, 'e'},
		{"verbose", no_argument, 0, 'v'},
		{"password", no_argument, 0, 'P'},
		{"prompt", no_argument, 0, 'p'},
		{"orderless", no_argument, 0, 'o'},
		{"insensitive", no_argument, 0, 'i'},
		{"starting-text", required_argument, 0, 's'},
		{0, 0, 0, 0}
	};

	int opt;
	int long_index = 0;
	while ((opt = getopt_long(argc, argv, "l:p:s:coievP", long_opts, &long_index)) != -1) {
		switch (opt) {
			case 'l':
				state->lines = MIN(atoi(optarg), array_size(state->items));
				break;
			case 'p':
				state->prompt = optarg;
				break;
			case 'c':
				state->center = true;
				break;
			case 'e':
				state->exact_match = true;
				break;
			case 'v':
				state->verbose = true;
				break;
			case 'P':
				state->password = true;
				break;
			case 'o':
				state->orderless = true;
				break;
			case 'i':
				state->strstr2 = strcasestr;
				break;
			case 's':
				array_insert_many(state->input_buffer, 0, optarg, strlen(optarg));
				state->cursor_index = strlen(optarg);
				break;
			default:
				fprintf(stderr, "Usage: NOT WHAT YOU TYPED\n");
				fprintf(stderr, "%c\n", opt);
				break;
		}
	}
}
