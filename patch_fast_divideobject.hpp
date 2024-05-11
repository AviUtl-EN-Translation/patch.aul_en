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
#ifdef PATCH_SWITCH_FAST_DIVIDEOBJECT

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"
#include "global.hpp"
#include "config_rw.hpp"

#include "patch_fast_yc_filter_effect.hpp"

namespace patch::fast {
	// init at exedit load
	// オブジェクト分割をyc対応
	inline class DivideObject_t {

		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "fast.divideobject";

	public:

		static BOOL __cdecl func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		inline static BOOL(__cdecl* func_proc_org)(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		struct efDivideObject_var {
			int running;
		};

		void init() {
			enabled_i = enabled;
			if (!enabled_i)return;

			if (!yc_filter_effect.is_enabled_i()) return;

			ExEdit::Filter* efp = reinterpret_cast<ExEdit::Filter*>(GLOBAL::exedit_base + OFS::ExEdit::efDivideObject_ptr);
			efp->flag |= static_cast<decltype(efp->flag)>(0x40);
			(func_proc_org) = (efp->func_proc);
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

	} DivideObject;
} // namespace patch::fast
#endif // ifdef PATCH_SWITCH_FAST_DIVIDEOBJECT
