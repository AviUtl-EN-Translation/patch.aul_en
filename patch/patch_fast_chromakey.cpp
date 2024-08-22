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

#include "patch_fast_chromakey.hpp"
#ifdef PATCH_SWITCH_FAST_CHROMAKEY


//#define PATCH_STOPWATCH

namespace patch::fast {

    void __cdecl border_color_mt1_avx2(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        int* memory_ptr = *(int**)(GLOBAL::exedit_base + OFS::ExEdit::memory_ptr);
        int keycb = *(short*)((int)efp->exdata_ptr + 2);
        int keycr = *(short*)((int)efp->exdata_ptr + 4);
        float keyangle = atan2((float)keycr, (float)keycb) * -10430.37835047045f;
        int satkey = max(std::abs(keycb), std::abs(keycr));
        int hue_range = efp->track[0] << 7;
        int sat_range = efp->track[1] * satkey >> 8;

        __m256 keyangle256 = _mm256_set1_ps(keyangle);
        __m256 divPI256 = _mm256_set1_ps(10430.37835047045f);
        __m256i satkey256 = _mm256_set1_epi32(satkey);
        __m256i hue_range256 = _mm256_set1_epi32(hue_range);
        __m256i sat_range256 = _mm256_set1_epi32(sat_range);
        __m256i offset256 = _mm256_set_epi32(56, 48, 40, 32, 24, 16, 8, 0);
        __m256i zero256 = _mm256_setzero_si256();
        __m256i max256 = _mm256_set1_epi32(0x1000);

        int y = efpip->obj_h * thread_id / thread_num;
        auto buf_alpha = (int*)efpip->obj_temp + efpip->obj_w * (efpip->obj_h + y);
        int offset = efpip->obj_line * y;
        for (y = efpip->obj_h * (thread_id + 1) / thread_num - y; 0 < y; y--) {
            auto buf_hue_angle = (int*)memory_ptr + offset;
            short* cbcr = &efpip->obj_edit[offset].cb;
            offset += efpip->obj_line;
            for (int x = efpip->obj_w >> 3; 0 < x; x--) {
                __m256i cbcr256 = _mm256_i32gather_epi32((int*)cbcr, offset256, 1);
                cbcr += 32;
                __m256i cb256 = _mm256_srai_epi32(_mm256_slli_epi32(cbcr256, 16), 16);
                __m256i cr256 = _mm256_srai_epi32(cbcr256, 16);
                __m256 angle256 = _mm256_atan2_ps(_mm256_cvtepi32_ps(cr256), _mm256_cvtepi32_ps(cb256));
                angle256 = _mm256_fmadd_ps(angle256, divPI256, keyangle256);
                __m256i norm_angle256 = _mm256_srai_epi32(_mm256_slli_epi32(_mm256_cvtps_epi32(angle256), 16), 16);
                __m256i sub_hue256 = _mm256_sub_epi32(_mm256_abs_epi32(norm_angle256), hue_range256);
                sub_hue256 = _mm256_max_epi32(zero256, sub_hue256);
                _mm256_storeu_epi32(buf_hue_angle, sub_hue256);
                buf_hue_angle += 8;
                __m256i max_sat256 = _mm256_max_epi32(_mm256_abs_epi32(cb256), _mm256_abs_epi32(cr256));
                __m256i sub_sat256 = _mm256_abs_epi32(_mm256_sub_epi32(max_sat256, satkey256));
                sub_sat256 = _mm256_max_epi32(zero256, _mm256_sub_epi32(sub_sat256, sat_range256));
                sub_sat256 = _mm256_slli_epi32(sub_sat256, 3);
                __m256i alpha256 = _mm256_min_epi32(_mm256_add_epi32(sub_sat256, sub_hue256), max256);
                _mm256_storeu_epi32(buf_alpha, alpha256);
                buf_alpha += 8;
            }
            for (int x = efpip->obj_w & 7; 0 < x; x--) {
                int pixcb = cbcr[0];
                int pixcr = cbcr[1];
                cbcr += 4;
                float pixangle = atan2((float)pixcr, (float)pixcb);
                int sub_hue = max(0, std::abs((short)round(fmaf(pixangle, 10430.37835047045f, keyangle))) - hue_range);
                *buf_hue_angle = sub_hue;
                buf_hue_angle++;
                sub_hue += max(0, std::abs(max(std::abs(pixcb), std::abs(pixcr)) - satkey) - sat_range) << 3;
                *buf_alpha = (short)min(sub_hue, 0x1000);
                buf_alpha++;
            }
        }
    }
    void __cdecl border_color_mt1(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        int* memory_ptr = *(int**)(GLOBAL::exedit_base + OFS::ExEdit::memory_ptr);
        int keycb = *(short*)((int)efp->exdata_ptr + 2);
        int keycr = *(short*)((int)efp->exdata_ptr + 4);
        float keyangle = atan2((float)keycr, (float)keycb) * -10430.37835047045f;
        int satkey = max(std::abs(keycb), std::abs(keycr));
        int hue_range = efp->track[0] << 7;
        int sat_range = efp->track[1] * satkey >> 8;

        int y = efpip->obj_h * thread_id / thread_num;
        auto buf_alpha = (int*)efpip->obj_temp + efpip->obj_w * (efpip->obj_h + y);
        int offset = efpip->obj_line * y;
        for (y = efpip->obj_h * (thread_id + 1) / thread_num - y; 0 < y; y--) {
            auto buf_hue_angle = (int*)memory_ptr + offset;
            short* cbcr = &efpip->obj_edit[offset].cb;
            offset += efpip->obj_line;
            for (int x = efpip->obj_w; 0 < x; x--) {
                int pixcb = cbcr[0];
                int pixcr = cbcr[1];
                cbcr += 4;
                float pixangle = atan2((float)pixcr, (float)pixcb);
                int sub_hue = max(0, std::abs((short)round(pixangle * 10430.37835047045f + keyangle)) - hue_range);
                *buf_hue_angle = sub_hue;
                buf_hue_angle++;
                sub_hue += max(0, std::abs(max(std::abs(pixcb), std::abs(pixcr)) - satkey) - sat_range) << 3;
                *buf_alpha = (short)min(sub_hue, 0x1000);
                buf_alpha++;
            }
        }
    }


