/*------------------------------------------------------------------------------
| This file is distributed under the BSD 2-Clause License.
| See LICENSE for details.
*-----------------------------------------------------------------------------*/
#include <cstdio>
#include <vector>

extern "C" {
#include <aig/gia/gia.h>
}

#include "kernel/cube32.hpp"
#include "kernel/two_lvl32.hpp"

namespace lsy {

Gia_Man_t *esop_to_aig(const two_lvl32 &esop)
{
	Gia_Man_t *aig;
	aig = Gia_ManStart(128);
	Gia_ManHashAlloc(aig);
	for (auto i = 0; i < esop._n_inputs; ++i)
		Gia_ManAppendCi(aig);
	for (auto i = 0; i < esop._cubes.size(); ++i) {
		auto cubes0 = esop._cubes[i];
		if (cubes0.empty()) {
			Gia_ManAppendCo(aig, 0);
			continue;
		}
		auto root_idx = 0;
		for (auto c : cubes0) {
			int Lit, and_idx = 1;
			if (c != cube32_one) {
				for (auto k = 0; k < esop._n_inputs; ++k) {
					if (c.mask & (1 << k)) {
						Lit = Abc_Var2Lit(k, !((c.polarity >> k) & 1));
						and_idx = Gia_ManHashAnd(aig, and_idx, Lit + 2);
					}
				}
			}
			root_idx = Gia_ManHashXor(aig, root_idx, and_idx);
		}
		Gia_ManAppendCo(aig, root_idx);
	}
	/* Cleanup */
	Gia_Man_t *tmp = aig;
	// aig = Gia_ManBalance(aig, 0, 0, 0);
	aig = Gia_ManCleanup(tmp);
	Gia_ManStop(tmp);
	return aig;
}

} // namespace lsy
