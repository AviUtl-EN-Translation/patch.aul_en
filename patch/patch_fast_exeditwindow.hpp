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
#ifdef PATCH_SWITCH_FAST_EXEDITWINDOW
#include <Windows.h>
#include "util_magic.hpp"
#include "global.hpp"
#include "config_rw.hpp"

namespace patch {
	// init at exedit load
	// 拡張編集ウィンドウの高速化
	inline class fast_exeditwindow_t {
		static void __cdecl FUN_10036a70_Wrap_gradation(HDC hDC, LPRECT prect, int r1, int g1, int b1, int r2, int g2, int b2, int real_left, int real_right);
		static void __cdecl FUN_10036a70_Wrap_step(HDC hDC, LPRECT prect, int r1, int g1, int b1, int r2, int g2, int b2, int real_left, int real_right);

		static void __cdecl timeline_select_draw(HDC hDC, int bottom);

		static HDC WINAPI GetDC_Wrap(HWND hwnd);
		static int WINAPI ReleaseDC_Wrap(HWND hwnd, HDC hdc);

		inline static auto GetDC_Wrap_ptr = &GetDC_Wrap;
		inline static auto ReleaseDC_Wrap_ptr = &ReleaseDC_Wrap;

		bool enabled = true;
		bool enabled_i;

		int step = 0;
		int step_i;

		inline static const char key[] = "fast.exeditwindow";
		inline static const char c_step_name[] = "step";

	public:
		void init() {
			enabled_i = enabled;
			step_i = step;

			if (!enabled_i)return;


			if (step >= 0) {
				decltype(&FUN_10036a70_Wrap_gradation) f_036a70_ptr;
				if (step == 0) {
					f_036a70_ptr = &FUN_10036a70_Wrap_gradation;
				}
				else {
					f_036a70_ptr = &FUN_10036a70_Wrap_step;
				}

				ReplaceNearJmp(GLOBAL::exedit_base + 0x0374fb, f_036a70_ptr);
				ReplaceNearJmp(GLOBAL::exedit_base + 0x037563, f_036a70_ptr);
				ReplaceNearJmp(GLOBAL::exedit_base + 0x0375bb, f_036a70_ptr);
			}
			{ // 選択範囲の部分をfillrectに変更

				/*
					10037ff9 ff74241c           push    dword ptr [esp+1c]
					10037ffd 56                 push    esi
					10037ffe e8XxXxXxXx         call    newfunc
					10038003 83c408             add     esp,+08
					10038006 e998000000         jmp     100380a3
					1003800b

					100380a3 8b6c2428           mov     ebp,dword ptr [esp+28]
					100380a7 57                 push    edi
					100380a8 e8c3aaffff         call    10032b70
					100380ad 83c404             add     esp,+04
					100380b0 8bd8               mov     ebx,eax
					100380b2 895c2424           mov     dword ptr [esp+24],ebx

					100381eb 0f8cb2feffff       jl      100380a3
				*/
				/*
				constexpr int vp_begin = 0x037ff9;
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x037ff9, 0x0381ee - 0x037ff9);

				h.store_i32(0x037ff9 - vp_begin, '\xff\x74\x24\x1c');
				h.store_i16(0x037ffd - vp_begin, '\x56\xe8');
				h.replaceNearJmp(0x037fff - vp_begin, &timeline_select_draw);
				h.store_i32(0x038003 - vp_begin, '\x83\xc4\x08\xe9');
				h.store_i32(0x038007 - vp_begin, '\x98\x00\x00\x00');

				h.store_i8(0x0380a3 - vp_begin, '\x8b');
				h.store_i32(0x0380a4 - vp_begin, '\x6c\x24\x28\x57');
				h.store_i32(0x0380a8 - vp_begin, '\xe8\xc3\xaa\xff');
				h.store_i32(0x0380ac - vp_begin, '\xff\x83\xc4\x04');
				h.store_i32(0x0380b0 - vp_begin, '\x8b\xd8\x89\x5c');
				h.store_i16(0x0380b4 - vp_begin, '\x24\x24');

				h.store_i8(0x0381ed - vp_begin, '\xb2');
				*/
			}

			OverWriteOnProtectHelper(GLOBAL::exedit_base + 0x0387fd, 4).store_i32(0, &GetDC_Wrap_ptr);
			OverWriteOnProtectHelper(GLOBAL::exedit_base + 0x038927, 4).store_i32(0, &ReleaseDC_Wrap_ptr);

			OverWriteOnProtectHelper(GLOBAL::exedit_base + 0x039309, 4).store_i32(0, &GetDC_Wrap_ptr);
			OverWriteOnProtectHelper(GLOBAL::exedit_base + 0x039350, 4).store_i32(0, &ReleaseDC_Wrap_ptr);

			OverWriteOnProtectHelper(GLOBAL::exedit_base + 0x0392a8, 4).store_i32(0, &GetDC_Wrap_ptr);
			OverWriteOnProtectHelper(GLOBAL::exedit_base + 0x0392e1, 4).store_i32(0, &ReleaseDC_Wrap_ptr);

			OverWriteOnProtectHelper(GLOBAL::exedit_base + 0x039379, 4).store_i32(0, &GetDC_Wrap_ptr);
			OverWriteOnProtectHelper(GLOBAL::exedit_base + 0x0393f0, 4).store_i32(0, &ReleaseDC_Wrap_ptr);
			
			OverWriteOnProtectHelper(GLOBAL::exedit_base + 0x0394d6, 4).store_i32(0, &GetDC_Wrap_ptr);
			OverWriteOnProtectHelper(GLOBAL::exedit_base + 0x03953c, 4).store_i32(0, &ReleaseDC_Wrap_ptr);
			
			//OverWriteOnProtectHelper(GLOBAL::exedit_base + 0x03943c, 4).store_i32(0, &GetDC_Wrap_ptr);
			//OverWriteOnProtectHelper(GLOBAL::exedit_base + 0x039483, 4).store_i32(0, &ReleaseDC_Wrap_ptr);

		}

        void switching(bool flag) { enabled = flag; }

        bool is_enabled() { return enabled; }
        bool is_enabled_i() { return enabled_i; }

		void set_step(int x) { step = x; }

		int get_step() { return step; }
		int get_step_i() { return step_i; }

        void switch_load(ConfigReader& cr) {
            cr.regist(key, [this](json_value_s* value) {
                ConfigReader::load_variable(value, enabled);
            });
        }

        void switch_store(ConfigWriter& cw) {
            cw.append(key, enabled);
        }

		void config_load(ConfigReader& cr) {
			cr.regist(c_step_name, [this](json_value_s* value) {
				ConfigReader::load_variable(value, step);
			});
		}

		void config_store(ConfigWriter& cw) {
			cw.append(c_step_name, step);
		}

	} fast_exeditwindow;
} // namespace patch
#endif // ifdef PATCH_SWITCH_FAST_EXEDITWINDOW