    void __cdecl border_mt1_avx2(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        int keycb = *(short*)((int)efp->exdata_ptr + 2);
        int keycr = *(short*)((int)efp->exdata_ptr + 4);
        float keyangle = atan2((float)keycr, (float)keycb) * -10430.37835047045f;
        int satkey = max(std::abs(keycb), std::abs(keycr));
        int hue_range = efp->track[0] << 7;
        int sat_range = efp->track[1] * satkey >> 8;

        __m256 keyangle256 = _mm256_set1_ps(keyangle);
        __m256 divPI256 = _mm256_set1_ps(10430.37835047045f);
        __m256i satkey256 = _mm256_set1_epi32(satkey);
        __m256i hue_range256 = _mm256_set1_epi32(hue_range);
        __m256i sat_range256 = _mm256_set1_epi32(sat_range);
        __m256i offset256 = _mm256_set_epi32(56, 48, 40, 32, 24, 16, 8, 0);
        __m256i zero256 = _mm256_setzero_si256();
        __m256i max256 = _mm256_set1_epi32(0x1000);

        int y = efpip->obj_h * thread_id / thread_num;
        auto buf_alpha = (int*)efpip->obj_temp + efpip->obj_w * (efpip->obj_h + y);
        int offset = efpip->obj_line * y;
        for (y = efpip->obj_h * (thread_id + 1) / thread_num - y; 0 < y; y--) {
            short* cbcr = &efpip->obj_edit[offset].cb;
            offset += efpip->obj_line;
            for (int x = efpip->obj_w >> 3; 0 < x; x--) {
                __m256i cbcr256 = _mm256_i32gather_epi32((int*)cbcr, offset256, 1);
                cbcr += 32;
                __m256i cb256 = _mm256_srai_epi32(_mm256_slli_epi32(cbcr256, 16), 16);
                __m256i cr256 = _mm256_srai_epi32(cbcr256, 16);
                __m256 angle256 = _mm256_atan2_ps(_mm256_cvtepi32_ps(cr256), _mm256_cvtepi32_ps(cb256));
                angle256 = _mm256_fmadd_ps(angle256, divPI256, keyangle256);
                __m256i norm_angle256 = _mm256_srai_epi32(_mm256_slli_epi32(_mm256_cvtps_epi32(angle256), 16), 16);
                __m256i sub_hue256 = _mm256_sub_epi32(_mm256_abs_epi32(norm_angle256), hue_range256);
                sub_hue256 = _mm256_max_epi32(zero256, sub_hue256);
                __m256i max_sat256 = _mm256_max_epi32(_mm256_abs_epi32(cb256), _mm256_abs_epi32(cr256));
                __m256i sub_sat256 = _mm256_abs_epi32(_mm256_sub_epi32(max_sat256, satkey256));
                sub_sat256 = _mm256_max_epi32(zero256, _mm256_sub_epi32(sub_sat256, sat_range256));
                sub_sat256 = _mm256_slli_epi32(sub_sat256, 3);
                __m256i alpha256 = _mm256_min_epi32(_mm256_add_epi32(sub_sat256, sub_hue256), max256);
                _mm256_storeu_epi32(buf_alpha, alpha256);
                buf_alpha += 8;
            }
            for (int x = efpip->obj_w & 7; 0 < x; x--) {
                int pixcb = cbcr[0];
                int pixcr = cbcr[1];
                cbcr += 4;
                float pixangle = atan2((float)pixcr, (float)pixcb);
                int sub_hue = max(0, std::abs((short)round(fmaf(pixangle, 10430.37835047045f, keyangle))) - hue_range);
                sub_hue += max(0, std::abs(max(std::abs(pixcb), std::abs(pixcr)) - satkey) - sat_range) << 3;
                *buf_alpha = min(sub_hue, 0x1000);
                buf_alpha++;
            }
        }
    }
    void __cdecl border_mt1(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        int keycb = *(short*)((int)efp->exdata_ptr + 2);
        int keycr = *(short*)((int)efp->exdata_ptr + 4);
        float keyangle = atan2((float)keycr, (float)keycb) * -10430.37835047045f;
        int satkey = max(std::abs(keycb), std::abs(keycr));
        int hue_range = efp->track[0] << 7;
        int sat_range = efp->track[1] * satkey >> 8;

        int y = efpip->obj_h * thread_id / thread_num;
        auto buf_alpha = (int*)efpip->obj_temp + efpip->obj_w * (efpip->obj_h + y);
        int offset = efpip->obj_line * y;
        for (y = efpip->obj_h * (thread_id + 1) / thread_num - y; 0 < y; y--) {
            short* cbcr = &efpip->obj_edit[offset].cb;
            offset += efpip->obj_line;
            for (int x = efpip->obj_w; 0 < x; x--) {
                int pixcb = cbcr[0];
                int pixcr = cbcr[1];
                cbcr += 4;
                float pixangle = atan2((float)pixcr, (float)pixcb);
                int sub_hue = max(0, std::abs((short)round(pixangle * 10430.37835047045f + keyangle)) - hue_range);
                sub_hue += max(0, std::abs(max(std::abs(pixcb), std::abs(pixcr)) - satkey) - sat_range) << 3;
                *buf_alpha = min(sub_hue, 0x1000);
                buf_alpha++;
            }
        }
    }

