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

#include <immintrin.h>

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

    /*
    bool mtflag[4096];
    int e1, e2;

    void rendering_t::rendering_mt_wrap(int thread_id, int thread_num, int unuse1, int unuse2) {
        int rendering_data_count = *rendering_data_count_ptr;
        for (int i = thread_id; i < rendering_data_count; i += thread_num) {
            if (mtflag[i]) {
                return;
            } else {
                mtflag[i] = true;
                rendering_mt_func(i, rendering_data_count, 0, 0);
            }
        }
        if (e2 <= e1) {
            return;
        }
        int i = e1;
        while (mtflag[i]) {
            i++;
            if (e2 < i) {
                e1 = i;
                return;
            }
        }
        e1 = i;

        i = e2;
        while (mtflag[i]) {
            i--;
            if (i < e1) {
                e2 = i;
                return;
            }
        }
        e2 = i;

        while (e1 <= i) {
            if (!mtflag[i]) {
                mtflag[i] = true;
                rendering_mt_func(i, rendering_data_count, 0, 0);
            }
            i--;
        }
        e1 = e2;
    }

    void __cdecl rendering_t::do_multi_thread_func_wrap(AviUtl::MultiThreadFunc func, BOOL flag) {
        if (!flag) {
            func(0, 1, 0, 0);
            return;
        }
        e1 = 0;
        e2 = *rendering_data_count_ptr - 1;
        memset(mtflag, 0, *rendering_data_count_ptr);
        reinterpret_cast<void(__cdecl*)(AviUtl::MultiThreadFunc, BOOL)>(GLOBAL::exedit_base + OFS::ExEdit::do_multi_thread_func)((AviUtl::MultiThreadFunc)&rendering_mt_wrap, flag);
    }
    */

    /*
    int __cdecl rendering_t::calc_xzuv_offset_range_wrap(polydata_double* pd, int p1, int p2, double* x_offset, double* x_range,
        double* u_offset, double* v_offset, double* u_range, double* v_range, double* z_offset, double* z_range) {
        
        double cam_screen_d16 = *cam_screen_d16_ptr;
        double z_offset_default = *z_offset_default_ptr;

        double rate;
        double u_ofs1 = pd[p1].u;
        double u_ofs2 = pd[p2].u;
        double v_ofs1 = pd[p1].v;
        double v_ofs2 = pd[p2].v;
        double z_ofs1 = pd[p1].z;
        double z_ofs2 = pd[p2].z;
        if (cam_screen_d16 != 0.0) {
            if (z_ofs1 == cam_screen_d16) {
                z_ofs1 = z_offset_default;
            } else {
                rate = cam_screen_d16;
                if (z_ofs1 != 0.0) {
                    rate /= z_ofs1;
                }
                u_ofs1 *= rate;
                v_ofs1 *= rate;
                z_ofs1 = z_offset_default * rate;
            }
            if (z_ofs2 == cam_screen_d16) {
                z_ofs2 = z_offset_default;
            } else {
                rate = cam_screen_d16;
                if (z_ofs2 != 0.0) {
                    rate /= z_ofs2;
                }
                u_ofs2 *= rate;
                v_ofs2 *= rate;
                z_ofs2 = z_offset_default * rate;
            }
        }
        *x_offset = pd[p1].x;
        *z_offset = z_ofs1;
        *u_offset = u_ofs1;
        *v_offset = v_ofs1;
        rate = pd[p2].y - pd[p1].y;
        if (abs(rate) < 65536.0) {
            if (rate < 1.0) {
                rate = 65536.0;
            } else {
                rate = 65536.0 / rate;
            }
            *x_range = std::clamp((pd[p2].x - pd[p1].x) * rate, -2000000000.0, 2000000000.0);
            *z_range = std::clamp((z_ofs2 - z_ofs1) * rate, -2000000000.0, 2000000000.0);
            *u_range = std::clamp((u_ofs2 - u_ofs1) * rate, -2000000000.0, 2000000000.0);
            *v_range = std::clamp((v_ofs2 - v_ofs1) * rate, -2000000000.0, 2000000000.0);
        } else {
            rate = 65536.0 / rate;
            *x_range = (pd[p2].x - pd[p1].x) * rate;
            *z_range = (z_ofs2 - z_ofs1) * rate;
            *u_range = (u_ofs2 - u_ofs1) * rate;
            *v_range = (v_ofs2 - v_ofs1) * rate;
        }
        return p2;
    }*/

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

    /*
    int __cdecl rendering_t::calc_xzuv_offset_range_avx2(polydata_double* pd, int p1, int p2, uvdata* uvd) {

        double cam_screen_d16 = *cam_screen_d16_ptr;
        double z_offset_default = *z_offset_default_ptr;

        double rate;

        __m256d* mofs1 = (__m256d*)&uvd->offset;
        *mofs1 = _mm256_loadu_pd(&pd[p1].y);
        __m256d* mofs2 = (__m256d*)&uvd->range;
        *mofs2 = _mm256_loadu_pd(&pd[p2].y);
        xzuv* ofs1 = (xzuv*)mofs1;
        xzuv* ofs2 = (xzuv*)mofs2;

        if (cam_screen_d16 != 0.0) {
            if (ofs1->z == cam_screen_d16) {
                ofs1->z = z_offset_default;
            } else {
                rate = cam_screen_d16;
                if (ofs1->z != 0.0) {
                    rate /= ofs1->z;
                }
                ofs1->z = z_offset_default;
                *mofs1 = _mm256_mul_pd(*mofs1, _mm256_set1_pd(rate));
            }
            if (ofs2->z == cam_screen_d16) {
                ofs2->z = z_offset_default;
            } else {
                rate = cam_screen_d16;
                if (ofs2->z != 0.0) {
                    rate /= ofs2->z;
                }
                ofs2->z = z_offset_default;
                *mofs2 = _mm256_mul_pd(*mofs2, _mm256_set1_pd(rate));
            }
        }
        ofs1->x = pd[p1].x;
        ofs2->x = pd[p2].x;
        rate = pd[p2].y - pd[p1].y;
        if (abs(rate) < 65536.0) {
            if (rate < 1.0) {
                rate = 65536.0;
            } else {
                rate = 65536.0 / rate;
            }
            *mofs2 = _mm256_sub_pd(*mofs2, *mofs1);
            *mofs2 = _mm256_mul_pd(*mofs2, _mm256_set1_pd(rate));
            *mofs2 = _mm256_max_pd(*mofs2, _mm256_set1_pd(-2000000000.0));
            *mofs2 = _mm256_min_pd(*mofs2, _mm256_set1_pd(2000000000.0));
        } else {
            rate = 65536.0 / rate;
            *mofs2 = _mm256_sub_pd(*mofs2, *mofs1);
            *mofs2 = _mm256_mul_pd(*mofs2, _mm256_set1_pd(rate));
        }
        return p2;
    }

    void __cdecl rendering_t::calc_xzuv_avx2(int poly_num, polydata_double* pd) {

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
            i = (i - inc_or_dec + poly_num) % poly_num;
        } while (pd[poly_begin].y == pd[i].y);

        int poly_end;
        i = poly_top;
        do {
            poly_end = i;
            i = (i + inc_or_dec + poly_num) % poly_num;
        } while (pd[poly_end].y == pd[i].y);


        __m256d uvb256[2];
        __m256d uve256[2];
        uvdata* uvb = (uvdata*)uvb256;
        uvdata* uve = (uvdata*)uve256;

        int bb = calc_xzuv_offset_range_avx2(pd, poly_begin, (poly_begin - inc_or_dec + poly_num) % poly_num, uvb);
        int ee = calc_xzuv_offset_range_avx2(pd, poly_end, (poly_end + inc_or_dec + poly_num) % poly_num, uve);
        if (poly_begin == poly_end) {
            if (uvb->range.x > uve->range.x) {
                bb = calc_xzuv_offset_range_avx2(pd, poly_begin, (poly_begin + inc_or_dec + poly_num) % poly_num, uvb);
                ee = calc_xzuv_offset_range_avx2(pd, poly_end, (poly_end - inc_or_dec + poly_num) % poly_num, uve);
                inc_or_dec = -inc_or_dec;
                *rendering_inc_or_dec_ptr = inc_or_dec;
            }
        } else if (uvb->offset.x == uve->offset.x) {
            if (uvb->range.x > uve->range.x) {
                bb = calc_xzuv_offset_range_avx2(pd, poly_end, (poly_end + inc_or_dec + poly_num) % poly_num, uvb);
                ee = calc_xzuv_offset_range_avx2(pd, poly_begin, (poly_begin - inc_or_dec + poly_num) % poly_num, uve);
                inc_or_dec = -inc_or_dec;
                *rendering_inc_or_dec_ptr = inc_or_dec;
            }
        } else if (uvb->offset.x > uve->offset.x) {
            bb = calc_xzuv_offset_range_avx2(pd, poly_end, (poly_end + inc_or_dec + poly_num) % poly_num, uvb);
            ee = calc_xzuv_offset_range_avx2(pd, poly_begin, (poly_begin - inc_or_dec + poly_num) % poly_num, uve);
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

        __m128i uvbi128[2];
        __m128i uvei128[2];
        __m256d rate256;
        __m256d range256;
        int x_begin3, x_end3;
        int next_y = top_y;

        if (top_fraction) {
            next_y += top_fraction;
            rate256 = _mm256_set1_pd((double)top_fraction * 1.52587890625e-05);
            uvbi128[0] = _mm256_cvtpd_epi32(uvb256[0]);
            uvbi128[1] = _mm256_cvtpd_epi32(uvb256[1]);
            range256 = _mm256_mul_pd(uvb256[1], rate256);
            uvb256[0] = _mm256_add_pd(uvb256[0], range256);
            x_begin3 = (int)(uvb->offset.x);
            if (uvb->offset.x < pd[bb].x && uvb->range.x < 0.0 || uvb->offset.x > pd[bb].x && uvb->range.x > 0.0) {
                uvb->offset.x = pd[bb].x;
            }

            uvei128[0] = _mm256_cvtpd_epi32(uve256[0]);
            uvei128[1] = _mm256_cvtpd_epi32(uve256[1]);
            range256 = _mm256_mul_pd(uve256[1], rate256);
            uve256[0] = _mm256_add_pd(uve256[0], range256);
            x_end3 = (int)(uve->offset.x);
            if (uve->offset.x < pd[ee].x && uve->range.x < 0.0 || uve->offset.x > pd[ee].x && uve->range.x > 0.0) {
                uve->offset.x = pd[ee].x;
            }

            push_rendering_data(top_y, next_y, uvbi128[0].m128i_i32[0], uvei128[0].m128i_i32[0], (int)(uvb->offset.x), (int)(uve->offset.x),
                uvbi128[0].m128i_i32[2], uvei128[0].m128i_i32[2], uvbi128[0].m128i_i32[3], uvei128[0].m128i_i32[3], uvbi128[1].m128i_i32[2], uvbi128[1].m128i_i32[3],
                x_begin3, x_end3, uvbi128[0].m128i_i32[1], uvei128[0].m128i_i32[1], uvbi128[1].m128i_i32[1], uvei128[1].m128i_i32[2], uvei128[1].m128i_i32[3], uvei128[1].m128i_i32[1]);


            double next_y_d = (double)next_y;
            while (pd[bb].y < next_y_d) {
                rate256 = _mm256_set1_pd((next_y_d - pd[bb].y) * 1.52587890625e-05);
                bb = calc_xzuv_offset_range_avx2(pd, bb, ((bb - inc_or_dec) + poly_num) % poly_num, uvb);
                range256 = _mm256_mul_pd(uvb256[1], rate256);
                uvb256[0] = _mm256_add_pd(uvb256[0], range256);
            }
            while (pd[ee].y < next_y_d) {
                rate256 = _mm256_set1_pd((next_y_d - pd[ee].y) * 1.52587890625e-05);
                ee = calc_xzuv_offset_range_avx2(pd, ee, (ee + inc_or_dec + poly_num) % poly_num, uve);
                range256 = _mm256_mul_pd(uve256[1], rate256);
                uve256[0] = _mm256_add_pd(uve256[0], range256);
            }
        }
        for (i = 0; i < y_loop; i++) {
            int yy = next_y;
            next_y += 0x10000;
            uvbi128[0] = _mm256_cvtpd_epi32(uvb256[0]);
            uvbi128[1] = _mm256_cvtpd_epi32(uvb256[1]);
            uvb256[0] = _mm256_add_pd(uvb256[0], uvb256[1]);
            x_begin3 = (int)(uvb->offset.x);
            if (uvb->offset.x < pd[bb].x && uvb->range.x < 0.0 || uvb->offset.x > pd[bb].x && uvb->range.x > 0.0) {
                uvb->offset.x = pd[bb].x;
            }

            uvei128[0] = _mm256_cvtpd_epi32(uve256[0]);
            uvei128[1] = _mm256_cvtpd_epi32(uve256[1]);
            uve256[0] = _mm256_add_pd(uve256[0], uve256[1]);
            x_end3 = (int)(uve->offset.x);
            if (uve->offset.x < pd[ee].x && uve->range.x < 0.0 || uve->offset.x > pd[ee].x && uve->range.x > 0.0) {
                uve->offset.x = pd[ee].x;
            }

            push_rendering_data(yy, next_y, uvbi128[0].m128i_i32[0], uvei128[0].m128i_i32[0], (int)(uvb->offset.x), (int)(uve->offset.x),
                uvbi128[0].m128i_i32[2], uvei128[0].m128i_i32[2], uvbi128[0].m128i_i32[3], uvei128[0].m128i_i32[3], uvbi128[1].m128i_i32[2], uvbi128[1].m128i_i32[3],
                x_begin3, x_end3, uvbi128[0].m128i_i32[1], uvei128[0].m128i_i32[1], uvbi128[1].m128i_i32[1], uvei128[1].m128i_i32[2], uvei128[1].m128i_i32[3], uvei128[1].m128i_i32[1]);

            double next_y_d = (double)next_y;
            while (pd[bb].y < next_y_d) {
                rate256 = _mm256_set1_pd((next_y_d - pd[bb].y) * 1.52587890625e-05);
                bb = calc_xzuv_offset_range_avx2(pd, bb, ((bb - inc_or_dec) + poly_num) % poly_num, uvb);
                range256 = _mm256_mul_pd(uvb256[1], rate256);
                uvb256[0] = _mm256_add_pd(uvb256[0], range256);
            }
            while (pd[ee].y < next_y_d) {
                rate256 = _mm256_set1_pd((next_y_d - pd[ee].y) * 1.52587890625e-05);
                ee = calc_xzuv_offset_range_avx2(pd, ee, (ee + inc_or_dec + poly_num) % poly_num, uve);
                range256 = _mm256_mul_pd(uve256[1], rate256);
                uve256[0] = _mm256_add_pd(uve256[0], range256);
            }
        }
        if (bottom_fraction) {
            double rate = (double)bottom_fraction * 1.52587890625e-05;

            uvbi128[0] = _mm256_cvtpd_epi32(uvb256[0]);
            uvbi128[1] = _mm256_cvtpd_epi32(uvb256[1]);
            uvb->offset.x += uvb->range.x * rate;
            x_begin3 = (int)(uvb->offset.x);
            if (uvb->offset.x < pd[bb].x && uvb->range.x < 0.0 || uvb->offset.x > pd[bb].x && uvb->range.x > 0.0) {
                uvb->offset.x = pd[bb].x;
            }

            uvei128[0] = _mm256_cvtpd_epi32(uve256[0]);
            uvei128[1] = _mm256_cvtpd_epi32(uve256[1]);
            uve->offset.x += uve->range.x * rate;
            x_end3 = (int)(uve->offset.x);
            if (uve->offset.x < pd[ee].x && uve->range.x < 0.0 || uve->offset.x > pd[ee].x && uve->range.x > 0.0) {
                uve->offset.x = pd[ee].x;
            }
            push_rendering_data(next_y, next_y + bottom_fraction, uvbi128[0].m128i_i32[0], uvei128[0].m128i_i32[0], (int)(uvb->offset.x), (int)(uve->offset.x),
                uvbi128[0].m128i_i32[2], uvei128[0].m128i_i32[2], uvbi128[0].m128i_i32[3], uvei128[0].m128i_i32[3], uvbi128[1].m128i_i32[2], uvbi128[1].m128i_i32[3],
                x_begin3, x_end3, uvbi128[0].m128i_i32[1], uvei128[0].m128i_i32[1], uvbi128[1].m128i_i32[1], uvei128[1].m128i_i32[2], uvei128[1].m128i_i32[3], uvei128[1].m128i_i32[1]);
        }
    }*/

} // namespace patch
#endif // ifdef PATCH_SWITCH_RENDERING