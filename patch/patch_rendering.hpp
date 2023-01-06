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

        static void __cdecl rendering_mt_wrap(int thread_id, int thread_num, int unuse1, int unuse2);
        static void __cdecl do_multi_thread_func_wrap(AviUtl::MultiThreadFunc func, BOOL is_multithread);

        static void __cdecl calc_xzuv_wrap(int poly_num, polydata_double* pd);
        //static int __cdecl calc_xzuv_offset_range_wrap(polydata_double* pd, int p1, int p2, double* x_offset, double* x_range, double* u_offset, double* v_offset, double* u_range, double* v_range, double* z_offset, double* z_range);
        
        //static void __cdecl calc_xzuv_avx2(int poly_num, polydata_double* pd);
        //static int __cdecl calc_xzuv_offset_range_avx2(polydata_double* pd, int p1, int p2, uvdata* uvd);


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
            { // マルチスレッド処理を効率化
                ReplaceNearJmp(GLOBAL::exedit_base + 0x7981a, do_multi_thread_func_wrap);
            }


            { // 回転45、obj.w==obj.h、obj.y==-obj.screen、(他条件有り)　の時の描画が正常ではないのを修正
                calc_xzuv_offset_range = reinterpret_cast<decltype(calc_xzuv_offset_range)>(GLOBAL::exedit_base + OFS::ExEdit::calc_xzuv_offset_range);
                push_rendering_data = reinterpret_cast<decltype(push_rendering_data)>(GLOBAL::exedit_base + OFS::ExEdit::push_rendering_data);

                ReplaceNearJmp(GLOBAL::exedit_base + 0x797dc, &calc_xzuv_wrap);
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
