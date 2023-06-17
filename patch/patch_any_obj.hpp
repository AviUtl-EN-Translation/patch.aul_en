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
	   
	   ・ついでの実装に関して
	   選択状態で動画ファイルなどの参照を変えると再生位置などを維持するように変更（ignore_media_param_reset に近いもの）
	   音声波形表示の参照を変える時に中間点があれば再生位置などを維持するように変更（音声波形以外で初めから実装されている機能）
	   音声波形表示にて元と同じTypeへの切り替えを許可するように変更(複数切り替えのためと、Typeの役割はプリセットなので出来る方が良いはず)
	*/
	// スクリプト系のトラックバー・チェックを切り替えた時に同スクリプトでなければまとめて変更しないようにする
	// Ctrlを押しながらトラック数値をクリックして動かした後に選択状態が解除されるのを修正
	// 操作によっては選択状態が解除されないままになるので拡張編集ウィンドウをアクティブにした時に判定するように変更

	inline class any_obj_t {
		inline static void(__cdecl* update_any_exdata)(ExEdit::ObjectFilterIndex, char*);
		inline static void(__cdecl* deselect_object)();

		static void post_deselect_object_tl_activate();
		static void __cdecl update_any_exdata_use_idx(ExEdit::Filter* efp, int idx);
		static char* __cdecl disp_extension_image_file_wrap(ExEdit::Filter* efp, char* path);
		static void __cdecl calc_milli_second_movie_file_wrap(ExEdit::Filter* efp, void* exdata);
		static void __cdecl calc_milli_second_audio_file_wrap(ExEdit::Filter* efp, void* exdata);
		static void __cdecl rename_object_audio_file_wrap(ExEdit::ObjectFilterIndex ofi, char* name);
		static void __cdecl calc_frame_scene_wrap(ExEdit::Filter* efp);
		static void __cdecl calc_frame_sceneaudio_wrap(ExEdit::Filter* efp);
		static void __stdcall rename_object_sceneaudio_wrap(ExEdit::ObjectFilterIndex ofi, char* name, ExEdit::Filter* efp, void* exdata);
		static void __cdecl update_obj_data_waveform_wrap(ExEdit::Filter* efp);
		static int __cdecl mov_eax_1_waveform_wrap();
		static void __cdecl rename_object_figure_wrap(ExEdit::Filter* efp, void* exdata);
		static int __stdcall count_section_num_wrap(ExEdit::Filter* efp, void* e1);
		static void __cdecl update_obj_data_extractedge_wrap(ExEdit::ObjectFilterIndex ofi);
		static void __cdecl update_any_dlg_param_exdata();
		static void __cdecl init_setting_dialog_script_wrap(ExEdit::Filter* efp, void* exdata, int upd_flag, int sw_flag, short type, char* name, int folder_flag);
		static void __cdecl init_setting_dialog_scenechange_wrap(ExEdit::Filter* efp, void* exdata, LPARAM lparam, int sw_flag, short type);
		static BOOL __cdecl disp_1st_dlg_script_wrap(HWND hwnd, ExEdit::Filter* efp, void* exdata, short type, char* name);
		static BOOL __cdecl update_script_param_wrap(ExEdit::Filter* efp, char* name, char* valuestr);
		static void __cdecl update_dlg_mask_wrap(ExEdit::Filter* efp, char* name, int sw_param);
		static int __cdecl get_same_track_id_wrap(int dst_idx, int src_idx, int track_idx);
		static int __cdecl get_same_check_id_wrap(int dst_idx, int src_idx, int check_idx);
		static void __cdecl update_dlg_displacementmap_wrap(ExEdit::Filter* efp, void* exdata, char* name, int sw_param, int edi, int esi, int ebp, int ebx, tagRECT rect, DWORD ret, HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, void* editp);
		static int __cdecl mov_eax_1_portion_filter_wrap();
		static void __cdecl update_dlg_chromakey_wrap(ExEdit::Filter* efp, int* exdata);
		static void __cdecl update_dlg_colorkey_wrap(ExEdit::Filter* efp, int* exdata);
		static void __cdecl update_dlg_shadow_wrap(ExEdit::Filter* efp, void* exdata);
		static void __cdecl update_dlg_border_wrap(ExEdit::Filter* efp, void* exdata);
		/*
		static int __cdecl mov_eax_1_blend_1_wrap(int e1, DWORD ret, HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, void* editp, ExEdit::Filter* efp);
		static int __cdecl mov_eax_1_type_1_wrap(int e1, DWORD ret, HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, void* editp, ExEdit::Filter* efp);
		static int __cdecl mov_eax_1_type_name_1_wrap(int e1, DWORD ret, HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, void* editp, ExEdit::Filter* efp);
		static int __cdecl mov_eax_1_mode_0_wrap(DWORD ret, HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, void* editp, ExEdit::Filter* efp);
		static int __cdecl mov_eax_1_mode_1_wrap(int e1, DWORD ret, HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, void* editp, ExEdit::Filter* efp);
		static int __cdecl mov_eax_1_mode_2_wrap(int e1, int e2, DWORD ret, HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, void* editp, ExEdit::Filter* efp);
		*/
		static int __cdecl mov_eax_1_use0_e0_wrap(DWORD ret, HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, void* editp, ExEdit::Filter* efp);
		static int __cdecl mov_eax_1_use2_e0_wrap(DWORD ret, HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, void* editp, ExEdit::Filter* efp);
		static int __cdecl mov_eax_1_use0_e1_wrap(int e1, DWORD ret, HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, void* editp, ExEdit::Filter* efp);
		static int __cdecl mov_eax_1_use1_e1_wrap(int e1, DWORD ret, HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, void* editp, ExEdit::Filter* efp);
		static int __cdecl mov_eax_1_use2_e1_wrap(int e1, DWORD ret, HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, void* editp, ExEdit::Filter* efp);
		static int __cdecl mov_eax_1_use5_e1_wrap(int e1, DWORD ret, HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, void* editp, ExEdit::Filter* efp);
		static int __cdecl mov_eax_1_use2_e2_wrap(int e1, int e2, DWORD ret, HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, void* editp, ExEdit::Filter* efp);
		static int __cdecl mov_eax_1_use0use1_e1_wrap(int e1, DWORD ret, HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, void* editp, ExEdit::Filter* efp);

		static void update_any_color_specialcolorconv(ExEdit::Filter* efp, int id, short status);
		static void __stdcall mov_status_0_specialcolorconv(ExEdit::Filter* efp);
		static void __stdcall mov_status2_0_specialcolorconv(ExEdit::Filter* efp);
		static void __stdcall mov_status_1_specialcolorconv(ExEdit::Filter* efp);
		static void __stdcall mov_status2_1_specialcolorconv(ExEdit::Filter* efp);
		static void __cdecl update_obj_data_before_clipping_wrap(int object_idx);
		static void __cdecl update_obj_data_camera_target_wrap(int object_idx);
		static void __cdecl delete_filter_effect_wrap(int object_idx, int filter_idx);

		inline static BOOL script_dlg_ok_cancel;

		bool enabled = true;
		bool enabled_i;

		inline static const char key[] = "any_obj";


	public:
		static void deselect_object_if();
		static void deselect_object_tl_activate();
		static void __cdecl update_any_range(ExEdit::Filter* efp);

		void init() {
			enabled_i = enabled;

			if (!enabled_i)return;

			update_any_exdata = reinterpret_cast<decltype(update_any_exdata)>(GLOBAL::exedit_base + OFS::ExEdit::update_any_exdata);
			deselect_object = reinterpret_cast<decltype(deselect_object)>(GLOBAL::exedit_base + OFS::ExEdit::deselect_object);

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
				{ // 動画ファイルと連携のチェックをいれた時にdispnameが更新されないのを修正
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
			{ // シーン のシーン選択
				constexpr int vp_begin = 0x83bf6;
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x83c7a - vp_begin);
				{ //ignore_media_param_reset に近いものを実装。複数選択中には再生位置などを維持する
					/*
						10083bf6 50                 push    eax
						10083bf7 ff5218             call    dword ptr [edx+18]
						10083bfa 83c408             add     esp,+08
						↓
						10083bf6 90                 nop
						10083bf7 56                 push    esi
						10083bf8 e8XxXxXxXx         call    new_func
					*/
					h.store_i32(0x83bf6 - vp_begin, '\x90\x56\xe8\x00');
					h.replaceNearJmp(0x83bf9 - vp_begin, &count_section_num_wrap);
				}
				{ // 本実装
					h.replaceNearJmp(0x83c76 - vp_begin, &calc_frame_scene_wrap);
				}
			}
			{ // シーン(音声) のシーン選択
				constexpr int vp_begin = 0x847da;
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x8488f - vp_begin);
				{ //ignore_media_param_reset に近いものを実装。複数選択中には再生位置などを維持する
					/*
						100847da 52                 push    edx
						100847db ff5118             call    dword ptr [ecx+18]
						100847de 83c408             add     esp,+08
						↓
						100847da 90                 nop
						100847db 56                 push    esi
						100847dc e8XxXxXxXx         call    new_func
					*/
					h.store_i32(0x847da - vp_begin, '\x90\x56\xe8\x00');
					h.replaceNearJmp(0x847dd - vp_begin, &count_section_num_wrap);
				}
				{ // 本実装
					h.replaceNearJmp(0x84851 - vp_begin, &calc_frame_sceneaudio_wrap);
				}
				{ // シーンと連携のチェックをいれた時にdispnameが更新されないのを修正
					/*
						10084889 ff5754             call    dword ptr [edi+54]
						1008488c 83c410             add     esp,+10
						↓
						10084889 90                 nop
						1008488a e8XxXxXxXx         call    new_func
					*/
					h.store_i16(0x84889 - vp_begin, '\x90\xe8');
					h.replaceNearJmp(0x8488b - vp_begin, &rename_object_sceneaudio_wrap);
				}
			}

			{ // 音声波形表示 の参照ファイル、全体音声チェック、Type切り替え
				constexpr int vp_begin = 0x8f096;
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x8f200 - vp_begin);
				{ // 中間点があっても維持されないようになっているので、複数選択中には再生位置などを維持するのも含めて実装
					/*
						1008f096 8b96f4000000       mov     edx,dword ptr [esi+000000f4]
						↓
						1008f096 90                 nop
						1008f097 e9XxXxXxXx         jmp     cursor

						10000000 ffb6e4000000       push    dword ptr [esi+000000e4]
						10000000 56                 push    esi
						10000000 e8XxXxXxXx         call    newfunc
						10000000 83f801             cmp     eax,+01
						10000000 0f85XxXxXxXx       jnz     ee+8f0e6
						10000000 8b96f4000000       mov     edx,dword ptr [esi+000000f4]
						10000000 e9XxXxXxXx         jmp     ee+8f09c
					*/
					h.store_i16(0x8f096 - vp_begin, '\x90\xe9');
					h.replaceNearJmp(0x8f098 - vp_begin, cursor);

					store_i32(cursor, '\xff\xb6\xe4\x00'); cursor += 4;
					store_i32(cursor, '\x00\x00\x56\xe8'); cursor += 4;
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
						1008f101 56                 push    esi
						1008f102 e8XxXxXxXx         call    newfunc
					*/
					h.store_i16(0x8f101 - vp_begin, '\x56\xe8');
					h.replaceNearJmp(0x8f103 - vp_begin, &update_obj_data_waveform_wrap);
				}
				{ // 編集全体の音声を元にするのチェックをいれた時にdispnameが更新されないのを修正
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
					h.replaceNearJmp(0x8f1fc - vp_begin, &mov_eax_1_waveform_wrap);
				}
			}

			{ // 図形 の図形の種類
				constexpr int vp_begin = 0x74562;
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x74611 - vp_begin);
				h.replaceNearJmp(0x74562 - vp_begin, &rename_object_figure_wrap);

				h.store_i8(0x745f8 - vp_begin, '\xe9');
				h.replaceNearJmp(0x745f9 - vp_begin, cursor); // ファイル選択ダイアログのキャンセルの挙動を追加
				/*
					10000000 e8XxXxXxXx         call    ee+20900
					10000000 85c0               test    eax,eax
					10000000 0f85XxXxXxXx       jnz     ee+745fd
					10000000 83c40c             add     esp,+0c
					10000000 e8XxXxxxXx         call    deselect_object_if
					10000000 e9XxXxXxXx         jmp     ee+74614
				*/
				store_i8(cursor, '\xe8'); cursor++;
				store_i32(cursor, GLOBAL::exedit_base + OFS::ExEdit::dlg_get_load_name - (int)cursor - 4); cursor += 4;
				store_i32(cursor, '\x85\xc0\x0f\x85'); cursor += 4;
				store_i32(cursor, GLOBAL::exedit_base + 0x745fd - (int)cursor - 4); cursor += 4;
				store_i32(cursor, '\x83\xc4\x0c\xe8'); cursor += 4;
				store_i32(cursor, (int)&deselect_object_if - (int)cursor - 4); cursor += 4;
				store_i8(cursor, '\xe9'); cursor++;
				store_i32(cursor, GLOBAL::exedit_base + 0x74614 - (int)cursor - 4); cursor += 4;

				h.replaceNearJmp(0x7460d - vp_begin, &rename_object_figure_wrap);
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

			{ // アニメーション効果・カスタムオブジェクト・カメラ効果・シーンチェンジ のスクリプト変更
				{ // アニメーション効果・カスタムオブジェクト・カメラ効果
					constexpr int vp_begin = 0x3e0b;
					OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x3f12 - vp_begin);
					h.replaceNearJmp(0x3e0c - vp_begin, &init_setting_dialog_script_wrap);
					h.replaceNearJmp(0x3f0e - vp_begin, &init_setting_dialog_script_wrap);
				}
				{ // シーンチェンジ
					constexpr int vp_begin = 0x870f7;
					OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x870fb - vp_begin);
					h.replaceNearJmp(0x870f7 - vp_begin, &init_setting_dialog_scenechange_wrap);
				}
			}
			{ // スクリプト系の設定・色・参照

				/* ダイアログのOK CANCELを取得
					10000000 e8XxXxXxXx         call    originalfunc
					↓
					10000000 e9XxXxXxXx         jmp     cursor

					10000000 e8XxXxXxXx         call    originalfunc
					10000000 a3XxXxXxXx         mov     dword ptr [script_dlg_ok_cancel],eax
					10000000 e9XxXxXxXx         jmp     ret
				*/
				
				{ // アニメーション効果・カスタムオブジェクト・カメラ効果・シーンチェンジ の1st_dlg（設定・色・参照）を変更
					{ // アニメーション効果・カスタムオブジェクト・カメラ効果
						constexpr int vp_begin = 0x3f56;
						OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x3f5a - vp_begin);
						h.replaceNearJmp(0x3f56 - vp_begin, &disp_1st_dlg_script_wrap);
					}
					{ // シーンチェンジ
						constexpr int vp_begin = 0x87140;
						OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x87144 - vp_begin);
						h.replaceNearJmp(0x87140 - vp_begin, &disp_1st_dlg_script_wrap);
					}
						
					{ // パラメータ設定
						{
							/* ダイアログのOK CANCELを取得
								; OK=1 CANCEL=2
								10000000 e8XxXxXxXx         call    originalfunc
								10000000 f7d8               neg     eax
								10000000 83c002             add     eax,+02
								10000000 a3XxXxXxXx         mov     dword ptr [script_dlg_ok_cancel],eax
								10000000 e9XxXxXxXx         jmp     ret
							*/
							constexpr int vp_begin = 0x3476;
							OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x347b - vp_begin);
							h.store_i8(0x3476 - vp_begin, '\xe9');
							h.replaceNearJmp(0x3477 - vp_begin, cursor);
							store_i8(cursor, '\xe8'); cursor++;
							store_i32(cursor, GLOBAL::exedit_base + 0x20800 - (int)cursor - 4); cursor += 4;
							store_i16(cursor, '\xf7\xd8'); cursor += 2;
							store_i32(cursor, '\x83\xc0\x02\xa3'); cursor += 4;
							store_i32(cursor, (int)&script_dlg_ok_cancel); cursor += 4;
							store_i8(cursor, '\xe9'); cursor++;
							store_i32(cursor, GLOBAL::exedit_base + 0x347b - (int)cursor - 4); cursor += 4;
						}
					}
					{ // 色の選択
						constexpr int vp_begin = 0x3188;
						OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x318d - vp_begin);
						h.store_i8(0x3188 - vp_begin, '\xe9');
						h.replaceNearJmp(0x3189 - vp_begin, cursor);
						store_i8(cursor, '\xe8'); cursor++;
						store_i32(cursor, GLOBAL::exedit_base + OFS::ExEdit::exfunc_6c - (int)cursor - 4); cursor += 4;
						store_i8(cursor, '\xa3'); cursor++;
						store_i32(cursor, (int)&script_dlg_ok_cancel); cursor += 4;
						store_i8(cursor, '\xe9'); cursor++;
						store_i32(cursor, GLOBAL::exedit_base + 0x318d - (int)cursor - 4); cursor += 4;
					}
					{ // 参照
						constexpr int vp_begin = 0x325e;
						OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x3263 - vp_begin);
						h.store_i8(0x325e - vp_begin, '\xe9');
						h.replaceNearJmp(0x325f - vp_begin, cursor);
						store_i8(cursor, '\xe8'); cursor++;
						store_i32(cursor, GLOBAL::exedit_base + OFS::ExEdit::dlg_get_load_name - (int)cursor - 4); cursor += 4;
						store_i8(cursor, '\xa3'); cursor++;
						store_i32(cursor, (int)&script_dlg_ok_cancel); cursor += 4;
						store_i8(cursor, '\xe9'); cursor++;
						store_i32(cursor, GLOBAL::exedit_base + 0x3263 - (int)cursor - 4); cursor += 4;
					}
				}
				{ // アニメーション効果・カスタムオブジェクト・カメラ効果・シーンチェンジ の2nd_dlg（設定ボタンありの時の色）を変更
					constexpr int vp_begin = 0x1fa0;
					OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x1ff2 - vp_begin);
					h.store_i8(0x1fa0 - vp_begin, '\xe9');
					h.replaceNearJmp(0x1fa1 - vp_begin, cursor);
					store_i8(cursor, '\xe8'); cursor++;
					store_i32(cursor, GLOBAL::exedit_base + OFS::ExEdit::exfunc_6c - (int)cursor - 4); cursor += 4;
					store_i8(cursor, '\xa3'); cursor++;
					store_i32(cursor, (int)&script_dlg_ok_cancel); cursor += 4;
					store_i8(cursor, '\xe9'); cursor++;
					store_i32(cursor, GLOBAL::exedit_base + 0x1fa5 - (int)cursor - 4); cursor += 4;

					h.replaceNearJmp(0x1fee - vp_begin, &update_script_param_wrap);
				}
			}
			{ // アニメーション効果・カスタムオブジェクト・カメラ効果・シーンチェンジ のトラックバー・チェックを切り替えた時に同スクリプトでなければ除外されるように変更
				constexpr int vp_begin = 0x40ea4;
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x4111a - vp_begin);
				h.replaceNearJmp(0x40ea4 - vp_begin, &get_same_track_id_wrap);
				h.replaceNearJmp(0x40f22 - vp_begin, &get_same_track_id_wrap);
				h.replaceNearJmp(0x40fa8 - vp_begin, &get_same_track_id_wrap);
				h.replaceNearJmp(0x41116 - vp_begin, &get_same_check_id_wrap);
			}

			{ // マスク のマスクの種類
				constexpr int vp_begin = 0x69e7a;
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x69eff - vp_begin);

				h.store_i8(0x69e7a - vp_begin, '\xe9');
				h.replaceNearJmp(0x69e7b - vp_begin, cursor); // ファイル選択ダイアログのキャンセルの挙動を追加
				/*
					10000000 e8XxXxXxXx         call    ee+20900
					10000000 85c0               test    eax,eax
					10000000 0f85XxXxXxXx       jnz     ee+69ef3
					10000000 e8XxXxxxXx         call    deselect_object_if
					10000000 e9XxXxXxXx         jmp     ee+69eff
				*/
				store_i8(cursor, '\xe8'); cursor++;
				store_i32(cursor, GLOBAL::exedit_base + OFS::ExEdit::dlg_get_load_name - (int)cursor - 4); cursor += 4;
				store_i32(cursor, '\x85\xc0\x0f\x85'); cursor += 4;
				store_i32(cursor, GLOBAL::exedit_base + 0x69ef3 - (int)cursor - 4); cursor += 4;
				store_i8(cursor, '\xe8'); cursor++;
				store_i32(cursor, (int)&deselect_object_if - (int)cursor - 4); cursor += 4;
				store_i8(cursor, '\xe9'); cursor++;
				store_i32(cursor, GLOBAL::exedit_base + 0x69eff - (int)cursor - 4); cursor += 4;

				h.replaceNearJmp(0x69efb - vp_begin, &update_dlg_mask_wrap);
			}
			{ // 部分フィルタ のマスクの種類
				constexpr int vp_begin = 0x6e2e3;
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x6e35b - vp_begin);
				h.store_i8(0x6e2e3 - vp_begin, '\xe8');
				h.replaceNearJmp(0x6e2e4 - vp_begin, &mov_eax_1_portion_filter_wrap);
				h.store_i8(0x6e31c - vp_begin, '\xe8');
				h.replaceNearJmp(0x6e31d - vp_begin, &mov_eax_1_portion_filter_wrap);
				h.store_i8(0x6e356 - vp_begin, '\xe8');
				h.replaceNearJmp(0x6e357 - vp_begin, &mov_eax_1_portion_filter_wrap);

				h.store_i8(0x6e30e - vp_begin, '\xe9');
				h.replaceNearJmp(0x6e30f - vp_begin, cursor); // ファイル選択ダイアログのキャンセルの挙動を追加
				/*
					10000000 e8XxXxXxXx         call    ee+20900
					10000000 85c0               test    eax,eax
					10000000 0f85XxXxXxXx       jnz     ee+6e313
					10000000 83c40c             add     esp,+0c
					10000000 e8XxXxxxXx         call    deselect_object_if
					10000000 e9XxXxXxXx         jmp     ee+6e2e0
				*/
				store_i8(cursor, '\xe8'); cursor++;
				store_i32(cursor, GLOBAL::exedit_base + OFS::ExEdit::dlg_get_load_name - (int)cursor - 4); cursor += 4;
				store_i32(cursor, '\x85\xc0\x0f\x85'); cursor += 4;
				store_i32(cursor, GLOBAL::exedit_base + 0x6e313 - (int)cursor - 4); cursor += 4;
				store_i32(cursor, '\x83\xc4\x0c\xe8'); cursor += 4;
				store_i32(cursor, (int)&deselect_object_if - (int)cursor - 4); cursor += 4;
				store_i8(cursor, '\xe9'); cursor++;
				store_i32(cursor, GLOBAL::exedit_base + 0x6e2e0 - (int)cursor - 4); cursor += 4;
			}

			{ // ディスプレイスメントマップ のコンボボックス
				constexpr int vp_begin = 0x200f0;
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x201b1 - vp_begin);

				h.store_i8(0x2012c - vp_begin, '\xe9');
				h.replaceNearJmp(0x2012d - vp_begin, cursor); // ファイル選択ダイアログのキャンセルの挙動を追加
				/*
					10000000 e8XxXxXxXx         call    ee+20900
					10000000 85c0               test    eax,eax
					10000000 0f85XxXxXxXx       jnz     ee+201a1
					10000000 83c40c             add     esp,+0c
					10000000 e8XxXxxxXx         call    deselect_object_if
					10000000 e9XxXxXxXx         jmp     ee+201b4
				*/
				store_i8(cursor, '\xe8'); cursor++;
				store_i32(cursor, GLOBAL::exedit_base + OFS::ExEdit::dlg_get_load_name - (int)cursor - 4); cursor += 4;
				store_i32(cursor, '\x85\xc0\x0f\x85'); cursor += 4;
				store_i32(cursor, GLOBAL::exedit_base + 0x201a1 - (int)cursor - 4); cursor += 4;
				store_i32(cursor, '\x83\xc4\x0c\xe8'); cursor += 4;
				store_i32(cursor, (int)&deselect_object_if - (int)cursor - 4); cursor += 4;
				store_i8(cursor, '\xe9'); cursor++;
				store_i32(cursor, GLOBAL::exedit_base + 0x201b4 - (int)cursor - 4); cursor += 4;

				h.replaceNearJmp(0x200f0 - vp_begin, &update_dlg_displacementmap_wrap);
				h.replaceNearJmp(0x201ad - vp_begin, &update_dlg_displacementmap_wrap);
			}

			{ // クロマキー のキー色の取得
				constexpr int vp_begin = 0x14429;
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x144a1 - vp_begin);
				h.replaceNearJmp(0x14429 - vp_begin, &update_dlg_chromakey_wrap);
				h.replaceNearJmp(0x14442 - vp_begin, &update_dlg_chromakey_wrap);
				h.replaceNearJmp(0x1449d - vp_begin, &update_dlg_chromakey_wrap);
			}
			{ // カラーキー のキー色の取得
				constexpr int vp_begin = 0x168a9;
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x16921 - vp_begin);
				h.replaceNearJmp(0x168a9 - vp_begin, &update_dlg_colorkey_wrap);
				h.replaceNearJmp(0x168c2 - vp_begin, &update_dlg_colorkey_wrap);
				h.replaceNearJmp(0x1691d - vp_begin, &update_dlg_colorkey_wrap);
			}
			{ // 特定色域変換 の色の取得
				constexpr int vp_begin = 0x16080;
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x1615e - vp_begin);
				h.store_i16(0x16080 - vp_begin, '\x90\xe8');
				h.replaceNearJmp(0x16082 - vp_begin, cursor);
				/*
					10016080 66c746060100       mov     word ptr [esi+06],0001
					↓
					10016080 90                 nop
					10016081 e8XxXxXxXx         call    cursor

					10000000 50                 push    eax
					10000000 57                 push    edi
					10000000 e8XxXxXxXx         call    newfunc
					10000000 58                 pop     eax
					10000000 c3                 ret
				*/
				store_i32(cursor, '\x50\x57\xe8\x00'); cursor += 3;
				store_i32(cursor, (int)&mov_status_1_specialcolorconv - (int)cursor - 4); cursor += 4;
				store_i16(cursor, '\x58\xc3'); cursor += 2;

				h.store_i16(0x160a5 - vp_begin, '\x57\xe8');
				h.replaceNearJmp(0x160a7 - vp_begin, &mov_status2_1_specialcolorconv);
				h.store_i16(0x160c2 - vp_begin, '\x57\xe8');
				h.replaceNearJmp(0x160c4 - vp_begin, &mov_status_0_specialcolorconv);
				h.store_i16(0x1612b - vp_begin, '\x57\xe8');
				h.replaceNearJmp(0x1612d - vp_begin, &mov_status_0_specialcolorconv);
				h.store_i16(0x160ce - vp_begin, '\x57\xe8');
				h.replaceNearJmp(0x160d0 - vp_begin, &mov_status2_0_specialcolorconv);
				h.store_i16(0x16158 - vp_begin, '\x57\xe8');
				h.replaceNearJmp(0x1615a - vp_begin, &mov_status2_0_specialcolorconv);
			}

			{ // 色ずれ・インターレース解除・ルミナンスキー・ミラー のコンボボックス
				constexpr int vp_begin = 0x180b8;
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 5);
				h.store_i8(0x180b8 - vp_begin, '\xe8');
				h.replaceNearJmp(0x180b9 - vp_begin, &mov_eax_1_use0_e1_wrap);
			}
			{ // グラデーション のコンボボックス
				constexpr int vp_begin = 0x59b8d;
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x59bb1 - vp_begin);
				h.store_i8(0x59b8d - vp_begin, '\xe8');
				h.replaceNearJmp(0x59b8e - vp_begin, &mov_eax_1_use0_e1_wrap);
				h.store_i8(0x59bac - vp_begin, '\xe8');
				h.replaceNearJmp(0x59bad - vp_begin, &mov_eax_1_use5_e1_wrap);
			}
			{ // 閃光 のコンボボックス
				constexpr int vp_begin = 0x4f437;
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 5);
				h.store_i8(0x4f437 - vp_begin, '\xe8');
				h.replaceNearJmp(0x4f438 - vp_begin, &mov_eax_1_use2_e2_wrap);
			}
			{ // グロー のコンボボックス
				constexpr int vp_begin = 0x58ef5;
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 5);
				h.store_i8(0x58ef5 - vp_begin, '\xe8');
				h.replaceNearJmp(0x58ef6 - vp_begin, &mov_eax_1_use2_e1_wrap);
			}
			{ // ワイプ のコンボボックス
				constexpr int vp_begin = 0x91751;
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x9178c - vp_begin);
				h.store_i8(0x91751 - vp_begin, '\xe8');
				h.replaceNearJmp(0x91752 - vp_begin, &mov_eax_1_use0use1_e1_wrap);
				h.store_i8(0x91787 - vp_begin, '\xe8');
				h.replaceNearJmp(0x91788 - vp_begin, &mov_eax_1_use0use1_e1_wrap);
			}
			{ // シャドー の影色の設定・パターン画像ファイル
				constexpr int vp_begin = 0x88e2d;
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x88f13 - vp_begin);
				h.replaceNearJmp(0x88e2d - vp_begin, &update_dlg_shadow_wrap); // パターン画像ファイル設定済→影色の変更に対応
				h.replaceNearJmp(0x88f0f - vp_begin, &update_dlg_shadow_wrap);
			}
			{ // 縁取り の縁色の設定・パターン画像ファイル
				constexpr int vp_begin = 0x520fa;
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x521e0 - vp_begin);
				h.replaceNearJmp(0x520fa - vp_begin, &update_dlg_border_wrap); // パターン画像ファイル設定済→縁色の変更に対応
				h.replaceNearJmp(0x521dc - vp_begin, &update_dlg_border_wrap);
			}
			{ // ノイズ のコンボボックス
				constexpr int vp_begin = 0x6d73d;
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x6d751 - vp_begin);
				h.store_i8(0x6d73d - vp_begin, '\xe8');
				h.replaceNearJmp(0x6d73e - vp_begin, &mov_eax_1_use1_e1_wrap);
				h.store_i8(0x6d74c - vp_begin, '\xe8');
				h.replaceNearJmp(0x6d74d - vp_begin, &mov_eax_1_use0_e1_wrap);
			}
			{ // 動画ファイル合成 のコンボボックス
				/*
					1000688b b801000000         mov     eax,00000001
					10006890 5b                 pop     ebx
					10006891 81c478010000       add     esp,00000178
					↓
					1000688b 5b                 pop     ebx
					1000688c 81c478010000       add     esp,00000178
					10006892 e8XxXxXxXx         call    newfunc
				*/
				constexpr int vp_begin = 0x688b;
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x6897 - vp_begin);
				h.store_i32(0x688b - vp_begin, '\x5b\x81\xc4\x78');
				h.store_i32(0x688f - vp_begin, '\x01\x00\x00\xe8');
				h.replaceNearJmp(0x6893 - vp_begin, &mov_eax_1_use2_e0_wrap);
			}
			{ // 画像ファイル合成 のコンボボックス
				/*
					1000e1e4 b801000000         mov     eax,00000001
					1000e1e9 5b                 pop     ebx
					1000e1ea 81c40c010000       add     esp,0000010c
					↓
					1000e1e4 5b                 pop     ebx
					1000e1e5 81c40c010000       add     esp,0000010c
					1000e1eb e8XxXxXxXx         call    newfunc
				*/
				constexpr int vp_begin = 0xe1e4;
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0xe1f0 - vp_begin);
				h.store_i32(0xe1e4 - vp_begin, '\x5b\x81\xc4\x0c');
				h.store_i32(0xe1e8 - vp_begin, '\x01\x00\x00\xe8');
				h.replaceNearJmp(0xe1ec - vp_begin, &mov_eax_1_use0_e0_wrap);
			}

			{ // カメラ制御の対象・上のオブジェクトでクリッピング の切り替え
				constexpr int vp_begin = 0x43562;
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x435c7 - vp_begin);
				h.replaceNearJmp(0x43562 - vp_begin, &update_obj_data_camera_target_wrap);
				h.replaceNearJmp(0x435c3 - vp_begin, &update_obj_data_before_clipping_wrap);
			}

			{ // フィルタ効果の削除
				ReplaceNearJmp(GLOBAL::exedit_base + 0x41b44, &delete_filter_effect_wrap);
			}

			{ // トラックの数値の場所をクリックして動かしたときに選択が解除されないようにする
				{ // Ctrlの状態にかかわらず解除されるようになっていたので変更
					ReplaceNearJmp(GLOBAL::exedit_base + 0x3bcbe, &deselect_object_if);
				}
				{ // 上記変更により解除がされなくなってしまうため
					/* WM_KEYUPのみで判定しているところにWM_SYSKEYUPも加える
						1002cfbe 2d01010000         sub     eax, 00000101
						↓
						1002cfbe e8XxXxXxXx         call    cursor

						10000000 2d01010000         sub     eax, 00000101
						10000000 7403               jz      skip,3
						10000000 83f804             cmp     eax,+04
						10000000 c3                 ret
					*/

					OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x2cfbe, 5);
					h.store_i8(0, '\xe8');
					h.replaceNearJmp(1, cursor);
					store_i32(cursor, '\x2d\x01\x01\x00'); cursor += 4;
					store_i32(cursor, '\x00\x74\x03\x83'); cursor += 3;
					store_i32(cursor, '\x83\xf8\x04\xc3'); cursor += 4;
				}
			}
			{ // 操作によっては選択状態が解除されないままになるので拡張編集ウィンドウをアクティブにした時に判定する
				/*
					1003b7f8 0f854e830000       jnz     10043b4c
					↓
					1003b7f8 0f85XxXxXxXx       call    cursor

					10000000 e8XxXxxxXx         call    post_deselect_object_if
					10000000 e9XxXxXxXx         jmp     ee+43b4c
				*/
				OverWriteOnProtectHelper(GLOBAL::exedit_base + 0x3b7fa, 4).replaceNearJmp(0, cursor);

				store_i8(cursor, '\xe8'); cursor++;
				store_i32(cursor, (int)&post_deselect_object_tl_activate - (int)cursor - 4); cursor += 4;
				store_i8(cursor, '\xe9'); cursor++;
				store_i32(cursor, GLOBAL::exedit_base + 0x43b4c - (int)cursor - 4); cursor += 4;
				
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
