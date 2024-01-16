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
#include "patch_fast_diffuselight.hpp"
#ifdef PATCH_SWITCH_FAST_DIFFUSELIGHT

#include <numbers>

#include "global.hpp"
#include "offset_address.hpp"
#include "util_int.hpp"


namespace patch::fast {

    struct PixelYCA_int {
        int32_t y;
        int32_t cb;
        int32_t cr;
        int32_t a;
    };
    struct PixelYC_int {
        int32_t y;
        int32_t cb;
        int32_t cr;
    };


    void __cdecl DiffuseLight_t::yca_mt(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) { // 1c710
        auto dl = reinterpret_cast<efDiffuseLight_var*>(GLOBAL::exedit_base + OFS::ExEdit::efDiffuseLight_var_ptr);
        int x = thread_id * efpip->obj_w / thread_num;
        auto dst0 = (ExEdit::PixelYCA*)efpip->obj_temp + dl->range + x;
        auto src01 = (ExEdit::PixelYCA*)efpip->obj_edit + x;
        auto src02 = src01;
        auto cnv0 = (PixelYCA_int*)((ExEdit::PixelYCA*)efpip->obj_temp + dl->range) - efpip->obj_w + x;
        int w = (thread_id + 1) * efpip->obj_w / thread_num - x;
        memset(cnv0, 0, w * sizeof(PixelYCA_int));

        int loop[4];
        if (dl->range <= efpip->obj_h) {
            loop[0] = dl->range;
            loop[1] = efpip->obj_h - dl->range;
            loop[2] = 0;
            loop[3] = dl->range - 1;
        } else {
            loop[0] = efpip->obj_h;
            loop[1] = 0;
            loop[2] = dl->range - efpip->obj_h;
            loop[3] = efpip->obj_h - 1;
        }

        for (int y = loop[0]; 0 < y; y--) {
            auto src1 = src01;
            src01 += efpip->obj_line;
            auto dst = dst0;
            dst0 += efpip->obj_line;
            auto cnv = cnv0;
            for (x = w; 0 < x; x--) {
                int a = src1->a;
                if (0 < a) {
                    if (a < 0x1000) {
                        cnv->y += src1->y * a >> 12;
                        cnv->cb += src1->cb * a >> 12;
                        cnv->cr += src1->cr * a >> 12;
                    } else {
                        cnv->y += src1->y;
                        cnv->cb += src1->cb;
                        cnv->cr += src1->cr;
                    }
                    cnv->a += a;
                }
                if (0 < cnv->a) {
                    double ia = 4096.0 / (double)cnv->a;
                    dst->y = (short)((double)cnv->y * ia);
                    dst->cb = (short)((double)cnv->cb * ia);
                    dst->cr = (short)((double)cnv->cr * ia);
                }
                dst->a = cnv->a / dl->range;
                dst++;
                cnv++;
                src1++;
            }
        }
        for (int y = loop[1]; 0 < y; y--) {
            auto src1 = src01;
            src01 += efpip->obj_line;
            auto src2 = src02;
            src02 += efpip->obj_line;
            auto dst = dst0;
            dst0 += efpip->obj_line;
            auto cnv = cnv0;
            for (x = w; 0 < x; x--) {
                int a = src1->a;
                if (0 < a) {
                    if (a < 0x1000) {
                        cnv->y += src1->y * a >> 12;
                        cnv->cb += src1->cb * a >> 12;
                        cnv->cr += src1->cr * a >> 12;
                    } else {
                        cnv->y += src1->y;
                        cnv->cb += src1->cb;
                        cnv->cr += src1->cr;
                    }
                    cnv->a += a;
                }
                a = src2->a;
                if (0 < a) {
                    if (a < 0x1000) {
                        cnv->y -= src2->y * a >> 12;
                        cnv->cb -= src2->cb * a >> 12;
                        cnv->cr -= src2->cr * a >> 12;
                    } else {
                        cnv->y -= src2->y;
                        cnv->cb -= src2->cb;
                        cnv->cr -= src2->cr;
                    }
                    cnv->a -= a;
                }
                if (0 < cnv->a) {
                    double ia = 4096.0 / (double)cnv->a;
                    dst->y = (short)((double)cnv->y * ia);
                    dst->cb = (short)((double)cnv->cb * ia);
                    dst->cr = (short)((double)cnv->cr * ia);
                }
                dst->a = cnv->a / dl->range;
                dst++;
                cnv++;
                src1++;
                src2++;
            }
        }
        for (int y = loop[2]; 0 < y; y--) {
            memcpy(dst0, dst0 - efpip->obj_line, w * sizeof(ExEdit::PixelYCA));
            dst0 += efpip->obj_line;
        }
        for (int y = loop[3]; 0 < y; y--) {
            auto src2 = src02;
            src02 += efpip->obj_line;
            auto dst = dst0;
            dst0 += efpip->obj_line;
            auto cnv = cnv0;
            for (x = w; 0 < x; x--) {
                int a = src2->a;
                if (0 < a) {
                    if (a < 0x1000) {
                        cnv->y -= src2->y * a >> 12;
                        cnv->cb -= src2->cb * a >> 12;
                        cnv->cr -= src2->cr * a >> 12;
                    } else {
                        cnv->y -= src2->y;
                        cnv->cb -= src2->cb;
                        cnv->cr -= src2->cr;
                    }
                    cnv->a -= a;
                }
                if (0 < cnv->a) {
                    double ia = 4096.0 / (double)cnv->a;
                    dst->y = (short)((double)cnv->y * ia);
                    dst->cb = (short)((double)cnv->cb * ia);
                    dst->cr = (short)((double)cnv->cr * ia);
                }
                dst->a = cnv->a / dl->range;
                dst++;
                cnv++;
                src2++;
            }
        }
    }

