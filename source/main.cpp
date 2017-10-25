/*------------------------------------------------------------------------------
| This file is distributed under the BSD 2-Clause License.
| See LICENSE for details.
*-----------------------------------------------------------------------------*/
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

extern "C" {
#include "aig/gia/gia.h"
}

static void usage(int status)
{
	if (status == EXIT_FAILURE)
		fprintf(stdout, "Try '-h' for more information\n");
	else
		fprintf(stdout, "Usage: <cmd> [-h] <input_file>\n\n" \
		        "Options:\n"\
		        "\t-h"     "\t : display available options.\n");
	exit(status);
}

int main(int argc, char **argv)
{
	char *in_fname = nullptr;
	char *dot;
	/* Opts parsing */
	int opt;
	extern int optind;
	extern int optopt;
	extern char* optarg;

	while ((opt = getopt(argc, argv, "hv")) != -1) {
		switch (opt) {
		case 'h':
			usage(EXIT_SUCCESS);
			break;
		default:
			break;
		}
	}
	if (optind == argc)
		usage(EXIT_FAILURE);
	in_fname = strdup(argv[optind]);

	/* Cheking command line */
	if ((dot = strrchr(in_fname, '.')) == NULL) {
		fprintf(stderr, "[e] Unrecognized input file format.\n");
		return EXIT_FAILURE;
	}
	if (strcmp(dot, ".aig")) {
		fprintf(stderr, "[e] Unsupported input file format: %s\n", dot);
		return EXIT_FAILURE;
	}

	Gia_Man_t *aig = Gia_AigerRead(in_fname, 0, 0, 1);
	Gia_ManPrintStats(aig, 0);

	return EXIT_SUCCESS;
}
