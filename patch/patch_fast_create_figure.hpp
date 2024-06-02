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
#ifdef PATCH_SWITCH_FAST_CREATE_FIGURE

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"
#include "global.hpp"
#include "config_rw.hpp"
#include "simd.hpp"

namespace patch::fast {
	// init at exedit load
	// ＜図形、部分フィルタ、マスク、ディスプレイスメントマップ＞の図形（円、三角、五角、六角、星）を速度アップ
	// マスク、ディスプレイスメントマップの場合に行われるぼかし処理の速度をアップ
	inline class CreateFigure_t {
		static void __cdecl CreateFigure_circle(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		static void __cdecl CreateFigure_polygons(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);

		static void __cdecl vertical_blur_mt_func_wrap(AviUtl::MultiThreadFunc mt, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		static void __cdecl horizontal_blur_mt(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		static void __cdecl vertical_blur_mt(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		static void __cdecl horizontal_blur_mt_func_wrap(AviUtl::MultiThreadFunc mt, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);

		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "fast.create_figure";

	public:

		struct CreateFigure_var { // 1e4798
			int type;
			int _padding1;
			char _module_path[260];
			int type_num;
			short color_cb; // 1e48a8
			short _padding2;
			int line_width;
			short color_cr;
			short _padding3;
			HMENU popup;
			short color_y;
			short _padding4;
			int size;
		};

		struct Map_var { // 1bace8
			int blur_size_h;
			int blur_size_w;
			int blur_range_h;
			int blur_range_w;
		};


		void init() {
			enabled_i = enabled;
			if (!enabled_i)return;

			{
				ReplaceNearJmp(GLOBAL::exedit_base + OFS::ExEdit::CreateFigure_circle_func_call, &CreateFigure_circle);
				{
					OverWriteOnProtectHelper h(GLOBAL::exedit_base + OFS::ExEdit::CreateFigure_circle_func_mt_call, 4);
					h.store_i32(0, &CreateFigure_circle);
				}

				ReplaceNearJmp(GLOBAL::exedit_base + OFS::ExEdit::CreateFigure_polygons_func_call, &CreateFigure_polygons);
				{
					OverWriteOnProtectHelper h(GLOBAL::exedit_base + OFS::ExEdit::CreateFigure_polygons_func_mt_call, 4);
					h.store_i32(0, &CreateFigure_polygons);
				}
			}
			
			if (has_flag(get_CPUCmdSet(), CPUCmdSet::F_AVX2)) {

				constexpr int vp_begin = 0x68bcf;
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x68cc4 - vp_begin);
				h.store_i16(0x68bcf - vp_begin, '\x90\xe8');
				h.replaceNearJmp(0x68bd1 - vp_begin, &vertical_blur_mt_func_wrap);

				h.store_i32(0x68c17 - vp_begin, &horizontal_blur_mt);
				h.store_i32(0x68c6b - vp_begin, &vertical_blur_mt);

				h.store_i16(0x68cbe - vp_begin, '\x90\xe8');
				h.replaceNearJmp(0x68cc0 - vp_begin, &horizontal_blur_mt_func_wrap);
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

	} create_figure;
} // namespace patch::fast
#endif // ifdef PATCH_SWITCH_FAST_CREATE_FIGURE
