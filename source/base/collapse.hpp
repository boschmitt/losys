/*------------------------------------------------------------------------------
| This file is distributed under the BSD 2-Clause License.
| See LICENSE for details.
*-----------------------------------------------------------------------------*/
#ifndef LOSYS_AIG_COLLAPSE_H
#define LOSYS_AIG_COLLAPSE_H

#include <chrono>
#include <unordered_set>
#include <vector>

#include "spdlog/spdlog.h"

extern "C" {
#include <aig/gia/gia.h>
}

#include "kernel/cube32.hpp"
#include "kernel/two_lvl32.hpp"

namespace lsy {

/*------------------------------------------------------------------------------
| AIG Extract manager
*-----------------------------------------------------------------------------*/
class aig_extr_mngr {
public:
	aig_extr_mngr(Gia_Man_t *);
	two_lvl32 run(bool);

private:
	void prepare_input(const std::uint32_t, std::uint32_t,
	                   std::vector<cube32> &);
	void compute_and(const std::uint32_t);
	void compute_xor(const std::uint32_t);
	void add_cube(cube32);
	void debug() const;

private:
	/* Loggers (Console, Data) */
	std::shared_ptr<spdlog::logger> _clogger;
	std::shared_ptr<spdlog::logger> _dlogger;

	Gia_Man_t *m_aig;
	std::vector<std::vector<cube32>> m_esops;
	/* Temporary */
	std::vector<cube32> m_esop0;
	std::vector<cube32> m_esop1;
	std::unordered_set<cube32, cube32_hash> m_curr_esop;
};

static two_lvl32 aig_extract(Gia_Man_t *aig, bool verbose)
{
	if (Gia_ManCiNum(aig) > 32) {
		spdlog::get("console")
			->info("Cannot handle more than 32 input variables");
		exit(0);
	}
	aig_extr_mngr mngr(aig);
	auto start = std::chrono::high_resolution_clock::now();
	auto ret = mngr.run(verbose);
	std::chrono::duration<double> aig2esop_time =
		std::chrono::high_resolution_clock::now() - start;
	printf("[i] Elapsed time: %f\n",  aig2esop_time.count());
	return ret;
}

} // namespace lsy

#endif
