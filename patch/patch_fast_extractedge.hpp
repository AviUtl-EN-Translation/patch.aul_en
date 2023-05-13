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
#ifdef PATCH_SWITCH_FAST_EXTRACTEDGE

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"
#include "global.hpp"
#include "config_rw.hpp"

namespace patch::fast {
	// init at exedit load
	// エッジ抽出の速度を少しだけ改善
	// 色指定無し を選べるように変更
	inline class ExtractEdge_t {
		static BOOL __cdecl yc_mt_func_wrap(AviUtl::MultiThreadFunc original_func_ptr, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		static BOOL __cdecl y_mt_func_wrap(AviUtl::MultiThreadFunc original_func_ptr, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		static void __cdecl alpha_mt_wrap(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		static void __cdecl disp_color_str_wrap(ExEdit::Filter* efp, uint8_t* exdata);
		static void __cdecl update_any_exdata_wrap(ExEdit::ObjectFilterIndex ofi, char* str);

		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "fast.extractedge";

	public:

		
		struct efExtractEdge_var {
			int threshold; // 134e6c
			short color_cb;
			short color_cr;
			int intensity; // 134e74
			short color_y;
		};
		

		void init() {
			enabled_i = enabled;
			if (!enabled_i)return;

			{ // 色指定無しを選べるようにする
				{ // FilterのExdataUseにno_colorを追加
					OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0xa2b54, 8);
					h.store_i8(0, 1);
					h.store_i32(4, GLOBAL::exedit_base + OFS::ExEdit::str_no_color);
				}
				{ // 設定ダイアログに色を表示する部分を変更
					OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x23c00, 5);
					h.store_i8(0, '\xe9');
					h.replaceNearJmp(1, &disp_color_str_wrap);
				}
				{ // 色指定なしを選べるように変更
					/*
					10023b34 8b5664             mov     edx,dword ptr [esi+64]
					10023b37 6a02               push    +02
					↓
					10023b34 e9XxXxXxXx         jmp     cursor

					10000000 8b5664             mov     edx,dword ptr [esi+64]
					10000000 6802010000         push    00000102
					10000000 e9XxXxXxXx         jmp     ee+23b39
					*/
					auto& cursor = GLOBAL::executable_memory_cursor;

					constexpr int vp_begin = 0x23b34;
					OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x23b5a - vp_begin);
					h.store_i8(0, '\xe9');
					h.replaceNearJmp(1, cursor);

					store_i32(cursor, '\x8b\x56\x64\x68'); cursor += 4;
					store_i32(cursor, '\x02\x01\x00\x00'); cursor += 4;
					store_i8(cursor, '\xe9'); cursor++;
					store_i32(cursor, GLOBAL::exedit_base + 0x23b39 - (int)cursor - 4); cursor += 4;

					h.replaceNearJmp(0x23b56 - vp_begin, &update_any_exdata_wrap);
				}
			}
			{ // 速度アップや色指定無しを追加置き換え
				constexpr int vp_begin = 0x22dd9;
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x22e06 - vp_begin);
				h.store_i16(0, '\x90\xe8');
				h.replaceNearJmp(2, &y_mt_func_wrap);

				h.store_i32(0x22dec - vp_begin, &alpha_mt_wrap);

				h.store_i16(0x22e00 - vp_begin, '\x90\xe8');
				h.replaceNearJmp(0x22e02 - vp_begin, &yc_mt_func_wrap);
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

	} ExtractEdge;
} // namespace patch::fast
#endif // ifdef PATCH_SWITCH_FAST_EXTRACTEDGE
