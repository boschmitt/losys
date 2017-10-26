/*------------------------------------------------------------------------------
| This file is distributed under the BSD 2-Clause License.
| See LICENSE for details.
*-----------------------------------------------------------------------------*/
#include <catch.hpp>

#include "kernel/cube32.hpp"

using namespace lsy;

TEST_CASE("instantiation")
{
	auto v = cube32{};
	REQUIRE(v == cube32_one);
	SECTION("initializing single literal cubes (polarity: 1)") {
		for (auto i = 0u; i < 32; ++i) {
			v = cube32{(1u << i), (1u << i)};
			REQUIRE(v == (0x0000000100000001ull << i));
		}
	}
	/* FIXME: this particular test assume a particular endianness */
	SECTION("initializing single literal cubes (polarity: 0)") {
		for (auto i = 0u; i < 32; ++i) {
			v = cube32{(1u << i), 0u};
			REQUIRE(v == (0x0000000100000000ull << i));
		}
	}
}

TEST_CASE("and")
{
	SECTION("Adding literals into the cube through AND") {
		auto cube = cube32{};
		auto ret  = 0ull;
		for (auto i = 0u; i < 32; i += 2) {
			cube = cube & cube32{(1u << i), (1u << i)};
			ret |= (0x0000000100000001ull << i);
			REQUIRE(cube == ret);
		}
	}
	SECTION("ANDing pairs of single literal cubes") {
		for (auto i = 0u; i < 32; i += 2) {
			auto c0 = cube32{(1u << i), (1u << i)};
			auto c1 = cube32{(1u << (i + 1)), (1u << (i + 1))};
			auto ret = c0 & c1;
			REQUIRE(ret == (0x0000000300000003ull << i));
		}
	}
	SECTION("ANDing a single literal cube with it's complement") {
		for (auto i = 0u; i < 32; ++i) {
			auto c0 = cube32{(1u << i), (1u << i)};
			auto c1 = cube32{(1u << i), 0u};
			auto ret = c0 & c1;
			REQUIRE(ret == cube32_zero);
		}
	}
}
