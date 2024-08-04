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
#ifdef PATCH_SWITCH_FAST_COLORKEY

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"
#include "global.hpp"
#include "config_rw.hpp"

namespace patch::fast {
	// init at exedit load
	// カラーキーを少しだけ速度アップ
	// 境界補正で画像周囲や元から透明な部分の透明が広がるのを修正
	inline class Colorkey_t {

		static BOOL __cdecl func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "fast.colorkey";

	public:

		struct efColorkey_var {
			int border_range; // 11ed34
			int border_size; // 11ed38
		};

		void init() {
			enabled_i = enabled;
			if (!enabled_i)return;

			OverWriteOnProtectHelper(GLOBAL::exedit_base + 0x0a1708, 4).store_i32(0, &func_proc);
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

	} Colorkey;
} // namespace patch::fast
#endif // ifdef PATCH_SWITCH_FAST_COLORKEY
