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

#include "patch_fast_blur.hpp"
#include "patch_fast_create_figure.hpp"
#ifdef PATCH_SWITCH_FAST_CREATE_FIGURE


namespace patch::fast {

    void __cdecl CreateFigure_t::CreateFigure_circle(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) { // 73d20
        auto& figure = *reinterpret_cast<CreateFigure_var*>(GLOBAL::exedit_base + OFS::ExEdit::CreateFigure_var_ptr);

        ExEdit::PixelYCA yca = { figure.color_y, figure.color_cb, figure.color_cr, 0 };

        int obj_half_h = (efpip->obj_h + 1) >> 1;
        int obj_half_w = (efpip->obj_w + 1) >> 1;
        int yy = thread_id * 2 - efpip->obj_h + 1;

        int sizesq = figure.size * figure.size;
        int size_t8 = sizesq * 8 / efpip->obj_w * figure.size / efpip->obj_h;
        int inner = 0;
        if (0 < figure.line_width) { // 0の場合塗りつぶしの挙動となるが、図形オブジェクトの場合0はここに来る前の関数で終了して描画無し。マスクは0で通す
            inner = figure.size - figure.line_width * 2 - 1;
        }

        if (0 < inner) {
            int innersq = inner * inner;
            int inner_t8 = figure.size * inner * 8 / efpip->obj_w * figure.size / efpip->obj_h;
            for (int y = thread_id; y < obj_half_h; y += thread_num) {
                int yysq = yy * figure.size / efpip->obj_h;
                yysq *= yysq;
                int innersq_myyaq = yysq + inner_t8 - innersq;

                ExEdit::PixelYCA* pixlt = (ExEdit::PixelYCA*)efpip->obj_edit + efpip->obj_line * y;
                ExEdit::PixelYCA* pixlb = (ExEdit::PixelYCA*)efpip->obj_edit + efpip->obj_line * (efpip->obj_h - 1 - y);
                ExEdit::PixelYCA* pixrt = pixlt + efpip->obj_w - 1;
                ExEdit::PixelYCA* pixrb = pixlb + efpip->obj_w - 1;
                int xx = 1 - efpip->obj_w;
                int x;
                for (x = 0; x < obj_half_w; x++) {
                    int xxsq = xx * figure.size / efpip->obj_w;
                    xxsq *= xxsq;
                    int a = sizesq - yysq - xxsq;
                    if (a <= 0) {
                        a = 0;
                    } else {
                        if (a < size_t8) {
                            a = a * 0x1000 / size_t8;
                        } else {
                            a = 0x1000;
                        }
                        int a_inner = xxsq + innersq_myyaq;
                        if (a_inner < 0) {
                            yca.a = 0;
                            break;
                        } else if (a_inner < inner_t8) {
                            a = a * a_inner / inner_t8;
                        }
                    }
                    yca.a = (short)a;
                    *pixlt = *pixrt = *pixlb = *pixrb = yca;
                    pixlt++; pixrt--; pixlb++; pixrb--;
                    xx += 2;
                }
                for (; x < obj_half_w; x++) { // a = 0 
                    *pixlt = *pixrt = *pixlb = *pixrb = yca;
                    pixlt++; pixrt--; pixlb++; pixrb--;
                }
                yy += 2 * thread_num;
            }
        } else {
            for (int y = thread_id; y < obj_half_h; y += thread_num) {
                int yysq = yy * figure.size / efpip->obj_h;
                yysq *= yysq;
                ExEdit::PixelYCA* pixlt = (ExEdit::PixelYCA*)efpip->obj_edit + efpip->obj_line * y;
                ExEdit::PixelYCA* pixlb = (ExEdit::PixelYCA*)efpip->obj_edit + efpip->obj_line * (efpip->obj_h - 1 - y);
                ExEdit::PixelYCA* pixrt = pixlt + efpip->obj_w - 1;
                ExEdit::PixelYCA* pixrb = pixlb + efpip->obj_w - 1;
                int xx = 1 - efpip->obj_w;
                int x;
                for (x = 0; x < obj_half_w; x++) {
                    int xxsq = xx * figure.size / efpip->obj_w;
                    xxsq *= xxsq;
                    int a = sizesq - yysq - xxsq;
                    if (a <= 0) {
                        a = 0;
                    } else if (a < size_t8) {
                        a = a * 0x1000 / size_t8;
                    } else {
                        yca.a = 0x1000;
                        break;
                    }
                    yca.a = (short)a;
                    *pixlt = *pixrt = *pixlb = *pixrb = yca;
                    pixlt++; pixrt--; pixlb++; pixrb--;
                    xx += 2;
                }
                for (; x < obj_half_w; x++) { // a = 0x1000 
                    *pixlt = *pixrt = *pixlb = *pixrb = yca;
                    pixlt++; pixrt--; pixlb++; pixrb--;
                }
                yy += 2 * thread_num;
            }
        }
    }

