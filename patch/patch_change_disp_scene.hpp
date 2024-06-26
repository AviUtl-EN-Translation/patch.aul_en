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

#ifdef PATCH_SWITCH_CHANGE_DISP_SCENE

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "restorable_patch.hpp"

#include "config_rw.hpp"

namespace patch {
	// init at exedit load
	/* シーン切り替え時のバグを修正
	　　・Shiftを押しながらシーン切り替えをすると範囲選択がされる（システムの設定＞フレーム移動時にSHIFTキーを押している時は範囲選択移動にする の影響）
	  　・拡張編集のウィンドウキャプションが変わらない
	    ・選択中のオブジェクトのIDが異常値になっている時にエラーとなるのを修正
	*/
	inline class change_disp_scene_t {
		static int __cdecl set_frame_wrap(AviUtl::EditHandle* editp, int frame);

		bool enabled = true;
		bool enabled_i;

		inline static const char key[] = "change_disp_scene";

		inline static int last_id = -1;

	public:

		void init() {
			enabled_i = enabled;

			if (!enabled_i)return;
			{ // Shiftを押しながらシーン切り替えをすると範囲選択がされる（システムの設定＞フレーム移動時にSHIFTキーを押している時は範囲選択移動にする の影響）
			  // 拡張編集のウィンドウキャプションが変わらない
				/*
					1002be69 8b4e60             mov     ecx,dword ptr [esi+60]
					1002be6c 52                 push    edx
					1002be6d 57                 push    edi
					1002be6e ff5124             call    dword ptr [ecx+24]
					1002be71
					↓
					1002be69 52                 push    edx
					1002be6a 57                 push    edi
					1002be6b 90                 nop
					1002be6c e8XxXxXxXx         call    new_func
					1002be71

				*/
				
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x02be69, 8);
				h.store_i32(0, '\x52\x57\x90\xe8');
				h.replaceNearJmp(4, &set_frame_wrap);
				
			}
			{ // 選択中のオブジェクトのIDが異常値になっている時にエラーとなるのを修正
				/*
					1002bddf 8b82707a1710       mov     eax,dword ptr [edx+10177a70]
					↓
					1002bddf 90                 nop
					1002bde0 e8XxXxXxXx         call    cursor

					10000000 8b82XxXxXxXx       mov     eax,dword ptr [edx+ee+177a70]
					10000000 3b05XxXxXxXx       cmp     eax,dword ptr [ee+1e0fa0]
					10000000 7c03               jnl     skip,03
					10000000 83c8ff             or      eax,0xffffffff
					10000000 c3                 ret

				*/
				auto& cursor = GLOBAL::executable_memory_cursor;

				OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x2bddf, 6);
				h.store_i16(0, '\x90\xe8');
				h.replaceNearJmp(2, cursor);

				store_i16(cursor, '\x8b\x82'); cursor += 2;
				store_i32(cursor, GLOBAL::exedit_base + OFS::ExEdit::SceneSetting + 0x20); cursor += 4;
				store_i16(cursor, '\x3b\x05'); cursor += 2;
				store_i32(cursor, GLOBAL::exedit_base + OFS::ExEdit::ObjectAllocNum); cursor += 4;
				store_i16(cursor, '\x7c\x03'); cursor += 2;
				store_i32(cursor, '\x83\xc8\xff\xc3'); cursor += 4;
			}


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

	} change_disp_scene;
} // namespace patch
#endif // ifdef PATCH_SWITCH_CHANGE_DISP_SCENE
