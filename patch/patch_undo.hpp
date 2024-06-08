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
#ifdef PATCH_SWITCH_UNDO

#include <exedit.hpp>
#include "util_magic.hpp"
#include "global.hpp"
#include "offset_address.hpp"
#include "config_rw.hpp"

// ty saunazo

namespace patch {

    // init at exedit load
    inline class undo_t {

        inline static ExEdit::Object** ObjectArrayPointer_ptr;
        inline static int* NextObjectIdxArray;
        inline static ExEdit::LayerSetting** layer_setting_ofsptr_ptr;
        inline static void** exdata_buffer_ptr;
        inline static int* timeline_obj_click_mode_ptr;
        inline static int* ObjDlg_ObjectIndex_ptr;
        inline static int* timeline_edit_both_adjacent_ptr;
        inline static int* UndoInfo_current_id_ptr;

        inline static ExEdit::SceneSetting* scene_setting;

        inline static void(__cdecl*set_undo)(unsigned int, unsigned int);
        inline static void(__cdecl*AddUndoCount)();
        inline static int(__cdecl*efDraw_func_WndProc)(HWND, UINT, WPARAM, LPARAM, AviUtl::EditHandle*, ExEdit::Filter*);
        inline static int(__cdecl*NormalizeExeditTimelineY)(int);
        inline static void(__cdecl *add_track_value)(ExEdit::Filter*, int, int);
        
        inline constexpr static int UNDO_INTERVAL = 1000;

		static void __cdecl set_undo_wrap_42878(unsigned int object_idx, int layer_id) {
			if (layer_id < (*ObjectArrayPointer_ptr)[object_idx].layer_disp) {
				set_undo(object_idx, 8);
			}
		}

		static void __cdecl set_undo_wrap_40e5c(unsigned int object_idx, unsigned int flag) {
			// select_idx_list
			set_undo(reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::SelectingObjectIdxArray)[object_idx], flag);
		}

