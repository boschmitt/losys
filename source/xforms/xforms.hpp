/*------------------------------------------------------------------------------
| This file is distributed under the BSD 2-Clause License.
| See LICENSE for details.
*-----------------------------------------------------------------------------*/
#ifndef LOSYS_XFORMS_HPP
#define LOSYS_XFORMS_HPP

#include <cuddObj.hh>

extern "C" {
#include "aig/gia/gia.h"
}


namespace lsy {

std::pair<Cudd, std::vector<BDD>> aig_to_bdd(Gia_Man_t *, int);
Gia_Man_t *esop_to_aig(const two_lvl32 &);

}

#endif
