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
#ifdef PATCH_SWITCH_FAST_EXFUNC_FILL

#include <aviutl.hpp>
#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"
#include "global.hpp"
#include "config_rw.hpp"

namespace patch::fast {
	// init at exedit load
	// efp->exfunc->fill(0x48)の速度アップ
	
	inline class exfunc_fill_t {
		static void __cdecl fill_simd_stream(void* ycp, int ox, int oy, int width, int height, short y, short cb, short cr, short a, int flag);

		inline static void(__cdecl* fill_org)(void* ycp, int ox, int oy, int width, int height, short y, short cb, short cr, short a, int flag);

		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "fast.exfunc_fill";

	public:

		void init() {
			enabled_i = enabled;
			if (!enabled_i)return;

			constexpr int vp_begin = 0x81f88;
			OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x81f95 - vp_begin);
			h.store_i32(0x81f88 - vp_begin, '\x55\x8b\xec\x8b');
			h.store_i32(0x81f8c - vp_begin, '\x55\x0c\xeb\x06');
			h.store_i8(0x81f90 - vp_begin, '\xe9');
			h.replaceNearJmp(0x81f91 - vp_begin, &fill_simd_stream);
			fill_org = reinterpret_cast<decltype(fill_org)>(GLOBAL::exedit_base + 0x81f88);

			OverWriteOnProtectHelper(GLOBAL::exedit_base + 0xa4228, 4).store_i32(0, &fill_simd_stream);
			

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

	} exfunc_fill;
} // namespace patch::fast
#endif // ifdef PATCH_SWITCH_FAST_EXFUNC_FILL
