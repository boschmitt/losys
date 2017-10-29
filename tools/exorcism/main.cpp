/*------------------------------------------------------------------------------
| This file is distributed under the BSD 2-Clause License.
| See LICENSE for details.
*-----------------------------------------------------------------------------*/
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

#include "io/read_pla.hpp"
#include "kernel/two_lvl32.hpp"
#include "opt/exorcism32.hpp"

static void
usage(int status)
{
	if (status == EXIT_FAILURE)
		fprintf(stdout, "Try '-h' for more information\n");
	else
		fprintf(stdout, "Usage: exorcism [-hv] <input_file>.pla <output_file>.pla\n\n" \
		        "Options:\n"\
		        "\t-h\t: display available options.\n" \
		        "\t-v\t: verbose mode.\n" \
		        "\t-w\t: very verbose mode.\n");
	exit(status);
}

static bool
fname_ok(char *fname)
{
	char *dot = nullptr;
	if ((dot = strrchr(fname, '.')) == nullptr) {
		fprintf(stderr, "[e] Unrecognized input file format.\n");
		return false;
	}
	if (strcmp(dot, ".pla") != 0) {
		fprintf(stderr, "[e] Unsupported input file format: %s\n", dot);
		return false;
	}
	return true;
}

int
main(int argc, char **argv)
{
	char *in_fname = nullptr;
	char *out_fname = nullptr;
	bool verbose = false;
	bool werbose = false;
	/* Opts parsing */
	int opt;
	extern int optind;
	extern int optopt;
	extern char* optarg;

	while ((opt = getopt(argc, argv, "hvw")) != -1) {
		switch (opt) {
		case 'h':
			usage(EXIT_SUCCESS);
			break;
		case 'v':
			verbose = true;
			break;
		case 'w':
			werbose = true;
		default:
			break;
		}
	}
	if (optind == argc) {
		usage(EXIT_FAILURE);
	}
	in_fname = strdup(argv[optind]);
	if (optind + 2 == argc) {
		out_fname = strdup(argv[optind + 1]);
	}

	/* Cheking command line */
	if (!fname_ok(in_fname)) {
		return EXIT_FAILURE;
	}
	if ((out_fname != nullptr) && !fname_ok(out_fname)) {
		return EXIT_FAILURE;
	}

	auto original = lsy::read_pla<lsy::two_lvl32>(in_fname, verbose | werbose);
	auto result   = lsy::exorcise(original, werbose);
	if (verbose | werbose) {
		fprintf(stdout, "ORIGINAL: "), print_stats(original);
		fprintf(stdout, "RESULT:   "), print_stats(result);
	}
	// TODO: implement write_pla
	//if (out_fname)
		// exorcism::write_pla(result, out_fname);

	return EXIT_SUCCESS;
}
