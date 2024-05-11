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

#include "patch_fast_pixelformat_conv.hpp"
#ifdef PATCH_SWITCH_FAST_PIXELFORMAT_CONV

#include "simd.hpp"

namespace patch::fast {

	void __cdecl pixelformat_conv_t::mt_sss2fbb(int thread_id, int thread_num, void* n1, void* n2) {
		auto pfc = reinterpret_cast<PixelFormatConv_var*>(GLOBAL::exedit_base + 0x1e42f4);
		int vram_w = *reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::yc_vram_w);
		int y = thread_id * pfc->h / thread_num;
		auto pix = reinterpret_cast<PixelYC_fbb*>(pfc->pix) + y * vram_w;
		int w = (pfc->w + 7) >> 3;
		int line = (vram_w - w * 8) * sizeof(*pix);

		__m256i ofs256 = _mm256_set_epi32(7 * sizeof(*pix) - 2, 6 * sizeof(*pix) - 2, 5 * sizeof(*pix) - 2, 4 * sizeof(*pix) - 2, 3 * sizeof(*pix) - 2, 2 * sizeof(*pix) - 2, 1 * sizeof(*pix) - 2, 0 * sizeof(*pix) - 2);
		__m256 intensity256 = _mm256_set1_ps((float)pfc->intensity);
		__m256 cf256 = _mm256_set1_ps(0.0625f);
		__m256 one256 = _mm256_set1_ps(1.0f);
		__m256i eight256 = _mm256_set1_epi16(8);
		__m256i min256 = _mm256_set1_epi16(-128);
		__m256i max256 = _mm256_set1_epi16(127);
		__m256i shfl256 = _mm256_set_epi8(0, 0, 14, 12, 0, 0, 10, 8, 0, 0, 6, 4, 0, 0, 2, 0, 0, 0, 14, 12, 0, 0, 10, 8, 0, 0, 6, 4, 0, 0, 2, 0);


		for (y = (thread_id + 1) * pfc->h / thread_num - y; 0 < y; y--) {
			for (int x = w; 0 < x; x--) {
				__m256i yi256 = _mm256_srai_epi32(_mm256_i32gather_epi32((int*)pix, ofs256, 1), 16);
				__m256i cbcr256 = _mm256_add_epi16(_mm256_i32gather_epi32((int*)pix + 1, ofs256, 1), eight256);
				cbcr256 = _mm256_clamp_epi16(_mm256_srai_epi16(cbcr256, 4), min256, max256);
				cbcr256 = _mm256_shuffle_epi8(cbcr256, shfl256);
				__m256 yf256 = _mm256_mul_ps(_mm256_cvtepi32_ps(yi256), cf256);
				yf256 = _mm256_sub_ps(_mm256_pow_ps(intensity256, yf256), one256);
				for (int i = 0; i < 8; i++) {
					pix->y = yf256.m256_f32[i];
					*(int16_t*)&pix->cb = cbcr256.m256i_i16[i << 1];
					pix++;
				}
			}
			pix = reinterpret_cast<decltype(pix)>((int)pix + line);
		}
	}
	void __cdecl pixelformat_conv_t::mt_ssss2fbbs(int thread_id, int thread_num, void* n1, void* n2) {
		auto pfc = reinterpret_cast<PixelFormatConv_var*>(GLOBAL::exedit_base + 0x1e42f4);
		int vram_w = *reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::yca_vram_w);
		int y = thread_id * pfc->h / thread_num;
		auto pix0 = reinterpret_cast<PixelYCA_fbbs*>(pfc->pix) + y * vram_w;

		__m256i ofs256 = _mm256_set_epi32(7 * sizeof(*pix0) - 2, 6 * sizeof(*pix0) - 2, 5 * sizeof(*pix0) - 2, 4 * sizeof(*pix0) - 2, 3 * sizeof(*pix0) - 2, 2 * sizeof(*pix0) - 2, 1 * sizeof(*pix0) - 2, 0 * sizeof(*pix0) - 2);
		__m256 intensity256 = _mm256_set1_ps((float)pfc->intensity);
		__m256 cf256 = _mm256_set1_ps(0.0625f);
		__m256 one256 = _mm256_set1_ps(1.0f);
		__m256i eight256 = _mm256_set1_epi16(8);
		__m256i min256 = _mm256_set1_epi16(-128);
		__m256i max256 = _mm256_set1_epi16(127);
		__m256i shfl256 = _mm256_set_epi8(0, 0, 14, 12, 0, 0, 10, 8, 0, 0, 6, 4, 0, 0, 2, 0, 0, 0, 14, 12, 0, 0, 10, 8, 0, 0, 6, 4, 0, 0, 2, 0);

		for (y = (thread_id + 1) * pfc->h / thread_num - y; 0 < y; y--) {
			auto pix = pix0;
			pix0 += vram_w;
			for (int x = pfc->w; 0 < x; x--) {
				if (0 < pix->a) {
					__m256i yi256 = _mm256_srai_epi32(_mm256_i32gather_epi32((int*)pix, ofs256, 1), 16);
					__m256i cbcr256 = _mm256_add_epi16(_mm256_i32gather_epi32((int*)pix + 1, ofs256, 1), eight256);
					cbcr256 = _mm256_clamp_epi16(_mm256_srai_epi16(cbcr256, 4), min256, max256);
					cbcr256 = _mm256_shuffle_epi8(cbcr256, shfl256);
					__m256 yf256 = _mm256_mul_ps(_mm256_cvtepi32_ps(yi256), cf256);
					 yf256 = _mm256_sub_ps(_mm256_pow_ps(intensity256, yf256), one256);
					for (int i = 0; i < 8; i++) {
						if (0 < pix->a) {
							pix->y = yf256.m256_f32[i];
							*(int16_t*)&pix->cb = cbcr256.m256i_i16[i << 1];
						} else {
							*(int32_t*)&pix->y = *(int32_t*)&pix->cb = 0;
						}
						pix++;
					}
					x -= 7;
				} else {
					*(int32_t*)&pix->y = *(int32_t*)&pix->cb = 0;
					pix++;
				}
			}
		}
	}

} // namespace patch::fast
#endif // ifdef PATCH_SWITCH_FAST_PIXELFORMAT_CONV
