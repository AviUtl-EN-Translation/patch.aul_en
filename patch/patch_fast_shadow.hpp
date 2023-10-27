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
#ifdef PATCH_SWITCH_FAST_SHADOW

#include <aviutl.hpp>
#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"
#include "global.hpp"
#include "config_rw.hpp"

namespace patch::fast {
	// init at exedit load
	// シャドーの速度アップ
	/* シャドーのバグ修正
	拡散がオブジェクトサイズを超えた場合に正常ではなくなるのを修正
	最大画像サイズ+8まで広げられてしまい、不具合を起こすのを修正
	XYと拡散が0以外の時に無駄な領域拡張が行われるのを修正
	透明度反転を掛けた時の見た目を改善
	*/
	inline class Shadow_t {
		static BOOL func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);

		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "fast.shadow";

	public:

		struct efShadow_var {
			short* buf1; // 231f90
			int diffuse1; // 231f94
			int diffuse2; // 231f98
			int intensity; // 231f9c
			short* buf2; // 231fa0
			int ox; // 231fa4
			int oy; // 231fa8
			int _undefined; // 231fac
			char _exdata_def[260]; // 231fb0
			short color_cb; // 2320b4
			short color_cr; // 2320b6
			short color_y; // 2320b8
		};


		void init() {
			enabled_i = enabled;
			if (!enabled_i)return;

			store_i32(GLOBAL::exedit_base + OFS::ExEdit::efShadow_func_proc_var_ptr, &func_proc);
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

	} Shadow;
} // namespace patch::fast
#endif // ifdef PATCH_SWITCH_FAST_SHADOW