    void __cdecl border_mt2(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        auto ck = reinterpret_cast<Chromakey_t::efChromakey_var*>(GLOBAL::exedit_base + OFS::ExEdit::efChromakey_var_ptr);
        int x = efpip->obj_w * thread_id / thread_num;
        int w = efpip->obj_w * (thread_id + 1) / thread_num - x;
        auto dst = (int*)efpip->obj_temp + x;
        auto src = dst + efpip->obj_w * (efpip->obj_h);
        int step = efpip->obj_w - w;
        int ngline = -efpip->obj_w;

        memset(dst, 0, w * sizeof(int));

        for (int y = ck->border_size; 0 <= y; y--) {
            for (x = w; 0 < x; x--) {
                *dst += *src;
                dst++; src++;
            }
            src += step;
            dst -= w;
        }
        dst += step + w;
        for (int y = ck->border_size; 0 < y; y--) {
            for (x = w; 0 < x; x--) {
                *dst = *(dst + ngline) + *src;
                dst++; src++;
            }
            dst += step; src += step;
        }

        int nglineb = -efpip->obj_w * ck->border_range;
        for (int y = efpip->obj_h - ck->border_range; 0 < y; y--) {
            for (x = w; 0 < x; x--) {
                *dst = *(dst + ngline) + *src - *(src + nglineb);
                dst++; src++;
            }
            dst += step; src += step;
        }
        src += nglineb;
        for (int y = ck->border_size; 0 < y; y--) {
            for (x = w; 0 < x; x--) {
                *dst = *(dst + ngline) - *src;
                dst++; src++;
            }
            dst += step; src += step;
        }
    }