    void __cdecl CreateFigure_t::CreateFigure_polygons(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        auto& figure = *reinterpret_cast<CreateFigure_var*>(GLOBAL::exedit_base + OFS::ExEdit::CreateFigure_var_ptr);

        int xp[10];
        int xylen[10];
        int yp[10];

        int inner = 0;
        if (figure.type == 10) {
            double angle = 0.0;
            double sina0 = 0.0;
            double cosa0 = 1.0;
            for (int i = 0; i < 10; i += 2) {
                angle += 1.256637061435917;
                double sina1 = sin(angle);
                double cosa1 = cos(angle);
                double xx0 = sina1 * 65536.0;
                xp[i] = (int)xx0;
                double yy0 = cosa1 * 65536.0;
                yp[i] = (int)-yy0;
                double xx1 = sina0 * 65536.0;
                xp[i + 1] = (int)xx1;
                double yy1 = cosa0 * 65536.0;
                yp[i + 1] = (int)-yy1;
                xylen[i] = (int)((sina0 * xx0 + cosa0 * yy0) * (double)figure.size) + 0x20000;
                xylen[i + 1] = (int)((sina1 * xx1 + cosa1 * yy1) * (double)figure.size) + 0x20000;
                sina0 = sina1;
                cosa0 = cosa1;
            }
            if (figure.line_width) {
                inner = (int)((double)figure.line_width * 40642.48062015503876) + 0x20000;
            }
        } else {
            double angle_rate = 3.141592653589793 / (double)figure.type;
            double angle0 = 0.0;
            for (int i = 0; i < figure.type; i++) {
                double angle1 = angle_rate + angle0;
                double xx = sin(angle1) * 65536.0;
                xp[i] = (int)xx;
                double yy = cos(angle1) * 65536.0;
                yp[i] = (int)-yy;
                xylen[i] = (int)((sin(angle0) * xx + cos(angle0) * yy) * (double)figure.size) + 0x20000;
                angle0 = angle1 + angle_rate;
            }
            if (figure.line_width) {
                inner = (int)((double)(figure.line_width << 17) * cos(angle_rate)) + 0x20000;
            }
        }


        int obj_half_w = (efpip->obj_w + 1) >> 1;
        ExEdit::PixelYCA yca = { figure.color_y, figure.color_cb, figure.color_cr, 0 };

        float angle_rate = (float)figure.type / 6.283185307179586f;
        int yy = (thread_id * 2 - efpip->obj_h) + 1;
        if (7 < obj_half_w && has_flag(get_CPUCmdSet(), CPUCmdSet::F_AVX2)) {
            __m256i xa = _mm256_set_epi32(-14, -12, -10, -8, -6, -4, -2, 0);
            __m256 pi256 = _mm256_set1_ps(3.141592653589793f);
            __m256 rate256 = _mm256_set1_ps(angle_rate);
            __m256i type256 = _mm256_set1_epi32(figure.type);
            __m256i min256 = _mm256_setzero_si256();
            __m256i max256 = _mm256_set1_epi32(0x1000);
            for (int y = thread_id; y < efpip->obj_h; y += thread_num) {
                int xx = efpip->obj_w - 1;
                ExEdit::PixelYCA* pixl = (ExEdit::PixelYCA*)efpip->obj_edit + efpip->obj_line * y;
                ExEdit::PixelYCA* pixr = pixl + xx;

                __m256i yy256 = _mm256_set1_epi32(yy);
                __m256 yf256 = _mm256_cvtepi32_ps(yy256);
                for (int x = (obj_half_w + 7) >> 3; 0 < x; x--) {
                    __m256i xx256 = _mm256_add_epi32(_mm256_set1_epi32(xx), xa);
                    __m256 angle256 = _mm256_atan2_ps(_mm256_cvtepi32_ps(xx256), yf256);
                    angle256 = _mm256_mul_ps(_mm256_add_ps(angle256, pi256), rate256);
                    __m256i pt256 = _mm256_mod_epi32(_mm256_cvttps_epi32(angle256), type256);
                    __m256i xp256 = _mm256_mullo_epi32(_mm256_i32gather_epi32(xp, pt256, 4), xx256);
                    __m256i yp256 = _mm256_mullo_epi32(_mm256_i32gather_epi32(yp, pt256, 4), yy256);
                    __m256i dist256 = _mm256_sub_epi32(xp256, yp256);
                    dist256 = _mm256_add_epi32(_mm256_i32gather_epi32(xylen, pt256, 4), dist256);
                    __m256i a256 = _mm256_clamp_epi32(_mm256_srai_epi32(dist256, 6), min256, max256);

                    for (int i = 0; i < 8; i++) {
                        int a = a256.m256i_i32[i];
                        if (0 < a && 0 < inner) {
                            int subinner = dist256.m256i_i32[i] - inner;
                            if (0 < subinner) {
                                subinner = 0x40000 - subinner;
                                if (0 < subinner) {
                                    a = a * subinner >> 18;
                                } else {
                                    a = 0;
                                }
                            }
                        }
                        yca.a = (short)a;
                        *pixl = *pixr = yca;
                        pixl++; pixr--;
                    }
                    xx -= 16;
                }
                yy += 2 * thread_num;
            }
        } else {
            for (int y = thread_id; y < efpip->obj_h; y += thread_num) {
                int xx = efpip->obj_w - 1;
                ExEdit::PixelYCA* pixl = (ExEdit::PixelYCA*)efpip->obj_edit + efpip->obj_line * y;
                ExEdit::PixelYCA* pixr = pixl + xx;

                for (int x = obj_half_w; 0 < x; x--) {
                    int pt = (int)((atan2((double)xx, (double)yy) + 3.141592653589793) * angle_rate) % figure.type;
                    int dist = xylen[pt] + xp[pt] * xx - yp[pt] * yy;

                    int a = std::clamp(dist >> 6, 0, 0x1000);
                    if (0 < a && 0 < inner) {
                        int subinner = dist - inner;
                        if (0 < subinner) {
                            subinner = 0x40000 - subinner;
                            if (0 < subinner) {
                                a = a * subinner >> 18;
                            } else {
                                a = 0;
                            }
                        }
                    }
                    yca.a = (short)a;
                    *pixl = *pixr = yca;
                    pixl++; pixr--;
                    xx -= 2;
                }
                yy += 2 * thread_num;
            }
        }
    }

    

