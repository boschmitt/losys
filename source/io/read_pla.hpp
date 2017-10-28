/*------------------------------------------------------------------------------
| This file is distributed under the BSD 2-Clause License.
| See LICENSE for details.
*-----------------------------------------------------------------------------*/
#ifndef LOSYS_READ_PLA_HPP
#define LOSYS_READ_PLA_HPP

#include <algorithm>
#include <cctype>
#include <clocale>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <tuple>
#include <vector>

#include "kernel/cube32.hpp"
#include "kernel/two_lvl32.hpp"

namespace lsy {
/*------------------------------------------------------------------------------
| PLA file format
| ------
| TLDR: Format for physical description of Programmable Logic Arrays.
|
| Lines beginning with a '#' are comments and are ignored.
| Lines beginning with a '.' contain control information about the PLA.
| Currently, the control information is given in the following order:
|    .i  <number of inputs>
|    .o  <number of outputs>
| and optionally,
|    .p    <number of product terms (pterms)>
|    .type <type (SOP, ESOP)>
|
| TODO: Implement other espresso directives (.ilb, .ob, .kiss)
*-----------------------------------------------------------------------------*/

static std::tuple<std::uint32_t, std::uint32_t, std::uint32_t, std::string>
read_pla_header(std::stringstream &buffer)
{
	auto n_inputs  = 0u;
	auto n_outputs = 0u;
	auto n_terms   = 0u;
	std::string type;
	while (buffer.peek() == '.' || buffer.peek() == '#') {
		std::string line;
		std::getline(buffer, line, ' ');
		if (line[0] == '#') {
			std::getline(buffer, line);
			continue;
		}
		if (line[0] == '.') {
			if (line[1] == 'i') {
				buffer >> n_inputs;
			}
			if (line[1] == 'o') {
				buffer >> n_outputs;
			}
			if (line[1] == 'p') {
				buffer >> n_terms;
			}
			if (line[1] == 't') {
				getline(buffer, type);
				continue;
			}
			// FIXME: feels kind of hacky
			std::getline(buffer, line);
			continue;
		}
	}
	std::transform(type.begin(), type.end(), type.begin(),
		       [](unsigned char c){ return std::tolower(c); });
	/* TODO: this need to be explicit in GCC-4.8, Clang seems to accept
	 * the implicit {val, val} format */
	return std::make_tuple(n_inputs, n_outputs, n_terms, type);
}

template<class PLA>
PLA read_pla(const char *fname, bool verbose)
{
	PLA two_lvl;
	std::ifstream in_file(fname);
	if (!in_file.is_open()) {
		fprintf(stderr, "[e] Couldn't open file: %s\n", fname);
		return two_lvl;
	}
	std::stringstream buffer;
	buffer << in_file.rdbuf();
	in_file.close();

	/* TODO: use C++17 structured bindings */
	auto n_inputs  = 0u;
	auto n_outputs = 0u;
	auto n_terms   = 0u;
	std::string type;
	std::tie(n_inputs, n_outputs, n_terms, type) = read_pla_header(buffer);
	two_lvl.kind(type);
	two_lvl.n_inputs(n_inputs);

	/* Parsing cubes */
	auto n_cubes = 0;
	while (true) {
		std::string input;
		std::string output;
		std::getline(buffer, input, ' ');
		if (input[0] == '.' && input[1] == 'e') {
			break;
		}
		if (input.size() != n_inputs) {
			fprintf(stderr, "[e] Cube input data is inconsistent "
				        "with the declared attributes\n");
			break;
		}

		std::getline(buffer, output);
		if (output.size() != n_outputs) {
			fprintf(stderr, "[e] Cube output data is inconsistent "
			                "with the declared attributes\n");
			break;
		}
		two_lvl.add_cube(input, output);
		n_cubes++;
	}
	if (verbose) {
		fprintf(stdout, "[i] # inputs: %d\n", n_inputs);
		fprintf(stdout, "[i] # outputs: %d\n", n_outputs);
		fprintf(stdout, "[i] # terms: %d\n", n_cubes);
	}
	return two_lvl;
}
} // namespace lsy

#endif
