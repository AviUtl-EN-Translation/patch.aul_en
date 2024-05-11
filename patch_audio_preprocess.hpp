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

#ifdef PATCH_SWITCH_AUDIO_PREPROCESS

#include <exedit.hpp>
#include "global.hpp"
#include "util.hpp"
#include "restorable_patch.hpp"

#include "config_rw.hpp"

namespace patch {
    // init at exedit load
    // audio_func_mainでefp->flag::preprocess(0x200)の仕組みが無いので追加

    inline class audio_preprocess_t {

        bool enabled = true;
        bool enabled_i;

        inline static const char key[] = "audio_preprocess";

        static void __stdcall do_func_preprocess(ExEdit::Object* obj, void* ptr) {
            auto LoadedFilterTable = (ExEdit::Filter**)(GLOBAL::exedit_base + OFS::ExEdit::LoadedFilterTable);
            auto ObjectArrayPointer = *(ExEdit::Object**)(GLOBAL::exedit_base + OFS::ExEdit::ObjectArrayPointer);
            auto efpip = reinterpret_cast<ExEdit::FilterProcInfo*>((int)ptr + 4 + 0x1c);
            auto check_ptr = reinterpret_cast<int*>((int)ptr + 4 + 0x1cc);
            auto exdata_ptr = reinterpret_cast<void*>((int)ptr + 4 + 0x28c);
            auto track_ptr = reinterpret_cast<int*>((int)ptr + 4 + 0x38c);

            for (int filter_idx = 0; filter_idx < 12; filter_idx++) {
                if (obj->filter_param[filter_idx].id < 0) break;
                auto efp = LoadedFilterTable[obj->filter_param[filter_idx].id];
                ExEdit::Object::FilterStatus flag;
                if (obj->index_midpt_leader < 0) {
                    flag = obj->filter_status[filter_idx];
                } else {
                    flag = ObjectArrayPointer[obj->index_midpt_leader].filter_status[filter_idx];
                }
                if (!has_flag(flag, ExEdit::Object::FilterStatus::Active) && has_flag(efp->flag, ExEdit::Filter::Flag::Input)) break;
                if (has_flag(flag, ExEdit::Object::FilterStatus::Active) || has_flag(efp->flag, ExEdit::Filter::Flag::Output)) {
                    if ((int)efp->flag & 0x200 && efp->func_proc != NULL) {
                        reinterpret_cast<void(__cdecl*)(ExEdit::Object*, int, int*, int*, void*, ExEdit::FilterProcInfo*)>(GLOBAL::exedit_base + OFS::ExEdit::store_filter)(obj, filter_idx, track_ptr, check_ptr, exdata_ptr, efpip);
                        efpip->flag |= (ExEdit::FilterProcInfo::Flag)0x400;
                        BOOL fpret = efp->func_proc(efp, efpip);
                        efpip->flag ^= (ExEdit::FilterProcInfo::Flag)0x400;
                        reinterpret_cast<void(__cdecl*)(ExEdit::Object*, int, int*, int*, void*)>(GLOBAL::exedit_base + OFS::ExEdit::restore_filter)(obj, filter_idx, track_ptr, check_ptr, exdata_ptr);
                        if (!fpret) break;
                    }
                }
            }
        }

    public:

        void init() {
            enabled_i = enabled;

            if (!enabled_i)return;

            auto& cursor = GLOBAL::executable_memory_cursor;
            /*
                10049ed6 8d5754             lea     edx,dword ptr [edi+54]
                10049ed9 33ed               xor     ebp,ebp
                ↓
                10049ed6 e8XxXxXxXx         call    cursor

                10000000 54                 push    esp
                10000000 57                 push    edi
                10000000 e8XxXxXxXx         call    newfunc_stdcall
                10000000 8d5754             lea     edx,dword ptr [edi+54]
                10000000 33ed               xor     ebp,ebp
                10000000 c3                 ret
            */
            OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x49ed6, 5);
            h.store_i8(0, '\xe8');
            h.replaceNearJmp(1, cursor);
            store_i32(cursor, '\x54\x57\xe8\x00'); cursor += 3;
            store_i32(cursor, (int)&do_func_preprocess - (int)cursor - 4); cursor += 4;
            store_i32(cursor, '\x8d\x57\x54\x33'); cursor += 4;
            store_i16(cursor, '\xed\xc3'); cursor += 2;

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

    } audio_preprocess;
} // namespace patch
#endif // ifdef PATCH_SWITCH_AUDIO_PREPROCESS
