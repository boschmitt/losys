/*------------------------------------------------------------------------------
| This file is distributed under the BSD 2-Clause License.
| See LICENSE for details.
*-----------------------------------------------------------------------------*/
#include <signal.h>
#include <cstring>
#include <string>
#include <vector>

#include "spdlog/spdlog.h"
#include "CLI/CLI.hpp"

extern "C" {
#include <aig/gia/gia.h>
#include <opt/dar/darInt.h>
#include <proof/cec/cec.h>
}

#include "base/collapse.hpp"
#include "bdd/collapse.hpp"
#include "io/write_pla.hpp"
#include "opt/exorcism32.hpp"
#include "xforms/xforms.hpp"

static void exit_SIGINT(int sig_num)
{
	if (sig_num == SIGINT) {
		auto _data = spdlog::get("data");
		if (_data != nullptr) {
			_data->flush();
		}
		spdlog::get("console")->critical("Interrupted");
	}
	exit(EXIT_SUCCESS);
}

static std::string extrac_fname(const std::string &filepath)
{
	std::string filename;
	auto pos = filepath.rfind("/");
	if (pos != std::string::npos) {
		filename = filepath.substr(pos + 1);
	} else {
		filename = filepath;
	}
	return filename;
}

static bool is_fname_ok(const std::string &filename)
{
	auto logger = spdlog::get("console");
	auto dot = filename.rfind(".");
	if (dot != std::string::npos) {
		if (filename.substr(dot + 1) == "aig")
			return true;
		logger->error("Unsupported file format: {}",
		              filename.substr(dot + 1));
	} else {
		logger->error("Unrecognized file format.");
	}
	return false;
}

int main(int argc, char **argv)
{
	auto console = spdlog::stdout_color_mt("console");
	console->set_pattern("[%L] %v");
	CLI::App app{"AIG Collapsing"};
	/* Default arguments */
	std::string method = "bdd";
	auto n_cofactor = 0;
	auto check    = false;
	auto data     = false;
	auto exorcise = false;
	auto verbose  = false;
	auto werbose  = false;
	app.add_flag("-c,--check", check, "use ABC's cec to check the result.");
	app.add_flag("-d,--data_collect", data, "turn on data collection mode.");
	app.add_flag("-e,--exorcise", exorcise, "apply exorcism in the collapsed result.");
	app.add_flag("-v,--verbose", verbose, "verbose mode.");
	app.add_flag("-w,--werbose", werbose, "very verbose mode");
	app.add_option("-f,--cofactors", n_cofactor,
	               "cofactor N variables <1 .. 8>.")->check(CLI::Range(1,8));
	app.add_set("-m,--method", method, {"aig", "bdd"}, "collapsing method.", true);
	app.allow_extras();
	app.ignore_case();

	signal(SIGINT, exit_SIGINT);

	/* TODO: consider using <filesystem> (C++17) */
	std::string filepath;
	try {
		auto extra = app.parse(argc, argv);
		if (extra.empty()) {
			console->error("No input file detected");
			return EXIT_FAILURE;
		}
		filepath = extra[0];
	} catch (const CLI::ParseError &e) {
		return app.exit(e);
	}

	/* Handling input file */
	auto filename = extrac_fname(filepath);
	if (!is_fname_ok(filename)) {
		return EXIT_FAILURE;
	}

	/* ABC don't use 'const' */
	auto abc_filepath = strdup(filepath.c_str());
	auto *aig = Gia_AigerRead(abc_filepath, 0, 0, 1);
	if (verbose) {
		Gia_ManPrintStats(aig, 0);
	}

	if (data) {
		spdlog::basic_logger_mt("data", filename + ".csv")->set_pattern("%v");
	}
	/* Cofactor the original AIG */
	Gia_Man_t *cf_aigs[(1 << n_cofactor)];
	for (auto i = 0; i < (1 << n_cofactor); ++i) {
		cf_aigs[i] = Gia_ManDup(aig);
		for (auto j = 0; j < n_cofactor; ++j) {
			cf_aigs[i] = Gia_ManDupCofactorVar(cf_aigs[i], j, ((i >> j) & 1));
		}
	}

	/* Collapse the cofactored AIGs */
	std::vector<lsy::two_lvl32> cf_results;
	auto i = 0;
	for (auto &a : cf_aigs) {
		console->info("[{} / {}] AIG", i, ((1 << n_cofactor) - 1));
		lsy::two_lvl32 ex_result;
		if (method == "bdd") {
			auto bdd = lsy::aig_to_bdd(a, verbose);
			cf_results.push_back(lsy::bdd_extract(bdd));
		} else {
			cf_results.push_back(lsy::aig_extract(a, verbose));
		}
		/* Add cofactored variables to all cubes */
		for (auto &esop : cf_results.back()._cubes) {
			for (auto &cube : esop) {
				for (auto j = 0; j < n_cofactor; ++j) {
					cube.add_lit(j, ((i >> j) & 1));
				}
			}
		}
		i++;
	}

	/* Stitch the results together */
	lsy::two_lvl32 result;
	result.n_inputs(Gia_ManCiNum(aig));
	result.n_outputs(Gia_ManCoNum(aig));
	for (auto &ret : cf_results) {
		for (auto k = 0; k < result._cubes.size(); ++k) {
			std::copy(ret._cubes[k].begin(),
				  ret._cubes[k].end(),
				  std::back_inserter(result._cubes[k]));
		}
		if (exorcise) {
			result = lsy::exorcise(result, werbose);
		}
	}
	if (verbose | werbose) {
		print_stats(result);
	}

	lsy::write_pla(filename, result);
	if (check) {
		auto result_aig = lsy::esop_to_aig(result);
		Dar_LibStart();
		Cec_ParCec_t pPars;
		Cec_ManCecSetDefaultParams(&pPars);
		auto miter = Gia_ManMiter(aig, result_aig, 0, 1, 0, 0, 0);
		if (miter == nullptr) {
			console->error("Couldn't create miter");
			return EXIT_FAILURE;
		}
		int Status = Cec_ManVerify(miter, &pPars);
		Dar_LibStop();
		Gia_ManStop(miter);
	}

	/* Leaking a bunch of stuff (: */
	return EXIT_SUCCESS;
}
