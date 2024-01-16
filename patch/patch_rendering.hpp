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

#ifdef PATCH_SWITCH_RENDERING
#include <exedit.hpp>
#include "global.hpp"
#include "util.hpp"
#include "restorable_patch.hpp"

#include "config_rw.hpp"

namespace patch {
    // init at exedit load
    /*
    ・高さ4097以上描画が正常に行えないことがあるのを修正

    ・回転45、obj.w==obj.h、obj.y==-obj.screen、(他条件有り)　の時の描画が正常ではないのを修正

    ・マルチスレッド処理を効率化

    以下は調査中

    ・オブジェクトを横から見た薄い状態の表示位置がずれる
    　https://twitter.com/nazono22/status/1592798629332652032
    */

    inline class rendering_t {

        struct polydata_double {
            double x, y, z, u, v;
        };
        struct xzuv {
            double x;
            double z;
            double u;
            double v;
        };
        struct uvdata {
            xzuv offset;
            xzuv range;
        };

        static int __stdcall mid_render();
        static void __cdecl calc_xzuv_wrap(int poly_num, polydata_double* pd);


        bool enabled = true;
        bool enabled_i;

        inline static const char key[] = "rendering";

        inline static void(__cdecl* rendering_mt_func)(int thread_id, int thread_num, int unuse1, int unuse2);

        inline static int (__cdecl* calc_xzuv_offset_range)(polydata_double* pd, int p1, int p2, double* x_offset, double* x_range,
            double* u_offset, double* v_offset, double* u_range, double* v_range, double* z_offset, double* z_range);

        inline static void(__cdecl* push_rendering_data)(int y_begin, int y_end,
            int x_begin1, int x_end1, int x_begin2, int x_end2,
            int u_begin, int u_end, int v_begin, int v_end, int u_range_b, int v_range_b,
            int x_begin3, int x_end3,
            int z_begin, int z_end, int z_range_b,
            int u_range_e, int v_range_e, int z_range_e);

        inline static int* rendering_data_count_ptr;
        inline static int* rendering_inc_or_dec_ptr;


        inline static double* cam_screen_d16_ptr;
        inline static double* z_offset_default_ptr;


    public:
        void init() {
            enabled_i = enabled;

            if (!enabled_i)return;


            rendering_data_count_ptr = (int*)(GLOBAL::exedit_base + OFS::ExEdit::rendering_data_count);
            rendering_inc_or_dec_ptr = (int*)(GLOBAL::exedit_base + OFS::ExEdit::rendering_inc_or_dec);
            cam_screen_d16_ptr = (double*)(GLOBAL::exedit_base + OFS::ExEdit::cam_screen_d16);
            z_offset_default_ptr = (double*)(GLOBAL::exedit_base + OFS::ExEdit::z_offset_default);
            
            
            rendering_mt_func = reinterpret_cast<decltype(rendering_mt_func)>(GLOBAL::exedit_base + OFS::ExEdit::rendering_mt_func);

            { // 高さ4097以上描画が正常に行えないことがあるのを修正
                /*
                    1007caaf 3d00100000         cmp     eax,00001000
                    1007cab4 5e                 pop     esi
                    1007cab5 7d06               jnl     1007cabd
                    1007cab7 40                 inc     eax
                    1007cab8 a328c71e10         mov     [101ec728],eax
                    1007cabd 5f                 pop     edi
                    1007cabe 5d                 pop     ebp
                    1007cabf 5b                 pop     ebx
                    1007cac0 83c41c             add     esp,+1c
                    1007cac3 c3                 ret
                    1007cac4 90                 nop
                    1007cac5 90                 nop
                    1007cac6 90                 nop
                    1007cac7 90                 nop
                    1007cac8 90                 nop
                    1007cac9 90                 nop
                    1007caca 90                 nop
                    1007cacb 90                 nop

                    ↓

                    1007caaf 3dff0f0000         cmp     eax,00000fff ; rendering_data_count == 4095
                    1007cab4 5e                 pop     esi
                    1007cab5 740d               jz      1007cac4
                    1007cab7 40                 inc     eax
                    1007cab8 a328c71e10         mov     [101ec728],eax ; rendering_data_count
                    1007cabd 5f                 pop     edi
                    1007cabe 5d                 pop     ebp
                    1007cabf 5b                 pop     ebx
                    1007cac0 83c41c             add     esp,+1c
                    1007cac3 c3                 ret
                    1007cac4 e8XxXxXxXx         call    ;__stdcall return 0;
                    1007cac9 ebed               jmp     1007cab8
                    1007cacb 90                 nop

                    描画の行データが4096に達する時、描画関数を実行しカウントを0にする
                */
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x7cab0, 27);
                h.store_i16(0, '\xff\x0f');
                h.store_i16(5, '\x74\x0d');
                h.store_i8(20, '\xe8');
                h.replaceNearJmp(21, &mid_render);
                h.store_i16(25, '\xeb\xed');

            }

            { // 回転45、obj.w==obj.h、obj.y==-obj.screen、(他条件有り)　の時の描画が正常ではないのを修正
                calc_xzuv_offset_range = reinterpret_cast<decltype(calc_xzuv_offset_range)>(GLOBAL::exedit_base + OFS::ExEdit::calc_xzuv_offset_range);
                push_rendering_data = reinterpret_cast<decltype(push_rendering_data)>(GLOBAL::exedit_base + OFS::ExEdit::push_rendering_data);

                ReplaceNearJmp(GLOBAL::exedit_base + 0x797dc, &calc_xzuv_wrap);
            }
            
            { // 浮動小数掛け算部分の無駄をなくす
                /* ; YC 
                    100756d7 db442410           fild    dword ptr [esp+10]
                    100756db d8c9               fmul    st,st(1)
                    100756dd dc0df0a30910       fmul    dword ptr [1009a3f0] ; 1.0/65536.0
                    100756e3 dec2               faddp   st(2),st
                    100756e5 db442414           fild    dword ptr [esp+14]
                    100756e9 d8c9               fmul    st,st(1)
                    100756eb dc0df0a30910       fmul    dword ptr [1009a3f0] ; 1.0/65536.0
                    100756f1 dec3               faddp   st(3),st
                    100756f3 db442434           fild    dword ptr [esp+34]
                    100756f7 d8c9               fmul    st,st(1)
                    100756f9 dc0df0a30910       fmul    dword ptr [1009a3f0] ; 1.0/65536.0
                    100756ff dec4               faddp   st(4),st
                    10075701 ddd8               fstp    st(0)
                    10075703 eb61               jmp     short 10075766

                    1007574e eb16               jmp     short 10075766
                    ↓

                    100756d7 dc0dXxXxXxXx       fmul    dword ptr [1009a3f0] ; 1.0/65536.0
                    100756dd db442410           fild    dword ptr [esp+10]
                    100756e1 d8c9               fmul    st,st(1)
                    100756e3 dec2               faddp   st(2),st
                    100756e5 db442414           fild    dword ptr [esp+14]
                    100756e9 d8c9               fmul    st,st(1)
                    100756eb dec3               faddp   st(3),st
                    100756ed db442434           fild    dword ptr [esp+34]
                    100756f1 d8c9               fmul    st,st(1)
                    100756f3 dec4               faddp   st(4),st
                    100756f5 ddd8               fstp    st(0)
                    100756f7 eb6d               jmp     short 10075766
                    100756f9

                    10075742 eb22               jmp     short 10075766
                    10075744
                */
                /*
                char yc_bin[] = {
                    "\xdc\x0dXXXX"             // fmul    dword ptr [ee+9a3f0] ; 1.0/65536.0
                    "\xdb\x44\x24\x10"         // fild    dword ptr [esp+10]
                    "\xd8\xc9"                 // fmul    st,st(1)
                    "\xde\xc2"                 // faddp   st(2),st
                    "\xdb\x44\x24\x14"         // fild    dword ptr [esp+14]
                    "\xd8\xc9"                 // fmul    st,st(1)
                    "\xde\xc3"                 // faddp   st(3),st
                    "\xdb\x44\x24\x34"         // fild    dword ptr [esp+34]
                    "\xd8\xc9"                 // fmul    st,st(1)
                    "\xde\xc4"                 // faddp   st(4),st
                    "\xdd\xd8"                 // fstp    st(0)
                    "\xeb\x6d"                 // jmp     short 10075766
                };
                store_i32(yc_bin + 2, GLOBAL::exedit_base + OFS::ExEdit::double_1div65536);
                constexpr int vp_begin = 0x756d7;
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x75744 - vp_begin);
                memcpy(reinterpret_cast<void*>(h.address()), yc_bin, sizeof(yc_bin) - 1);
                memcpy(reinterpret_cast<void*>(h.address(0x75722 - vp_begin)), yc_bin, sizeof(yc_bin) - 2);
                h.store_i8(0x75743 - vp_begin, 0x22);
                */
                /* ; YCA
                    10075cfe db442418           fild    dword ptr [esp+18]
                    10075d02 db442410           fild    dword ptr [esp+10]
                    10075d06 d8c9               fmul    st,st(1)
                    10075d08 dc0df0a30910       fmul    dword ptr [1009a3f0] ; 1.0/65536.0
                    10075d0e dec2               faddp   st(2),st
                    10075d10 db442414           fild    dword ptr [esp+14]
                    10075d14 d8c9               fmul    st,st(1)
                    10075d16 dc0df0a30910       fmul    dword ptr [1009a3f0] ; 1.0/65536.0
                    10075d1c dec3               faddp   st(3),st
                    10075d1e db44243c           fild    dword ptr [esp+3c]
                    10075d22 d8c9               fmul    st,st(1)
                    10075d24 dc0df0a30910       fmul    dword ptr [1009a3f0] ; 1.0/65536.0
                    10075d2a dec4               faddp   st(4),st
                    10075d2c db442450           fild    dword ptr [esp+50]
                    10075d30 d8c9               fmul    st,st(1)
                    10075d32 dc0df0a30910       fmul    dword ptr [1009a3f0] ; 1.0/65536.0
                    10075d38 dec5               faddp   st(5),st
                    10075d3a ddd8               fstp    st(0)
                    10075d3c eb71               jmp     short 10075daf

                    10075d91 eb1c               jmp     short 10075daf
                    ↓
                    10075cfe db442418           fild    dword ptr [esp+18]
                    10075d02 dc0dXxXxXxXx       fmul    dword ptr [1009a3f0] ; 1.0/65536.0
                    10075d08 db442410           fild    dword ptr [esp+10]
                    10075d0c d8c9               fmul    st,st(1)
                    10075d0e dec2               faddp   st(2),st
                    10075d10 db442414           fild    dword ptr [esp+14]
                    10075d14 d8c9               fmul    st,st(1)
                    10075d16 dec3               faddp   st(3),st
                    10075d18 db44243c           fild    dword ptr [esp+3c]
                    10075d1c d8c9               fmul    st,st(1)
                    10075d1e dec4               faddp   st(4),st
                    10075d20 db442450           fild    dword ptr [esp+50]
                    10075d24 d8c9               fmul    st,st(1)
                    10075d26 dec5               faddp   st(5),st
                    10075d28 ddd8               fstp    st(0)
                    10075d2a e980000000         jmp     10075daf
                    10075d2f

                    10075d7f eb2e               jmp     short 10075daf
                    10075d81
                */
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

    } Rendering;
} // namespace patch
#endif // ifdef PATCH_SWITCH_RENDERING
