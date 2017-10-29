/*------------------------------------------------------------------------------
| This file is distributed under the BSD 2-Clause License.
| See LICENSE for details.
*-----------------------------------------------------------------------------*/
#include <algorithm>
#include <cassert>
#include <chrono>
#include <unordered_set>
#include <vector>

#include "kernel/cube32.hpp"
#include "exorcism32.hpp"

namespace lsy {

static std::array<cube32, 4>
exorlink(cube32 c0, cube32 c1, std::uint32_t dist, std::uint32_t *group)
{
	assert(dist <= 4);
	auto diff = difference(c0, c1);
	std::array<cube32, 4> ret;
	if (c0.value > c1.value)
		std::swap(c0.value, c1.value);
	auto c_polarity = ~(c0.polarity) & ~(c1.polarity);
	auto c_mask = c0.mask ^ c1.mask;
	for (auto i = 0; i < dist; ++i) {
		auto tmp_polarity = c0.polarity;
		auto tmp_mask = c0.mask;
		auto tmp_pos  = diff;
		for (auto j = 0; j < dist; ++j) {
			/* compute next position */
			std::uint64_t p = tmp_pos & -(tmp_pos);
			tmp_pos &= tmp_pos - 1;
			switch (*group++) {
			case 0:
				/* take from c0 */
				break;
			case 1:
				/* take from c1 */
				tmp_polarity ^= ((c1.polarity & p) ^ tmp_polarity) & p;
				tmp_mask ^= ((c1.mask & p) ^ tmp_mask) & p;
				break;
			case 2:
				/* take other */
				tmp_polarity ^= ((c_polarity & p) ^ tmp_polarity) & p;
				tmp_mask ^= ((c_mask & p) ^ tmp_mask) & p;
				break;
			}
		}
		ret[i].polarity = tmp_polarity;
		ret[i].mask = tmp_mask;
	}
	return ret;
}

void exorcism_mngr::pairs_bookmark()
{
	m_pairs_bookmark[0] = m_pairs[0].size();
	m_pairs_bookmark[1] = m_pairs[1].size();
}

void exorcism_mngr::pairs_rollback()
{
	m_pairs[0].resize(m_pairs_bookmark[0]);
	m_pairs[1].resize(m_pairs_bookmark[1]);
}

std::uint32_t exorcism_mngr::n_cubes()
{
	std::uint32_t n_cubes = 0;
	for (auto &buckt : m_cubes)
		n_cubes += buckt.size();
	return n_cubes;
}

int exorcism_mngr::add_cube(const cube32 &c, bool add = true)
{
	m_pairs_tmp[0].clear();
	m_pairs_tmp[1].clear();

	const auto n_lits = c.n_lits();
	auto begin = std::max((int)(n_lits - m_max_dist), 0);
	auto end = std::min(m_n_vars, n_lits + m_max_dist);
	for (auto i = begin; i <= end; ++i) {
		for (auto it : m_cubes[i]) {
			const auto dist = distance(c, it);
			if (dist == 1) {
				auto new_cube = merge(c, it);
				m_cubes[i].erase(it);
				return add_cube(new_cube) + 1;
			} else if (dist == 0) {
				m_cubes[i].erase(it);
				m_pairs_tmp[0].clear();
				return 2;
			} else if (dist <= m_max_dist) {
				m_pairs_tmp[dist - 2].push_back(std::make_pair(c, it));
			}
		}
	}
	if (add) {
		m_cubes[n_lits].insert(c);
	}
	for (auto d = 0; d <= (m_max_dist - 2); ++d) {
		std::copy(m_pairs_tmp[d].begin(), m_pairs_tmp[d].end(), std::back_inserter(m_pairs[d]));
	}
	return 0;
}

unsigned exorcism_mngr::exorlink2()
{
	std::uint32_t n_attempts = 0;
	std::uint32_t n_reshapes = 0;
	std::uint32_t old_size = n_cubes();
	auto &pairs = m_pairs[0];
	auto n_pairs = pairs.size();
	for (auto i = 0; i < n_pairs; ++i) {
		const auto cube_pair = pairs.front();
		pairs.erase(pairs.begin());
		const cube32 cube0 = cube_pair.first;
		const cube32 cube1 = cube_pair.second;
		std::uint32_t cube0_sz = cube0.n_lits();
		std::uint32_t cube1_sz = cube1.n_lits();

		// Remove pair and cubes (cube0, cube1) for now
		auto cube0_it = m_cubes[cube0_sz].find(cube0);
		auto cube1_it = m_cubes[cube1_sz].find(cube1);
		if (cube0_it == m_cubes[cube0_sz].end() || cube1_it == m_cubes[cube1_sz].end())
			continue;
		m_cubes[cube0_sz].erase(cube0_it);
		m_cubes[cube1_sz].erase(cube1_it);

		pairs_bookmark();
		auto n = exorlink(cube0, cube1, 2, &cube_groups2[0]);
		++n_attempts;
		++n_reshapes;
		if (add_cube(n[0], false)) {
			add_cube(n[1]);
		} else if (add_cube(n[1], false)) {
			add_cube(n[0]);
		} else {
			pairs_rollback();
			n = exorlink(cube0, cube1, 2, &cube_groups2[4]);
			if (add_cube(n[0], false)) {
				add_cube(n[1]);
			} else if (add_cube(n[1], false)) {
				add_cube(n[0]);
			} else {
				/* TODO: lit minimization ? */
				m_cubes[cube0_sz].insert(cube0);
				m_cubes[cube1_sz].insert(cube1);
				--n_reshapes;
				pairs_rollback();
				pairs.push_back(cube_pair);
			}
		}
	}
	auto curr_size = n_cubes();
	if (m_verbose) {
		fprintf(stdout, "ExorLink-2");
		fprintf(stdout, ": Que= %5lu", n_pairs);
		fprintf(stdout, "  Att= %4u", n_attempts);
		fprintf(stdout, "  Resh= %4u", n_reshapes);
		fprintf(stdout, "  NoResh= %4d", n_attempts - n_reshapes);
		fprintf(stdout, "  Cubes= %3d", curr_size);
		fprintf(stdout, "  (%d)", old_size - curr_size);
		fprintf(stdout, "\n");
	}
	return old_size - curr_size;
}

unsigned exorcism_mngr::exorlink3()
{
	std::uint32_t n_attempts = 0;
	std::uint32_t n_reshapes = 0;
	std::uint32_t old_size = n_cubes();
	auto &pairs = m_pairs[1];
	auto n_pairs = pairs.size();

	for (auto i = 0; i < n_pairs; ++i) {
		const auto cube_pair = pairs.front();
		pairs.erase(pairs.begin());
		const cube32 cube0 = cube_pair.first;
		const cube32 cube1 = cube_pair.second;
		std::uint32_t cube0_sz = cube0.n_lits();
		std::uint32_t cube1_sz = cube1.n_lits();

		// Remove pair and cubes (cube0, cube1) for now
		auto cube0_it = m_cubes[cube0_sz].find(cube0);
		auto cube1_it = m_cubes[cube1_sz].find(cube1);
		if (cube0_it == m_cubes[cube0_sz].end() || cube1_it == m_cubes[cube1_sz].end())
			continue;
		m_cubes[cube0_sz].erase(cube0_it);
		m_cubes[cube1_sz].erase(cube1_it);

		pairs_bookmark();
		++n_attempts;
		bool found = false;
		for (auto g = 0u; g < 54u; g += 9u) {
			const auto n = exorlink(cube0, cube1, 3, &cube_groups3[g]);
			for (auto j = 0u; j < 3u; ++j) {
				const auto gain = add_cube(n[j], false);
				if (gain >= 1) {
					for (auto k = 0u; k < 3u; ++k) {
						if (j != k)
							add_cube(n[k]);
					}
					goto END_LOOP;
				}
				pairs_rollback();
			}
		}
		m_cubes[cube0_sz].insert(cube0);
		m_cubes[cube1_sz].insert(cube1);
END_LOOP: {}
	}
	auto curr_size = n_cubes();
	if (m_verbose) {
		fprintf(stdout, "ExorLink-3");
		fprintf(stdout, ": Que= %5lu", n_pairs);
		fprintf(stdout, "  Att= %4u", n_attempts);
		fprintf(stdout, "  Resh= %4u", n_reshapes);
		fprintf(stdout, "  NoResh= %4d", n_attempts - n_reshapes);
		fprintf(stdout, "  Cubes= %3d", curr_size);
		fprintf(stdout, "  (%d)", old_size - curr_size);
		fprintf(stdout, "\n");
	}
	return old_size - curr_size;
}

exorcism_mngr::exorcism_mngr(const std::vector<cube32> &original, std::uint32_t n_vars, bool verbose)
	: m_cubes(n_vars + 1),
	  m_n_vars(n_vars),
	  m_max_dist(3),
	  m_pairs(2),
	  m_pairs_tmp(2),
	  m_pairs_bookmark({0, 0, 0, 0}),
	  m_verbose(verbose)
{
	for (auto &pairs : m_pairs)
		pairs.reserve(original.size() * original.size());
	for (const auto &c : original)
		add_cube(c);
}

std::vector<cube32> exorcism_mngr::run()
{
	auto gain = 0;
	auto without_improv = 0;
	auto iteration = 0;

	do {
		if (m_verbose)
			fprintf(stdout, "\nITERATION: #%2d\n\n", iteration++);
		gain = 0;
		gain += exorlink2();
		gain += exorlink3();
		gain += exorlink2();
		gain += exorlink3();
		gain += exorlink2();
		gain += exorlink3();
		gain += exorlink2();
		gain += exorlink3();
		gain += exorlink2();
		gain += exorlink3();
		gain += exorlink2();
		gain += exorlink3();
		if (gain > 0)
			without_improv = 0;
		else
			++without_improv;
	} while (without_improv <= 2);

	if (m_verbose)
		fprintf(stdout, "\n");
	std::vector<cube32> result;
	for (auto buckt : m_cubes)
		for (auto cube : buckt)
			result.push_back(cube);
	return result;
}

} // namespace lsy
