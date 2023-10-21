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
#ifdef PATCH_SWITCH_FAST_COLORDRIFT

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"
#include "global.hpp"
#include "config_rw.hpp"

namespace patch::fast {
	// init at exedit load
	// 色ずれを少しだけ速度アップ
	inline class ColorDrift_t {
		static void __cdecl media_mt1(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		static void __cdecl media_mt2(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);

		static void __cdecl filter_mt1(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		static void __cdecl filter_mt2(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);

		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "fast.colordrift";

	public:

		struct efColorDrift_var { // 11ed88
			int intensity; // 11ed88
			int type; // 11ed8c
			int b_y; // 11ed90
			int b_x; // 11ed94
			int g_y; // 11ed98
			int g_x; // 11ed9c
			int r_x; // 11eda0
			int r_y; // 11eda4
			int w; // 11eda8
			int h; // 11edac
			int ox; // 11edb0
			int oy; // 11edb4
		};

		void init() {
			enabled_i = enabled;
			if (!enabled_i)return;

			constexpr int vp_begin = 0x1725f;
			OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x17372 - vp_begin);
			h.store_i32(0x1725f - vp_begin, &media_mt1);
			h.store_i32(0x172ab - vp_begin, &media_mt2);
			h.store_i32(0x1733e - vp_begin, &filter_mt1);
			h.store_i32(0x1736e - vp_begin, &filter_mt2);
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

	} ColorDrift;
} // namespace patch::fast
#endif // ifdef PATCH_SWITCH_FAST_COLORDRIFT
