/*------------------------------------------------------------------------------
| This file is distributed under the BSD 2-Clause License.
| See LICENSE for details.
*-----------------------------------------------------------------------------*/
#ifndef LOSYS_WRITE_PLA_HPP
#define LOSYS_WRITE_PLA_HPP

#include <fstream>
#include <sstream>
#include <vector>

#include "kernel/cube32.hpp"
#include "kernel/two_lvl32.hpp"

namespace lsy {

/* FIXME: This is horrible, but necessary for now; instead of creating one PLA
   file multiple output cubes, it creates multiple files with one output cubes */
void write_pla(const std::string &fname, const two_lvl32 &pla)
{
	std::stringstream name;
	auto i = 0;
	for (auto &esop : pla._cubes) {
		name << fname << "_" << i++ << ".pla";
		std::ofstream out_file(name.str());
		out_file << ".i "  << pla._n_inputs << "\n";
		out_file << ".o 1\n";
		out_file << ".p " << esop.size() << "\n";
		for (auto &cube : esop) {
			out_file << cube.str(pla._n_inputs) << " 1\n";
		}
		out_file << ".e\n";
		out_file.close();
		name.str(std::string());
	}
}

} // namespace lsy

#endif