    void __cdecl border_color_mt3(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        auto ck = reinterpret_cast<Chromakey_t::efChromakey_var*>(GLOBAL::exedit_base + OFS::ExEdit::efChromakey_var_ptr);
        int* memory_ptr = *(int**)(GLOBAL::exedit_base + OFS::ExEdit::memory_ptr);
        int loop3 = efpip->obj_w - ck->border_range;
        int keycb = *(short*)((int)efp->exdata_ptr + 2);
        int keycr = *(short*)((int)efp->exdata_ptr + 4);
        int satkey = max(1, max(std::abs(keycb), std::abs(keycr)));
        int oa = (1 - ck->border_size) << 12;
        int thres = (-oa) / ck->border_size;
        int ye = efpip->obj_h * (thread_id + 1) / thread_num;
        int y = efpip->obj_h * thread_id / thread_num;
        auto srca = (int*)efpip->obj_temp + efpip->obj_w * (efpip->obj_h + y);
        auto srcb1 = (int*)efpip->obj_temp + efpip->obj_w * y;
        int offset = efpip->obj_line * y;
        for (; y < ye; y++) {
            int yrange = min(ck->border_size + min(y + 1, efpip->obj_h - y), ck->border_range);
            auto srcb2 = srcb1;
            auto srcha = (int*)memory_ptr + offset;
            auto dst = (ExEdit::PixelYCA*)efpip->obj_edit + offset;
            int sum = 0;
            for (int x = ck->border_size; 0 < x; x--) {
                sum += *srcb1;
                srcb1++;
            }
            int xrange = ck->border_size;
            for (int x = ck->border_size; 0 <= x; x--) {
                sum += *srcb1;
                xrange++;
                int a = (sum / (xrange * yrange) * *srca) >> 12;
                if (a <= thres) {
                    dst->a = 0;
                } else {
                    a = a * ck->border_size + oa;
                    int sub_hue = max((((satkey - max(abs(dst->cr), abs(dst->cb))) << 12)) / satkey, *srcha);
                    if (sub_hue < 0x1000) {
                        if (2 <= sub_hue) {
                            dst->cb = (short)std::clamp(((dst->cb - keycb) << 12) / sub_hue + keycb, -0x800, 0x800);
                            dst->cr = (short)std::clamp(((dst->cr - keycr) << 12) / sub_hue + keycr, -0x800, 0x800);
                        }
                        if (efp->check[1]) {
                            a = a * sub_hue >> 12;
                        }
                    }
                    dst->a = dst->a * a >> 12;
                }
                srcb1++;
                srca++;
                srcha++;
                dst++;
            }
            int border_sq_range = (xrange * yrange);
            for (int x = loop3; 0 < x; x--) {
                sum += *srcb1 - *srcb2;
                int a = (sum / border_sq_range * *srca) >> 12;
                if (a <= thres) {
                    dst->a = 0;
                } else {
                    a = a * ck->border_size + oa;
                    int sub_hue = max((((satkey - max(abs(dst->cr), abs(dst->cb))) << 12)) / satkey, *srcha);
                    if (sub_hue < 0x1000) {
                        if (2 <= sub_hue) {
                            dst->cb = (short)std::clamp(((dst->cb - keycb) << 12) / sub_hue + keycb, -0x800, 0x800);
                            dst->cr = (short)std::clamp(((dst->cr - keycr) << 12) / sub_hue + keycr, -0x800, 0x800);
                        }
                        if (efp->check[1]) {
                            a = a * sub_hue >> 12;
                        }
                    }
                    dst->a = dst->a * a >> 12;
                }
                srcb1++;
                srcb2++;
                srca++;
                srcha++;
                dst++;
            }
            for (int x = ck->border_size; 0 < x; x--) {
                sum -= *srcb2;
                xrange--;
                int a = (sum / (xrange * yrange) * *srca) >> 12;
                if (a <= thres) {
                    dst->a = 0;
                } else {
                    a = a * ck->border_size + oa;
                    int sub_hue = max((((satkey - max(abs(dst->cr), abs(dst->cb))) << 12)) / satkey, *srcha);
                    if (sub_hue < 0x1000) {
                        if (2 <= sub_hue) {
                            dst->cb = (short)std::clamp(((dst->cb - keycb) << 12) / sub_hue + keycb, -0x800, 0x800);
                            dst->cr = (short)std::clamp(((dst->cr - keycr) << 12) / sub_hue + keycr, -0x800, 0x800);
                        }
                        if (efp->check[1]) {
                            a = a * sub_hue >> 12;
                        }
                    }
                    dst->a = dst->a * a >> 12;
                }
                srcb2++;
                srca++;
                srcha++;
                dst++;
            }
            offset += efpip->obj_line;
        }
    }

