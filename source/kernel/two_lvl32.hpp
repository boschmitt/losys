/*------------------------------------------------------------------------------
| This file is distributed under the BSD 2-Clause License.
| See LICENSE for details.
*-----------------------------------------------------------------------------*/
#ifndef LOSYS_TWO_LVL32_HPP
#define LOSYS_TWO_LVL32_HPP

#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

#include "cube32.hpp"

namespace lsy {

/*------------------------------------------------------------------------------
| two_lvl32
| ------
| TLDR: data structure for two level representation of a Boolean functions with
|       up to 32 variables.
|
| Any Boolean function can be represented as a two-level sum of products (SOP),
| which is a Boolean OR of cubes, or as exclusive-sum of products (ESOP) which
| is a Boolean XOR of cubes.
|
| FIXME: support for multiple output functions.
*-----------------------------------------------------------------------------*/
struct two_lvl32 {
	enum class kind_t {
		SOP,
		ESOP,
		UNDEF
	};

	kind_t _kind;
	std::uint32_t _n_inputs;
	std::vector<std::vector<cube32>> _cubes;

	void n_inputs(const std::uint32_t n_in)
	{
		_n_inputs = n_in;
	}

	void n_outputs(const std::uint32_t n_out)
	{
		_cubes.resize(n_out);
	}

	void kind(const std::string &k)
	{
		if (k.find("esop") != std::string::npos) {
			_kind = kind_t::ESOP;
		} else if (k.find("sop") != std::string::npos) {
			_kind = kind_t::SOP;
		} else {
			_kind = kind_t::UNDEF;
		}
	}

	void add_cube(const std::string &in, const std::string &out)
	{
		cube32 cube;
		for (auto i = 0u; i < in.size(); ++i) {
			switch (in[i]) {
			case '-': break;
			case '1': cube.add_lit(i, 1); break;
			case '0': cube.add_lit(i, 0); break;
			}
		}
		for (auto i = 0u; i < out.size(); ++i) {
			if (out[i] == '1') {
				_cubes[i].push_back(cube);
			}
		}
	}
};

static void print_stats(const two_lvl32 &fnt)
{
	if (fnt._kind == two_lvl32::kind_t::SOP) {
		fprintf(stdout, "[Two-level SOP]\n");
	} else if (fnt._kind == two_lvl32::kind_t::ESOP) {
		fprintf(stdout, "[Two-level ESOP]\n");
	} else {
		fprintf(stdout, "[Two-level]\n");
	}
	for (auto i = 0; i < fnt._cubes.size(); ++i)
		fprintf(stdout, "[%d] Cubes : %5lu\n", i, fnt._cubes[i].size());
}
} // namespace lsy

#endif
