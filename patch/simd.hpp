/*
	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU Lesser General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include <immintrin.h>

class simd {
public:

#define  _mm256_mod_epi32(dividend, divisor) _mm256_sub_epi32(dividend, _mm256_mullo_epi32(_mm256_div_epi32(dividend, divisor), divisor));

#define _mm256_clamp_epi32(n, min, max) _mm256_min_epi32(_mm256_max_epi32(n, min), max)

};