    void __cdecl DiffuseLight_t::yca_plus_mt(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) { // 1cb10
        auto dl = reinterpret_cast<efDiffuseLight_var*>(GLOBAL::exedit_base + OFS::ExEdit::efDiffuseLight_var_ptr);
        int wstep = (thread_num * efpip->obj_line - efpip->obj_w) * sizeof(ExEdit::PixelYCA);
        auto dst = (ExEdit::PixelYCA*)efpip->obj_temp + thread_id * efpip->obj_line;
        int wrstep = (thread_num * efpip->obj_line - (efpip->obj_w + dl->range - 1)) * sizeof(ExEdit::PixelYCA);

        int loop[12];
        if (dl->range < efpip->obj_w) {
            loop[0] = dl->range;
            loop[1] = efpip->obj_w - dl->range;
            loop[2] = 0;
            loop[3] = dl->range - 1;

            loop[4] = dl->size;
            loop[5] = 0;
            loop[6] = dl->size + 1;
            loop[7] = efpip->obj_w - dl->range;
            loop[8] = 0;
            loop[9] = dl->size;
            loop[10] = 0;
            loop[11] = dl->size;
        } else {
            loop[0] = efpip->obj_w;
            loop[1] = 0;
            loop[2] = dl->range - efpip->obj_w;
            loop[3] = efpip->obj_w - 1;
            loop[7] = 0;
            if (dl->size < efpip->obj_w) { // b+ b b+a b+-a ba b-a b b-
                loop[4] = dl->size;
                loop[5] = 0;
                loop[6] = efpip->obj_w - dl->size;
                loop[8] = dl->range - efpip->obj_w;
                loop[9] = efpip->obj_w - dl->size - 1;
                loop[10] = 0;
                loop[11] = dl->size;
            } else {
                loop[4] = efpip->obj_w;
                loop[5] = dl->size - efpip->obj_w;
                loop[6] = 0;
                loop[8] = efpip->obj_w;
                loop[9] = 0;
                loop[10] = dl->size - efpip->obj_w + 1;
                loop[11] = efpip->obj_w - 1;
            }
        }

        int yend[4] = { dl->size, efpip->obj_h - dl->size, efpip->obj_h, 0 };
        int* yep = yend;
        int y = thread_id;
        for (int i = 2; 0 < i; i--) {
            for (; y < *yep; y += thread_num) {
                auto src1 = dst + dl->range;
                int cnv_y = 0;
                int cnv_cb = 0;
                int cnv_cr = 0;
                int cnv_a = 0;
                for (int x = loop[0]; 0 < x; x--) {
                    int a = src1->a;
                    if (0 < a) {
                        if (a < 0x1000) {
                            cnv_y += src1->y * a >> 12;
                            cnv_cb += src1->cb * a >> 12;
                            cnv_cr += src1->cr * a >> 12;
                        } else {
                            cnv_y += src1->y;
                            cnv_cb += src1->cb;
                            cnv_cr += src1->cr;
                        }
                        cnv_a += a;
                    }
                    if (dl->range <= cnv_a) {
                        double ia = 4096.0 / (double)cnv_a;
                        dst->y = (short)((double)cnv_y * ia);
                        dst->cb = (short)((double)cnv_cb * ia);
                        dst->cr = (short)((double)cnv_cr * ia);
                        dst->a = cnv_a / dl->range * dl->intensity >> 12;
                    } else {
                        memset(dst, 0, sizeof(ExEdit::PixelYCA));
                    }
                    dst++;
                    src1++;
                }
                for (int x = loop[1]; 0 < x; x--) {
                    int a = src1->a;
                    if (0 < a) {
                        if (a < 0x1000) {
                            cnv_y += src1->y * a >> 12;
                            cnv_cb += src1->cb * a >> 12;
                            cnv_cr += src1->cr * a >> 12;
                        } else {
                            cnv_y += src1->y;
                            cnv_cb += src1->cb;
                            cnv_cr += src1->cr;
                        }
                        cnv_a += a;
                    }
                    a = dst->a;
                    if (0 < a) {
                        if (a < 0x1000) {
                            cnv_y -= dst->y * a >> 12;
                            cnv_cb -= dst->cb * a >> 12;
                            cnv_cr -= dst->cr * a >> 12;
                        } else {
                            cnv_y -= dst->y;
                            cnv_cb -= dst->cb;
                            cnv_cr -= dst->cr;
                        }
                        cnv_a -= a;
                    }
                    if (dl->range <= cnv_a) {
                        double ia = 4096.0 / (double)cnv_a;
                        dst->y = (short)((double)cnv_y * ia);
                        dst->cb = (short)((double)cnv_cb * ia);
                        dst->cr = (short)((double)cnv_cr * ia);
                        dst->a = cnv_a / dl->range * dl->intensity >> 12;
                    } else {
                        memset(dst, 0, sizeof(ExEdit::PixelYCA));
                    }
                    dst++;
                    src1++;
                }

                if (0 < loop[2]) {
                    int x = loop[2];
                    auto pix = *(dst - 1);
                    for (; 0 < x; x--) {
                        *dst = pix;
                        dst++;
                    }
                }
                for (int x = loop[3]; 0 < x; x--) {
                    int a = dst->a;
                    if (0 < a) {
                        if (a < 0x1000) {
                            cnv_y -= dst->y * a >> 12;
                            cnv_cb -= dst->cb * a >> 12;
                            cnv_cr -= dst->cr * a >> 12;
                        } else {
                            cnv_y -= dst->y;
                            cnv_cb -= dst->cb;
                            cnv_cr -= dst->cr;
                        }
                        cnv_a -= a;
                    }
                    if (dl->range <= cnv_a) {
                        double ia = 4096.0 / (double)cnv_a;
                        dst->y = (short)((double)cnv_y * ia);
                        dst->cb = (short)((double)cnv_cb * ia);
                        dst->cr = (short)((double)cnv_cr * ia);
                        dst->a = cnv_a / dl->range * dl->intensity >> 12;
                    } else {
                        memset(dst, 0, sizeof(ExEdit::PixelYCA));
                    }
                    dst++;
                }
                dst = (ExEdit::PixelYCA*)((int)dst + wrstep);
            }
            auto src = (ExEdit::PixelYCA*)efpip->obj_edit + (y - *yep) * efpip->obj_line;
            yep++;
            for (; y < *yep; y += thread_num) {
                auto src1 = dst + dl->range;
                int cnv_y = 0;
                int cnv_cb = 0;
                int cnv_cr = 0;
                int cnv_a = 0;
                for (int x = loop[4]; 0 < x; x--) {
                    int a = src1->a;
                    if (0 < a) {
                        if (a < 0x1000) {
                            cnv_y += src1->y * a >> 12;
                            cnv_cb += src1->cb * a >> 12;
                            cnv_cr += src1->cr * a >> 12;
                        } else {
                            cnv_y += src1->y;
                            cnv_cb += src1->cb;
                            cnv_cr += src1->cr;
                        }
                        cnv_a += a;
                    }
                    if (dl->range <= cnv_a) {
                        double ia = 4096.0 / (double)cnv_a;
                        dst->y = (short)((double)cnv_y * ia);
                        dst->cb = (short)((double)cnv_cb * ia);
                        dst->cr = (short)((double)cnv_cr * ia);
                        dst->a = cnv_a / dl->range * dl->intensity >> 12;
                    } else {
                        memset(dst, 0, sizeof(ExEdit::PixelYCA));
                    }
                    dst++;
                    src1++;
                }
                if (0 < loop[5]) {
                    int x = loop[5];
                    auto pix = *(dst - 1);
                    for (; 0 < x; x--) {
                        *dst = pix;
                        dst++;
                    }
                }
                for (int x = loop[6]; 0 < x; x--) {
                    int a = src1->a;
                    if (0 < a) {
                        if (a < 0x1000) {
                            cnv_y += src1->y * a >> 12;
                            cnv_cb += src1->cb * a >> 12;
                            cnv_cr += src1->cr * a >> 12;
                        } else {
                            cnv_y += src1->y;
                            cnv_cb += src1->cb;
                            cnv_cr += src1->cr;
                        }
                        cnv_a += a;
                    }
                    a = cnv_a / dl->range * dl->intensity >> 12;
                    if (0 < a) {
                        double ia = 4096.0 / (double)cnv_a;
                        int dif_y = (int)((double)cnv_y * ia);
                        int y1 = max(0, src->y * src->a >> 12);
                        int y2 = dif_y - y1;
                        if (0 < y2) {
                            int dif_cb = ((src->cb * src->a >> 12) * y1 + (int)((double)cnv_cb * ia) * y2) / dif_y;
                            int dif_cr = ((src->cr * src->a >> 12) * y1 + (int)((double)cnv_cr * ia) * y2) / dif_y;
                            if (0x1000 <= a) {
                                dst->y = dif_y;
                                dst->cb = dif_cb;
                                dst->cr = dif_cr;
                                dst->a = 0x1000;
                            } else if (0x1000 <= src->a) {
                                dst->y = ((dif_y - src->y) * a >> 12) + src->y;
                                dst->cb = ((dif_cb - src->cb) * a >> 12) + src->cb;
                                dst->cr = ((dif_cr - src->cr) * a >> 12) + src->cr;
                                dst->a = 0x1000;
                            } else if (src->a <= 0) {
                                dst->y = dif_y;
                                dst->cb = dif_cb;
                                dst->cr = dif_cr;
                                dst->a = a;
                            } else {
                                int new_a = (0x1000800 - (0x1000 - src->a) * (0x1000 - a)) >> 12;
                                int a1 = (0x1000 - a) * src->a / new_a;
                                int a2 = (a << 12) / new_a;
                                dst->y = (src->y * a1 + dif_y * a2) >> 12;
                                dst->cb = (src->cb * a1 + dif_cb * a2) >> 12;
                                dst->cr = (src->cr * a1 + dif_cr * a2) >> 12;
                                dst->a = new_a;
                            }
                        } else {
                            *dst = *src;
                        }
                    } else {
                        *dst = *src;
                    }
                    dst++;
                    src1++;
                    src++;
                }
                for (int x = loop[7]; 0 < x; x--) {
                    int a = src1->a;
                    if (0 < a) {
                        if (a < 0x1000) {
                            cnv_y += src1->y * a >> 12;
                            cnv_cb += src1->cb * a >> 12;
                            cnv_cr += src1->cr * a >> 12;
                        } else {
                            cnv_y += src1->y;
                            cnv_cb += src1->cb;
                            cnv_cr += src1->cr;
                        }
                        cnv_a += a;
                    }
                    a = dst->a;
                    if (0 < a) {
                        if (a < 0x1000) {
                            cnv_y -= dst->y * a >> 12;
                            cnv_cb -= dst->cb * a >> 12;
                            cnv_cr -= dst->cr * a >> 12;
                        } else {
                            cnv_y -= dst->y;
                            cnv_cb -= dst->cb;
                            cnv_cr -= dst->cr;
                        }
                        cnv_a -= a;
                    }
                    a = cnv_a / dl->range * dl->intensity >> 12;
                    if (0 < a) {
                        double ia = 4096.0 / (double)cnv_a;
                        int dif_y = (int)((double)cnv_y * ia);
                        int y1 = max(0, src->y * src->a >> 12);
                        int y2 = dif_y - y1;
                        if (0 < y2) {
                            int dif_cb = ((src->cb * src->a >> 12) * y1 + (int)((double)cnv_cb * ia) * y2) / dif_y;
                            int dif_cr = ((src->cr * src->a >> 12) * y1 + (int)((double)cnv_cr * ia) * y2) / dif_y;
                            if (0x1000 <= a) {
                                dst->y = dif_y;
                                dst->cb = dif_cb;
                                dst->cr = dif_cr;
                                dst->a = 0x1000;
                            } else if (0x1000 <= src->a) {
                                dst->y = ((dif_y - src->y) * a >> 12) + src->y;
                                dst->cb = ((dif_cb - src->cb) * a >> 12) + src->cb;
                                dst->cr = ((dif_cr - src->cr) * a >> 12) + src->cr;
                                dst->a = 0x1000;
                            } else if (src->a <= 0) {
                                dst->y = dif_y;
                                dst->cb = dif_cb;
                                dst->cr = dif_cr;
                                dst->a = a;
                            } else {
                                int new_a = (0x1000800 - (0x1000 - src->a) * (0x1000 - a)) >> 12;
                                int a1 = (0x1000 - a) * src->a / new_a;
                                int a2 = (a << 12) / new_a;
                                dst->y = (src->y * a1 + dif_y * a2) >> 12;
                                dst->cb = (src->cb * a1 + dif_cb * a2) >> 12;
                                dst->cr = (src->cr * a1 + dif_cr * a2) >> 12;
                                dst->a = new_a;
                            }
                        } else {
                            *dst = *src;
                        }
                    } else {
                        *dst = *src;
                    }
                    dst++;
                    src1++;
                    src++;
                }
                if (0 < loop[8]) {
                    int x = loop[8];
                    int a = cnv_a / dl->range * dl->intensity >> 12;
                    if (0 < a) {
                        double ia = 4096.0 / (double)cnv_a;
                        int dif_y = (int)((double)cnv_y * ia);
                        if (0 < dif_y) {
                            int cb2 = (int)((double)cnv_cb * ia);
                            int cr2 = (int)((double)cnv_cr * ia);
                            for (; 0 < x; x--) {
                                int y1 = max(0, src->y * src->a >> 12);
                                int y2 = dif_y - y1;
                                if (0 < y2) {
                                    int dif_cb = ((src->cb * src->a >> 12) * y1 + cb2 * y2) / dif_y;
                                    int dif_cr = ((src->cr * src->a >> 12) * y1 + cr2 * y2) / dif_y;
                                    if (0x1000 <= a) {
                                        dst->y = dif_y;
                                        dst->cb = dif_cb;
                                        dst->cr = dif_cr;
                                        dst->a = 0x1000;
                                    } else if (0x1000 <= src->a) {
                                        dst->y = ((dif_y - src->y) * a >> 12) + src->y;
                                        dst->cb = ((dif_cb - src->cb) * a >> 12) + src->cb;
                                        dst->cr = ((dif_cr - src->cr) * a >> 12) + src->cr;
                                        dst->a = 0x1000;
                                    } else if (src->a <= 0) {
                                        dst->y = dif_y;
                                        dst->cb = dif_cb;
                                        dst->cr = dif_cr;
                                        dst->a = a;
                                    } else {
                                        int new_a = (0x1000800 - (0x1000 - src->a) * (0x1000 - a)) >> 12;
                                        int a1 = (0x1000 - a) * src->a / new_a;
                                        int a2 = (a << 12) / new_a;
                                        dst->y = (src->y * a1 + dif_y * a2) >> 12;
                                        dst->cb = (src->cb * a1 + dif_cb * a2) >> 12;
                                        dst->cr = (src->cr * a1 + dif_cr * a2) >> 12;
                                        dst->a = new_a;
                                    }
                                } else {
                                    *dst = *src;
                                }
                                dst++;
                                src++;
                            }
                        } else {
                            memcpy(dst, src, x * sizeof(ExEdit::PixelYCA));
                            dst += x;
                            src += x;
                        }
                    } else {
                        memcpy(dst, src, x * sizeof(ExEdit::PixelYCA));
                        dst += x;
                        src += x;
                    }
                }
                for (int x = loop[9]; 0 < x; x--) {
                    int a = dst->a;
                    if (0 < a) {
                        if (a < 0x1000) {
                            cnv_y -= dst->y * a >> 12;
                            cnv_cb -= dst->cb * a >> 12;
                            cnv_cr -= dst->cr * a >> 12;
                        } else {
                            cnv_y -= dst->y;
                            cnv_cb -= dst->cb;
                            cnv_cr -= dst->cr;
                        }
                        cnv_a -= a;
                    }
                    a = cnv_a / dl->range * dl->intensity >> 12;
                    if (0 < a) {
                        double ia = 4096.0 / (double)cnv_a;
                        int dif_y = (int)((double)cnv_y * ia);
                        int y1 = max(0, src->y * src->a >> 12);
                        int y2 = dif_y - y1;
                        if (0 < y2) {
                            int dif_cb = ((src->cb * src->a >> 12) * y1 + (int)((double)cnv_cb * ia) * y2) / dif_y;
                            int dif_cr = ((src->cr * src->a >> 12) * y1 + (int)((double)cnv_cr * ia) * y2) / dif_y;
                            if (0x1000 <= a) {
                                dst->y = dif_y;
                                dst->cb = dif_cb;
                                dst->cr = dif_cr;
                                dst->a = 0x1000;
                            } else if (0x1000 <= src->a) {
                                dst->y = ((dif_y - src->y) * a >> 12) + src->y;
                                dst->cb = ((dif_cb - src->cb) * a >> 12) + src->cb;
                                dst->cr = ((dif_cr - src->cr) * a >> 12) + src->cr;
                                dst->a = 0x1000;
                            } else if (src->a <= 0) {
                                dst->y = dif_y;
                                dst->cb = dif_cb;
                                dst->cr = dif_cr;
                                dst->a = a;
                            } else {
                                int new_a = (0x1000800 - (0x1000 - src->a) * (0x1000 - a)) >> 12;
                                int a1 = (0x1000 - a) * src->a / new_a;
                                int a2 = (a << 12) / new_a;
                                dst->y = (src->y * a1 + dif_y * a2) >> 12;
                                dst->cb = (src->cb * a1 + dif_cb * a2) >> 12;
                                dst->cr = (src->cr * a1 + dif_cr * a2) >> 12;
                                dst->a = new_a;
                            }
                        } else {
                            *dst = *src;
                        }
                    } else {
                        *dst = *src;
                    }
                    dst++;
                    src++;
                }
                if (0 < loop[10]) {
                    int x = loop[10];
                    ExEdit::PixelYCA pix;
                    if (dl->range <= cnv_a) {
                        double ia = 4096.0 / (double)cnv_a;
                        pix.y = (short)((double)cnv_y * ia);
                        pix.cb = (short)((double)cnv_cb * ia);
                        pix.cr = (short)((double)cnv_cr * ia);
                        pix.a = cnv_a / dl->range * dl->intensity >> 12;
                    } else {
                        memset(&pix, 0, sizeof(ExEdit::PixelYCA));
                    }
                    for (; 0 < x; x--) {
                        *dst = pix;
                        dst++;
                    }
                }
                for (int x = loop[11]; 0 < x; x--) {
                    int a = dst->a;
                    if (0 < a) {
                        if (a < 0x1000) {
                            cnv_y -= dst->y * a >> 12;
                            cnv_cb -= dst->cb * a >> 12;
                            cnv_cr -= dst->cr * a >> 12;
                        } else {
                            cnv_y -= dst->y;
                            cnv_cb -= dst->cb;
                            cnv_cr -= dst->cr;
                        }
                        cnv_a -= a;
                    }
                    if (dl->range <= cnv_a) {
                        double ia = 4096.0 / (double)cnv_a;
                        dst->y = (short)((double)cnv_y * ia);
                        dst->cb = (short)((double)cnv_cb * ia);
                        dst->cr = (short)((double)cnv_cr * ia);
                        dst->a = cnv_a / dl->range * dl->intensity >> 12;
                    } else {
                        memset(dst, 0, sizeof(ExEdit::PixelYCA));
                    }
                    dst++;
                }
                src = (ExEdit::PixelYCA*)((int)src + wstep);
                dst = (ExEdit::PixelYCA*)((int)dst + wrstep);
            }
            yep++;
        }
    }