    void __cdecl border_mt3(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        auto ck = reinterpret_cast<Chromakey_t::efChromakey_var*>(GLOBAL::exedit_base + OFS::ExEdit::efChromakey_var_ptr);
        int transparent_check = efp->check[1];
        int loop3 = efpip->obj_w - ck->border_range;
        int keycb = *(short*)((int)efp->exdata_ptr + 2);
        int keycr = *(short*)((int)efp->exdata_ptr + 4);
        int satkey = max(1, max(std::abs(keycb), std::abs(keycr)));
        int oa = (1 - ck->border_size) << 12;
        int thres = (-oa) / ck->border_size;
        int ye = efpip->obj_h * (thread_id + 1) / thread_num;
        int y = efpip->obj_h * thread_id / thread_num;
        auto srca = (int*)efpip->obj_temp + efpip->obj_w * (efpip->obj_h + y);
        auto srcb1 = (int*)efpip->obj_temp + efpip->obj_w * y;
        int offset = efpip->obj_line * y;
        for (; y < ye; y++) {
            int yrange = min(ck->border_size + min(y + 1, efpip->obj_h - y), ck->border_range);
            auto srcb2 = srcb1;
            auto dst = (ExEdit::PixelYCA*)efpip->obj_edit + offset;
            int sum = 0;
            for (int x = ck->border_size; 0 < x; x--) {
                sum += *srcb1;
                srcb1++;
            }
            int xrange = ck->border_size;
            for (int x = ck->border_size; 0 <= x; x--) {
                sum += *srcb1;
                xrange++;
                int a = (sum / (xrange * yrange) * *srca) >> 12;
                if (a <= thres) {
                    dst->a = 0;
                } else {
                    a = a * ck->border_size + oa;
                    dst->a = dst->a * a >> 12;
                    int sub_hue = max((((satkey - max(abs(dst->cr), abs(dst->cb))) << 12)) / satkey, a);
                    if (sub_hue < 0x1000) {
                        if (2 <= sub_hue) {
                            dst->cb = (short)std::clamp(((dst->cb - keycb) << 12) / sub_hue + keycb, -0x800, 0x800);
                            dst->cr = (short)std::clamp(((dst->cr - keycr) << 12) / sub_hue + keycr, -0x800, 0x800);
                        }
                    }
                }
                srcb1++;
                srca++;
                dst++;
            }
            int border_sq_range = (xrange * yrange);
            for (int x = loop3; 0 < x; x--) {
                sum += *srcb1 - *srcb2;
                int a = (sum / border_sq_range * *srca) >> 12;
                if (a <= thres) {
                    dst->a = 0;
                } else {
                    a = a * ck->border_size + oa;
                    dst->a = dst->a * a >> 12;
                    int sub_hue = max((((satkey - max(abs(dst->cr), abs(dst->cb))) << 12)) / satkey, a);
                    if (sub_hue < 0x1000) {
                        if (2 <= sub_hue) {
                            dst->cb = (short)std::clamp(((dst->cb - keycb) << 12) / sub_hue + keycb, -0x800, 0x800);
                            dst->cr = (short)std::clamp(((dst->cr - keycr) << 12) / sub_hue + keycr, -0x800, 0x800);
                        }
                    }
                }
                srcb1++;
                srcb2++;
                srca++;
                dst++;
            }
            for (int x = ck->border_size; 0 < x; x--) {
                sum -= *srcb2;
                xrange--;
                int a = (sum / (xrange * yrange) * *srca) >> 12;
                if (a <= thres) {
                    dst->a = 0;
                } else {
                    a = a * ck->border_size + oa;
                    dst->a = dst->a * a >> 12;
                    int sub_hue = max((((satkey - max(abs(dst->cr), abs(dst->cb))) << 12)) / satkey, a);
                    if (sub_hue < 0x1000) {
                        if (2 <= sub_hue) {
                            dst->cb = (short)std::clamp(((dst->cb - keycb) << 12) / sub_hue + keycb, -0x800, 0x800);
                            dst->cr = (short)std::clamp(((dst->cr - keycr) << 12) / sub_hue + keycr, -0x800, 0x800);
                        }
                    }
                }
                srcb2++;
                srca++;
                dst++;
            }
            offset += efpip->obj_line;
        }
    }

