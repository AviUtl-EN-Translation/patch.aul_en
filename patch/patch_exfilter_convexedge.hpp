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
#ifdef PATCH_SWITCH_FAST_CONVEXEDGE
#ifdef PATCH_SWITCH_FAST_CONVEXEDGE_CL
#ifdef PATCH_SWITCH_EXFILTER_CONVEXEDGE

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"
#include "global.hpp"
#include "config_rw.hpp"

#include "patch_exfilter.hpp"
#include "patch_fast_convexedge.hpp"

namespace patch::exfilter {
	// init at exedit load
	// フィルタオブジェクトの凸エッジを追加
	inline class ConvexEdge_t {

		inline static ExEdit::Filter ef;
		inline static int track_e[3] = { 1000,1000,3600 };

		static BOOL __cdecl func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		inline static BOOL(__cdecl* func_proc_org)(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);

		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "exfilter.convexedge";

	public:

		void init() {
			enabled_i = enabled;
			if (!enabled_i)return;
			if (!fast::ConvexEdge.is_enabled() && !fast::ConvexEdgeCL.is_enabled())return;

			auto efp = reinterpret_cast<ExEdit::Filter*>(GLOBAL::exedit_base + OFS::ExEdit::efConvexEdge_ptr);
			ef = *efp;

			*(int*)&ef.flag = 0;
			(func_proc_org) = (ef.func_proc);
			(ef.func_proc) = (func_proc);

			ef.track_e = track_e;

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

	} ConvexEdge;
} // namespace patch::exfilter
#endif // ifdef PATCH_SWITCH_EXFILTER_CONVEXEDGE
#endif // ifdef PATCH_SWITCH_FAST_CONVEXEDGE_CL
#endif // ifdef PATCH_SWITCH_FAST_CONVEXEDGE
