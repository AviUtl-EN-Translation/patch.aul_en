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

#include "patch_rendering.hpp"


#ifdef PATCH_SWITCH_RENDERING
namespace patch {


    int __stdcall rendering_t::mid_render() {
        int fast_process = *(int*)(GLOBAL::exedit_base + OFS::ExEdit::fast_process);
        if (fast_process) {
            int* rendering_data_array = (int*)(GLOBAL::exedit_base + OFS::ExEdit::rendering_data_array);
            int j = 10;
            for (int i = 0; i < 4096; i++) {
                rendering_data_array[j] = 0;
                rendering_data_array[j + 1] = 0;
                j += 17;
            }
        }
        *rendering_data_count_ptr = 4096;
        reinterpret_cast<void(__cdecl*)(int, BOOL)>(GLOBAL::exedit_base + OFS::ExEdit::do_multi_thread_func)(GLOBAL::exedit_base + OFS::ExEdit::rendering_mt_func, TRUE);
        //do_multi_thread_func_wrap((AviUtl::MultiThreadFunc)rendering_mt_func, TRUE);
        return 0; // rendering_data_countをいくつにするか
    }


    inline int add_inc_or_dec(int n, int inc_or_dec, int max) {
        if (inc_or_dec < 0) {
            n--;
            if (n < 0) {
                n = max - 1;
            }
        } else {
            n++;
            if (max <= n) {
                n = 0;
            }
        }
        return n;
    }