    void __cdecl DiffuseLight_t::yca_cs_mt(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) { // 1d710
        auto dl = reinterpret_cast<efDiffuseLight_var*>(GLOBAL::exedit_base + OFS::ExEdit::efDiffuseLight_var_ptr);
        int x = thread_id * efpip->obj_w / thread_num;
        auto dst0 = (ExEdit::PixelYCA*)efpip->obj_temp + x;
        auto src01 = (ExEdit::PixelYCA*)efpip->obj_edit + x;
        auto src02 = src01;
        auto cnv0 = (PixelYCA_int*)efpip->obj_temp - efpip->obj_w + x;
        int w = (thread_id + 1) * efpip->obj_w / thread_num - x;
        memset(cnv0, 0, w * sizeof(PixelYCA_int));

        int loop[5];
        if (dl->range <= efpip->obj_h) {
            loop[0] = dl->size;
            loop[1] = dl->size + 1;
            loop[2] = efpip->obj_h - dl->range;
            loop[3] = 0;
            loop[4] = dl->size;
        } else {
            int size = min(dl->size, efpip->obj_h - 1);
            loop[0] = size;
            loop[1] = efpip->obj_h - size;
            loop[2] = 0;
            loop[3] = size * 2 + 1 - efpip->obj_h;
            loop[4] = efpip->obj_h - size - 1;
        }

        for (int y = loop[0]; 0 < y; y--) {
            auto src1 = src01;
            src01 += efpip->obj_line;
            auto cnv = cnv0;
            for (x = w; 0 < x; x--) {
                int a = src1->a;
                if (0 < a) {
                    if (a < 0x1000) {
                        cnv->y += src1->y * a >> 12;
                        cnv->cb += src1->cb * a >> 12;
                        cnv->cr += src1->cr * a >> 12;
                    } else {
                        cnv->y += src1->y;
                        cnv->cb += src1->cb;
                        cnv->cr += src1->cr;
                    }
                    cnv->a += a;
                }
                cnv++;
                src1++;
            }
        }
        int n = loop[0];
        for (int y = loop[1]; 0 < y; y--) {
            auto src1 = src01;
            src01 += efpip->obj_line;
            auto dst = dst0;
            dst0 += efpip->obj_w;
            auto cnv = cnv0;
            n++;
            for (x = w; 0 < x; x--) {
                int a = src1->a;
                if (0 < a) {
                    if (a < 0x1000) {
                        cnv->y += src1->y * a >> 12;
                        cnv->cb += src1->cb * a >> 12;
                        cnv->cr += src1->cr * a >> 12;
                    } else {
                        cnv->y += src1->y;
                        cnv->cb += src1->cb;
                        cnv->cr += src1->cr;
                    }
                    cnv->a += a;
                }
                if (0 < cnv->a) {
                    double ia = 4096.0 / (double)cnv->a;
                    dst->y = (short)((double)cnv->y * ia);
                    dst->cb = (short)((double)cnv->cb * ia);
                    dst->cr = (short)((double)cnv->cr * ia);
                }
                dst->a = cnv->a / n;
                dst++;
                cnv++;
                src1++;
            }
        }
        for (int y = loop[2]; 0 < y; y--) {
            auto src1 = src01;
            src01 += efpip->obj_line;
            auto src2 = src02;
            src02 += efpip->obj_line;
            auto dst = dst0;
            dst0 += efpip->obj_w;
            auto cnv = cnv0;
            for (x = w; 0 < x; x--) {
                int a = src1->a;
                if (0 < a) {
                    if (a < 0x1000) {
                        cnv->y += src1->y * a >> 12;
                        cnv->cb += src1->cb * a >> 12;
                        cnv->cr += src1->cr * a >> 12;
                    } else {
                        cnv->y += src1->y;
                        cnv->cb += src1->cb;
                        cnv->cr += src1->cr;
                    }
                    cnv->a += a;
                }
                a = src2->a;
                if (0 < a) {
                    if (a < 0x1000) {
                        cnv->y -= src2->y * a >> 12;
                        cnv->cb -= src2->cb * a >> 12;
                        cnv->cr -= src2->cr * a >> 12;
                    } else {
                        cnv->y -= src2->y;
                        cnv->cb -= src2->cb;
                        cnv->cr -= src2->cr;
                    }
                    cnv->a -= a;
                }
                if (0 < cnv->a) {
                    double ia = 4096.0 / (double)cnv->a;
                    dst->y = (short)((double)cnv->y * ia);
                    dst->cb = (short)((double)cnv->cb * ia);
                    dst->cr = (short)((double)cnv->cr * ia);
                }
                dst->a = cnv->a / n;
                dst++;
                cnv++;
                src1++;
                src2++;
            }
        }
        for (int y = loop[3]; 0 < y; y--) {
            memcpy(dst0, dst0 - efpip->obj_w, w * sizeof(ExEdit::PixelYCA));
            dst0 += efpip->obj_w;
        }
        for (int y = loop[4]; 0 < y; y--) {
            auto src2 = src02;
            src02 += efpip->obj_line;
            auto dst = dst0;
            dst0 += efpip->obj_w;
            auto cnv = cnv0;
            n--;
            for (x = w; 0 < x; x--) {
                int a = src2->a;
                if (0 < a) {
                    if (a < 0x1000) {
                        cnv->y -= src2->y * a >> 12;
                        cnv->cb -= src2->cb * a >> 12;
                        cnv->cr -= src2->cr * a >> 12;
                    } else {
                        cnv->y -= src2->y;
                        cnv->cb -= src2->cb;
                        cnv->cr -= src2->cr;
                    }
                    cnv->a -= a;
                }
                if (0 < cnv->a) {
                    double ia = 4096.0 / (double)cnv->a;
                    dst->y = (short)((double)cnv->y * ia);
                    dst->cb = (short)((double)cnv->cb * ia);
                    dst->cr = (short)((double)cnv->cr * ia);
                }
                dst->a = cnv->a / n;
                dst++;
                cnv++;
                src2++;
            }
        }
    }


