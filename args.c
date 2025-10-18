#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

#include "args.h"

void parse_args(client_state* state, int argc, char* argv[]) {
	static struct option long_opts[] = {
		{"lines", required_argument, 0, 'l'},
		{"center", no_argument, 0, 'c'},
		{"allow-free-result", no_argument, 0, 'a'},
		{"verbose", no_argument, 0, 'v'},
		{0, 0, 0, 0}
	};

    int opt;
    int long_index = 0;
	while ((opt = getopt_long(argc, argv, "l:cav", long_opts, &long_index)) != -1) {
		switch (opt) {
		case 'l':
			state->lines = atoi(optarg);
			break;
		case 'c':
			state->center = true;
			break;
		case 'a':
			state->allow_free_result = true;
			break;
		case 'v':
			state->verbose = true;
			break;
		default:
			fprintf(stderr, "Usage: NOT WHAT YOU TYPED\n");
			break;
		}
	}
}