    void __cdecl rendering_t::calc_xzuv_wrap(int poly_num, polydata_double* pd) {

        double rate;

        double x_begin;
        double z_begin;
        double u_begin;
        double v_begin;

        double x_range_b;
        double z_range_b;
        double u_range_b;
        double v_range_b;

        double x_range_e;
        double z_range_e;
        double u_range_e;
        double v_range_e;

        double x_end;
        double z_end;
        double u_end;
        double v_end;

        *rendering_data_count_ptr = 0;
        int inc_or_dec = *rendering_inc_or_dec_ptr;


        int poly_top = 0;
        int poly_bottom = 0;
        int i;
        for (i = 1; i < poly_num; i++) {
            if (pd[i].y < pd[poly_top].y || (pd[poly_top].y == pd[i].y && pd[i].x < pd[poly_top].x)) {
                poly_top = i;
            }
            if (pd[poly_bottom].y < pd[i].y || (pd[poly_bottom].y == pd[i].y && pd[poly_bottom].x < pd[i].x)) {
                poly_bottom = i;
            }
        }


        if (pd[poly_top].y == pd[poly_bottom].y) {
            return;
        }

        int poly_begin;
        i = poly_top;
        do {
            poly_begin = i;
            //i = (i - inc_or_dec + poly_num) % poly_num;
            i = add_inc_or_dec(i, -inc_or_dec, poly_num);
        } while (pd[poly_begin].y == pd[i].y);

        int poly_end;
        i = poly_top;
        do {
            poly_end = i;
            //i = (i + inc_or_dec + poly_num) % poly_num;
            i = add_inc_or_dec(i, inc_or_dec, poly_num);
        } while (pd[poly_end].y == pd[i].y);


        int bb = calc_xzuv_offset_range(pd, poly_begin, add_inc_or_dec(poly_begin, -inc_or_dec, poly_num), &x_begin, &x_range_b,
            &u_begin, &v_begin, &u_range_b, &v_range_b, &z_begin, &z_range_b);
        int ee = calc_xzuv_offset_range(pd, poly_end, add_inc_or_dec(poly_end, inc_or_dec, poly_num), &x_end, &x_range_e,
            &u_end, &v_end, &u_range_e, &v_range_e, &z_end, &z_range_e);
        if (poly_begin == poly_end) {
            if (x_range_b > x_range_e) {
                bb = calc_xzuv_offset_range(pd, poly_begin, add_inc_or_dec(poly_begin, inc_or_dec, poly_num), &x_begin, &x_range_b,
                    &u_begin, &v_begin, &u_range_b, &v_range_b, &z_begin, &z_range_b);
                ee = calc_xzuv_offset_range(pd, poly_end, add_inc_or_dec(poly_end, -inc_or_dec, poly_num), &x_end, &x_range_e,
                    &u_end, &v_end, &u_range_e, &v_range_e, &z_end, &z_range_e);
                inc_or_dec = -inc_or_dec;
                *rendering_inc_or_dec_ptr = inc_or_dec;
            }
        } else if (x_begin == x_end) {
            if (x_range_b > x_range_e) {
                bb = calc_xzuv_offset_range(pd, poly_end, add_inc_or_dec(poly_end, inc_or_dec, poly_num), &x_begin, &x_range_b,
                    &u_begin, &v_begin, &u_range_b, &v_range_b, &z_begin, &z_range_b);
                ee = calc_xzuv_offset_range(pd, poly_begin, add_inc_or_dec(poly_begin, -inc_or_dec, poly_num), &x_end, &x_range_e,
                    &u_end, &v_end, &u_range_e, &v_range_e, &z_end, &z_range_e);
                inc_or_dec = -inc_or_dec;
                *rendering_inc_or_dec_ptr = inc_or_dec;
            }
        } else if (x_begin > x_end) {
            bb = calc_xzuv_offset_range(pd, poly_end, add_inc_or_dec(poly_end, inc_or_dec, poly_num), &x_begin, &x_range_b,
                &u_begin, &v_begin, &u_range_b, &v_range_b, &z_begin, &z_range_b);
            ee = calc_xzuv_offset_range(pd, poly_begin, add_inc_or_dec(poly_begin, -inc_or_dec, poly_num), &x_end, &x_range_e,
                &u_end, &v_end, &u_range_e, &v_range_e, &z_end, &z_range_e);
            inc_or_dec = -inc_or_dec;
            *rendering_inc_or_dec_ptr = inc_or_dec;
        }


        int top_y = (int)(pd[poly_top].y);
        int top_fraction = (-top_y & 0xffff);
        int bottom_y = (int)(pd[poly_bottom].y);
        int bottom_fraction = (int)bottom_y & 0xffff;
        int y_loop = (bottom_y - top_y - top_fraction - bottom_fraction) >> 16;
        if (y_loop < 0) {
            bottom_fraction += top_fraction - 0x10000;
            top_fraction = 0;
        }
        int next_y = top_y;
        int x_begin1, x_begin3;
        int x_end1, x_end3;

        if (top_fraction) {
            rate = (double)top_fraction * 1.52587890625e-05;
            next_y += top_fraction;
            x_begin1 = (int)(x_begin);
            x_begin += x_range_b * rate;
            x_begin3 = (int)(x_begin);
            if (x_begin < pd[bb].x && x_range_b < 0.0 || x_begin > pd[bb].x && x_range_b > 0.0) {
                x_begin = pd[bb].x;
            }
            x_end1 = (int)(x_end);
            x_end += x_range_e * rate;
            x_end3 = (int)(x_end);
            if (x_end < pd[ee].x && x_range_e < 0.0 || x_end > pd[ee].x && x_range_e > 0.0) {
                x_end = pd[ee].x;
            }
            push_rendering_data(top_y, next_y, x_begin1, x_end1, (int)(x_begin), (int)(x_end),
                (int)(u_begin), (int)(u_end), (int)(v_begin), (int)(v_end), (int)(u_range_b), (int)(v_range_b),
                x_begin3, x_end3, (int)(z_begin), (int)(z_end), (int)(z_range_b), (int)(u_range_e), (int)(v_range_e), (int)(z_range_e));
            u_begin += u_range_b * rate;
            v_begin += v_range_b * rate;
            z_begin += z_range_b * rate;
            u_end += u_range_e * rate;
            v_end += v_range_e * rate;
            z_end += z_range_e * rate;

            double next_y_d = (double)next_y;
            while (pd[bb].y < next_y_d) {
                rate = (next_y_d - pd[bb].y) * 1.52587890625e-05;
                bb = calc_xzuv_offset_range(pd, bb, add_inc_or_dec(bb, -inc_or_dec, poly_num), &x_begin, &x_range_b,
                    &u_begin, &v_begin, &u_range_b, &v_range_b, &z_begin, &z_range_b);
                x_begin += x_range_b * rate;
                u_begin += u_range_b * rate;
                v_begin += v_range_b * rate;
                z_begin += z_range_b * rate;
            }
            while (pd[ee].y < next_y_d) {
                rate = (next_y_d - pd[ee].y) * 1.52587890625e-05;
                ee = calc_xzuv_offset_range(pd, ee, add_inc_or_dec(ee, inc_or_dec, poly_num), &x_end, &x_range_e,
                    &u_end, &v_end, &u_range_e, &v_range_e, &z_end, &z_range_e);
                x_end += x_range_e * rate;
                u_end += u_range_e * rate;
                v_end += v_range_e * rate;
                z_end += z_range_e * rate;
            }
        }
        for (i = 0; i < y_loop; i++) {
            int yy = next_y;
            next_y += 0x10000;
            x_begin1 = (int)(x_begin);
            x_begin += x_range_b;
            x_begin3 = (int)(x_begin);
            if (x_begin < pd[bb].x && x_range_b < 0.0 || x_begin > pd[bb].x && x_range_b > 0.0) {
                x_begin = pd[bb].x;
            }
            x_end1 = (int)(x_end);
            x_end += x_range_e;
            x_end3 = (int)(x_end);
            if (x_end < pd[ee].x && x_range_e < 0.0 || x_end > pd[ee].x && x_range_e > 0.0) {
                x_end = pd[ee].x;
            }
            push_rendering_data(yy, next_y, x_begin1, x_end1, (int)(x_begin), (int)(x_end),
                (int)(u_begin), (int)(u_end), (int)(v_begin), (int)(v_end), (int)(u_range_b), (int)(v_range_b),
                x_begin3, x_end3, (int)(z_begin), (int)(z_end), (int)(z_range_b), (int)(u_range_e), (int)(v_range_e), (int)(z_range_e));
            u_begin += u_range_b;
            v_begin += v_range_b;
            z_begin += z_range_b;
            u_end += u_range_e;
            v_end += v_range_e;
            z_end += z_range_e;
            double next_y_d = (double)next_y;
            while (pd[bb].y < next_y_d) {
                rate = (next_y_d - pd[bb].y) * 1.52587890625e-05;
                bb = calc_xzuv_offset_range(pd, bb, add_inc_or_dec(bb, -inc_or_dec, poly_num), &x_begin, &x_range_b,
                    &u_begin, &v_begin, &u_range_b, &v_range_b, &z_begin, &z_range_b);
                x_begin += x_range_b * rate;
                u_begin += u_range_b * rate;
                v_begin += v_range_b * rate;
                z_begin += z_range_b * rate;
            }
            while (pd[ee].y < next_y_d) {
                rate = (next_y_d - pd[ee].y) * 1.52587890625e-05;
                ee = calc_xzuv_offset_range(pd, ee, add_inc_or_dec(ee, inc_or_dec, poly_num), &x_end, &x_range_e,
                    &u_end, &v_end, &u_range_e, &v_range_e, &z_end, &z_range_e);
                x_end += x_range_e * rate;
                u_end += u_range_e * rate;
                v_end += v_range_e * rate;
                z_end += z_range_e * rate;
            }
        }
        if (bottom_fraction) {
            rate = (double)bottom_fraction * 1.52587890625e-05;
            x_begin1 = (int)(x_begin);
            x_begin += x_range_b * rate;
            x_begin3 = (int)(x_begin);
            if (x_begin < pd[bb].x && x_range_b < 0.0 || x_begin > pd[bb].x && x_range_b > 0.0) {
                x_begin = pd[bb].x;
            }
            x_end1 = (int)(x_end);
            x_end += x_range_e * rate;
            x_end3 = (int)(x_end);
            if (x_end < pd[ee].x && x_range_e < 0.0 || x_end > pd[ee].x && x_range_e > 0.0) {
                x_end = pd[ee].x;
            }
            push_rendering_data(next_y, next_y + bottom_fraction, x_begin1, x_end1, (int)(x_begin), (int)(x_end),
                (int)(u_begin), (int)(u_end), (int)(v_begin), (int)(v_end), (int)(u_range_b), (int)(v_range_b),
                x_begin3, x_end3, (int)(z_begin), (int)(z_end), (int)(z_range_b), (int)(u_range_e), (int)(v_range_e), (int)(z_range_e));
        }
    }

} // namespace patch
#endif // ifdef PATCH_SWITCH_RENDERING