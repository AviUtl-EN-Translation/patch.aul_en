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
#ifdef PATCH_SWITCH_EXFILTER_PLUGINS
#include <exedit.hpp>
#include "util_magic.hpp"
#include "offset_address.hpp"
#include "global.hpp"
#include "config_rw.hpp"

#include "patch_exfilter.hpp"

namespace patch::exfilter {
    // init at exedit load
    // 拡張編集用フィルタプラグインを読み込む

    inline class Plugins_t {
        bool enabled = true;
        bool enabled_i;
        inline static const char key[] = "exfilter.plugins";

        inline static int exedit_video_func_exit_mov_eax_1_wrap() {
            auto LoadedFilterTable = (ExEdit::Filter**)(GLOBAL::exedit_base + OFS::ExEdit::LoadedFilterTable);
            auto LoadedFilterCount = *reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::LoadedFilterCount);
            for (int i = LoadedFilterCount - 1; 0 <= i; i--) {
                auto efp = LoadedFilterTable[i];
                if (has_flag(efp->flag, ExEdit::Filter::Flag::ExEditFilter) && !has_flag(efp->flag, (ExEdit::Filter::Flag)(AviUtl::FilterPlugin::Flag::MultiFilter)) && efp->dll_hinst != NULL) {
                    FreeLibrary((HMODULE)efp->dll_hinst);
                }
            }
            return 1; // mov eax 1
        }

    public:
        inline static const char extension[] = "*.eef";

        void init() {
            enabled_i = enabled;

            if (!enabled_i)return;

            char* aviutl_dir = reinterpret_cast<char* (__stdcall*)(void)>(GLOBAL::aviutl_base + OFS::AviUtl::get_exe_dir)();
            char str[261];
            wsprintfA(str, reinterpret_cast<LPCSTR>(GLOBAL::aviutl_base + OFS::AviUtl::str_percent_s_percent_s_percent_s), aviutl_dir, reinterpret_cast<LPCSTR>(GLOBAL::aviutl_base + OFS::AviUtl::str_plugins_backslash), extension);
            // wsprintfA(path, "%s%s%s", aviutl_path_ptr, "plugins\\", "*.eef");
            WIN32_FIND_DATA wfd;
            HANDLE hfile = FindFirstFileA(str, &wfd);
            if (hfile != INVALID_HANDLE_VALUE) {
                do {
                    wsprintfA(str, reinterpret_cast<LPCSTR>(GLOBAL::aviutl_base + OFS::AviUtl::str_percent_s_percent_s_percent_s), aviutl_dir, reinterpret_cast<LPCSTR>(GLOBAL::aviutl_base + OFS::AviUtl::str_plugins_backslash), wfd.cFileName);
                    auto hmodule = LoadLibraryA(str);
                    if (hmodule != NULL) {
                        int filter_count = 0;
                        auto farproc = GetProcAddress(hmodule, reinterpret_cast<LPCSTR>(GLOBAL::aviutl_base + OFS::AviUtl::str_GetFilterTableList));
                        if (farproc != NULL) {
                            auto efpp = reinterpret_cast<ExEdit::Filter**>(farproc());
                            for (auto efp = *efpp; efp != nullptr; efp = *(++efpp)) {
                                efp->dll_hinst = hmodule;
                                if (0 < filter_count) {
                                    efp->flag |= (ExEdit::Filter::Flag)(AviUtl::FilterPlugin::Flag::MultiFilter);
                                } else {
                                    efp->flag &= (ExEdit::Filter::Flag)~(AviUtl::FilterPlugin::Flag::MultiFilter);
                                }
                                if (!exfilter.apend_filter(efp)) {
                                    break;
                                }
                                filter_count++;
                            }
                        }
                        if (filter_count == 0) {
                            FreeLibrary(hmodule);
                        }
                    }
                } while (FindNextFileA(hfile, &wfd));
                FindClose(hfile);
            }

            OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x3185e, 5);
            h.store_i8(0, '\xe8');
            h.replaceNearJmp(1, &exedit_video_func_exit_mov_eax_1_wrap);
        }


        void switch_load(ConfigReader& cr) {
            cr.regist(key, [this](json_value_s* value) {
                ConfigReader::load_variable(value, enabled);
                });
        }

        void switch_store(ConfigWriter& cw) {
            cw.append(key, enabled);
        }

        char* get_info() {
            if (!enabled_i) return (char*)key;

            auto path = *(char**)(GLOBAL::exedit_base + OFS::ExEdit::memory_ptr);
            auto buf = path + _MAX_PATH + 16;
            auto ptr = buf;
            char* pre_str = nullptr;
            auto LoadedFilterTable = (ExEdit::Filter**)(GLOBAL::exedit_base + OFS::ExEdit::LoadedFilterTable);
            auto LoadedFilterCount = *reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::LoadedFilterCount);
            for (int i = 0; i < LoadedFilterCount; i++) {
                auto efp = LoadedFilterTable[i];
                if (has_flag(efp->flag, ExEdit::Filter::Flag::ExEditFilter) && efp->dll_hinst != nullptr) {
                    char* disp_str;
                    if (efp->information == nullptr) {
                        disp_str = (char*)efp->name;
                    } else {
                        disp_str = efp->information;
                    }
                    // フィルタ効果とフィルタプラグインの2つが追加されている場合に完全に同じ情報になりがちなので除外（連続している場合のみ）
                    if (pre_str != disp_str) { // ポインタ比較で良い
                        pre_str = disp_str;
                        GetModuleFileNameA((HMODULE)efp->dll_hinst, path, _MAX_PATH);
                        ptr += wsprintfA(ptr, "%s ( %s )\n", disp_str, reinterpret_cast<char* (__fastcall*)(char*)>(GLOBAL::aviutl_base + OFS::AviUtl::get_filename_cursor)(path));
                    }
                }
            }

            *ptr = '\0';
            return buf;
        }

    } Plugins;
} // namespace patch::exfilter
#endif // ifdef PATCH_SWITCH_EXFILTER_PLUGINS
