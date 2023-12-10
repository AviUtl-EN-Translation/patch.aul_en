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
#ifdef PATCH_SWITCH_FAST_BORDERBLUR

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"
#include "global.hpp"
#include "config_rw.hpp"

namespace patch::fast {
	// init at exedit load
	// 境界ぼかしの速度アップ
	// 範囲がマイナスの時に処理を行わないように修正
	// 処理範囲がオブジェクトサイズを超えないように修正
	inline class BorderBlur_t {

		static void __cdecl object_mt1(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		static void __cdecl object_mt2(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);

		static void __cdecl alpha_mt1(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		static void __cdecl alpha_mt2(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);

		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "fast.borderblur";

	public:

		struct efBorderBlur_var {
			int size_h; // 11ec48
			int size_w;
			int range_h;
			int range_w;
		};

		void init() {
			enabled_i = enabled;
			if (!enabled_i)return;
			
			constexpr int vp_begin = 0x11b0d;
			OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x11c1a - vp_begin);
			/* 範囲がマイナスの時に処理を行わないように修正
				if (efp->track[0] == 0) return 1;
				↓
				if (efp->track[0] <= 0) return 1;
			*/
			h.store_i8(0x11b0d - vp_begin, '\x8e');

			/* 処理範囲がオブジェクトサイズを超えないように修正
				int max_range_w = efpip->obj_w / 2;
				↓
				if(efpip->obj_w - 1 < 0) return 1;
				int max_range_w = (efpip->obj_w - 1) >> 1;

				10011b72 99                 cdq
				10011b73 2bc2               sub     eax,edx
				10011b75 d1f8               sar     eax,1
				↓
				10011b72 48                 dec     eax
				10011b73 7c6b               jl      skip,+6b ; 10011be0
				10011b75 d1f8               sar     eax,1


				10011b91 99                 cdq
				10011b92 2bc2               sub     eax,edx
				10011b94 d1f8               sar     eax,1
				↓
				10011b91 48                 dec     eax
				10011b92 7c4c               jl      skip,+4c ; 10011be0
				10011b94 d1f8               sar     eax,1
			*/
			h.store_i8(0x11b72 - vp_begin, '\x48');
			h.store_i16(0x11b73 - vp_begin, '\x7c\x6b');
			h.store_i8(0x11b91 - vp_begin, '\x48');
			h.store_i16(0x11b92 - vp_begin, '\x7c\x4c');


			h.store_i32(0x11bc3 - vp_begin, &object_mt1);
			h.store_i32(0x11bd3 - vp_begin, &object_mt2);
			h.store_i32(0x11c00 - vp_begin, &alpha_mt1);
			h.store_i32(0x11c10 - vp_begin, &alpha_mt2);
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

	} BorderBlur;
} // namespace patch::fast
#endif // ifdef PATCH_SWITCH_FAST_BORDERBLUR
