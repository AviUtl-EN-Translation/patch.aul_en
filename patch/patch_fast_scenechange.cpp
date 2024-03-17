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

#include "patch_fast_scenechange.hpp"
#ifdef PATCH_SWITCH_FAST_SCENECHANGE


//#define PATCH_STOPWATCH

namespace patch::fast {


    void __cdecl SceneChange_t::scCircleWipe(ExEdit::PixelYC* dst, int w, int h, ExEdit::PixelYC* src1, ExEdit::PixelYC* src2, int rate, int track1, int line, ExEdit::Filter* efp) {
        int sqr = (int)((double)(w * w + h * h) * (double)rate * 0.000244140625) - (w - 1) * (w - 1) - (h - 1) * (h - 1);
        int md = (int)sqrt((double)(w * w + h * h) * (double)rate) >> 4;
        int sqdp = 0;
        if (rate < 1788) { // 16:9の画面ではこれくらいのタイミングで面積が1:1になる
            efp->exfunc->bufcpy(dst, 0, 0, src1, 0, 0, w, h, 0, 0);
            for (int y = 1 - h; y < h; y += 2) {
                auto dsty = dst;
                auto srcy = src2;
                int sqd = sqdp;
                for (int x = 1 - w; x < w; x += 2) {
                    int sd, d = sqr - sqd * 4;
                    if (d <= 0) {
                        if (0 <= x) {
                            break;
                        }
                    } else if (sd = md - d, 0 < sd) {
                        dsty->y = (dsty->y * sd + srcy->y * d) / md;
                        dsty->cb = (dsty->cb * sd + srcy->cb * d) / md;
                        dsty->cr = (dsty->cr * sd + srcy->cr * d) / md;
                    } else {
                        x = -x;
                        memcpy(dsty, srcy, (1 + x) * sizeof(ExEdit::PixelYC));
                        dsty += x;
                        srcy += x;
                    }
                    dsty++;
                    srcy++;
                    sqd += x + 1;
                }
                dst += line;
                src2 += line;
                sqdp += y + 1;
            }
        } else {
            efp->exfunc->bufcpy(dst, 0, 0, src2, 0, 0, w, h, 0, 0);
            for (int y = 1 - h; y < h; y += 2) {
                auto dsty = dst;
                auto srcy = src1;
                int sqd = sqdp;
                for (int x = 1 - w; x < w; x += 2) {
                    int sd, d = sqr - sqd * 4;
                    if (d <= 0) {
                        dsty->y = srcy->y;
                        dsty->cb = srcy->cb;
                        dsty->cr = srcy->cr;
                    } else if (sd = md - d, 0 < sd) {
                        dsty->y = (srcy->y * sd + dsty->y * d) / md;
                        dsty->cb = (srcy->cb * sd + dsty->cb * d) / md;
                        dsty->cr = (srcy->cr * sd + dsty->cr * d) / md;
                    } else {
                        x = -x;
                        dsty += x;
                        srcy += x;
                    }
                    dsty++;
                    srcy++;
                    sqd += x + 1;
                }
                dst += line;
                src1 += line;
                sqdp += y + 1;
            }
        }
    }
    void __cdecl SceneChange_t::scFanWipe(ExEdit::PixelYC* dst, int w, int h, ExEdit::PixelYC* src1, ExEdit::PixelYC* src2, int rate, int track1, int line, ExEdit::Filter* efp) {
        efp->exfunc->bufcpy(dst, 0, 0, src2, 0, 0, w, h, 0, 0);
        SceneChange_var sc = { dst, w, h, src1, src2, rate, track1, line };
        efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)scFanWipe_mt, efp, &sc);
    }
    void __cdecl SceneChange_t::scFanWipe_mt(int thread_id, int thread_num, ExEdit::Filter* efp, SceneChange_var* sc) {
        int cy = sc->h >> 1;
        int cx = sc->w >> 1;
        int y = thread_id;
        ExEdit::PixelYC* srcr[4] = { sc->src1 + y * sc->line, sc->src1 + sc->w - 1 + y * sc->line, sc->src1 + (sc->h - 1 - y) * sc->line, sc->src1 + (sc->h - 1 - y) * sc->line + sc->w - 1 };
        ExEdit::PixelYC* dstr[4] = { sc->dst + y * sc->line, sc->dst + sc->w - 1 + y * sc->line, sc->dst + (sc->h - 1 - y) * sc->line, sc->dst + (sc->h - 1 - y) * sc->line + sc->w - 1 };
        int line = sc->line * thread_num;
        int rate = sc->rate << 4;
        int track1 = -sc->track1 * 0x80 + 0x4000;
        __m256 step256, radcvt256;
        if (has_flag(get_CPUCmdSet(), CPUCmdSet::F_AVX2)) {
            step256 = _mm256_setr_ps(0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f);
            radcvt256 = _mm256_set1_ps(10430.37835047045f);
        }
        for (y -= cy; y < 0; y += thread_num) {
            int x = -cx;
            if (has_flag(get_CPUCmdSet(), CPUCmdSet::F_AVX2)) {
                __m256 y256 = _mm256_set1_ps((float)y);
                for (x = -cx; x < -7; x += 8) {
                    __m256 x256 = _mm256_add_ps(_mm256_set1_ps((float)x), step256);
                    __m256i angle256 = _mm256_cvttps_epi32(_mm256_mul_ps(_mm256_atan2_ps(y256, x256), radcvt256));
                    for (int j = 0; j < 8; j++) {
                        int angleofs = angle256.m256i_i32[j];
                        if (rate < ((track1 + angleofs) & 0xffff)) {
                            dstr[0]->y = srcr[0]->y;
                            dstr[0]->cb = srcr[0]->cb;
                            dstr[0]->cr = srcr[0]->cr;
                        }
                        if (rate < ((0x8000 + track1 - angleofs) & 0xffff)) {
                            dstr[1]->y = srcr[1]->y;
                            dstr[1]->cb = srcr[1]->cb;
                            dstr[1]->cr = srcr[1]->cr;
                        }
                        if (rate < ((track1 - angleofs) & 0xffff)) {
                            dstr[2]->y = srcr[2]->y;
                            dstr[2]->cb = srcr[2]->cb;
                            dstr[2]->cr = srcr[2]->cr;
                        }
                        if (rate < ((0x8000 + track1 + angleofs) & 0xffff)) {
                            dstr[3]->y = srcr[3]->y;
                            dstr[3]->cb = srcr[3]->cb;
                            dstr[3]->cr = srcr[3]->cr;
                        }
                        dstr[0]++; dstr[1]--;
                        dstr[2]++; dstr[3]--;
                        srcr[0]++; srcr[1]--;
                        srcr[2]++; srcr[3]--;
                    }
                }
            }
            for (; x < 0; x++) {
                float angle = atan2((float)y, (float)x);
                int angleofs = (int)(angle * 10430.37835047045f); // 10430.37835047045f == (32768 / math.PI)
                if (rate < ((track1 + angleofs) & 0xffff)) {
                    dstr[0]->y = srcr[0]->y;
                    dstr[0]->cb = srcr[0]->cb;
                    dstr[0]->cr = srcr[0]->cr;
                }
                if (rate < ((0x8000 + track1 - angleofs) & 0xffff)) {
                    dstr[1]->y = srcr[1]->y;
                    dstr[1]->cb = srcr[1]->cb;
                    dstr[1]->cr = srcr[1]->cr;
                }
                if (rate < ((track1 - angleofs) & 0xffff)) {
                    dstr[2]->y = srcr[2]->y;
                    dstr[2]->cb = srcr[2]->cb;
                    dstr[2]->cr = srcr[2]->cr;
                }
                if (rate < ((0x8000 + track1 + angleofs) & 0xffff)) {
                    dstr[3]->y = srcr[3]->y;
                    dstr[3]->cb = srcr[3]->cb;
                    dstr[3]->cr = srcr[3]->cr;
                }
                dstr[0]++; dstr[1]--;
                dstr[2]++; dstr[3]--;
                srcr[0]++; srcr[1]--;
                srcr[2]++; srcr[3]--;
            }
            if (sc->w & 1) {
                if (rate < ((track1 - 0x4000) & 0xffff)) {
                    dstr[0]->y = srcr[0]->y;
                    dstr[0]->cb = srcr[0]->cb;
                    dstr[0]->cr = srcr[0]->cr;
                }
                if (rate < ((track1 + 0x4000) & 0xffff)) {
                    dstr[3]->y = srcr[3]->y;
                    dstr[3]->cb = srcr[3]->cb;
                    dstr[3]->cr = srcr[3]->cr;
                }
            }
            dstr[0] += line - cx; dstr[1] += line + cx;
            dstr[2] -= line + cx; dstr[3] -= line - cx;
            srcr[0] += line - cx; srcr[1] += line + cx;
            srcr[2] -= line + cx; srcr[3] -= line - cx;
        }
        if (sc->h & 1) {
            if (rate < ((track1 + 0x8000) & 0xffff)) {
                int x = thread_id * cx / thread_num;
                memcpy(sc->dst + cy * sc->line + x,
                    sc->src1 + cy * sc->line + x,
                    ((thread_id + 1) * cx / thread_num - x) * sizeof(ExEdit::PixelYC));
            }
            if (rate < (track1 & 0xffff)) {
                int x = thread_id * cx / thread_num;
                memcpy(sc->dst + cy * sc->line + x + sc->w - cx,
                    sc->src1 + cy * sc->line + x + sc->w - cx,
                    ((thread_id + 1) * cx / thread_num - x) * sizeof(ExEdit::PixelYC));
            }
        }
    }


    void __cdecl SceneChange_t::scHorizontalWipe(ExEdit::PixelYC* dst, int w, int h, ExEdit::PixelYC* src1, ExEdit::PixelYC* src2, int rate, int track1, int line, ExEdit::Filter* efp) {
        int x = w * rate >> 12;
        efp->exfunc->bufcpy(dst, x, 0, src1, x, 0, w - x, h, 0, 0);
        efp->exfunc->bufcpy(dst, 0, 0, src2, 0, 0, x, h, 0, 0);

    }

    void __cdecl SceneChange_t::scVerticalWipe(ExEdit::PixelYC* dst, int w, int h, ExEdit::PixelYC* src1, ExEdit::PixelYC* src2, int rate, int track1, int line, ExEdit::Filter* efp) {
        int y = h * rate >> 12;
        efp->exfunc->bufcpy(dst, 0, y, src1, 0, y, w, h - y, 0, 0);
        efp->exfunc->bufcpy(dst, 0, 0, src2, 0, 0, w, y, 0, 0);
    }


    void __cdecl SceneChange_t::scRandomLine(ExEdit::PixelYC* dst, int w, int h, ExEdit::PixelYC* src1, ExEdit::PixelYC* src2, int rate, int track1, int line, ExEdit::Filter* efp) {
        if (0 <= track1) {
            reinterpret_cast<void(__cdecl*)(ExEdit::PixelYC*, int, int, ExEdit::PixelYC*, ExEdit::PixelYC*, int, int, int, ExEdit::Filter*)>(GLOBAL::exedit_base + 0x85f30)(dst, w, h, src1, src2, rate, track1, line, efp);
            return;
        }
        auto mem = *reinterpret_cast<int**>(GLOBAL::exedit_base + OFS::ExEdit::memory_ptr);
        for (int i = 0; i < w; i++) {
            mem[i] = i;
        }
        int r = 0x137bd137;
        for (int i = 0; i < w; i++) {
            int rr = (int)r % w;
            r = (int)r % 0x1c2d + r * 0x4a;
            r = r ^ rr << 7 ^ (int)r >> 2;
            std::swap(mem[i], mem[rr]);
        }
        SceneChange_var sc = { dst, w, h, src1, (ExEdit::PixelYC*)mem, w * rate >> 12, -track1, line };
        if (rate < 0x800) {
            sc.src1 = src2;
            efp->exfunc->bufcpy(dst, 0, 0, src1, 0, 0, w, h, 0, 0);
            efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)scRandomLine_v_mt1, efp, &sc);
        } else {
            efp->exfunc->bufcpy(dst, 0, 0, src2, 0, 0, w, h, 0, 0);
            efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)scRandomLine_v_mt2, efp, &sc);
        }
    }
    void __cdecl SceneChange_t::scRandomLine_v_mt1(int thread_id, int thread_num, ExEdit::Filter* efp, SceneChange_var* sc) {
        int w = sc->w / sc->track1;
        int x = thread_id * w / thread_num;
        auto src = sc->src1 + x * sc->track1;
        auto dst = sc->dst + x * sc->track1;
        auto mem = reinterpret_cast<int*>(sc->src2) + x;
        int size = 0;
        for (x = (thread_id + 1) * w / thread_num - x; 0 < x; x--) {
            if (sc->rate > *mem) {
                size += sc->track1;
            } else if (0 < size) {
                auto srct = src;
                auto dstt = dst;
                for (int y = sc->h; 0 < y; y--) {
                    memcpy(dstt, srct, size * sizeof(ExEdit::PixelYC));
                    dstt += sc->line;
                    srct += sc->line;
                }
                src += size + sc->track1;
                dst += size + sc->track1;
                size = 0;
            } else {
                src += sc->track1;
                dst += sc->track1;
            }
            mem++;
        }
        if (thread_id + 1 == thread_num) {
            if (sc->rate > *mem) {
                size += sc->w - w * sc->track1;
            }
        }
        if (0 < size) {
            for (int y = sc->h; 0 < y; y--) {
                memcpy(dst, src, size * sizeof(ExEdit::PixelYC));
                dst += sc->line;
                src += sc->line;
            }
        }
    }
    void __cdecl SceneChange_t::scRandomLine_v_mt2(int thread_id, int thread_num, ExEdit::Filter* efp, SceneChange_var* sc) {
        int w = sc->w / sc->track1;
        int x = thread_id * w / thread_num;
        auto src = sc->src1 + x * sc->track1;
        auto dst = sc->dst + x * sc->track1;
        auto mem = reinterpret_cast<int*>(sc->src2) + x;
        int size = 0;
        for (x = (thread_id + 1) * w / thread_num - x; 0 < x; x--) {
            if (sc->rate <= *mem) {
                size += sc->track1;
            } else if (0 < size) {
                auto srct = src;
                auto dstt = dst;
                for (int y = sc->h; 0 < y; y--) {
                    memcpy(dstt, srct, size * sizeof(ExEdit::PixelYC));
                    dstt += sc->line;
                    srct += sc->line;
                }
                src += size + sc->track1;
                dst += size + sc->track1;
                size = 0;
            } else {
                src += sc->track1;
                dst += sc->track1;
            }
            mem++;
        }
        if (thread_id + 1 == thread_num) {
            if (sc->rate <= *mem) {
                size += sc->w - w * sc->track1;
            }
        }
        if (0 < size) {
            for (int y = sc->h; 0 < y; y--) {
                memcpy(dst, src, size * sizeof(ExEdit::PixelYC));
                dst += sc->line;
                src += sc->line;
            }
        }
    }

} // namespace patch::fast
#endif // ifdef PATCH_SWITCH_FAST_SCENECHANGE
