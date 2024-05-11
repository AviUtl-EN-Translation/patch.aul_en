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
#ifdef PATCH_SWITCH_FAST_LIGHTEMISSION
#ifdef PATCH_SWITCH_FAST_BLUR

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"
#include "global.hpp"
#include "config_rw.hpp"

namespace patch::fast {
	// init at exedit load
	// 発光の速度アップ

	inline class LightEmission_t {

		static void __cdecl vertical_yc_fb_cs_mt_wrap(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		static void __cdecl vertical_yc_cs_mt_wrap(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);

		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "fast.lightemission";

	public:

		struct efLightEmission_var { // 1b1fdc
			int h;
			int w;
			int intensity;
			int size_h;
			int size_w;
			int range_h;
			short light_cb;
			short _padding1;
			int range_w;
			short light_cr;
			short _padding2;
			int speed;
			short light_y;
			short _padding3;
			int threshold;
			int intensity_over;
		};


		void init() {
			enabled_i = enabled;
			if (!enabled_i)return;

			auto cpucmdset = get_CPUCmdSet();
			if (!has_flag(cpucmdset, CPUCmdSet::F_AVX2))return;

			constexpr int vp_begin = 0x53a8d;
			OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x53ac4 - vp_begin);
			h.store_i32(0x53a8d - vp_begin, &vertical_yc_fb_cs_mt_wrap);
			h.store_i32(0x53ac0 - vp_begin, &vertical_yc_cs_mt_wrap);

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

	} LightEmission;
} // namespace patch::fast
#endif // ifdef PATCH_SWITCH_FAST_BLUR
#endif // ifdef PATCH_SWITCH_FAST_LIGHTEMISSION