    void __cdecl color_mt(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        int keycb = *(short*)((int)efp->exdata_ptr + 2);
        int keycr = *(short*)((int)efp->exdata_ptr + 4);
        float keyangle = atan2((float)keycr, (float)keycb) * -10430.37835047045f;
        int satkey = max(std::abs(keycb), std::abs(keycr));
        int hue_range = efp->track[0] << 7;
        int sat_range = efp->track[1] * satkey >> 8;
        satkey = max(1, satkey);

        __m256 keyangle256 = _mm256_set1_ps(keyangle);
        __m256 divPI256 = _mm256_set1_ps(10430.37835047045f);
        __m256i satkey256 = _mm256_set1_epi32(satkey);
        __m256i hue_range256 = _mm256_set1_epi32(hue_range);
        __m256i sat_range256 = _mm256_set1_epi32(sat_range);
        __m256i offset256 = _mm256_set_epi32(56, 48, 40, 32, 24, 16, 8, 0);
        __m256i zero256 = _mm256_setzero_si256();
        __m256i max256 = _mm256_set1_epi32(0x1000);

        int y = efpip->obj_h * thread_id / thread_num;
        short* cbcra0 = &efpip->obj_edit[efpip->obj_line * y].cb;
        for (y = efpip->obj_h * (thread_id + 1) / thread_num - y; 0 < y; y--) {
            short* cbcra = cbcra0;
            cbcra0 += efpip->obj_line * 4;
            for (int x = efpip->obj_w >> 3; 0 < x; x--) {
                __m256i cbcr256 = _mm256_i32gather_epi32((int*)cbcra, offset256, 1);
                __m256i cb256 = _mm256_srai_epi32(_mm256_slli_epi32(cbcr256, 16), 16);
                __m256i cr256 = _mm256_srai_epi32(cbcr256, 16);
                __m256 angle256 = _mm256_atan2_ps(_mm256_cvtepi32_ps(cr256), _mm256_cvtepi32_ps(cb256));
                angle256 = _mm256_fmadd_ps(angle256, divPI256, keyangle256);
                __m256i norm_angle256 = _mm256_srai_epi32(_mm256_slli_epi32(_mm256_cvtps_epi32(angle256), 16), 16);
                __m256i sub_hue256 = _mm256_sub_epi32(_mm256_abs_epi32(norm_angle256), hue_range256);
                sub_hue256 = _mm256_max_epi32(zero256, sub_hue256);
                __m256i max_sat256 = _mm256_max_epi32(_mm256_abs_epi32(cb256), _mm256_abs_epi32(cr256));
                __m256i sub_sat256 = _mm256_sub_epi32(satkey256, max_sat256);
                __m256i sat_hue256 = _mm256_sub_epi32(_mm256_abs_epi32(sub_sat256), sat_range256);
                sat_hue256 = _mm256_slli_epi32(_mm256_max_epi32(zero256, sat_hue256), 3);
                __m256i alpha256 = _mm256_min_epi32(_mm256_add_epi32(sat_hue256, sub_hue256), max256);

                __m256i cra256 = _mm256_i32gather_epi32((int*)&cbcra[1], offset256, 1);
                __m256i a256 = _mm256_srai_epi32(cra256, 16);
                a256 = _mm256_srai_epi32(_mm256_mullo_epi32(a256, alpha256), 12);

                sub_sat256 = _mm256_div_epi32(_mm256_slli_epi32(sub_sat256, 12), satkey256);
                alpha256 = _mm256_max_epi32(sub_hue256, sub_sat256);

                for (int i = 0; i < 16; i += 2) {
                    int a = alpha256.m256i_i16[i];
                    if (a < 0x1000) {
                        if (2 <= a) {
                            cbcra[0] = (short)std::clamp(((cb256.m256i_i16[i] - keycb) << 12) / a + keycb, -0x800, 0x800);
                            cbcra[1] = (short)std::clamp(((cr256.m256i_i16[i] - keycr) << 12) / a + keycr, -0x800, 0x800);
                        }
                        if (efp->check[1]) {
                            a256.m256i_i16[i] = a256.m256i_i16[i] * a >> 12;
                        }
                    }
                    cbcra[2] = a256.m256i_i16[i];

                    cbcra += 4;
                }
            }
            for (int x = efpip->obj_w & 7; 0 < x; x--) {
                int pixcb = cbcra[0];
                int pixcr = cbcra[1];
                float pixangle = atan2((float)pixcr, (float)pixcb);
                int sub_hue = max(0, std::abs((short)round(fmaf(pixangle, 10430.37835047045f, keyangle))) - hue_range);
                int sub_sat = satkey - max(std::abs(pixcb), std::abs(pixcr));
                int a = cbcra[2];
                int hue_a = max(sub_hue, (sub_sat << 12) / satkey);
                if (hue_a < 0x1000) {
                    if (2 <= hue_a) {
                        cbcra[0] = (short)std::clamp(((pixcb - keycb) << 12) / hue_a + keycb, -0x800, 0x800);
                        cbcra[1] = (short)std::clamp(((pixcr - keycr) << 12) / hue_a + keycr, -0x800, 0x800);
                    }
                    if (efp->check[1]) {
                        a = a * hue_a >> 12;
                    }
                }
                sub_hue += max(0, std::abs(sub_sat) - sat_range) << 3;
                if (sub_hue < 0x1000) {
                    a = a * sub_hue >> 12;
                }
                cbcra[2] = a;
                cbcra += 4;
            }
        }
    }


