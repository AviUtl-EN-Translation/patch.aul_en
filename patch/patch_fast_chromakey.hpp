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
#ifdef PATCH_SWITCH_FAST_CHROMAKEY

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"
#include "global.hpp"
#include "config_rw.hpp"

namespace patch::fast {
	// init at exedit load
	// クロマキーを少しだけ速度アップ
	inline class Chromakey_t {

		static void __cdecl border_color_mt1(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		static void __cdecl border_mt1(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		static void __cdecl border_mt2(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		static void __cdecl border_color_mt3(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		static void __cdecl border_mt3(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		static void __cdecl color_mt(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		static void __cdecl else_mt(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);

		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "fast.chromakey";

	public:
		
		struct efChromakey_var {
			void* buf_border; // 11ec7c
			void* buf_alpha; // 11ec80
			void* buf_hue_angle; // 11ec84
			int border_range; // 11ec88
			int border_size; // 11ec8c
		};
		
		void init() {
			enabled_i = enabled;
			if (!enabled_i)return;

			auto cpucmdset = get_CPUCmdSet();
			if (!has_flag(cpucmdset, CPUCmdSet::F_AVX2))return;
			constexpr int vp_begin = 0x12e52;
			OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x12efd - vp_begin);
			h.store_i32(0x12e52 - vp_begin, &border_color_mt1);
			h.store_i32(0x12e6a - vp_begin, &border_mt2);
			h.store_i32(0x12e7a - vp_begin, &border_color_mt3);
			h.store_i32(0x12e93 - vp_begin, &border_mt1);
			h.store_i32(0x12ea3 - vp_begin, &border_mt2);
			h.store_i32(0x12eb3 - vp_begin, &border_mt3);
			h.store_i32(0x12eda - vp_begin, &color_mt);
			h.store_i32(0x12ef9 - vp_begin, &else_mt);


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

	} Chromakey;
} // namespace patch::fast
#endif // ifdef PATCH_SWITCH_FAST_CHROMAKEY
