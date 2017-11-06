/*------------------------------------------------------------------------------
| This file is distributed under the BSD 2-Clause License.
| See LICENSE for details.
*-----------------------------------------------------------------------------*/
#include <cstdio>
#include <vector>
#include <utility> /* std::pair */

#include <cuddObj.hh>
extern "C" {
#include "aig/gia/gia.h"
#include "cudd.h"
}

namespace lsy {

/* Black magic */
static inline int test(DdManager *dd, DdNode *l)
{
	if (l == NULL)
		return 0;
	DdNode *lb = l;
	::Cudd_Ref(lb);
	while (lb != ::Cudd_ReadLogicZero(dd)) {
		DdNode *implicant = ::Cudd_LargestCube(dd, lb, NULL);
		if (implicant == NULL) {
			::Cudd_RecursiveDeref(dd, lb);
			return 0;
		}
		::Cudd_Ref(implicant);

		DdNode *prime = ::Cudd_bddMakePrime(dd, implicant, l);
		if (prime == NULL) {
			::Cudd_RecursiveDeref(dd, lb);
			::Cudd_RecursiveDeref(dd, implicant);
			return 0;
		}
		::Cudd_Ref(prime);
		::Cudd_RecursiveDeref(dd, implicant);

		DdNode *tmp = ::Cudd_bddAnd(dd, lb, Cudd_Not(prime));
		if (tmp == NULL) {
			::Cudd_RecursiveDeref(dd, lb);
			::Cudd_RecursiveDeref(dd, prime);
			return 0;
		}
		::Cudd_Ref(tmp);
		::Cudd_RecursiveDeref(dd, lb);
		::Cudd_RecursiveDeref(dd, prime);
		lb = tmp;
	}
	::Cudd_RecursiveDeref(dd, lb);
	return 1;
}

std::pair<Cudd, std::vector<BDD>> aig_to_bdd(Gia_Man_t *aig, bool reorder, int verbose)
{
	uint32_t i, id;
	Gia_Obj_t* obj;
	Cudd bdd_mngr;
	std::vector<BDD> nodes(Gia_ManObjNum(aig));

	if (reorder) {
		bdd_mngr.AutodynEnable();
	}
	//bdd_mngr.AutodynEnable(CUDD_REORDER_EXACT);
	printf("[i] Converting AIG to BDD\n");
	Gia_ManForEachCiId(aig, id, i) {
		nodes[id] = bdd_mngr.bddVar(i);
	}
	Gia_ManForEachAnd(aig, obj, i) {
		const auto b1 = nodes[Gia_ObjFaninId0(obj, i)];
		const auto c1 = Gia_ObjFaninC0(obj);
		const auto b2 = nodes[Gia_ObjFaninId1(obj, i)];
		const auto c2 = Gia_ObjFaninC1(obj);
		nodes[i] = (c1 ? !b1 : b1) & (c2 ? !b2 : b2);
		if (verbose) {
			fprintf(stdout, "\rNode %4d / %4d", i, nodes.size());
			fflush(stdout);
		}
	}
	//bdd_mngr.ReduceHeap();
	//bdd_mngr.ReduceHeap(CUDD_REORDER_EXACT);
	if (verbose) {
		fprintf(stdout, " Done\n");
	}
	/* Outputs */
	std::vector<BDD> outputs;
	Gia_ManForEachCo(aig, obj, i) {
		const auto b = nodes[Gia_ObjFaninId0p(aig, obj)];
		const auto c = Gia_ObjFaninC0(obj);
		outputs.push_back(c ? !b : b);
		test(bdd_mngr.getManager(), outputs.back().getNode());
		fprintf(stdout, "[%d] ", i);
		if (outputs.back().getNode() == NULL) {
			fprintf(stdout, "empty DD.\n");
			continue;
		}
		outputs.back().summary(Gia_ManCiNum(aig));
		// outputs.back().PrintCover();
	}
	fprintf(stdout, " [Done]\n");
	if (reorder) {
		bdd_mngr.AutodynDisable();
	}
	return std::make_pair(bdd_mngr, outputs);
}

} // namespace lsy
