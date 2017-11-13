/*------------------------------------------------------------------------------
| This file is distributed under the BSD 2-Clause License.
| See LICENSE for details.
*-----------------------------------------------------------------------------*/
#include <chrono>
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

	auto start = std::chrono::high_resolution_clock::now();
	if (reorder) {
		bdd_mngr.AutodynEnable();
	}
	printf("[i] Converting AIG to BDD\n");
	Gia_ManForEachCiId(aig, id, i) {
		nodes[id] = bdd_mngr.bddVar(i);
	}
	Gia_ManForEachAnd(aig, obj, i) {
		const auto b1 = nodes[Gia_ObjFaninId0(obj, i)];
		const auto c1 = Gia_ObjFaninC0(obj);
		const auto b2 = nodes[Gia_ObjFaninId1(obj, i)];
		const auto c2 = Gia_ObjFaninC1(obj);
		if (verbose) {
			fprintf(stdout, "\rNode %4d / %4lu", i,
			        nodes.size() - 1 - Gia_ManCoNum(aig));
			fflush(stdout);
		}
		nodes[i] = (c1 ? !b1 : b1) & (c2 ? !b2 : b2);
	}
	if (verbose) {
		fprintf(stdout, " [Done]\n");
	}
	/* Outputs */
	std::vector<BDD> outputs;
	Gia_ManForEachCo(aig, obj, i) {
		const auto b = nodes[Gia_ObjFaninId0p(aig, obj)];
		const auto c = Gia_ObjFaninC0(obj);
		if (verbose) {
			fprintf(stdout, "\rOutput %2d / %2d", i, Gia_ManCoNum(aig) - 1);
			fflush(stdout);
		}
		outputs.push_back(c ? !b : b);
	}
	if (verbose) {
		fprintf(stdout, " [Done]\n");
		std::chrono::duration<double> aig2bdd_time=
			std::chrono::high_resolution_clock::now() - start;

		printf("[i] Elapsed time: %f\n",  aig2bdd_time.count());
		for (auto &dd : outputs) {
			if (dd.getNode() == NULL) {
				fprintf(stdout, "empty DD.\n");
				continue;
			}
			dd.summary(Gia_ManCiNum(aig));
		}
	}
	if (reorder) {
		bdd_mngr.AutodynDisable();
	}
	return std::make_pair(bdd_mngr, outputs);
}

} // namespace lsy
