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

#ifdef PATCH_SWITCH_SETTINGDIALOG_CHROMAKEY
#include <exedit.hpp>
#include "global.hpp"
#include "util.hpp"
#include "config_rw.hpp"
#include "restorable_patch.hpp"

namespace patch {
	// init at exedit load
	// クロマキーの透過補正チェックの位置を変更&色彩補正がONでなければ隠す
	// EnableWindowで無効化した場合、func_window_hide()を作って有効化するコストがかかり面倒なため、ShowWindowにて隠す方針にした
	inline class dialog_chromakey_t {

		std::optional<restorable_patch_i8> rp144b0;
		std::optional<restorable_patch_i32> rp144b1;
		std::optional<restorable_patch_i8> rp14571;
		std::optional<restorable_patch_i32> rp14572;

		static int __cdecl dialog_init_wrap(HINSTANCE hinstance, HWND hwnd, int y, int base_id, int sw_param, ExEdit::Filter* efp);
		static BOOL __cdecl dialog_WndProc_wrap(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, void* editp, ExEdit::Filter* efp);
		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "settingdialog_chromakey";

	public:

		void init() {
			enabled_i = enabled;
			
			rp144b0.emplace(GLOBAL::exedit_base + 0x144b0, 0xe9);
			rp144b1.emplace(GLOBAL::exedit_base + 0x144b1, (int)&dialog_WndProc_wrap - GLOBAL::exedit_base - 0x144b5);
			rp14571.emplace(GLOBAL::exedit_base + 0x14571, 0xe9);
			rp14572.emplace(GLOBAL::exedit_base + 0x14572, (int)&dialog_init_wrap - GLOBAL::exedit_base - 0x14576);

			rp144b0->switching(enabled);
			rp144b1->switching(enabled);
			rp14571->switching(enabled);
			rp14572->switching(enabled);


			/*
			if (!enabled_i) return;

			constexpr int vp_begin = 0x144b0;
			OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x14576 - vp_begin);
			h.store_i8(0x144b0 - vp_begin, '\xe9');
			h.replaceNearJmp(0x144b1 - vp_begin, &dialog_WndProc_wrap);
			h.store_i8(0x14571 - vp_begin, '\xe9');
			h.replaceNearJmp(0x14572 - vp_begin, &dialog_init_wrap);
			*/
		}

		void switching(bool flag) {
			enabled = flag;
			rp144b0->switching(flag);
			rp144b1->switching(flag);
			rp14571->switching(flag);
			rp14572->switching(flag);
		}

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

	} dialog_chromakey;
} // namespace patch
#endif // ifdef PATCH_SWITCH_SETTINGDIALOG_CHROMAKEY
