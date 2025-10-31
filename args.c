#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

#include "args.h"

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
		{0, 0, 0, 0}
	};

	int opt;
	int long_index = 0;
	while ((opt = getopt_long(argc, argv, "l:p:coievP", long_opts, &long_index)) != -1) {
		switch (opt) {
		case 'l':
			state->lines = atoi(optarg);
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
			state->strstr = strcasestr;
			break;
		default:
			fprintf(stderr, "Usage: NOT WHAT YOU TYPED\n");
			fprintf(stderr, "%c\n", opt);
			break;
		}
	}
}
