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

#ifdef PATCH_SWITCH_ANY_OBJ

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "restorable_patch.hpp"

#include "config_rw.hpp"

namespace patch {
	// init at exedit load
	/* 複数選択で変更しても変わらない部分を変わるようにする
	*/
	inline class any_obj_t {
		static char* __cdecl disp_extension_image_file_wrap(ExEdit::Filter* efp, char* path);
		static void __cdecl calc_milli_second_movie_file_wrap(ExEdit::Filter* efp, void* exdata);
		static void __cdecl calc_milli_second_audio_file_wrap(ExEdit::Filter* efp, void* exdata);
		static void __cdecl rename_object_audio_file_wrap(ExEdit::ObjectFilterIndex ofi, char* name);
		static void __cdecl update_obj_data_waveform_wrap(ExEdit::ObjectFilterIndex ofi);
		static BOOL __cdecl update_any_waveform_type();
		static void __cdecl set_figure_type_text_wrap(ExEdit::Filter* efp, void* exdata);
		static int __stdcall count_section_num_wrap(ExEdit::Filter* efp, ExEdit::ObjectFilterIndex ofi);
		static void __cdecl update_obj_data_extractedge_wrap(ExEdit::ObjectFilterIndex ofi);
		static void __cdecl update_any_dlg_param_exdata();
		static void __cdecl init_setting_dialog_file_script_wrap(ExEdit::Filter* efp, void* exdata, int upd_flag, int sw_flag, short type, char* name, int folder_flag);


		bool enabled = true;
		bool enabled_i;

		inline static const char key[] = "any_obj";


	public:

		static void __cdecl update_any_range(ExEdit::Filter* efp);

		void init() {
			enabled_i = enabled;

			if (!enabled_i)return;

			auto& cursor = GLOBAL::executable_memory_cursor;

			{ // 画像ファイル・画像ファイル合成 の参照ファイル
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0xe18c, 4);
				h.replaceNearJmp(0, &disp_extension_image_file_wrap);
			}
			{ // 動画ファイル・動画ファイル合成 の参照ファイル
				constexpr int vp_begin = 0x6744;
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x682e - vp_begin);
				{ //ignore_media_param_reset に近いものを実装。複数選択中には再生位置などを維持する
					/*
						10006744 ff5218             call    dword ptr [edx+18]
						10006747 83c404             add     esp,+04
						↓
						10006744 56                 push    esi
						10006745 e8XxXxXxXx         call    new_func
					*/
					h.store_i16(0, '\x56\xe8');
					h.replaceNearJmp(2, &count_section_num_wrap);
				}
				{ // 参照ファイル変更 本実装
					h.replaceNearJmp(0x682a - vp_begin, &calc_milli_second_movie_file_wrap);
				}
			}
			{ // 音声ファイル の参照ファイル、動画と連動チェック
				constexpr int vp_begin = 0x9010d;
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x9027b - vp_begin);
				{ // ignore_media_param_reset に近いものを実装。複数選択中には再生位置などを維持する
					/*
						1009010d ff5218             call    dword ptr [edx+18]
						10090110 83c404             add     esp,+04
						↓
						1009010d 56                 push    esi
						1009010e e8XxXxXxXx         call    new_func
					*/
					h.store_i16(0x9010d - vp_begin, '\x56\xe8');
					h.replaceNearJmp(0x9010f - vp_begin, &count_section_num_wrap);
				}
				{ // 参照ファイル変更 本実装
					h.replaceNearJmp(0x901b7 - vp_begin, &calc_milli_second_audio_file_wrap);
				}
				{ // 動画ファイルと連動のチェックをいれた時にdispnameが更新されない
					/*
						1009026d 8b5664             mov     edx,dword ptr [esi+64]
						10090270 50                 push    eax
						10090271 8b86e4000000       mov     eax,dword ptr [esi+000000e4]
						10090277 50                 push    eax
						10090278 ff5254             call    dword ptr [edx+54]
						↓
						1009026d 50                 push    eax
						1009026e 8b86e4000000       mov     eax,dword ptr [esi+000000e4]
						10090274 50                 push    eax
						10090275 90 
						10090276 e8XxXxXxXx         call    newfunc
					*/
					h.store_i32(0x9026d - vp_begin, '\x50\x8b\x86\xe4');
					h.store_i32(0x90271 - vp_begin, '\x00\x00\x00\x50');
					h.store_i16(0x90275 - vp_begin, '\x90\xe8');
					h.replaceNearJmp(0x90277 - vp_begin, &rename_object_audio_file_wrap);
				}
			}

			{ // 音声波形表示 の参照ファイル、全体音声チェック、Type切り替え
				constexpr int vp_begin = 0x8f096;
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x8f200 - vp_begin);
				{ // ignore_media_param_reset に近いものを実装。複数選択中には再生位置などを維持する
					/*
						1008f096 8b96f4000000       mov     edx,dword ptr [esi+000000f4]
						↓
						1008f096 90                 nop
						1008f097 e9XxXxXxXx         jmp     cursor

						10000000 8b86e4000000       mov     eax,dword ptr [esi+000000e4]
						10000000 50                 push    eax
						10000000 56                 push    esi
						10000000 e8XxXxXxXx         call    newfunc
						10000000 83f801             cmp     eax,+01
						10000000 0f85XxXxXxXx       jnz     ee+8f0e6
						10000000 8b96f4000000       mov     edx,dword ptr [esi+000000f4]
						10000000 e9XxXxXxXx         jmp     ee+8f09c
					*/
					h.store_i16(0x8f096 - vp_begin, '\x90\xe9');
					h.replaceNearJmp(0x8f098 - vp_begin, cursor);

					store_i32(cursor, '\x8b\x86\xe4\x00'); cursor += 4;
					store_i32(cursor, '\x00\x00\x50\x56'); cursor += 4;
					store_i8(cursor, '\xe8'); cursor++;
					store_i32(cursor, (int)&count_section_num_wrap - (int)cursor - 4); cursor += 4;
					store_i32(cursor, '\x83\xf8\x01\x0f'); cursor += 4;
					store_i8(cursor, '\x85'); cursor++;
					store_i32(cursor, GLOBAL::exedit_base + 0x8f0e6 - (int)cursor - 4); cursor += 4;
					store_i32(cursor, '\x8b\x96\xf4\x00'); cursor += 4;
					store_i32(cursor, '\x00\x00\xe9\x00'); cursor += 3;
					store_i32(cursor, GLOBAL::exedit_base + 0x8f09c - (int)cursor - 4); cursor += 4;
				}
				{ // 参照ファイル変更 本実装
					/*
						1008f101 8b4664             mov     eax,dword ptr [esi+64]
						1008f104 51                 push    ecx
						1008f105 ff10               call    dword ptr [eax]
						↓
						1008f101 51                 push    ecx
						1008f102 e8XxXxXxXx         call    newfunc
					*/
					h.store_i16(0x8f101 - vp_begin, '\x51\xe8');
					h.replaceNearJmp(0x8f103 - vp_begin, &update_obj_data_waveform_wrap);
				}
				{ // 編集全体の音声を元にするのチェックをいれた時にdispnameが更新されない
					// 音声ファイルのと同じ
					h.store_i32(0x8f137 - vp_begin, '\x50\x8b\x86\xe4');
					h.store_i32(0x8f13b - vp_begin, '\x00\x00\x00\x50');
					h.store_i16(0x8f13f - vp_begin, '\x90\xe8');
					h.replaceNearJmp(0x8f141 - vp_begin, &rename_object_audio_file_wrap);
				}
				{ // 元と同じTypeへの切り替えを許可する(複数切り替えのためと、Typeの役割はプリセットなので出来る方が良いはず)
					h.store_i32(0x8f193 - vp_begin, '\x66\x0f\x1f\x44'); // 6byte nop
					h.store_i16(0x8f197 - vp_begin, '\x00\x00');
				}
				{ // Typeを切り替え 本実装
					h.store_i8(0x8f1fb - vp_begin, '\xe8');
					h.replaceNearJmp(0x8f1fc - vp_begin, &update_any_waveform_type);
				}
			}

			{ // 図形 の図形の種類
				constexpr int vp_begin = 0x74562;
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x74611 - vp_begin);
				h.replaceNearJmp(0, &set_figure_type_text_wrap);
				h.replaceNearJmp(0x7460d - vp_begin, &set_figure_type_text_wrap);
			}

			/* 時間制御・グループ制御・カメラ制御 の対象レイヤー数
			    undo.cppの部分から呼び出す
			*/

			{ // エッジ抽出のチェック切り替え
				/*
					10023b94 83780401           cmp     dword ptr [eax+04],+01
					10023b98 7551               jnz     10023beb
					↓
					10023b94 660f1f440000       nop

					10023ba7 8b4664             mov     eax,dword ptr [esi+64]
					10023baa 51                 push    ecx
					10023bab ff10               call    dword ptr [eax]
					↓
					10023ba7 51                 push    ecx
					10023ba8 e8XxXxXxXx         call    newfunc
				*/
				constexpr int vp_begin = 0x23b94;
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x23bdf - vp_begin);
				h.store_i32(0x23b94 - vp_begin, '\x66\x0f\x1f\x44');
				h.store_i16(0x23b98 - vp_begin, '\x00\x00');

				h.store_i16(0x23ba7 - vp_begin, '\x51\xe8');
				h.replaceNearJmp(0x23ba9 - vp_begin, &update_obj_data_extractedge_wrap);

				h.store_i32(0x23bc2 - vp_begin, '\x0f\x1f\x44\x00'); // 5byte-nop
				h.store_i8(0x23bc6 - vp_begin, '\x00');

				h.store_i16(0x23bd9 - vp_begin, '\x50\xe8'); // こっちはeax
				h.replaceNearJmp(0x23bdb - vp_begin, &update_obj_data_extractedge_wrap);
			}

			{ // 音声波形表示・波紋・ノイズ の設定ボタンのダイアログ
				/*
					10022d54 b801000000         mov     eax,00000001
					10022d59 5b                 pop     ebx
					10022d5a c3                 ret
					10022d5b 90                 nop
					10022d5c 90                 nop
					10022d5d 90                 nop
					10022d5e 90                 nop
					10022d5f 90                 nop
					↓
					10022d54 5b                 pop     ebx
					10022d55 e8XxXxXxXx         call    newfunc
					10022d5a b801000000         mov     eax,00000001
					10022d5f c3                 ret
				*/
				constexpr int vp_begin = 0x22d54;
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x22d60 - vp_begin);
				h.store_i16(0x22d54 - vp_begin, '\x5b\xe8');
				h.replaceNearJmp(0x22d56 - vp_begin, &update_any_dlg_param_exdata);
				h.store_i32(0x22d5a - vp_begin, '\xb8\x01\x00\x00');
				h.store_i16(0x22d5e - vp_begin, '\x00\xc3');
			}

			{ // アニメーション効果・カスタムオブジェクト・カメラ効果 のスクリプト変更
				constexpr int vp_begin = 0x3e0b;
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x3f12 - vp_begin);
				h.replaceNearJmp(0x3e0c - vp_begin, &init_setting_dialog_file_script_wrap);
				h.replaceNearJmp(0x3f0e - vp_begin, &init_setting_dialog_file_script_wrap);
				
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

	} any_obj;
} // namespace patch
#endif // ifdef PATCH_SWITCH_ANY_OBJ
