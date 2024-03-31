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
#ifdef PATCH_SWITCH_EXFILTER_SPECIALCOLORCONV
#ifdef PATCH_SWITCH_FAST_SPECIALCOLORCONV

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"
#include "global.hpp"
#include "config_rw.hpp"

#include "patch_exfilter.hpp"

namespace patch::exfilter {
	// init at exedit load
	// 特定色域変換のフィルタオブジェクトを追加
	inline class SpecialColorConv_t {

		inline static ExEdit::Filter ef;

		static void __cdecl mt(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		static void __cdecl mt_border(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		static void __cdecl mt_blur2(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);

		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "exfilter.specialcolorconv";

	public:
		static BOOL __cdecl func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);

		struct efSpecialColorConv_var { // 11ecfc
			void* temp1;
			void* temp2;
			int range;
			int size;
		};


		void init() {
			enabled_i = enabled;
			if (!enabled_i)return;

			auto efp = reinterpret_cast<ExEdit::Filter*>(GLOBAL::exedit_base + OFS::ExEdit::efSpecialColorConv_ptr);
			ef = *efp;

			*(int*)&ef.flag = 0;
			(ef.func_proc) = (func_proc);

			exfilter.apend_filter(&ef);

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

	} SpecialColorConv;
} // namespace patch::exfilter
#endif // ifdef PATCH_SWITCH_FAST_SPECIALCOLORCONV
#endif // ifdef PATCH_SWITCH_EXFILTER_SPECIALCOLORCONV