    void __cdecl DiffuseLight_t::yca_cs_plus_mt(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) { // 1dbd0
        auto dl = reinterpret_cast<efDiffuseLight_var*>(GLOBAL::exedit_base + OFS::ExEdit::efDiffuseLight_var_ptr);
        int y = thread_id * efpip->obj_h / thread_num;
        auto src1 = (ExEdit::PixelYCA*)efpip->obj_temp + y * efpip->obj_w;
        auto dst0 = (ExEdit::PixelYCA*)efpip->obj_edit + y * efpip->obj_line;

        int loop[5];
        if (dl->range <= efpip->obj_w) {
            loop[0] = dl->size;
            loop[1] = dl->size + 1;
            loop[2] = efpip->obj_w - dl->range;
            loop[3] = 0;
            loop[4] = dl->size;
        } else {
            int size = min(dl->size, efpip->obj_w - 1);
            loop[0] = size;
            loop[1] = efpip->obj_w - size;
            loop[2] = 0;
            loop[3] = size * 2 + 1 - efpip->obj_w;
            loop[4] = efpip->obj_w - size - 1;
        }

        for (y = (thread_id + 1) * efpip->obj_h / thread_num - y; 0 < y; y--) {
            auto src2 = src1;
            auto dst = dst0;
            dst0 += efpip->obj_line;
            int cnv_y = 0;
            int cnv_cb = 0;
            int cnv_cr = 0;
            int cnv_a = 0;
            for (int x = loop[0]; 0 < x; x--) {
                int a = src1->a;
                if (0 < a) {
                    if (a < 0x1000) {
                        cnv_y += src1->y * a >> 12;
                        cnv_cb += src1->cb * a >> 12;
                        cnv_cr += src1->cr * a >> 12;
                    } else {
                        cnv_y += src1->y;
                        cnv_cb += src1->cb;
                        cnv_cr += src1->cr;
                    }
                    cnv_a += a;
                }
                src1++;
            }
            int n = loop[0];
            for (int x = loop[1]; 0 < x; x--) {
                int a = src1->a;
                if (0 < a) {
                    if (a < 0x1000) {
                        cnv_y += src1->y * a >> 12;
                        cnv_cb += src1->cb * a >> 12;
                        cnv_cr += src1->cr * a >> 12;
                    } else {
                        cnv_y += src1->y;
                        cnv_cb += src1->cb;
                        cnv_cr += src1->cr;
                    }
                    cnv_a += a;
                }
                n++;
                a = cnv_a / n * dl->intensity >> 12;
                if (0 < a) {
                    double ia = 4096.0 / (double)cnv_a;
                    int dif_y = (int)((double)cnv_y * ia);
                    int y1 = max(0, dst->y * dst->a >> 12);
                    int y2 = dif_y - y1;
                    if (0 < y2) {
                        int dif_cb = ((dst->cb * dst->a >> 12) * y1 + (int)((double)cnv_cb * ia) * y2) / dif_y;
                        int dif_cr = ((dst->cr * dst->a >> 12) * y1 + (int)((double)cnv_cr * ia) * y2) / dif_y;
                        if (0x1000 <= a) {
                            dst->y = dif_y;
                            dst->cb = dif_cb;
                            dst->cr = dif_cr;
                            dst->a = 0x1000;
                        } else if (0x1000 <= dst->a) {
                            dst->y += (dif_y - dst->y) * a >> 12;
                            dst->cb += (dif_cb - dst->cb) * a >> 12;
                            dst->cr += (dif_cr - dst->cr) * a >> 12;
                            dst->a = 0x1000;
                        } else if (dst->a <= 0) {
                            dst->y = dif_y;
                            dst->cb = dif_cb;
                            dst->cr = dif_cr;
                            dst->a = a;
                        } else {
                            int new_a = (0x1000800 - (0x1000 - dst->a) * (0x1000 - a)) >> 12;
                            int a1 = (0x1000 - a) * dst->a / new_a;
                            int a2 = (a << 12) / new_a;
                            dst->y = (dst->y * a1 + dif_y * a2) >> 12;
                            dst->cb = (dst->cb * a1 + dif_cb * a2) >> 12;
                            dst->cr = (dst->cr * a1 + dif_cr * a2) >> 12;
                            dst->a = new_a;
                        }
                    }
                }
                dst++;
                src1++;
            }
            for (int x = loop[2]; 0 < x; x--) {
                int a = src1->a;
                if (0 < a) {
                    if (a < 0x1000) {
                        cnv_y += src1->y * a >> 12;
                        cnv_cb += src1->cb * a >> 12;
                        cnv_cr += src1->cr * a >> 12;
                    } else {
                        cnv_y += src1->y;
                        cnv_cb += src1->cb;
                        cnv_cr += src1->cr;
                    }
                    cnv_a += a;
                }
                a = src2->a;
                if (0 < a) {
                    if (a < 0x1000) {
                        cnv_y -= src2->y * a >> 12;
                        cnv_cb -= src2->cb * a >> 12;
                        cnv_cr -= src2->cr * a >> 12;
                    } else {
                        cnv_y -= src2->y;
                        cnv_cb -= src2->cb;
                        cnv_cr -= src2->cr;
                    }
                    cnv_a -= a;
                }
                a = cnv_a / n * dl->intensity >> 12;
                if (0 < a) {
                    double ia = 4096.0 / (double)cnv_a;
                    int dif_y = (int)((double)cnv_y * ia);
                    int y1 = max(0, dst->y * dst->a >> 12);
                    int y2 = dif_y - y1;
                    if (0 < y2) {
                        int dif_cb = ((dst->cb * dst->a >> 12) * y1 + (int)((double)cnv_cb * ia) * y2) / dif_y;
                        int dif_cr = ((dst->cr * dst->a >> 12) * y1 + (int)((double)cnv_cr * ia) * y2) / dif_y;
                        if (0x1000 <= a) {
                            dst->y = dif_y;
                            dst->cb = dif_cb;
                            dst->cr = dif_cr;
                            dst->a = 0x1000;
                        } else if (0x1000 <= dst->a) {
                            dst->y += (dif_y - dst->y) * a >> 12;
                            dst->cb += (dif_cb - dst->cb) * a >> 12;
                            dst->cr += (dif_cr - dst->cr) * a >> 12;
                            dst->a = 0x1000;
                        } else if (dst->a <= 0) {
                            dst->y = dif_y;
                            dst->cb = dif_cb;
                            dst->cr = dif_cr;
                            dst->a = a;
                        } else {
                            int new_a = (0x1000800 - (0x1000 - dst->a) * (0x1000 - a)) >> 12;
                            int a1 = (0x1000 - a) * dst->a / new_a;
                            int a2 = (a << 12) / new_a;
                            dst->y = (dst->y * a1 + dif_y * a2) >> 12;
                            dst->cb = (dst->cb * a1 + dif_cb * a2) >> 12;
                            dst->cr = (dst->cr * a1 + dif_cr * a2) >> 12;
                            dst->a = new_a;
                        }
                    }
                }
                dst++;
                src1++;
                src2++;
            }

            if (0 < loop[3]) {
                int x = loop[3];
                int a = cnv_a / n * dl->intensity >> 12;
                if (0 < a) {
                    double ia = 4096.0 / (double)cnv_a;
                    int dif_y = (int)((double)cnv_y * ia);
                    if (0 < dif_y) {
                        int cb2 = (int)((double)cnv_cb * ia);
                        int cr2 = (int)((double)cnv_cr * ia);
                        for (; 0 < x; x--) {
                            int y1 = max(0, dst->y * dst->a >> 12);
                            int y2 = dif_y - y1;
                            if (0 < y2) {
                                int dif_cb = ((dst->cb * dst->a >> 12) * y1 + cb2 * y2) / dif_y;
                                int dif_cr = ((dst->cr * dst->a >> 12) * y1 + cr2 * y2) / dif_y;
                                if (0x1000 <= a) {
                                    dst->y = dif_y;
                                    dst->cb = dif_cb;
                                    dst->cr = dif_cr;
                                    dst->a = 0x1000;
                                } else if (0x1000 <= dst->a) {
                                    dst->y += (dif_y - dst->y) * a >> 12;
                                    dst->cb += (dif_cb - dst->cb) * a >> 12;
                                    dst->cr += (dif_cr - dst->cr) * a >> 12;
                                    dst->a = 0x1000;
                                } else if (dst->a <= 0) {
                                    dst->y = dif_y;
                                    dst->cb = dif_cb;
                                    dst->cr = dif_cr;
                                    dst->a = a;
                                } else {
                                    int new_a = (0x1000800 - (0x1000 - dst->a) * (0x1000 - a)) >> 12;
                                    int a1 = (0x1000 - a) * dst->a / new_a;
                                    int a2 = (a << 12) / new_a;
                                    dst->y = (dst->y * a1 + dif_y * a2) >> 12;
                                    dst->cb = (dst->cb * a1 + dif_cb * a2) >> 12;
                                    dst->cr = (dst->cr * a1 + dif_cr * a2) >> 12;
                                    dst->a = new_a;
                                }
                            }
                            dst++;
                        }
                    } else {
                        dst += x;
                    }
                } else {
                    dst += x;
                }
            }
            for (int x = loop[4]; 0 < x; x--) {
                int a = src2->a;
                if (0 < a) {
                    if (a < 0x1000) {
                        cnv_y -= src2->y * a >> 12;
                        cnv_cb -= src2->cb * a >> 12;
                        cnv_cr -= src2->cr * a >> 12;
                    } else {
                        cnv_y -= src2->y;
                        cnv_cb -= src2->cb;
                        cnv_cr -= src2->cr;
                    }
                    cnv_a -= a;
                }
                n--;
                a = cnv_a / n * dl->intensity >> 12;
                if (0 < a) {
                    double ia = 4096.0 / (double)cnv_a;
                    int dif_y = (int)((double)cnv_y * ia);
                    int y1 = max(0, dst->y * dst->a >> 12);
                    int y2 = dif_y - y1;
                    if (0 < y2) {
                        int dif_cb = ((dst->cb * dst->a >> 12) * y1 + (int)((double)cnv_cb * ia) * y2) / dif_y;
                        int dif_cr = ((dst->cr * dst->a >> 12) * y1 + (int)((double)cnv_cr * ia) * y2) / dif_y;
                        if (0x1000 <= a) {
                            dst->y = dif_y;
                            dst->cb = dif_cb;
                            dst->cr = dif_cr;
                            dst->a = 0x1000;
                        } else if (0x1000 <= dst->a) {
                            dst->y += (dif_y - dst->y) * a >> 12;
                            dst->cb += (dif_cb - dst->cb) * a >> 12;
                            dst->cr += (dif_cr - dst->cr) * a >> 12;
                            dst->a = 0x1000;
                        } else if (dst->a <= 0) {
                            dst->y = dif_y;
                            dst->cb = dif_cb;
                            dst->cr = dif_cr;
                            dst->a = a;
                        } else {
                            int new_a = (0x1000800 - (0x1000 - dst->a) * (0x1000 - a)) >> 12;
                            int a1 = (0x1000 - a) * dst->a / new_a;
                            int a2 = (a << 12) / new_a;
                            dst->y = (dst->y * a1 + dif_y * a2) >> 12;
                            dst->cb = (dst->cb * a1 + dif_cb * a2) >> 12;
                            dst->cr = (dst->cr * a1 + dif_cr * a2) >> 12;
                            dst->a = new_a;
                        }
                    }
                }
                dst++;
                src2++;
            }
        }
    }



