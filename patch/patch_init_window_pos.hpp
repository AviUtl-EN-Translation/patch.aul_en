/*
This program is free software : you can redistribute itand /or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.If not, see < https://www.gnu.org/licenses/>.
*/

#pragma once
#include "macro.h"
#ifdef PATCH_SWITCH_INIT_WINDOW_POS

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"
#include "global.hpp"
#include "config_rw.hpp"

namespace patch {
	// init at exedit load
	// 「ウィンドウの位置を初期化」で設定ダイアログを拡張編集と同じ位置に動かす
	inline class init_window_pos_t {

		static BOOL __stdcall GetWindowRect_wrap(HWND hWnd, LPRECT lpRect);

		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "init_window_pos";

	public:

		void init() {
			enabled_i = enabled;
			if (!enabled_i)return;

			OverWriteOnProtectHelper h(GLOBAL::aviutl_base + 0x05551, 6);
			h.store_i16(0,'\x90\xe8');
			h.replaceNearJmp(2, &GetWindowRect_wrap);
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

	} init_window_pos;
}
#endif // ifdef PATCH_SWITCH_INIT_WINDOW_POS
