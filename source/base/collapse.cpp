/*------------------------------------------------------------------------------
| This file is distributed under the BSD 2-Clause License.
| See LICENSE for details.
*-----------------------------------------------------------------------------*/
#include <chrono>
#include <unordered_set>
#include <vector>

#include "spdlog/spdlog.h"

extern "C" {
#include <aig/gia/gia.h>
}

#include "collapse.hpp"
#include "kernel/cube32.hpp"
#include "kernel/two_lvl32.hpp"

namespace lsy {

aig_extr_mngr::aig_extr_mngr(Gia_Man_t *aig)
	: m_aig(aig)
{
	_clogger = spdlog::get("console");
	_dlogger = spdlog::get("data");
}

two_lvl32 aig_extr_mngr::run(bool verbose = false)
{
	using time = std::chrono::high_resolution_clock;
	m_esops.resize(Gia_ManObjNum(m_aig));
	std::uint32_t i, id;
	Gia_Obj_t *obj;

	_clogger->info_if(verbose, "Collapsing using AIG");

	if (_dlogger != nullptr) {
		_dlogger->info("# [%c] - AIG collapsing");
	}
	/* Elementary input ESOPs */
	Gia_ManForEachCiId(m_aig, id, i) {
		cube32 cube((1 << i), (1 << i));
		m_esops[id].push_back(cube);
	}

	Gia_ManForEachAnd(m_aig, obj, i) {
		auto start = time::now();
		prepare_input(Gia_ObjFaninId0(obj, i), Gia_ObjFaninC0(obj), m_esop0);
		prepare_input(Gia_ObjFaninId1(obj, i), Gia_ObjFaninC1(obj), m_esop1);
		compute_and(i);
		auto end = time::now();
		std::chrono::duration<double> duration = end - start;
		if (verbose) {
			fprintf(stdout, "\rNode %6d / %6lu, child 1: %6lu child 2: %6lu",
				i, m_esops.size(), m_esop0.size(), m_esop1.size());
			fflush(stdout);
		}
		if (_dlogger != nullptr) {
			_dlogger->info("{},{},{},{},{}", i, m_esop0.size(),
				       m_esop1.size(),m_esops[i].size(),
				       duration.count());
		}
	}
	if (verbose)
		fprintf(stdout, "\n");
	std::vector<std::vector<cube32>> ret = {};
	Gia_ManForEachCo(m_aig, obj, i) {
		auto start = time::now();
		prepare_input(Gia_ObjFaninId0p(m_aig, obj), Gia_ObjFaninC0(obj), m_esop0);
		ret.push_back(m_esop0);
		auto end = time::now();
		std::chrono::duration<double> duration = end - start;
		if (_dlogger != nullptr) {
			_dlogger->info("{},{},-1,-1,{}", i, m_esop0.size(),
				       duration.count());
		}
	}
	return {two_lvl32::kind_t::ESOP, (std::uint32_t) Gia_ManCiNum(m_aig), ret};
}

void aig_extr_mngr::prepare_input(const std::uint32_t idx, std::uint32_t cmpl, std::vector<cube32> &out)
{
	auto offset = 0;
	out.clear();
	if (cmpl) {
		if (m_esops[idx].empty())
			out.push_back(cube32_one);
		else {
			cube32 first = m_esops[idx].front();
			if (first == cube32_one)
				offset = 1;
			else if (first.n_lits() == 1) {
				first.invert();
				out.push_back(first);
				offset = 1;
			} else {
				out.push_back(cube32_one);
			}
		}
	}
	out.insert(out.end(), m_esops[idx].begin() + offset, m_esops[idx].end());
}

void aig_extr_mngr::compute_and(std::uint32_t index)
{
	/* one of the children is 0 function */
	if (m_esop0.empty() || m_esop1.empty())
		return;
	for (const auto &cube0 : m_esop0) {
		/* left child is 1 function */
		if (cube0 == cube32_one) {
			for (const auto& cube1 : m_esop1)
				add_cube(cube1);
			continue;
		}
		for (const auto &cube1 : m_esop1) {
			if (cube1 == cube32_one) {
				add_cube(cube0);
				continue;
			}
			cube32 tmp = cube0 & cube1;
			if (tmp != cube32_zero)
				add_cube(tmp);
		}
	}
	std::copy(m_curr_esop.begin(), m_curr_esop.end(), std::back_inserter(m_esops[index]));
	m_curr_esop.clear();
}

void aig_extr_mngr::add_cube(cube32 cube0)
{
	auto cont = 0;
	do {
		cont = 0;
		if (m_curr_esop.erase(cube0))
			return;
		if (cube0 == cube32_one) {
			m_curr_esop.insert(cube0);
			return;
		}
		for (auto i = 0; i < Gia_ManCiNum(m_aig); ++i) {
			auto cube1 = cube0;
			cube1.rotate(i);
			auto c_tmp = m_curr_esop.find(cube1);
			if (c_tmp != m_curr_esop.end()) {
				cube0 = merge(cube0, *c_tmp);
				m_curr_esop.erase(c_tmp);
				cont = 1;
				break;
			}
			cube1.rotate(i);
			c_tmp = m_curr_esop.find(cube1);
			if (c_tmp != m_curr_esop.end()) {
				cube0 = merge(cube0, *c_tmp);
				m_curr_esop.erase(c_tmp);
				cont = 1;
				break;
			}
		}
	} while (cont);
	m_curr_esop.insert(cube0);
}

} // namespace lsy
