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
#ifdef PATCH_SWITCH_FAST_DIFFUSELIGHT

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"
#include "global.hpp"
#include "config_rw.hpp"

namespace patch::fast {
	// init at exedit load
	// 拡散光の速度アップ

	inline class DiffuseLight_t {

		static void __cdecl yca_mt(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		static void __cdecl yca_plus_mt(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		static void __cdecl yca_cs_mt(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		static void __cdecl yca_cs_plus_mt(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		static void __cdecl yc_cs_mt(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		static void __cdecl yc_cs_plus_mt(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);


		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "fast.diffuselight";

	public:

		struct efDiffuseLight_var { // 11efdc
			int intensity;
			int range;
			int size;
		};


		void init() {
			enabled_i = enabled;
			if (!enabled_i)return;

			constexpr int vp_begin = 0x1c3d9;
			OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x1c6f7 - vp_begin);
			/* 不要なfill0を消す
				1001c3d9 8d0ca9             lea     ecx,dword ptr [ecx+ebp*4]
				↓
				1001c3d9 eb56               jmp     1001c431
				1001c3db
			*/
			h.store_i16(0x1c3d9 - vp_begin, '\xeb\x56');

			/* 拡張処理を消す
				1001c44a a820               test    al,20
				1001c44c 0f84d8000000       jz      1001c52a
				↓
				1001c44a e9db000000         jmp     1001c52a
				1001c44f 000000             error
			*/
			h.store_i8(0x1c44a - vp_begin, '\xe9');
			h.store_i32(0x1c44b - vp_begin, '\xdb\x00\x00\x00');
			
			h.store_i32(0x1c566 - vp_begin, &yca_mt);
			h.store_i32(0x1c5b8 - vp_begin, &yca_plus_mt);
			h.store_i32(0x1c642 - vp_begin, &yca_mt);
			h.store_i32(0x1c6a8 - vp_begin, &yca_plus_mt);
			h.store_i32(0x1c556 - vp_begin, &yca_cs_mt);
			h.store_i32(0x1c5ae - vp_begin, &yca_cs_plus_mt);
			h.store_i32(0x1c632 - vp_begin, &yca_cs_mt);
			h.store_i32(0x1c68c - vp_begin, &yca_cs_plus_mt);
			h.store_i32(0x1c590 - vp_begin, &yc_cs_mt);
			h.store_i32(0x1c5f7 - vp_begin, &yc_cs_plus_mt);
			h.store_i32(0x1c66b - vp_begin, &yc_cs_mt);
			h.store_i32(0x1c6f3 - vp_begin, &yc_cs_plus_mt);

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

	} DiffuseLight;
} // namespace patch::fast
#endif // ifdef PATCH_SWITCH_FAST_DIFFUSELIGHT
