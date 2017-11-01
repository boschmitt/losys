/*------------------------------------------------------------------------------
| This file is distributed under the BSD 2-Clause License.
| See LICENSE for details.
*-----------------------------------------------------------------------------*/
#include <cassert>
#include <map>
#include <vector>
#include <unordered_set>
#include <utility> /* std::pair */

extern "C" {
#include <cudd.h>
}
#include <cuddObj.hh>

#include "bdd/collapse.hpp"
#include "kernel/cube32.hpp"

namespace lsy {

psdkro::psdkro(DdManager *cudd, uint32_t size)
	: m_cudd(cudd), m_var_values(size, UNUSED)
{ }

std::vector<cube32> psdkro::generate_exact(DdNode *f)
{
	if (f == NULL)
		return {};
	m_exp_costs.clear();
	m_esop.clear();
	std::fill(m_var_values.begin(), m_var_values.end(), UNUSED);
	count_cubes(f);
	generate_exact(f, -1);
	std::vector<cube32> ret;
	std::copy(m_esop.begin(), m_esop.end(), std::back_inserter(ret));
	return ret;
}

void psdkro::add_cube(const cube32 c)
{
	auto c1 = c;
	auto cont = 0;
	do {
		cont=0;
		uint32_t tmp = m_esop.erase(c1);
		if (tmp) {
			return;
		}

		if (c1 == cube32_one) {
			m_esop.insert(c1);
			return;
		}

		for (uint32_t i = 0; i < m_var_values.size(); ++i) {
			auto c2 = c1;
			c2.rotate(i);
			auto c_tmp = m_esop.find(c2);
			if (c_tmp != m_esop.end() && *c_tmp == c2) {
				c1 = merge(c1, *c_tmp);
				m_esop.erase(c_tmp);
				cont=1;
				break;
			}
			c2.rotate(i);
			c_tmp = m_esop.find(c2);
			if (c_tmp != m_esop.end() && *c_tmp == c2) {
				c1 = merge(c1, *c_tmp);
				m_esop.erase(c_tmp);
				cont=1;
				break;
			}
		}
	} while (cont);
	m_esop.insert(c1);
}

void psdkro::generate_exact(DdNode *f, int prev_idx)
{
	/* Terminal cases */
	if (f == Cudd_ReadLogicZero(m_cudd))
		return;
	if (f == Cudd_ReadOne(m_cudd)) {
		cube32 cube(0u);
		for (auto i = 0; i < prev_idx + 1; ++i) {
			cube.mask |= ((m_var_values[i] != UNUSED) << i);
			cube.polarity |= ((m_var_values[i] == POSITIVE) << i);
		}
		// add_cube(cube);
		m_esop.insert(cube);
		return;
	}
	/* Find the best expansion by a cache lookup */
	assert(m_exp_costs.find(f) != m_exp_costs.end());
	exp_type expension = m_exp_costs[f].first;

	/* Determine the top-most variable */
	auto idx = Cudd_NodeReadIndex(f);
	/* Clear intermediate variables that have not been used */
	for (auto i = prev_idx + 1; i < idx; ++i) {
		m_var_values[i] = UNUSED;
	}

	/* Determine cofactors */
	DdNode *f0 = Cudd_NotCond(Cudd_E(f), Cudd_IsComplement(f));
	DdNode *f1 = Cudd_NotCond(Cudd_T(f), Cudd_IsComplement(f));
	DdNode *f2 = Cudd_bddXor(m_cudd, f0, f1);
	Cudd_Ref(f2);

	/* Generate cubes in the left/right branches */
	if (expension == POSITIVE_DAVIO) {
		m_var_values[idx] = UNUSED;
		generate_exact(f0, idx);
		m_var_values[idx] = POSITIVE;
		generate_exact(f2, idx);
	} else if (expension == NEGATIVE_DAVIO) {
		m_var_values[idx] = UNUSED;
		generate_exact(f1, idx);
		m_var_values[idx] = NEGATIVE;
		generate_exact(f2, idx);
	} else { /* SHANNON */
		m_var_values[idx] = NEGATIVE;
		generate_exact(f0, idx);
		m_var_values[idx] = POSITIVE;
		generate_exact(f1, idx);
	}
	Cudd_RecursiveDeref(m_cudd, f2);
}

/* Recursive function */
std::pair<psdkro::exp_type, std::uint32_t> psdkro::count_cubes(DdNode *f)
{
	/* Check for terminal cases */
	if (f == Cudd_ReadLogicZero(m_cudd))
		return std::make_pair(POSITIVE_DAVIO, 0u);
	if (f == Cudd_ReadOne(m_cudd))
		return std::make_pair(POSITIVE_DAVIO, 1u);
	auto it = m_exp_costs.find(f);
	if (it != m_exp_costs.end()) {
		return it->second;
	}

	/* Determine cofactors */
	DdNode *f0 = Cudd_NotCond(Cudd_E(f), Cudd_IsComplement(f));
	DdNode *f1 = Cudd_NotCond(Cudd_T(f), Cudd_IsComplement(f));
	DdNode *f2 = Cudd_bddXor(m_cudd, f0, f1);
	Cudd_Ref(f2);

	/* Recursively solve subproblems */
	std::uint32_t n0 = count_cubes(f0).second;
	std::uint32_t n1 = count_cubes(f1).second;
	std::uint32_t n2 = count_cubes(f2).second;

	/* Determine the mostly costly expansion */
	std::uint32_t n_max = std::max(std::max(n0, n1), n2);

	/* Choose the least costly expansion */
	std::pair<exp_type, std::uint32_t> ret;
	if (n_max == n0) {
		ret = std::make_pair(NEGATIVE_DAVIO, n1 + n2);
	} else if (n_max == n1) {
		ret = std::make_pair(POSITIVE_DAVIO, n0 + n2);
	} else {
		ret = std::make_pair(SHANNON, n0 + n1);
	}
	m_exp_costs[f] = ret;
	return ret;
}

} // namespace lsy
