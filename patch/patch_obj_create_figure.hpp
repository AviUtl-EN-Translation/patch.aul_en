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
#ifdef PATCH_SWITCH_OBJ_CREATE_FIGURE

#include "offset_address.hpp"
#include "util.hpp"
#include "global.hpp"
#include "config_rw.hpp"

namespace patch {
	// init at exedit load
	// 図形の縦横比が-100未満や100超の時にエラーが出たり描画がおかしくなるのを修正
	inline class obj_CreateFigure_t {

		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "obj_create_figure";

	public:

		void init() {
			enabled_i = enabled;
			if (!enabled_i)return;

			/*
				100734eb b8d34d6210         mov     eax,10624dd3
				↓
				100734eb e8XxXxXxXx         call    cursor

				1007350a b8d34d6210         mov     eax,10624dd3
				1007350f 2bcb               sub     ecx,ebx
				↓
				1007350a 2bcb               sub     ecx,ebx
				1007350c e8XxXxXxXx         call    cursor

				10000000 85c9               test    ecx,ecx
				10000000 7d02               jnl     skip,02
				10000000 33c9               xor     ecx,ecx
				10000000 b8d34d6210         mov     eax,10624dd3
				10000000 c3                 ret
			*/
			auto& cursor = GLOBAL::executable_memory_cursor;

			static const char code_put[] = {
				"\x85\xc9"                 // test    ecx,ecx
				"\x7d\x02"                 // jng     skip,02
				"\x33\xc9"                 // xor     ecx,ecx
				"\xb8\xd3\x4d\x62\x10"     // mov     eax,10624dd3
				"\xc3"                     // ret
			};
			
			constexpr int vp_begin = 0x734eb;
			OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x73511 - vp_begin);
			h.store_i8(0x734eb - vp_begin, '\xe8');
			h.replaceNearJmp(0x734ec - vp_begin, cursor);
			h.store_i32(0x7350a - vp_begin, '\x2b\xcb\xe8\x00');
			h.replaceNearJmp(0x7350d - vp_begin, cursor);

			memcpy(cursor, code_put, sizeof(code_put) - 1);
			cursor += sizeof(code_put) - 1;
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

	} CreateFigure;
} // namespace patch
#endif // ifdef PATCH_SWITCH_OBJ_CREATE_FIGURE
