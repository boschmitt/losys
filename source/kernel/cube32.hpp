/*------------------------------------------------------------------------------
| This file is distributed under the BSD 2-Clause License.
| See LICENSE for details.
*-----------------------------------------------------------------------------*/
#ifndef LOSYS_CUBE32_HPP
#define LOSYS_CUBE32_HPP

#include <cassert>
#include <cstdint>

namespace lsy {

/*------------------------------------------------------------------------------
| cube32
| ------
| TLDR: efficient cube data structure for Boolean functions with up to 32
|       variables
|
| A cube is the Boolean AND or 'k' literals. Given a Boolean function with N
| variables, there are 2*N literals. In a cube of this function each literal
| can either appear as a positive literal ('1'), as a negative literal ('0') or
| not appear at all (don't care, '-').
|
| Since each literal can assume one to these three values, we need at least two
| bits to represent the state of each literal. Thus, if we limit the number of
| variables in the function to 32, it's clear we only need 64 bits to encode a
| cube.
|
| Here, 'mask' tells us if a literal appears ('1') or not ('0' - don't care).
| For the cases where the literal appears in the cube, the 'polarity' indicates
| it's polarity (duh :)
|
| * A constant 1 is represented by all literal being don't care.
*-----------------------------------------------------------------------------*/
struct cube32 {
	using ui32_t = std::uint32_t;
	using ui64_t = std::uint64_t;
	using c32_t  = struct cube32;

	union {
		struct {
			ui32_t polarity;
			ui32_t mask;
		};
		ui64_t value;
	};

	cube32()
	: value{0u}
	{ }

	cube32(const ui64_t v)
	: value{v}
	{ }

	constexpr cube32(const ui32_t m, const ui32_t p)
	: polarity{p}, mask{m}
	{ }

	bool operator==(const c32_t that) const
	{ return value == that.value; }

	bool operator!=(const c32_t that) const
	{ return value != that.value; }

	bool operator< (const c32_t that) const
	{ return value <  that.value; }

	bool operator==(const ui64_t v) const
	{ return value == v; }

	bool operator!=(const ui64_t v) const
	{ return value != v; }

	c32_t operator&(const c32_t that) const
	{
		const auto tmp_mask = mask & that.mask;
		if ((polarity ^ that.polarity) & tmp_mask) {
			return cube32{0xFFFFFFFFu, 0x00000000u};
		}
		return cube32{value | that.value};
	}

	ui32_t n_lits() const
	{ return __builtin_popcount(mask); }

	void add_lit(const ui32_t var, const ui32_t p)
	{
		assert(p <= 1);
		mask |= (1 << var);
		polarity |= (p << var);
	}
};

constexpr auto cube32_zero = cube32{0xFFFFFFFFu, 0x00000000u};
constexpr auto cube32_one  = cube32{0u, 0u};

} // namespace lsy

#endif