    void __cdecl DiffuseLight_t::yc_cs_mt(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) { // 1e4f0
        auto dl = reinterpret_cast<efDiffuseLight_var*>(GLOBAL::exedit_base + OFS::ExEdit::efDiffuseLight_var_ptr);
        int x = thread_id * efpip->scene_w / thread_num;
        auto dst0 = (ExEdit::PixelYC*)efpip->frame_temp + x;
        auto src01 = (ExEdit::PixelYC*)efpip->frame_edit + x;
        auto src02 = src01;
        auto cnv0 = *(PixelYC_int**)(GLOBAL::exedit_base + OFS::ExEdit::memory_ptr) + x;
        int w = ((thread_id + 1) * efpip->scene_w / thread_num - x) * (sizeof(ExEdit::PixelYC) / sizeof(short));
        memset(cnv0, 0, w * sizeof(int));

        int loop[5];
        if (dl->range <= efpip->scene_h) {
            loop[0] = dl->size;
            loop[1] = dl->size + 1;
            loop[2] = efpip->scene_h - dl->range;
            loop[3] = 0;
            loop[4] = dl->size;
        } else {
            int size = min(dl->size, efpip->scene_h - 1);
            loop[0] = size;
            loop[1] = efpip->scene_h - size;
            loop[2] = 0;
            loop[3] = size * 2 + 1 - efpip->scene_h;
            loop[4] = efpip->scene_h - size - 1;
        }

        for (int y = loop[0]; 0 < y; y--) {
            auto src1 = (short*)src01;
            src01 += efpip->scene_line;
            auto cnv = (int*)cnv0;
            for (x = w; 0 < x; x--) {
                *cnv += *src1;
                cnv++;
                src1++;
            }
        }
        int n = loop[0];
        for (int y = loop[1]; 0 < y; y--) {
            auto src1 = (short*)src01;
            src01 += efpip->scene_line;
            auto dst = (short*)dst0;
            dst0 += efpip->scene_w;
            auto cnv = (int*)cnv0;
            n++;
            for (x = w; 0 < x; x--) {
                *cnv += *src1;
                *dst = *cnv / n;
                dst++;
                cnv++;
                src1++;
            }
        }
        for (int y = loop[2]; 0 < y; y--) {
            auto src1 = (short*)src01;
            src01 += efpip->scene_line;
            auto src2 = (short*)src02;
            src02 += efpip->scene_line;
            auto dst = (short*)dst0;
            dst0 += efpip->scene_w;
            auto cnv = (int*)cnv0;
            for (x = w; 0 < x; x--) {
                *cnv += *src1 - *src2;
                *dst = *cnv / n;
                dst++;
                cnv++;
                src1++;
                src2++;
            }
        }
        for (int y = loop[3]; 0 < y; y--) {
            memcpy(dst0, dst0 - efpip->scene_w, w * sizeof(short));
            dst0 += efpip->scene_w;
        }
        for (int y = loop[4]; 0 < y; y--) {
            auto src2 = (short*)src02;
            src02 += efpip->scene_line;
            auto dst = (short*)dst0;
            dst0 += efpip->scene_w;
            auto cnv = (int*)cnv0;
            n--;
            for (x = w; 0 < x; x--) {
                *cnv -= *src2;
                *dst = *cnv / n;
                dst++;
                cnv++;
                src2++;
            }
        }
    }

