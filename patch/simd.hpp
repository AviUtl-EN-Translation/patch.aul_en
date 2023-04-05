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
	inline static __m256i _mm256_mod_epi32(__m256i dividend,__m256i divisor) {
		__m256i div = _mm256_div_epi32(dividend, divisor);
		__m256i mul = _mm256_mullo_epi32(div, divisor);
		return _mm256_sub_epi32(dividend, mul);
		/*
		return _mm256_set_epi32(
			dividend.m256i_u32[0]%divisor.m256i_i32[0],
			dividend.m256i_u32[6] % divisor.m256i_i32[6], 
			dividend.m256i_u32[5] % divisor.m256i_i32[5],
			dividend.m256i_u32[4] % divisor.m256i_i32[4],
			dividend.m256i_u32[3] % divisor.m256i_i32[3],
			dividend.m256i_u32[2] % divisor.m256i_i32[2],
			dividend.m256i_u32[1] % divisor.m256i_i32[1], 
			dividend.m256i_u32[0] % divisor.m256i_i32[0]);
			*/
	}


	inline static __m256i _mm256_clamp_epi32(__m256i n, __m256i min, __m256i max) {
		return _mm256_min_epi32(_mm256_max_epi32(min, n), max);
	}
};
