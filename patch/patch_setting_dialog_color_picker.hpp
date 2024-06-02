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

#ifdef PATCH_SWITCH_SETTINGDIALOG_COLOR_PICKER
#include <exedit.hpp>
#include "global.hpp"
#include "util.hpp"
#include "config_rw.hpp"

namespace patch {
	// init at exedit load
	// クロマキーなどで色の選択をしている時にもう一度ボタンを押すことで色選択ダイアログを出すようにする

	inline class dialog_color_picker_t {

		static void __cdecl color_dialog_chromakey(ExEdit::Filter* efp);

		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "settingdialog_color_picker";

		inline static const char newstr[] = "クリックで色を取得。再度ボタンを押してダイアログ表示";

	public:
		static void __stdcall color_dialog_specialcolorconv1(ExEdit::Filter* efp);
		static void __stdcall color_dialog_specialcolorconv2(ExEdit::Filter* efp);

		void init() {
			enabled_i = enabled;
			if (!enabled_i)return;

			{ // クロマキー
				/*
					10014490 c7460800000000     mov     dword ptr [esi+08],00000000
					↓
					10014490 57                 push    edi
					10014491 e8XxXxXxXx         call    newfunc
					10014496 5f                 pop     edi
				*/
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x14490, 7);
				h.store_i16(0, '\x57\xe8');
				h.replaceNearJmp(2, &color_dialog_chromakey);
				h.store_i8(6, '\x5f');

				OverWriteOnProtectHelper(GLOBAL::exedit_base + 0x1452f, 4).store_i32(0, &newstr);
			}

			{ // カラーキー
				/*
					10016910 c7460800000000     mov     dword ptr [esi+08],00000000
					↓
					10016910 57                 push    edi
					10016911 e8XxXxXxXx         call    newfunc
					10016916 5f                 pop     edi
				*/
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x16910, 7);
				h.store_i16(0, '\x57\xe8');
				h.replaceNearJmp(2, &color_dialog_chromakey);
				h.store_i8(6, '\x5f');

				OverWriteOnProtectHelper(GLOBAL::exedit_base + 0x169af, 4).store_i32(0, &newstr);
			}

			{ // 特定色域変換
				/*
					1001612b 66c746060000       mov     word ptr [esi+06],0000
					↓
					1001612b 57                 push    edi
					1001612c e8XxXxXxXx         call    newfunc_stdcall
				*/
				constexpr int vp_begin = 0x1612b;
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x1626c - vp_begin);
				h.store_i16(0x1612b - vp_begin, '\x57\xe8');
				h.replaceNearJmp(0x1612d - vp_begin, &color_dialog_specialcolorconv1); // patch_any_objと同じ箇所
				h.store_i16(0x16158 - vp_begin, '\x57\xe8');
				h.replaceNearJmp(0x1615a - vp_begin, &color_dialog_specialcolorconv2); // patch_any_objと同じ箇所
				
				h.store_i32(0x161e8 - vp_begin, &newstr);
				h.store_i32(0x16268 - vp_begin, &newstr);
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

	} dialog_color_picker;
} // namespace patch
#endif // ifdef PATCH_SWITCH_SETTINGDIALOG_COLOR_PICKER
