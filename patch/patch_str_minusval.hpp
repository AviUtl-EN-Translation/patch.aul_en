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

#ifdef PATCH_SWITCH_STR_MINUSVAL

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"

#include "config_rw.hpp"

namespace patch {
	// init at exedit load
	// オブジェクトファイル、スクリプトファイルのトラックバーの値にて，-0.99～-0.01の負号が消え、正の値になるのを修正
	inline class str_minusval_t {

		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "str_minusval";

	public:
		void init() {
			enabled_i = enabled;

			if (!enabled_i)return;

			/*
				10002d5a 85c9               test    ecx,ecx
				10002d5c 7d02               jnl     10002d60
				10002d5e f7d8               neg     eax

				1002911c 85c9               test    ecx,ecx
				1002911e 7d02               jnl     10029122
				10029120 f7d8               neg     eax

				↓

				10002d5a 90                 nop
				10002d5b e8XxXxXxXx         call    cursor
			*/

			auto& cursor = GLOBAL::executable_memory_cursor;

			static const char code_put[] = {
				"\x85\xc9"                 // test    ecx,ecx
				"\x7f\x1c"                 // jg      skip,1c
				"\x7c\x18"                 // jl      skip,18
				"\x56"                     // push    esi
				"\x4e"                     // dec     esi
				"\x80\x3e\x30"             // cmp     byte ptr [esi],30 ; '0'
				"\x74\xfa"                 // jz      back,06
				"\x80\x3e\x00"             // cmp     byte ptr [esi],00 ; '\0' (元々'.')
				"\x75\x0f"                 // jnz     skip,0f
				"\x4e"                     // dec     esi
				"\x80\x3e\x30"             // cmp     byte ptr [esi],30 ; '0'
				"\x74\xfa"                 // jz      back,06
				"\x80\x3e\x2d"             // cmp     byte ptr [esi],2d ; '-'
				"\x75\x04"                 // jnz     skip,04
				"\x5e"                     // pop     esi
				"\xf7\xd8"                 // neg     eax
				"\xc3"                     // ret
				"\x5e"                     // pop     esi
				"\xc3"                     // ret
			};

			{
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x2d5a, 6);
				h.store_i16(0, '\x90\xe8');
				h.replaceNearJmp(2, cursor);
			}
			{
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x2911c, 6);
				h.store_i16(0, '\x90\xe8');
				h.replaceNearJmp(2, cursor);
			}

			memcpy(cursor, code_put, sizeof(code_put) - 1);
			cursor += sizeof(code_put) - 1;

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

	} str_minusval;
} // namespace patch
#endif // ifdef PATCH_SWITCH_STR_MINUSVAL