        static int __cdecl change_any_exdata_set_undo(unsigned int select_id, void* dst, void* src, int size) {
            if (memcmp(dst, src, size)) {
                set_undo(reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::SelectingObjectIdxArray)[select_id], 0);
                return size;
            }
            return 0;
        }

        inline constexpr static int FILTER_ID_MOVIE = 0; // track 0 check 0 exdata 268 = maxframe
        inline constexpr static int FILTER_ID_AUDIO = 2; // track 0 check 0,1 exdata 268 = maxframe
        inline constexpr static int FILTER_ID_WAVEFORM = 6; // track 0 check 3 exdata 268 = maxframe
        inline constexpr static int FILTER_ID_SCENE = 7; // track 0 check 0 exdata 0 = sceneid
        inline constexpr static int FILTER_ID_SCENE_AUDIO = 8; // track 0 check 0,1 exdata 0 = sceneid
        inline constexpr static int FILTER_ID_MOVIE_MIX = 82; // track 0 check 0 exdata 268 = maxframe

        static void __cdecl set_undo_wrap_3e037(unsigned int object_idx, unsigned int flag);

        static int __cdecl f8d506(int object_idx);

        static int __cdecl efDraw_func_WndProc_wrap_06e2b4(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, AviUtl::EditHandle* editp, ExEdit::Filter* efp);

        static int __stdcall f8b97f(HWND hwnd, ExEdit::Filter* efp, WPARAM wparam, LPARAM lparam);

        static int __stdcall f8ba87_8bad5(ExEdit::Filter* efp, HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

        static BOOL __cdecl update_any_exdata_wrap(int offset, char* exdata_str, int8_t* exdata, ExEdit::Filter* efp, int value);

        static int* __stdcall f59e27(WPARAM wparam, LPARAM lparam, ExEdit::Filter* efp, UINT message);

        static int __stdcall f8b9f0(ExEdit::Filter* efp, HWND hWnd, LPWSTR lpString, int nMaxCount);
        
        static int __stdcall f875ef(ExEdit::Filter* efp, HWND hWnd, LPWSTR lpString);

        static int __cdecl NormalizeExeditTimelineY_wrap_3c8fa_42629_42662_42924_42a0a(int timeline_y);

        static int __cdecl NormalizeExeditTimelineY_wrap_4253e(int timeline_y);

        static void __cdecl set_null_terminated_string_wrap(char* str, int layer_id);

        static ExEdit::UndoData* __stdcall set_undodata_layer_plus(ExEdit::UndoData* undodata, int layer_id);
        static void __stdcall set_layer_undodata_plus(ExEdit::UndoData* undodata, int layer_id);

        static ExEdit::Object* __stdcall f42617();

        static void __stdcall f4355c(ExEdit::Object* obj);

        static void __stdcall f435bd(ExEdit::Object* obj);

        static void __cdecl add_track_value_wrap(ExEdit::Filter* efp, int track_id, int add_value);

        static void interval_set_undo(int object_idx, int flag) {
            static ULONGLONG pretime = 0;
            static int pre_undo_id = 0;
            int& UndoInfo_current_id = *UndoInfo_current_id_ptr;
            ULONGLONG time = GetTickCount64();
            if (pretime < time - UNDO_INTERVAL || pre_undo_id != UndoInfo_current_id) {
                AddUndoCount();
                set_undo(object_idx, flag);
            }
            pretime = time;
            pre_undo_id = UndoInfo_current_id;
        }

        bool enabled = true;
        bool enabled_i;

        inline static const char key[] = "undo";

    public:

		void init() {
            enabled_i = enabled;
            if (!enabled_i) return;

            ObjectArrayPointer_ptr = reinterpret_cast<decltype(ObjectArrayPointer_ptr)>(GLOBAL::exedit_base + OFS::ExEdit::ObjectArrayPointer);
            NextObjectIdxArray = reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::NextObjectIdxArray);
            layer_setting_ofsptr_ptr = reinterpret_cast<decltype(layer_setting_ofsptr_ptr)>(GLOBAL::exedit_base + 0x0a4058);
            exdata_buffer_ptr = reinterpret_cast<void**>(GLOBAL::exedit_base + 0x1e0fa8);
            timeline_obj_click_mode_ptr = reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::timeline_obj_click_mode);
            ObjDlg_ObjectIndex_ptr = reinterpret_cast<int*>(GLOBAL::exedit_base + 0x177a10);
            timeline_edit_both_adjacent_ptr = reinterpret_cast<int*>(GLOBAL::exedit_base + 0x14ea00);
            scene_setting = reinterpret_cast<decltype(scene_setting)>(GLOBAL::exedit_base + 0x177a50);
            UndoInfo_current_id_ptr = reinterpret_cast<decltype(UndoInfo_current_id_ptr)>(GLOBAL::exedit_base + 0x244e14);
			
            set_undo = reinterpret_cast<decltype(set_undo)>(GLOBAL::exedit_base + 0x08d290);
            AddUndoCount = reinterpret_cast<decltype(AddUndoCount)>(GLOBAL::exedit_base + 0x08d150);
            efDraw_func_WndProc = reinterpret_cast<decltype(efDraw_func_WndProc)>(GLOBAL::exedit_base + 0x01b550);
            NormalizeExeditTimelineY = reinterpret_cast<decltype(NormalizeExeditTimelineY)>(GLOBAL::exedit_base + 0x032c10);
            add_track_value = reinterpret_cast<decltype(add_track_value)>(GLOBAL::exedit_base + 0x01c0f0);

            auto& cursor = GLOBAL::executable_memory_cursor;

			// レイヤー削除→元に戻すで他シーンのオブジェクトが消える
			{
                constexpr int vp_begin = 0x042875;
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x04287e - vp_begin);
				h.store_i16(0x042875 - vp_begin, '\x56\x90'); // push esi=layer_id  nop
                h.replaceNearJmp(0x042879 - vp_begin, &set_undo_wrap_42878);
			}

			// Ctrlで複数オブジェクトを選択しながらトラックバーを動かすと一部オブジェクトが正常に戻らない
			ReplaceNearJmp(GLOBAL::exedit_base + 0x040e5d, &set_undo_wrap_40e5c);

            // Ctrlで複数オブジェクトを選択しながら一部フィルタの色などを変更すると正常に戻らない
            {
                /*
                1004a933 03f8               add     edi,eax
                1004a935 c1e902             shr     ecx,02
                ↓
                1004a933 e8XxXxXxXx         call    cursor

                10000000 03f8               add     edi,eax
                10000000 52                 push    edx
                10000000 51                 push    ecx
                10000000 56                 push    esi
                10000000 57                 push    edi
                10000000 ff742424           push    dword ptr [esp+24] ; select_id
                10000000 e8XxXxXxXx         call    new func
                10000000 83c410             add     esp,10
                10000000 5a                 pop     edx
                10000000 8bc8               mov     ecx,eax
                10000000 c1e902             shr     ecx,02
                10000000 c3                 ret
                */

                static const char code_put[] = {
                    "\x03\xf8"                 // add     edi,eax
                    "\x52"                     // push    edx
                    "\x51"                     // push    ecx
                    "\x56"                     // push    esi
                    "\x57"                     // push    edi
                    "\xff\x74\x24\x24"         // push    dword ptr [esp+24] ; select_id
                    "\xe8XXXX"                 // call    new func
                    "\x83\xc4\x10"             // add     esp,10
                    "\x5a"                     // pop     edx
                    "\x8b\xc8"                 // mov     ecx,eax
                    "\xc1\xe9\x02"             // shr     ecx,02
                    "\xc3"                     // ret
                };

                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x04a933, 5);
                h.store_i8(0, '\xe8');
                h.replaceNearJmp(1, cursor);

                memcpy(cursor, code_put, sizeof(code_put) - 1);
                store_i32(cursor + 11, (int)&change_any_exdata_set_undo - ((int)cursor + 15));
                cursor += sizeof(code_put) - 1;

            }


			// オブジェクトの左端をつまんで動かすと再生位置パラメータが変わるが、それが元に戻らない
			ReplaceNearJmp(GLOBAL::exedit_base + 0x03e038, &set_undo_wrap_3e037);
			
            // 中間点ありオブジェクトで色などを変更→元に戻すで設定ダイアログが更新されない
			// 一部フィルタのファイル参照を変更→元に戻すで設定ダイアログが更新されない(音声波形など)
            {
                /*
                    1008d505 a1107a1710         mov     eax,[10177a10]
                    1008d50a 3bd8               cmp     ebx,eax
                    1008d50c 751d               jnz     1008d52b
                    1008d50e 3bde               cmp     ebx,esi
                    1008d510 7419               jz      1008d52b
                    1008d512 f7c300000001       test    ebx,01000000
                    1008d518 895c2410           mov     dword ptr [esp+10],ebx
                    ↓
                    1008d505 52                 push    edx
                    1008d506 53                 push    ebx
                    1008d507 e8XxXxXxXx         call    f8d507
                    1008d50c 5b                 pop     ebx
                    1008d50d 5a                 pop     edx
                    1008d50e 85c0               test    eax,eax
                    1008d510 7c19               jl      1008d52b
                    1008d512 f7c300000001       test    ebx,01000000
                    1008d518 89442410           mov     dword ptr [esp+10],eax
                */

                constexpr int vp_begin = 0x8d505;
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x8d51a - vp_begin);
                h.store_i32(0x8d505 - vp_begin, '\x52\x53\xe8\x00');
                h.replaceNearJmp(0x8d508 - vp_begin, &f8d506);
                h.store_i32(0x8d50c - vp_begin, '\x5b\x5a\x85\xc0');
                h.store_i8(0x8d510 - vp_begin, '\x7c');
                h.store_i8(0x8d519 - vp_begin, '\x44');
            }
			// OverWriteOnProtectHelper(GLOBAL::exedit_base + 0x08d50e, 4).store_i32(0, '\x0f\x1f\x40\x00'); // nop

            // グループ化されたオブジェクトの中間点を分割した場合にグループ化IDを正しく戻せなくなるのを修正
            {
                /*
                    1003fc72 898abc040000       mov     dword ptr [edx+000004bc],ecx
                    ↓
                    1003fc72 90                 nop
                    1003fc73 e8XxXxXxXx         call    cursor

                    10000000 51                 push    ecx
                    10000000 52                 push    edx ; obj_ofs
                    10000000 e8XxXxXxXx         call    object2idx
                    10000000 53                 push    ebx ; 0
                    10000000 50                 push    eax
                    10000000 e8XxXxXxXx         call    set_undo
                    10000000 83c408             add     esp,+08
                    10000000 5a                 pop     edx
                    10000000 59                 pop     ecx
                    10000000 898abc040000       mov     dword ptr [edx+000004bc],ecx
                    10000000 c3                 ret
                */

                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x3fc72, 6);
                h.store_i16(0, '\x90\xe8');
                h.replaceNearJmp(2, cursor);
                store_i32(cursor, '\x51\x52\xe8\x00'); cursor += 3;
                store_i32(cursor, GLOBAL::exedit_base + OFS::ExEdit::object2idx - (uint32_t)(cursor + 4)); cursor += 4;
                store_i32(cursor, '\x53\x50\xe8\x00'); cursor += 3;
                store_i32(cursor, GLOBAL::exedit_base + OFS::ExEdit::set_undo - (uint32_t)(cursor + 4)); cursor += 4;
                store_i32(cursor, '\x83\xc4\x08\x5a'); cursor += 4;
                store_i32(cursor, '\x59\x89\x8a\xbc'); cursor += 4;
                store_i32(cursor, '\x04\x00\x00\xc3'); cursor += 4;
            }

            // 動画ファイル合成のコンボボックスを変更してもUndoデータが生成されない
            {
                /*
                    1000687c 0f840af9ffff       jz      1000618c
                    ↓
                    1000687c 90                 nop
                    1000687d e9XxXxXxXx         jmp     cursor

                    10000000 0f84XxXxXxXx       jz      ee+618c
                    10000000 50                 push    eax
                    10000000 8b5664             mov     edx,dword ptr [esi+64]
                    10000000 6a00               push    +00
                    10000000 ffb6e4000000       push    dword ptr [esi+000000e4]
                    10000000 ff9280000000       call    dword ptr [edx+00000080]
                    10000000 83c408             add     esp,+08
                    10000000 58                 pop     eax
                    10000000 e9XxXxXxXx         jmp     ee+6882
                */
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x687c, 6);
                h.store_i16(0, '\x90\xe9'); // nop; jmp;
                h.replaceNearJmp(2, cursor);

                store_i16(cursor, '\x0f\x84'); cursor += 2; // jz
                store_i32(cursor, GLOBAL::exedit_base + 0x618c - (uint32_t)(cursor + 4)); cursor += 4;
                static const char code_put[] = {
                    "\x50"                     // push    eax
                    "\x8b\x56\x64"             // mov     edx,dword ptr [esi+64]
                    "\x6a\x00"                 // push    +00
                    "\xff\xb6\xe4\x00\x00\x00" // push    dword ptr [esi+000000e4]
                    "\xff\x92\x80\x00\x00\x00" // call    dword ptr [edx+00000080]
                    "\x83\xc4\x08"             // add     esp,+08
                    "\x58"                     // pop     eax
                    "\xe9"                     // jmp     ee+6882
                };
                memcpy(cursor, code_put, sizeof(code_put) - 1); cursor += sizeof(code_put) - 1;
                store_i32(cursor, GLOBAL::exedit_base + 0x6882 - (uint32_t)(cursor + 4)); cursor += 4;
            }

            // 部分フィルタのマスクの種類を変更してもUndoデータが生成されない
            ReplaceNearJmp(GLOBAL::exedit_base + 0x06e2b5, &efDraw_func_WndProc_wrap_06e2b4);

            // テキストオブジェクトのフォントを変更してもUndoデータが生成されない
            {
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x08b97c, 8);
                h.store_i32(0, '\x90\x57\x51\xe8'); // nop; push edi=efp; push ecx; call (rel32)
                h.replaceNearJmp(4, &f8b97f);
            }

            // テキストオブジェクトの影付き・縁付きを変更してもUndoデータが生成されない
            {
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x08ba86, 2);
                h.store_i16(0, '\x57\xe8'); // push edi=efp; call (rel32)
                ReplaceNearJmp(GLOBAL::exedit_base + 0x08ba88, &f8ba87_8bad5);
            }

            // テキストオブジェクトの文字配置(左寄せ[上]など)を変更してもUndoデータが生成されない
            {
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x08bad4, 6);
                h.store_i16(0, '\x57\xe8'); // push edi=efp; call (rel32)
                h.replaceNearJmp(2, &f8ba87_8bad5);
            }

            // テキストオブジェクトの字間・行間を変更してもUndoデータが生成されない
            // 複数選択状態で何かしら操作をした時に字間・行間が意図せず変わることがあるのを修正
            // 設定ダイアログのテキストを開くと余計な描画処理が2回走るのを修正
            {
                constexpr int vp_begin = 0x8bb5e;
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x8bbf7 - vp_begin);
                {
                    /* 字間
                        1008bb5e 884605             mov     [esi+05],al
                        1008bb61 8b8fe4000000       mov     ecx,dword ptr [edi+000000e4]
                        1008bb67 68389a0b10         push    100b9a38
                        1008bb6c 51                 push    ecx
                        1008bb6d e86eecfbff         call    update_any_exdata
                        1008bb72 83c408             add     esp,+08
                        1008bb75 b801000000         mov     eax,00000001
                        ↓
                        1008bb5e 50                 push    eax ; value
                        1008bb5f 57                 push    edi ; efp
                        1008bb60 56                 push    esi ; exdata
                        1008bb61 90                 nop
                        1008bb62 b905000000         mov     ecx,00000005
                        1008bb67 68389a0b10         push    100b9a38
                        1008bb6c 51                 push    ecx
                        1008bb6d e8XxXxXxXx         call    update_any_exdata_wrap
                        1008bb72 83c414             add     esp,+14
                        1008bb75 0f1f440000         nop


                        exdata[5] = value;
                        update_any_exdata(efp->processing, s_spacing_x_100b9a38);
                        return TRUE;
                        ↓
                        if(exdata[5] == value)return FALSE; // 変更点が無ければ描画しない＆複数選択オブジェクトを書き換えない
                        efp->exfunc->set_undo(efp->processing, 0);
                        exdata[5] = value;
                        update_any_exdata(efp->processing, s_spacing_x_100b9a38);
                        return TRUE;
                    */
                    h.store_i32(0x8bb5e - vp_begin, '\x50\x57\x56\x90');
                    h.store_i16(0x8bb62 - vp_begin, '\xb9\x05');
                    h.replaceNearJmp(0x8bb6e - vp_begin, &update_any_exdata_wrap);
                    h.store_i32(0x8bb74 - vp_begin, '\x14\x0f\x1f\x44');
                }
                {
                    /* 行間
                        1008bbdd 884606             mov     [esi+06],al
                        1008bbe0 8b87e4000000       mov     eax,dword ptr [edi+000000e4]
                        1008bbe6 682c9a0b10         push    100b9a2c
                        1008bbeb 50                 push    eax
                        1008bbec e8efebfbff         call    update_any_exdata
                        1008bbf1 83c408             add     esp,+08
                        1008bbf4 b801000000         mov     eax,00000001
                        ↓
                        1008bbdd 50                 push    eax ; value
                        1008bbde 57                 push    edi ; efp
                        1008bbdf 56                 push    esi ; exdata
                        1008bbe0 90                 nop
                        1008bbe1 b806000000         mov     eax,00000006
                        1008bbe6 682c9a0b10         push    100b9a2c
                        1008bbeb 50                 push    eax
                        1008bbec e8XxXxXxXx         call    update_any_exdata_wrap
                        1008bbf1 83c414             add     esp,+14
                        1008bbf4 0f1f440000         nop
                    */
                    h.store_i32(0x8bbdd - vp_begin, '\x50\x57\x56\x90');
                    h.store_i16(0x8bbe1 - vp_begin, '\xb8\x06');
                    h.replaceNearJmp(0x8bbed - vp_begin, &update_any_exdata_wrap);
                    h.store_i32(0x8bbf3 - vp_begin, '\x14\x0f\x1f\x44');
                }
            }

            // グループ制御とかの対象レイヤー数を変更してもUndoデータが生成されない
            {
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x059e1b, 20);
                const char patch[] = {
                    "\x51" // push ecx=message
                    "\x50" // push eax=efp
                    "\x8b\x4c\x24\x28" // mov ecx, dword ptr[esp + 0x28]=lparam
                    "\x51" // push ecx
                    "\x8b\x4c\x24\x28" // mov ecx, dword ptr[esp + 0x28]=wparam
                    "\x51" // push ecx
                    "\xe8XXXX" // call rel32
                    "\x85\xc0" // test eax, eax
                    "\x74" /* 0x6e */ // JZ +0x6e
                };
                memcpy(reinterpret_cast<void*>(h.address()), patch, sizeof(patch) - 1);
                h.replaceNearJmp(13, &f59e27);
            }

            // テキストを変更してもUndoデータが生成されない
            {
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x08b9ef, 6);
                h.store_i16(0, '\x57\xe8'); // push edi; call (rel32)
                h.replaceNearJmp(2, &f8b9f0);
            }

            // スクリプト制御・カメラスクリプトを変更してもUndoデータが生成されない
            {
                // 100875e7 68 00 04 00 00    PUSH       0x400
                // 100875ec 56                PUSH       ESI
                // 100875ed 51                PUSH       ECX
                // 100875ee ff 15 54 a2 09 10 CALL       dword ptr [->USER32.DLL::GetWindowTextW] = 0009bc30
                // ↓
                // 100875e7 8b 54 24 38       MOV EDX, DWORD PTR [ESP+38H]
                // 100875eb 90                NOP
                // 100875ec 56                PUSH ESI
                // 100875ed 51                PUSH ECX
                // 100875ee 52                PUSH EDX
                // 100875ef e8 XXXX           CALL rel32
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x0875e7, 13);
                h.store_i32(0, '\x8b\x54\x24\x38');
                h.store_i32(4, '\x90\x56\x51\x52');
                h.store_i8(8, '\xe8');
                h.replaceNearJmp(9, &f875ef);
            }


            // 左クリックよりレイヤーの表示状態を変更してもUndoデータが生成されない
            ReplaceNearJmp(GLOBAL::exedit_base + 0x03c8fb, &NormalizeExeditTimelineY_wrap_3c8fa_42629_42662_42924_42a0a);

            // 右クリックメニューよりレイヤーの表示状態を変更してもUndoデータが生成されない
            ReplaceNearJmp(GLOBAL::exedit_base + 0x04262a, &NormalizeExeditTimelineY_wrap_3c8fa_42629_42662_42924_42a0a);

            // 右クリックメニューよりレイヤーのロック状態を変更してもUndoデータが生成されない
            ReplaceNearJmp(GLOBAL::exedit_base + 0x042663, &NormalizeExeditTimelineY_wrap_3c8fa_42629_42662_42924_42a0a);

            // 右クリックメニューよりレイヤーの座標のリンク状態を変更してもUndoデータが生成されない
            ReplaceNearJmp(GLOBAL::exedit_base + 0x042925, &NormalizeExeditTimelineY_wrap_3c8fa_42629_42662_42924_42a0a);

            // 右クリックメニューより上クリッピング状態を変更してもUndoデータが生成されない
            ReplaceNearJmp(GLOBAL::exedit_base + 0x042a0b, &NormalizeExeditTimelineY_wrap_3c8fa_42629_42662_42924_42a0a);

            // 右クリックメニューより他のレイヤーを全表示/非表示を押してもUndoデータが生成されない
            ReplaceNearJmp(GLOBAL::exedit_base + 0x04253f, &NormalizeExeditTimelineY_wrap_4253e);

            { // 右クリックメニューよりレイヤー名を変更してもUndoデータが生成されない
                /*
                    100426e3 6a40               push    +40
                    100426e5 68c8f01110         push    1011f0c8
                    100426ea e83180feff         call    1002a720
                    ↓
                    100426e3 56                 push    esi
                    100426e4 90                 nop
                    100426e5 68c8f01110         push    1011f0c8
                    100426ea e8XxXxXxXx         call    newfunc
                */
                constexpr int vp_begin = 0x426e3;
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x426ef - vp_begin);
                h.store_i16(0x426e3 - vp_begin, '\x56\x90');
                h.replaceNearJmp(0x426eb - vp_begin, &set_null_terminated_string_wrap);
            }
            { // 元に戻すによってレイヤー名が勝手に変わってしまうのを修正
                // ついでにUndoData.object_layer_dispにシーン番号を保存しておく
                
                { // set_undo内を変える
                    /* layer_nameを入れるために0x40バイト分多くする
                        1008d339 bb24000000         mov     ebx,00000024
                        ↓
                        1008d339 bb64000000         mov     ebx,00000064
                    */
                    OverWriteOnProtectHelper(GLOBAL::exedit_base + 0x8d33a, 1).store_i8(0, '\x64');
                    /* undodataにlayer_nameを直接保存する
                        1008d3f0 8b54ea04           mov     edx,dword ptr [edx+ebp*8+04]
                        1008d3f4 895020             mov     dword ptr [eax+20],edx
                        ↓
                        1008d3f0 55                 push    ebp ; layer_id
                        1008d3f1 50                 push    eax ; undobuffer_ptr
                        1008d3f2 e8XxXxXxXx         call    func_stdcall_ret_arg0
                    */
                    constexpr int vp_begin = 0x8d3f0;
                    OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x8d3f7 - vp_begin);
                    h.store_i32(0x8d3f0 - vp_begin, '\x55\x50\xe8\x00');
                    h.replaceNearJmp(0x8d3f3 - vp_begin, &set_undodata_layer_plus);
                }
                { // run_undo内を変える
                    /*
                        1008d552 8b4d20             mov     ecx,dword ptr [ebp+20]
                        1008d555 894cda04           mov     dword ptr [edx+ebx*8+04],ecx
                        ↓
                        1008d552 53                 push    ebx ; layer_id
                        1008d553 55                 push    ebp ; undobuffer_ptr
                        1008d554 e8XxXxXxXx         call    func_stdcall
                    */
                    constexpr int vp_begin = 0x8d552;
                    OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x8d559 - vp_begin);
                    h.store_i32(0x8d552 - vp_begin, '\x53\x55\xe8\x00');
                    h.replaceNearJmp(0x8d555 - vp_begin, &set_layer_undodata_plus);
                }
            }

            // ショートカットよりレイヤーの表示状態を変更してもUndoデータが生成されない
            {
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x042617, 5);
                h.store_i8(0, '\xe8');
                h.replaceNearJmp(1, &f42617);
            }

            // カメラ制御の対象 を切り替えてもUndoデータが生成されない
            {
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x04355b, 6);
                h.store_i16(0, '\x51\xe8'); // push ecx, call (rel32)
                h.replaceNearJmp(2, &f4355c);
            }
            // 上のオブジェクトでクリッピング を切り替えてもUndoデータが生成されない
            {
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x0435ba, 8);
                h.store_i32(0, '\x90\x90\x50\xe8'); // nop, push eax, call (rel32)
                h.replaceNearJmp(4, &f435bd);
            }

            // テンキー2468+-*/ Ctrl+テンキー2468 で(座標XY 回転 拡大率 中心XY)トラックバーを変えてもUndoデータが生成されない
            ReplaceNearJmp(GLOBAL::exedit_base + 0x01b611, &add_track_value_wrap); // テンキー4座標X-
            ReplaceNearJmp(GLOBAL::exedit_base + 0x01b646, &add_track_value_wrap); // Ctrl+テンキー6中心X+
            ReplaceNearJmp(GLOBAL::exedit_base + 0x01b674, &add_track_value_wrap); // テンキー6座標X+
            ReplaceNearJmp(GLOBAL::exedit_base + 0x01b6ab, &add_track_value_wrap); // Ctrl+テンキー8中心Y-
            ReplaceNearJmp(GLOBAL::exedit_base + 0x01b6db, &add_track_value_wrap); // テンキー8座標Y-
            ReplaceNearJmp(GLOBAL::exedit_base + 0x01b710, &add_track_value_wrap); // Ctrl+テンキー2中心Y+, Ctrl+テンキー4中心X-
            ReplaceNearJmp(GLOBAL::exedit_base + 0x01b73e, &add_track_value_wrap); // テンキー2座標Y+
            ReplaceNearJmp(GLOBAL::exedit_base + 0x01b765, &add_track_value_wrap); // テンキー-拡大率-
            ReplaceNearJmp(GLOBAL::exedit_base + 0x01b78c, &add_track_value_wrap); // テンキー+拡大率+
            ReplaceNearJmp(GLOBAL::exedit_base + 0x01b7b0, &add_track_value_wrap); // テンキー/回転-
            ReplaceNearJmp(GLOBAL::exedit_base + 0x01b7d4, &add_track_value_wrap); // テンキー*回転+

            // エッジ抽出のチェックを切り替えた時に余計なUndoデータが生成される
            {
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x023b79, 6);
                h.store_i32(0, '\x66\x0f\x1f\x44'); // nop 4/6byte
                h.store_i16(4, '\x00\x00'); // nop 6/6byte
                
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
	} undo;
}


#endif
