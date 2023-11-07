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

#ifdef PATCH_SWITCH_EXO_ABNORMAL_POS

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"
#include "restorable_patch.hpp"

#include "config_rw.hpp"

namespace patch {
	// init at exedit load
	// 読み込み時にオブジェクトの開始フレーム・終了フレーム・レイヤーのどれかが0未満の時に中断されないように変更
	// 長さが0以下の異常なオブジェクトを読み込まないように変更

	inline class exo_abnormal_pos_t {

		bool enabled = true;
		bool enabled_i;

		inline static const char key[] = "exo_abnormal_pos";

	public:

		void init() {
			enabled_i = enabled;

			if (!enabled_i)return;
			{
				/*
					10029436 85db               test    ebx,ebx
					10029438 0f8c00030000       jl      1002973e
					1002943e 8b4c2424           mov     ecx,dword ptr [esp+24]
					10029442 85c9               test    ecx,ecx
					10029444 0f8cf4020000       jl      1002973e
					1002944a 85ed               test    ebp,ebp
					1002944c 0f8cec020000       jl      1002973e
					↓
					10029436 85db               test    ebx,ebx
					10029438 0f8cd5020000       jl      10029713
					1002943e 8b4c2424           mov     ecx,dword ptr [esp+24]
					10029442 3bcb               cmp     ecx,ebx
					10029444 0f8cc9020000       jl      10029713
					1002944a 85ed               test    ebp,ebp
					1002944c 0f8cc1020000       jl      10029713
					10029452
				*/
				constexpr int vp_begin = 0x2943a;
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x2944f - vp_begin);
				h.store_i16(0x2943a - vp_begin, '\xd5\x02');
				h.store_i16(0x29442 - vp_begin, '\x3b\xcb');
				h.store_i8(0x29446 - vp_begin, '\xc9');
				h.store_i8(0x2944e - vp_begin, '\xc1');
			}

		}
		void switching(bool flag) {
			enabled = flag;
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

	} exo_abnormal_pos;
} // namespace patch
#endif // ifdef PATCH_SWITCH_EXO_ABNORMAL_POS