    void __cdecl else_mt(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        int keycb = *(short*)((int)efp->exdata_ptr + 2);
        int keycr = *(short*)((int)efp->exdata_ptr + 4);
        float keyangle = atan2((float)keycr, (float)keycb) * -10430.37835047045f;
        int satkey = max(std::abs(keycb), std::abs(keycr));
        int hue_range = efp->track[0] << 7;
        int sat_range = efp->track[1] * satkey >> 8;

        __m256 keyangle256 = _mm256_set1_ps(keyangle);
        __m256 divPI256 = _mm256_set1_ps(10430.37835047045f);
        __m256i satkey256 = _mm256_set1_epi32(satkey);
        __m256i hue_range256 = _mm256_set1_epi32(hue_range);
        __m256i sat_range256 = _mm256_set1_epi32(sat_range);
        __m256i offset256 = _mm256_set_epi32(56, 48, 40, 32, 24, 16, 8, 0);
        __m256i zero256 = _mm256_setzero_si256();
        __m256i max256 = _mm256_set1_epi32(0x1000);

        int y = efpip->obj_h * thread_id / thread_num;
        short* cbcra0 = &efpip->obj_edit[efpip->obj_line * y].cb;
        for (y = efpip->obj_h * (thread_id + 1) / thread_num - y; 0 < y; y--) {
            short* cbcra = cbcra0;
            cbcra0 += efpip->obj_line * 4;
            for (int x = efpip->obj_w >> 3; 0 < x; x--) {
                __m256i cbcr256 = _mm256_i32gather_epi32((int*)cbcra, offset256, 1);
                __m256i cb256 = _mm256_srai_epi32(_mm256_slli_epi32(cbcr256, 16), 16);
                __m256i cr256 = _mm256_srai_epi32(cbcr256, 16);
                __m256 angle256 = _mm256_atan2_ps(_mm256_cvtepi32_ps(cr256), _mm256_cvtepi32_ps(cb256));
                angle256 = _mm256_fmadd_ps(angle256, divPI256, keyangle256);
                __m256i norm_angle256 = _mm256_srai_epi32(_mm256_slli_epi32(_mm256_cvtps_epi32(angle256), 16), 16);
                __m256i sub_hue256 = _mm256_sub_epi32(_mm256_abs_epi32(norm_angle256), hue_range256);
                sub_hue256 = _mm256_max_epi32(zero256, sub_hue256);
                __m256i max_sat256 = _mm256_max_epi32(_mm256_abs_epi32(cb256), _mm256_abs_epi32(cr256));
                __m256i sub_sat256 = _mm256_abs_epi32(_mm256_sub_epi32(max_sat256, satkey256));
                sub_sat256 = _mm256_max_epi32(zero256, _mm256_sub_epi32(sub_sat256, sat_range256));
                sub_sat256 = _mm256_slli_epi32(sub_sat256, 3);
                __m256i alpha256 = _mm256_min_epi32(_mm256_add_epi32(sub_sat256, sub_hue256), max256);

                __m256i cra256 = _mm256_i32gather_epi32((int*)&cbcra[1], offset256, 1);
                __m256i a256 = _mm256_srai_epi32(cra256, 16);
                a256 = _mm256_srai_epi32(_mm256_mullo_epi32(a256, alpha256), 12);

                cbcra += 2;
                for (int i = 0; i < 16; i += 2) {
                    *cbcra = a256.m256i_i16[i];
                    cbcra += 4;
                }
                cbcra -= 2;
            }
            for (int x = efpip->obj_w & 7; 0 < x; x--) {
                int pixcb = cbcra[0];
                int pixcr = cbcra[1];
                float pixangle = atan2((float)pixcr, (float)pixcb);
                int sub_hue = max(0, std::abs((short)round(fmaf(pixangle, 10430.37835047045f, keyangle))) - hue_range);
                sub_hue += max(0, std::abs(max(std::abs(pixcb), std::abs(pixcr)) - satkey) - sat_range) << 3;
                if (sub_hue < 0x1000) {
                    cbcra[2] = (short)(cbcra[2] * sub_hue >> 12);
                }
                cbcra += 4;
            }
        }
    }


