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

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"
#include "global.hpp"
#include "config_rw.hpp"

namespace patch::fast {

#ifdef PATCH_SWITCH_FAST_CONVEXEDGE
	// init at exedit load
	// 凸エッジの速度を少しだけ改善
	inline class ConvexEdge_t {
		static void __cdecl mt(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		
		inline static void(__cdecl* mt_org)(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);

		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "fast.convexedge";

	public:


		struct efConvexEdge_var {
			double height_rate; // d7588
			int width; // d7590
			int step_y16;
			int step_x16;
		};


		void init() {
			enabled_i = enabled;
			if (!enabled_i)return;


			mt_org = reinterpret_cast<decltype(mt_org)>(*(DWORD*)(GLOBAL::exedit_base + 0x7b3f));
			OverWriteOnProtectHelper(GLOBAL::exedit_base + 0x7b3f, 4).store_i32(0, &mt);
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
#endif // ifdef PATCH_SWITCH_FAST_CONVEXEDGE

#ifdef PATCH_SWITCH_FAST_CONVEXEDGE_CL
	// init at exedit load
	// 凸エッジOpecCL処理
	inline class ConvexEdgeCL_t {
		static BOOL mt_func(AviUtl::MultiThreadFunc original_func_ptr, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);

		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "fast.convexedge_cl";

	public:
		struct efConvexEdge_var {
			double height_rate; // d7588
			int width; // d7590
			int step_y16;
			int step_x16;
		};


		void init() {
			enabled_i = enabled;
			if (!enabled_i)return;

			OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x007b62, 6);
			h.store_i16(0, '\x90\xe8'); // nop; call (rel32)
			h.replaceNearJmp(2, &mt_func);

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

	} ConvexEdgeCL;
#endif // ifdef PATCH_SWITCH_FAST_CONVEXEDGE_CL
} // namespace patch::fast
