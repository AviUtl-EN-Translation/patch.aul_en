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
#ifdef PATCH_SWITCH_FAST_SPECIALCOLORCONV

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"
#include "global.hpp"
#include "config_rw.hpp"

namespace patch::fast {
	// init at exedit load
	// 特定色域変換を速度アップ
	inline class SpecialColorConv_t {

		static void __cdecl mt_avx2(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		static void __cdecl mt_border(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		static void __cdecl mt_blur2(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);

		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "fast.specialcolorconv";

	public:

		static void __cdecl mt_blur1(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		
		struct efSpecialColorConv_var { // 11ecfc
			void* temp1;
			void* temp2;
			int range;
			int size;
		};
		

		void init() {
			enabled_i = enabled;
			if (!enabled_i)return;

			auto avx2enable = has_flag(get_CPUCmdSet(), CPUCmdSet::F_AVX2);

			/*
			  ;(short*)efpip->obj_temp + efpip->obj_line * efpip->obj_h;
			  100154b5 8b87ec000000       mov     eax,dword ptr [edi+000000ec]
			  100154bb 0faf87b8000000     imul    eax,dword ptr [edi+000000b8]
			  100154c2 8b8fb0000000       mov     ecx,dword ptr [edi+000000b0]
			  100154c8 8d1441             lea     edx,dword ptr [ecx+eax*2]
			  ↓
			  ;(int*)efpip->obj_temp + efpip->obj_w * efpip->obj_h;
			  100154b5 8b87b4000000       mov     eax,dword ptr [edi+000000b4]
			  100154bb 0faf87b8000000     imul    eax,dword ptr [edi+000000b8]
			  100154c2 8b8fb0000000       mov     ecx,dword ptr [edi+000000b0]
			  100154c8 8d1481             lea     edx,dword ptr [ecx+eax*4]
			*/

			constexpr int vp_begin = 0x154ab;
			OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x15528 - vp_begin);
			
			h.store_i32(0x154ab - vp_begin, &mt_border);
			h.store_i8(0x154b7 - vp_begin, '\xb4');
			h.store_i8(0x154ca - vp_begin, '\x81');
			h.store_i32(0x154f5 - vp_begin, &mt_blur1);
			h.store_i32(0x15505 - vp_begin, &mt_blur2);
			if (avx2enable) {
				h.store_i32(0x15524 - vp_begin, &mt_avx2);
			}


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
} // namespace patch::fast
#endif // ifdef PATCH_SWITCH_FAST_SPECIALCOLORCONV
