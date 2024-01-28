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
    // exfunc->avi_file_read_audio_sampleを改造する
    // 引数startがマイナスの時の計算が変だったのを修正
    // 引数lengthにマイナスを入れた時に逆再生のデータを取得するように変更
    // シークで音がおかしくなる問題の解決のためにキャッシュ作成・いくらか前の音声を仮に読み込ませる処理を行うようにした

    inline class read_audio_t {

        inline static int(__cdecl* exfunc_avi_file_read_audio_sample_org)(AviUtl::AviFileHandle*, int, int, short*);
        
        static int __fastcall update_waveformat_wrap(AviUtl::AviFileHandle* afh, WAVEFORMATEX* wfe);

        static void* __cdecl get_audio_shared_mem(AviUtl::AviFileHandle* afh, int start);
        static void* __cdecl create_audio_shared_mem(AviUtl::AviFileHandle* afh, int start);
        static int __cdecl exfunc_avi_file_read_audio_sample_wrap(AviUtl::AviFileHandle* afh, int start, int length, short* buf);

        static int get_WaveFileReader_idx();
        inline static int WaveFileReader_idx = -1;
        
#define CACHEINFO_N 256
        inline static struct AudioCacheInfo {
            AviUtl::SharedMemoryInfo* smi;
            void* input_plugin_info;
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

            { // aviutl.exe + 0x0004101d の対策
                auto& cursor = GLOBAL::executable_memory_cursor;
                /*
                    00440ffd 03d0               add     edx,eax
                    00440fff 56                 push    esi
                    00441000 c1fa03             sar     edx,03
                    ↓
                    00440ffd 56                 push    esi
                    00440ffe e9XxXxXxXx         jmp     cursor

                    00000000 03d0               add     edx,eax
                    00000000 c1fa03             sar     edx,03
                    00000000 85d2               test    edx,edx
                    00000000 0f8fXxXxXxXx       jg      aviutl+41003
                    00000000 e9XxXxXxXx         jmp     aviutl+4105b
                */
                constexpr int vp_begin = 0x40ffd;
                OverWriteOnProtectHelper h(GLOBAL::aviutl_base + vp_begin, 0x41003 - vp_begin);
                h.store_i16(0x40ffd - vp_begin, '\x56\xe9');
                h.replaceNearJmp(0x40fff - vp_begin, cursor);

                store_i32(cursor, '\x03\xd0\xc1\xfa'); cursor += 4;
                store_i8(cursor, '\x03'); cursor++;
                store_i32(cursor, '\x85\xd2\x0f\x8f'); cursor += 4;
                store_i32(cursor, GLOBAL::aviutl_base + 0x41003 - (int)cursor - 4); cursor += 4;
                store_i8(cursor, '\xe9'); cursor++;
                store_i32(cursor, GLOBAL::aviutl_base + 0x4105b - (int)cursor - 4); cursor += 4;
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
    } read_audio;
} // namespace patch

#endif // ifdef PATCH_SWITCH_READ_AUDIO