    void __cdecl DiffuseLight_t::yc_cs_plus_mt(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) { // 1e770
        auto dl = reinterpret_cast<efDiffuseLight_var*>(GLOBAL::exedit_base + OFS::ExEdit::efDiffuseLight_var_ptr);
        int y = thread_id * efpip->scene_h / thread_num;
        auto src1 = (ExEdit::PixelYC*)efpip->frame_temp + y * efpip->scene_w;
        auto dst0 = (ExEdit::PixelYC*)efpip->frame_edit + y * efpip->scene_line;

        int loop[5];
        if (dl->range <= efpip->scene_w) {
            loop[0] = dl->size;
            loop[1] = dl->size + 1;
            loop[2] = efpip->scene_w - dl->range;
            loop[3] = 0;
            loop[4] = dl->size;
        } else {
            int size = min(dl->size, efpip->scene_w - 1);
            loop[0] = size;
            loop[1] = efpip->scene_w - size;
            loop[2] = 0;
            loop[3] = size * 2 + 1 - efpip->scene_w;
            loop[4] = efpip->scene_w - size - 1;
        }
        y = (thread_id + 1) * efpip->scene_h / thread_num - y;
        if (dl->intensity < 0x1000) {
            for (; 0 < y; y--) {
                auto src2 = src1;
                auto dst = dst0;
                dst0 += efpip->scene_line;
                int cnv_y = 0;
                int cnv_cb = 0;
                int cnv_cr = 0;
                for (int x = loop[0]; 0 < x; x--) {
                    cnv_y += src1->y;
                    cnv_cb += src1->cb;
                    cnv_cr += src1->cr;
                    src1++;
                }
                int n = loop[0];
                for (int x = loop[1]; 0 < x; x--) {
                    cnv_y += src1->y;
                    cnv_cb += src1->cb;
                    cnv_cr += src1->cr;
                    n++;
                    if (n <= cnv_y) {
                        int dif_y = cnv_y / n;
                        int y1 = max(0, dst->y);
                        int y2 = dif_y - y1;
                        if (0 < y2) {
                            dst->y += (dif_y - dst->y) * dl->intensity >> 12;
                            dst->cb += ((dst->cb * y1 + cnv_cb / n * y2) / dif_y - dst->cb) * dl->intensity >> 12;
                            dst->cr += ((dst->cr * y1 + cnv_cr / n * y2) / dif_y - dst->cr) * dl->intensity >> 12;
                        }
                    }
                    dst++;
                    src1++;
                }
                for (int x = loop[2]; 0 < x; x--) {
                    cnv_y += src1->y - src2->y;
                    cnv_cb += src1->cb - src2->cb;
                    cnv_cr += src1->cr - src2->cr;
                    if (n <= cnv_y) {
                        int dif_y = cnv_y / n;
                        int y1 = max(0, dst->y);
                        int y2 = dif_y - y1;
                        if (0 < y2) {
                            dst->y += (dif_y - dst->y) * dl->intensity >> 12;
                            dst->cb += ((dst->cb * y1 + cnv_cb / n * y2) / dif_y - dst->cb) * dl->intensity >> 12;
                            dst->cr += ((dst->cr * y1 + cnv_cr / n * y2) / dif_y - dst->cr) * dl->intensity >> 12;
                        }
                    }
                    dst++;
                    src1++;
                    src2++;
                }

                if (0 < loop[3]) {
                    int x = loop[3];
                    if (n <= cnv_y) {
                        int dif_y = cnv_y / n;
                        int dif_cb = cnv_cb / n;
                        int dif_cr = cnv_cr / n;
                        for (; 0 < x; x--) {
                            int y1 = max(0, dst->y);
                            int y2 = dif_y - y1;
                            if (0 < y2) {
                                dst->y += (dif_y - dst->y) * dl->intensity >> 12;
                                dst->cb += ((dst->cb * y1 + cnv_cb / n * y2) / dif_y - dst->cb) * dl->intensity >> 12;
                                dst->cr += ((dst->cr * y1 + cnv_cr / n * y2) / dif_y - dst->cr) * dl->intensity >> 12;
                            }
                            dst++;
                        }
                    } else {
                        dst += x;
                    }
                }
                for (int x = loop[4]; 0 < x; x--) {
                    cnv_y -= src2->y;
                    cnv_cb -= src2->cb;
                    cnv_cr -= src2->cr;
                    n--;
                    if (n <= cnv_y) {
                        int dif_y = cnv_y / n;
                        int y1 = max(0, dst->y);
                        int y2 = dif_y - y1;
                        if (0 < y2) {
                            dst->y += (dif_y - dst->y) * dl->intensity >> 12;
                            dst->cb += ((dst->cb * y1 + cnv_cb / n * y2) / dif_y - dst->cb) * dl->intensity >> 12;
                            dst->cr += ((dst->cr * y1 + cnv_cr / n * y2) / dif_y - dst->cr) * dl->intensity >> 12;
                        }
                    }
                    dst++;
                    src2++;
                }
            }
        } else {
            for (; 0 < y; y--) {
                auto src2 = src1;
                auto dst = dst0;
                dst0 += efpip->scene_line;
                int cnv_y = 0;
                int cnv_cb = 0;
                int cnv_cr = 0;
                for (int x = loop[0]; 0 < x; x--) {
                    cnv_y += src1->y;
                    cnv_cb += src1->cb;
                    cnv_cr += src1->cr;
                    src1++;
                }
                int n = loop[0];
                for (int x = loop[1]; 0 < x; x--) {
                    cnv_y += src1->y;
                    cnv_cb += src1->cb;
                    cnv_cr += src1->cr;
                    n++;
                    if (n <= cnv_y) {
                        int dif_y = cnv_y / n;
                        int y1 = max(0, dst->y);
                        int y2 = dif_y - y1;
                        if (0 < y2) {
                            dst->y = dif_y;
                            dst->cb = (dst->cb * y1 + cnv_cb / n * y2) / dif_y;
                            dst->cr = (dst->cr * y1 + cnv_cr / n * y2) / dif_y;
                        }
                    }
                    dst++;
                    src1++;
                }
                for (int x = loop[2]; 0 < x; x--) {
                    cnv_y += src1->y - src2->y;
                    cnv_cb += src1->cb - src2->cb;
                    cnv_cr += src1->cr - src2->cr;
                    if (n <= cnv_y) {
                        int dif_y = cnv_y / n;
                        int y1 = max(0, dst->y);
                        int y2 = dif_y - y1;
                        if (0 < y2) {
                            dst->y = dif_y;
                            dst->cb = (dst->cb * y1 + cnv_cb / n * y2) / dif_y;
                            dst->cr = (dst->cr * y1 + cnv_cr / n * y2) / dif_y;
                        }
                    }
                    dst++;
                    src1++;
                    src2++;
                }

                if (0 < loop[3]) {
                    int x = loop[3];
                    if (n <= cnv_y) {
                        int dif_y = cnv_y / n;
                        int dif_cb = cnv_cb / n;
                        int dif_cr = cnv_cr / n;
                        for (; 0 < x; x--) {
                            int y1 = max(0, dst->y);
                            int y2 = dif_y - y1;
                            if (0 < y2) {
                                dst->y = dif_y;
                                dst->cb = (dst->cb * y1 + dif_cb * y2) / dif_y;
                                dst->cr = (dst->cr * y1 + dif_cr * y2) / dif_y;
                            }
                            dst++;
                        }
                    } else {
                        dst += x;
                    }
                }
                for (int x = loop[4]; 0 < x; x--) {
                    cnv_y -= src2->y;
                    cnv_cb -= src2->cb;
                    cnv_cr -= src2->cr;
                    n--;
                    if (n <= cnv_y) {
                        int dif_y = cnv_y / n;
                        int y1 = max(0, dst->y);
                        int y2 = dif_y - y1;
                        if (0 < y2) {
                            dst->y = dif_y;
                            dst->cb = (dst->cb * y1 + cnv_cb / n * y2) / dif_y;
                            dst->cr = (dst->cr * y1 + cnv_cr / n * y2) / dif_y;
                        }
                    }
                    dst++;
                    src2++;
                }
            }
        }
    }



}
#endif // ifdef PATCH_SWITCH_FAST_DIFFUSELIGHT
