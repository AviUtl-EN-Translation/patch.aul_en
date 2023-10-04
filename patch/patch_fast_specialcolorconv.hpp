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
#ifdef PATCH_SWITCH_FAST_SPECIALCOLORCONV

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"
#include "global.hpp"
#include "config_rw.hpp"

namespace patch::fast {
	// init at exedit load
	// 特定色域変換を速度アップ
	inline class SpecialColorConv_t {

		static void __cdecl mt_border(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		static void __cdecl mt(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);

		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "fast.specialcolorconv";

	public:
		/*
		struct efSpecialColorConv_var { // 11ecfc
			short* buf_border;
			short* buf_alpha;
			int border_range;
			int border_size;
		};
		*/

		void init() {
			enabled_i = enabled;
			if (!enabled_i)return;

			auto cpucmdset = get_CPUCmdSet();
			if (!has_flag(cpucmdset, CPUCmdSet::F_AVX2))return;
			constexpr int vp_begin = 0x154ab;
			OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x15528 - vp_begin);
			h.store_i32(0x154ab - vp_begin, &mt_border);
			h.store_i32(0x15524 - vp_begin, &mt);


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

	} SpecialColorConv;
} // namespace patch::fast
#endif // ifdef PATCH_SWITCH_FAST_SPECIALCOLORCONV
