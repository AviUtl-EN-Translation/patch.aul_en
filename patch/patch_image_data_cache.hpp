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

#ifdef PATCH_SWITCH_IMAGE_DATA_CACHE


#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"

#include "config_rw.hpp"

namespace patch {

	// init at exedit load
	// 画像データのキャッシュ数関係のサイズをページ単位（4096の倍数）になるように広げる

	inline class image_data_cache_t {

		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "image_data_cache";

	public:

		void init() {
			enabled_i = enabled;

			if (!enabled_i)return;

			/*
				1000d50f 8d04c510000000     lea     eax,dword ptr [eax*8+00000010]
				1000d516 3d00000400         cmp     eax,00040000
				↓
				1000d50f 8d04c50f100000     lea     eax,dword ptr [eax*8+0000100f]
				1000d516 e8XxXxXxXx         call    cursor

				10000000 2500f0ffff         and     eax,fffff000
				10000000 3d00000400         cmp     eax,00040000
				10000000 c3                 ret
			*/

			auto& cursor = GLOBAL::executable_memory_cursor;

			constexpr int vp_begin = 0xd512;
			OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0xd51b - vp_begin);
			h.store_i16(0xd512 - vp_begin, '\x0f\x10');
			h.store_i8(0xd516 - vp_begin, '\xe8');
			h.replaceNearJmp(0xd517 - vp_begin, cursor);

			store_i32(cursor, '\x25\x00\xf0\xff'); cursor += 4;
			store_i32(cursor, '\xff\x3d\x00\x00'); cursor += 3;
			store_i32(cursor, '\x00\x04\x00\xc3'); cursor += 4;

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


	} image_data_cache;
} // namespace patch

#endif // ifdef PATCH_SWITCH_IMAGE_DATA_CACHE