    BOOL __cdecl Chromakey_t::func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        auto exdata = reinterpret_cast<ExEdit::Exdata::efChromakey*>(efp->exdata_ptr);
        if (exdata->status != 1) {
            return TRUE;
        }
        bool avx2 = has_flag(get_CPUCmdSet(), CPUCmdSet::F_AVX2);
        auto ck = reinterpret_cast<Chromakey_t::efChromakey_var*>(GLOBAL::exedit_base + OFS::ExEdit::efChromakey_var_ptr);
        ck->border_size = min(efp->track[2], (min(efpip->obj_w, efpip->obj_h) - 1) >> 1);
        if (0 < ck->border_size) {
            ck->border_range = ck->border_size * 2 + 1;
            if (efp->check[0]) {
                ck->buf_hue_angle = *reinterpret_cast<int**>(GLOBAL::exedit_base + OFS::ExEdit::memory_ptr);
                if (avx2) {
                    efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)&border_color_mt1_avx2, efp, efpip);
                } else {
                    efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)&border_color_mt1, efp, efpip);
                }
                efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)&border_mt2, efp, efpip);
                efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)&border_color_mt3, efp, efpip);
            } else {
                ck->buf_hue_angle = *reinterpret_cast<int**>(GLOBAL::exedit_base + OFS::ExEdit::memory_ptr);
                if (avx2) {
                    efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)&border_mt1_avx2, efp, efpip);
                } else {
                    efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)&border_mt1, efp, efpip);
                }
                efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)&border_mt2, efp, efpip);
                efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)&border_mt3, efp, efpip);
            }
        } else if (efp->check[0]) {
            if (avx2) {
                efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)&color_mt, efp, efpip);
            } else {
                efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)(GLOBAL::exedit_base + 0x0130b0), efp, efpip);
            }
        } else {
            if (avx2) {
                efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)&else_mt, efp, efpip);
            } else {
                efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)(GLOBAL::exedit_base + 0x012f10), efp, efpip);
            }
        }
        return TRUE;
    }


} // namespace patch::fast


#endif // ifdef PATCH_SWITCH_FAST_CHROMAKEY
