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

#ifdef PATCH_SWITCH_READ_AUDIO

#include <aviutl.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"

#include "config_rw.hpp"

namespace patch {

    // init at patch load
    // exedit->avi_file_read_audio_sampleを改造して音声データのキャッシュを取るように変更

    inline class read_audio_t {

        inline static int(__cdecl* exfunc_avi_file_read_audio_sample_org)(AviUtl::AviFileHandle*, int, int, short*);
        
        static int __fastcall update_waveformat_wrap(AviUtl::AviFileHandle* afh, WAVEFORMATEX* wfe);

        static void* __cdecl get_audio_shared_mem(AviUtl::AviFileHandle* afh, int start);
        static void* __cdecl create_audio_shared_mem(AviUtl::AviFileHandle* afh, int start);
        static int __cdecl exfunc_avi_file_read_audio_sample_wrap(AviUtl::AviFileHandle* afh, int start, int length, short* buf);


#define CACHEINFO_N 256
        inline static struct AudioCacheInfo {
            AviUtl::SharedMemoryInfo* smi;
            void* input_handle_info;
            unsigned int audio_rate;
            unsigned short audio_ch;
            unsigned short n;
        } cache_info[CACHEINFO_N];



        bool enabled = true;
        bool enabled_i;
        inline static const char key[] = "read_audio";
    public:
        void init() {
            enabled_i = enabled;
            if (!enabled_i)return;

            (exfunc_avi_file_read_audio_sample_org) = reinterpret_cast<decltype(exfunc_avi_file_read_audio_sample_org)>(*(int*)(GLOBAL::aviutl_base + 0x2d0fe));
            OverWriteOnProtectHelper(GLOBAL::aviutl_base + 0x2d0fe, 4).store_i32(0, &exfunc_avi_file_read_audio_sample_wrap);

            OverWriteOnProtectHelper(GLOBAL::aviutl_base + 0x23119, 4).replaceNearJmp(0, &update_waveformat_wrap);
            //OverWriteOnProtectHelper(GLOBAL::aviutl_base + 0x231b2, 1).store_i8(0, 0x10);
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
    } read_audio;
} // namespace patch

#endif // ifdef PATCH_SWITCH_READ_AUDIO
