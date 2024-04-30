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
#ifdef PATCH_SWITCH_EXFILTER_GRADATION
#include <exedit.hpp>
#include "util_magic.hpp"
#include "offset_address.hpp"
#include "global.hpp"
#include "config_rw.hpp"

#include "patch_exfilter.hpp"

namespace patch::exfilter {
    // init at exedit load
    // グラデーションのフィルタオブジェクト追加
    // ついでに凸ループを追加
    inline class Gradation_t {
        inline static ExEdit::Filter ef;

        inline static char check_name_1[] = "線\0円\0四角形\0凸形\0円ループ\0四角ループ\0凸ループ\0凸ループ2";
        inline static int track_scale[5] = { 10,1,1,10,1 };
        inline static int track_link[5] = { 0,1,-1,0,0 };

        static BOOL __stdcall apend_form(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
        static BOOL __cdecl func_WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, AviUtl::EditHandle* editp, ExEdit::Filter* efp);

        bool enabled = true;
        bool enabled_i;
        inline static const char key[] = "exfilter.gradation";
    public:
        static BOOL __cdecl func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);

        struct efGradation_var { // 1b2050
            int16_t cb_s; // 1b2050
            int16_t cb_e; // 1b2052
            int16_t cr_s; // 1b2054
            int16_t cr_e; // 1b2056
            int16_t intensity; // 1b2058
            int16_t a_s; // 1b205a
            int16_t a_e; // 1b205c
            int16_t y_s; // 1b205e
            int16_t y_e; // 1b2060
            int16_t _padding;
            int32_t cx; // 1b2064
            int32_t cy; // 1b2068
            int32_t sin; // 1b206c
            int32_t cos; // 1b2070
            int32_t w; // 1b2074
            void(__cdecl* blendfunc)(ExEdit::PixelYC* dst, int y, int cb, int cr, int a); // 1b2078
        };

        void init() {
            enabled_i = enabled;
            if (!enabled_i)return;

            auto& cursor = GLOBAL::executable_memory_cursor;

            auto efp = reinterpret_cast<ExEdit::Filter*>(GLOBAL::exedit_base + OFS::ExEdit::efGradation_ptr);
            efp->check_name[1] = check_name_1;
            ef = *efp;

            ef.flag = (ExEdit::Filter::Flag)0;
            (ef.func_proc) = (func_proc);
            (ef.func_WndProc) = (func_WndProc);

            ef.track_gui = reinterpret_cast<ExEdit::Filter::TrackGuiIdx*>(GLOBAL::exedit_base + 0x9fcb0); // 放射ブラーから流用
            ef.track_extra->track_scale = track_scale;
            ef.track_extra->track_link = track_link;

            exfilter.apend_filter(&ef);

            { // フィルタ効果の追加形状対応
                /*
                    10059211 a174201b10         mov     eax,[101b2074]
                    ↓
                    10059211 e9XxXxXxXx         jmp     cursor

                    10000000 e8XxXxXxXx         call    newfunc_stdcall
                    10000000 85c0               test    eax,eax
                    10000000 0f85XxXxXxXx       jnz     ee+5922d
                    10000000 57                 push    edi
                    10000000 a1XxXxXxXx         mov     eax,[ee+1b2074]
                    10000000 56                 push    esi
                    10000000 e9XxXxXxXx         jmp     ee+59216
                */
                constexpr int vp_begin = 0x59211;
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 5);
                h.store_i8(0, '\xe9');
                h.replaceNearJmp(1, cursor);

                store_i8(cursor, '\xe8'); cursor++;
                store_i32(cursor, (int)&apend_form - (int)cursor - 4); cursor += 4;
                store_i32(cursor, '\x85\xc0\x0f\x85'); cursor += 4;
                store_i32(cursor, GLOBAL::exedit_base + 0x5922d - (int)cursor - 4); cursor += 4;
                store_i16(cursor, '\x57\xa1'); cursor += 2;
                store_i32(cursor, GLOBAL::exedit_base + 0x1b2074); cursor += 4;
                store_i16(cursor, '\x56\xe9'); cursor += 2;
                store_i32(cursor, GLOBAL::exedit_base + 0x59216 - (int)cursor - 4); cursor += 4;
            }
        }

        void switch_load(ConfigReader& cr) {
            cr.regist(key, [this](json_value_s* value) {
                ConfigReader::load_variable(value, enabled);
                });
        }

        void switch_store(ConfigWriter& cw) {
            cw.append(key, enabled);
        }

    } Gradation;
} // namespace patch::exfilter
#endif // ifdef PATCH_SWITCH_EXFILTER_GRADATION
