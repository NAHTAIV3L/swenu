#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

#include "args.h"

void parse_args(client_state* state, int argc, char* argv[]) {
	static struct option long_opts[] = {
		{"lines", required_argument, 0, 'l'},
		{0, 0, 0, 0}
	};

    int opt;
    int long_index = 0;
	while ((opt = getopt_long(argc, argv, "l:", long_opts, &long_index)) != -1) {
		switch (opt) {
		case 'l':
			state->lines = atoi(optarg);
			break;
		default:
			fprintf(stderr, "Usage: NOT WHAT YOU TYPED\n");
			break;
		}
	}
}
