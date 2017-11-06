/*------------------------------------------------------------------------------
| This file is distributed under the BSD 2-Clause License.
| See LICENSE for details.
*-----------------------------------------------------------------------------*/
#ifndef LOSYS_BDD_COLLAPSE_HPP
#define LOSYS_BDD_COLLAPSE_HPP

#include <chrono>
#include <cstdint>
#include <map>
#include <vector>
#include <unordered_set>
#include <utility> /* std::pair */

#include <cuddObj.hh>
extern "C" {
#include <cudd.h>
}

#include "kernel/cube32.hpp"
#include "kernel/two_lvl32.hpp"

namespace lsy {

/*------------------------------------------------------------------------------
| Pseudo-Kronecker (PSDKRO) expressions
*-----------------------------------------------------------------------------*/
class psdkro {
public:
	psdkro(DdManager *, std::uint32_t);
	std::vector<cube32> extract_esop(DdNode *);

private:
	enum var_value : std::uint8_t {
		POSITIVE,
		NEGATIVE,
		UNUSED
	};
	enum exp_type : std::uint8_t {
		POSITIVE_DAVIO,
		NEGATIVE_DAVIO,
		SHANNON
	};

	void add_cube(const cube32);
	void generate_exact(DdNode *);

	/* Recursive function */
	std::pair<exp_type, std::uint32_t> count_cubes(DdNode *);

private:
	DdManager *m_cudd;
	std::vector<std::uint32_t> m_vars;
	std::vector<var_value> m_var_values;
	std::map<DdNode *, std::pair<exp_type, std::uint32_t>> m_exp_costs;
	std::unordered_set<cube32, cube32_hash> m_esop;
};

static two_lvl32 bdd_extract(std::pair<Cudd, std::vector<BDD>> &bdd)
{
	if (bdd.first.ReadSize() > 32) {
		fprintf(stdout, "Cannot handle more than 32 input variables\n");
		exit(0);
	}
	printf("[i] Collapsing using BDD\n");
	psdkro mngr(bdd.first.getManager(), bdd.first.ReadSize());
	std::vector<std::vector<cube32>> fncts;
	auto start = std::chrono::high_resolution_clock::now();
	for (auto &i : bdd.second) {
		fncts.push_back(mngr.extract_esop(i.getNode()));
	}
	std::chrono::duration<double> bdd2esop_time =
		std::chrono::high_resolution_clock::now() - start;

	printf("[i] Elapsed time: %f\n",  bdd2esop_time.count());
	return {two_lvl32::kind_t::ESOP, (uint32_t) bdd.first.ReadSize(), fncts};
}

} // namespace lsy
#endif
