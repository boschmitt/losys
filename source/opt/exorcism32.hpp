/*------------------------------------------------------------------------------
| This file is distributed under the BSD 2-Clause License.
| See LICENSE for details.
*-----------------------------------------------------------------------------*/
#ifndef LOSYS_EXORCISM_HPP
#define LOSYS_EXORCISM_HPP

#include <array>
#include <chrono>
#include <unordered_set>
#include <vector>

#include "kernel/cube32.hpp"
#include "kernel/two_lvl32.hpp"

namespace lsy {

/*------------------------------------------------------------------------------
| Exorcism manager
|
| TODO: add support for multiple output functions.
*-----------------------------------------------------------------------------*/
class exorcism_mngr {
public:
	exorcism_mngr(const std::vector<cube32> &, std::uint32_t, bool);
	std::vector<cube32> run();

private:
	std::uint32_t n_cubes();
	int add_cube(const cube32 &, bool);
	int find_pairs(const cube32 &, std::uint32_t);
	unsigned exorlink2();
	unsigned exorlink3();

	void pairs_bookmark();
	void pairs_rollback();

private:
	bool m_verbose;
	typedef std::unordered_set<cube32, cube32_hash> hash_bucket;
	std::vector<hash_bucket> m_cubes;
	std::uint32_t m_n_vars;

	std::vector<std::vector<std::pair<cube32, cube32>>> m_pairs;
	std::vector<std::vector<std::pair<cube32, cube32>>> m_pairs_tmp;

	/* Bookkeeping */
	std::array<std::uint32_t, 4> m_pairs_bookmark;

	/* Algorithm Control */
	std::uint32_t m_max_dist;

	unsigned cube_groups2[8] = {2, 0, 1, 2,
                                    0, 2, 2, 1};

	unsigned cube_groups3[54] = {2, 0, 0, 1, 2, 0, 1, 1, 2,
	                             2, 0, 0, 1, 0, 2, 1, 2, 1,
	                             0, 2, 0, 2, 1, 0, 1, 1, 2,
	                             0, 2, 0, 0, 1, 2, 2, 1, 1,
	                             0, 0, 2, 2, 0, 1, 1, 2, 1,
	                             0, 0, 2, 0, 2, 1, 2, 1, 1};
};

static two_lvl32 exorcise(const two_lvl32 &original, bool verbose = false)
{
	printf("[i] Exorcism\n");
	std::vector<std::vector<cube32>> ret;
	for (auto &esop : original._cubes) {
		exorcism_mngr exor(esop, original._n_inputs, verbose);
		ret.push_back(exor.run());

	}
	return {original._kind, original._n_inputs, ret};
}

}

#endif