    void __cdecl CreateFigure_t::vertical_blur_mt_func_wrap(AviUtl::MultiThreadFunc mt, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)Blur_t::mt_calc_yc_ssss, nullptr, efpip);
        efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)vertical_blur_mt, efp, efpip);
    }

    void __cdecl CreateFigure_t::horizontal_blur_mt(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        auto map = (Map_var*)(GLOBAL::exedit_base + OFS::ExEdit::Map_var_ptr);
        Blur_t::blur_yca_mt(thread_id * efpip->obj_h / thread_num, (thread_id + 1) * efpip->obj_h / thread_num,
            efpip->obj_edit, *reinterpret_cast<void**>(GLOBAL::exedit_base + OFS::ExEdit::memory_ptr),
            sizeof(ExEdit::PixelYCA), efpip->obj_line * sizeof(ExEdit::PixelYCA), efpip->obj_w, map->blur_size_w);
    }


    void __cdecl CreateFigure_t::vertical_blur_mt(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        auto map = (Map_var*)(GLOBAL::exedit_base + OFS::ExEdit::Map_var_ptr);
        Blur_t::blur_yca_mt(thread_id * efpip->obj_w / thread_num, (thread_id + 1) * efpip->obj_w / thread_num,
            *reinterpret_cast<void**>(GLOBAL::exedit_base + OFS::ExEdit::memory_ptr), efpip->obj_edit,
            efpip->obj_line * sizeof(ExEdit::PixelYCA), sizeof(ExEdit::PixelYCA), efpip->obj_h, map->blur_size_h);
    }

    void __cdecl CreateFigure_t::horizontal_blur_mt_func_wrap(AviUtl::MultiThreadFunc mt, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)horizontal_blur_mt, efp, efpip);
        auto map = (Map_var*)(GLOBAL::exedit_base + OFS::ExEdit::Map_var_ptr);
        efpip->obj_w += map->blur_size_w * 2;
        efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)Blur_t::mt_calc_yca_ssss, nullptr, efpip);
        efpip->obj_w -= map->blur_size_w * 2;
    }


} // namespace patch::fast
#endif // ifdef PATCH_SWITCH_FAST_CREATE_FIGURE
