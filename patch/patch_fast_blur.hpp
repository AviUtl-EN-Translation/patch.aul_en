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
#include "macro.h"
#ifdef PATCH_SWITCH_FAST_BLUR

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"
#include "global.hpp"
#include "config_rw.hpp"

namespace patch::fast {
	// init at exedit load
	// ぼかしをちょっとだけ速度アップ　AVX2を使用
	// サイズ固定でぼかしサイズが大きい時のバグを修正
	inline class Blur_t {

		static BOOL __cdecl efBlur_effect_func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		
		static void __cdecl vertical_yc_fb_csa_mt_wrap(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip); // 11400
		static void __cdecl horizontal_yc_fb_csa_mt_wrap(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip); // 116d0
		static void __cdecl vertical_yc_csa_mt_wrap(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip); // fcb0
		static void __cdecl horizontal_yc_csa_mt_wrap(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip); // ff40

		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "fast.blur";

	public:

		struct efBlur_var { // 11ec34
			int h;
			int w;
			int h_range;
			int w_range;
		};


		struct fastBlurYC256 {
			__m256i range;
			__m256i y, cb, cr;
			__m256i offset;
		};

		static void __declspec(noinline) __fastcall yc256_add(fastBlurYC256* fb256, ExEdit::PixelYC* src);
		static void __declspec(noinline) __fastcall yc256_sub(fastBlurYC256* fb256, ExEdit::PixelYC* src);
		static void __declspec(noinline) __fastcall yc256_put_average(fastBlurYC256* fb256, ExEdit::PixelYC* dst, int buf_step2);


		static void blur_yc_fb_cs_mt(int n_begin, int n_end, void* buf_dst, void* buf_src, int buf_step1, int buf_step2, int obj_size, int blur_size);
		static void blur_yc_fb_csa_mt(int n_begin, int n_end, void* buf_dst, void* buf_src, int buf_step1, int buf_step2, int obj_size, int blur_size);
		static void blur_yca_fb_mt(int n_begin, int n_end, void* buf_dst, void* buf_src, int buf_step1, int buf_step2, int obj_size, int blur_size);
		static void blur_yca_fb_csa_mt(int n_begin, int n_end, void* buf_dst, void* buf_src, int buf_step1, int buf_step2, int obj_size, int blur_size);
		static void blur_yc_cs_mt(int n_begin, int n_end, void* buf_dst, void* buf_src, int buf_step1, int buf_step2, int obj_size, int blur_size);
		static void blur_yc_csa_mt(int n_begin, int n_end, void* buf_dst, void* buf_src, int buf_step1, int buf_step2, int obj_size, int blur_size);
		static void blur_yca_mt(int n_begin, int n_end, void* buf_dst, void* buf_src, int buf_step1, int buf_step2, int obj_size, int blur_size);
		static void blur_yca_csa_mt(int n_begin, int n_end, void* buf_dst, void* buf_src, int buf_step1, int buf_step2, int obj_size, int blur_size);


		void init() {
			enabled_i = enabled;
			if (!enabled_i)return;

			{ // effect
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + OFS::ExEdit::efBlur_func_proc_ptr, 4);
				h.store_i32(0, &efBlur_effect_func_proc);
			}
			{ // filter
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x0e5fe, 1204);
				h.store_i32(0x0e5fe - 0x0e5fe, &vertical_yc_fb_csa_mt_wrap);
				h.store_i32(0x0e697 - 0x0e5fe, &horizontal_yc_fb_csa_mt_wrap);
				h.store_i32(0x0e738 - 0x0e5fe, &vertical_yc_fb_csa_mt_wrap);
				h.store_i32(0x0e7cd - 0x0e5fe, &horizontal_yc_fb_csa_mt_wrap);

				h.store_i32(0x0e8c7 - 0x0e5fe, &vertical_yc_csa_mt_wrap);
				h.store_i32(0x0e964 - 0x0e5fe, &horizontal_yc_csa_mt_wrap);
				h.store_i32(0x0ea0a - 0x0e5fe, &vertical_yc_csa_mt_wrap);
				h.store_i32(0x0eaae - 0x0e5fe, &horizontal_yc_csa_mt_wrap);
			}
			{ // scene change
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x11a1c, 113);
				h.store_i32(0x11a1c - 0x11a1c, &vertical_yc_csa_mt_wrap);
				h.store_i32(0x11a4f - 0x11a1c, &horizontal_yc_csa_mt_wrap);
				h.store_i32(0x11a89 - 0x11a1c, &vertical_yc_csa_mt_wrap);
				h.store_i32(0x11abc - 0x11a1c, &horizontal_yc_csa_mt_wrap);
			}
			
		}

		void switching(bool flag) { enabled = flag; }

		bool is_enabled() { return enabled; }
		bool is_enabled_i() { return enabled_i; }

		void switch_load(ConfigReader& cr) {
			cr.regist(key, [this](json_value_s* value) {
				ConfigReader::load_variable(value, enabled);
				});
		}

		void switch_store(ConfigWriter& cw) {
			cw.append(key, enabled);
		}

	} Blur;
} // namespace patch::fast
#endif // ifdef PATCH_SWITCH_FAST_BLUR
