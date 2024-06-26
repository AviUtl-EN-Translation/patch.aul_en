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
#ifdef PATCH_SWITCH_FAST_RESIZE

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"
#include "global.hpp"
#include "config_rw.hpp"

#include "patch_fast_yc_filter_effect.hpp"

namespace patch::fast {
	// init at exedit load
	// リサイズをyc対応
	inline class Resize_t {

		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "fast.resize";

	public:

		struct efResize_var {
			int h;
			int w;
		};
		static BOOL __cdecl func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		static void __cdecl mt_yc2yc(int thread_id, int thread_num, efResize_var* resize, ExEdit::FilterProcInfo* efpip);
		static void __cdecl mt_yc2yca(int thread_id, int thread_num, efResize_var* resize, ExEdit::FilterProcInfo* efpip);


		void init() {
			enabled_i = enabled;
			if (!enabled_i)return;

			if (!yc_filter_effect.is_enabled_i()) return;

			ExEdit::Filter* efp = reinterpret_cast<ExEdit::Filter*>(GLOBAL::exedit_base + OFS::ExEdit::efResize_ptr);
			efp->flag |= static_cast<decltype(efp->flag)>(0x40);
			(efp->func_proc) = (func_proc);
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

	} Resize;
} // namespace patch::fast
#endif // ifdef PATCH_SWITCH_FAST_RESIZE
