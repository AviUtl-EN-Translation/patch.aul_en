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

#include "patch_fast_colordrift.hpp"
#ifdef PATCH_SWITCH_FAST_COLORDRIFT


//#define PATCH_STOPWATCH

namespace patch::fast {

    struct PixelRGBA {
        short r, g, b, a;
    };

    void __cdecl ColorDrift_t::media_mt1(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        auto cd = reinterpret_cast<efColorDrift_var*>(GLOBAL::exedit_base + 0x11ed88);
        int y = efpip->obj_h * thread_id / thread_num;
        int ofs = y * efpip->obj_line * sizeof(ExEdit::PixelYCA);
        y = efpip->obj_h * (thread_id + 1) / thread_num - y;
        if (0x1000 <= cd->intensity) {
            for (; 0 < y; y--) {
                auto src = (ExEdit::PixelYCA*)((int)efpip->obj_edit + ofs);
                auto dst = (PixelRGBA*)((int)efpip->obj_temp + ofs);
                for (int x = efpip->obj_w; 0 < x; x--) {
                    dst->r = (src->y + ((                  src->cr * 11485) >> 13)) * src->a >> 12;
                    dst->g = (src->y + ((src->cb * -2818 + src->cr * -5849) >> 13)) * src->a >> 12;
                    dst->b = (src->y + ((src->cb * 14516                  ) >> 13)) * src->a >> 12;
                    dst->a = src->a;
                    src++; dst++;
                }
                ofs += efpip->obj_line * sizeof(ExEdit::PixelYCA);
            }
        } else {
            auto ee_memory_ptr = *reinterpret_cast<void**>(GLOBAL::exedit_base + OFS::ExEdit::memory_ptr);
            for (; 0 < y; y--) {
                auto src = (ExEdit::PixelYCA*)((int)efpip->obj_edit + ofs);
                auto dst = (PixelRGBA*)((int)efpip->obj_temp + ofs);
                auto mem = (ExEdit::PixelYCA*)((int)ee_memory_ptr + ofs);
                for (int x = efpip->obj_w; 0 < x; x--) {
                    int a = (src->a * cd->intensity) >> 12;
                    dst->r = (src->y + ((                  src->cr * 11485) >> 13)) * a >> 12;
                    dst->g = (src->y + ((src->cb * -2818 + src->cr * -5849) >> 13)) * a >> 12;
                    dst->b = (src->y + ((src->cb * 14516                  ) >> 13)) * a >> 12;
                    dst->a = a;

                    a = src->a - a;
                    mem->y = src->y * a >> 12;
                    mem->cb = src->cb * a >> 12;
                    mem->cr = src->cr * a >> 12;
                    mem->a = a * 3;

                    src++; dst++; mem++;
                }
                ofs += efpip->obj_line * sizeof(ExEdit::PixelYCA);
            }
        }
    }
    

    
    void __cdecl ColorDrift_t::media_mt2(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        auto cd = reinterpret_cast<efColorDrift_var*>(GLOBAL::exedit_base + 0x11ed88);
        auto ee_memory_ptr = *reinterpret_cast<void**>(GLOBAL::exedit_base + OFS::ExEdit::memory_ptr);

        int yc_w, r_w, g_w, b_w;
        yc_w = r_w = g_w = b_w = cd->w - cd->ox;

        int dst_yc_oxy, dst_r_oxy, dst_g_oxy, dst_b_oxy;
        dst_yc_oxy = dst_r_oxy = dst_g_oxy = dst_b_oxy = -cd->oy * efpip->obj_line;

        int src_yc_oxy = cd->ox;
        if (src_yc_oxy < 0) {
            yc_w += src_yc_oxy;
            dst_yc_oxy -= src_yc_oxy;
            src_yc_oxy = 0;
        } else if (efpip->obj_w - cd->w < 0) {
            yc_w += efpip->obj_w - cd->w;
        }
        yc_w = min(yc_w, efpip->obj_w);

        int src_r_oxy = cd->ox + cd->r_x;
        if (src_r_oxy < 0) {
            r_w += src_r_oxy;
            dst_r_oxy -= src_r_oxy;
            src_r_oxy = 0;
        } else if (efpip->obj_w - cd->w - cd->r_x < 0) {
            r_w += efpip->obj_w - cd->w - cd->r_x;
        }
        src_r_oxy += cd->r_y * efpip->obj_line;
        r_w = min(r_w, efpip->obj_w);

        int src_g_oxy = cd->ox + cd->g_x;
        if (src_g_oxy < 0) {
            g_w += src_g_oxy;
            dst_g_oxy -= src_g_oxy;
            src_g_oxy = 0;
        } else if (efpip->obj_w - cd->w - cd->g_x < 0) {
            g_w += efpip->obj_w - cd->w - cd->g_x;
        }
        src_g_oxy += cd->g_y * efpip->obj_line;
        g_w = min(g_w, efpip->obj_w);

        int src_b_oxy = cd->ox + cd->b_x;
        if (src_b_oxy < 0) {
            b_w += src_b_oxy;
            dst_b_oxy -= src_b_oxy;
            src_b_oxy = 0;
        } else if (efpip->obj_w - cd->w - cd->b_x < 0) {
            b_w += efpip->obj_w - cd->w - cd->b_x;
        }
        src_b_oxy += cd->b_y * efpip->obj_line;
        b_w = min(b_w, efpip->obj_w);

        for (int y = thread_id + cd->oy; y < cd->h; y += thread_num) {
            int yline = y * efpip->obj_line;
            if (cd->intensity < 0x1000 && 0 <= y && y < efpip->obj_h) {
                auto src = (ExEdit::PixelYCA*)ee_memory_ptr + yline + src_yc_oxy;
                auto dst = (ExEdit::PixelYCA*)efpip->obj_edit + yline + dst_yc_oxy;
                for (int x = yc_w; 0 < x; x--) {
                    dst->y += src->y;
                    dst->cb += src->cb;
                    dst->cr += src->cr;
                    dst->a += src->a;
                    src++; dst++;
                }
            }

            int src_oy = cd->r_y + y;
            if (0 <= src_oy && src_oy < efpip->obj_h) {
                auto src = (PixelRGBA*)efpip->obj_temp + yline + src_r_oxy;
                auto dst = (ExEdit::PixelYCA*)efpip->obj_edit + yline + dst_r_oxy;
                for (int x = r_w; 0 < x; x--) {
                    if (1 < src->r) {
                        dst->y += (short)(src->r * 2449 >> 13);
                        dst->cb += (short)(src->r * -173 >> 10);
                        dst->cr += (short)(src->r >> 1);
                    }
                    dst->a += src->a;
                    src++; dst++;
                }
            }
            src_oy = cd->g_y + y;
            if (0 <= src_oy && src_oy < efpip->obj_h) {
                auto src = (PixelRGBA*)efpip->obj_temp + yline + src_g_oxy;
                auto dst = (ExEdit::PixelYCA*)efpip->obj_edit + yline + dst_g_oxy;
                for (int x = g_w; 0 < x; x--) {
                    if (1 < src->g) {
                        dst->y += (short)(src->g * 9617 >> 14);
                        dst->cb += (short)(src->g * -5423 >> 14);
                        dst->cr += (short)(src->g * -429 >> 10);
                    }
                    dst->a += src->a;
                    src++; dst++;
                }
            }
            src_oy = cd->b_y + y;
            if (0 <= src_oy && src_oy < efpip->obj_h) {
                auto src = (PixelRGBA*)efpip->obj_temp + yline + src_b_oxy;
                auto dst = (ExEdit::PixelYCA*)efpip->obj_edit + yline + dst_b_oxy;
                for (int x = b_w; 0 < x; x--) {
                    if (1 < src->b) {
                        dst->y += (short)(src->b * 1867 >> 14);
                        dst->cb += (short)(src->b >> 1);
                        dst->cr += (short)(src->b * -1327 >> 14);
                    }
                    dst->a += src->a;
                    src++; dst++;
                }
            }
            auto dst = (ExEdit::PixelYCA*)efpip->obj_edit + (y - cd->oy) * efpip->obj_line;
            if (cd->type == 0) {
                for (int x = cd->w - cd->ox; 0 < x; x--) {
                    int a = dst->a;
                    if (0 < a) {
                        if (a < 4096) {
                            dst->y = (short)(((int)dst->y << 12) / a);
                            dst->cb = (short)(((int)dst->cb << 12) / a);
                            dst->cr = (short)(((int)dst->cr << 12) / a);
                        }
                        dst->a = a / 3;
                    }
                    dst++;
                }
            } else {
                for (int x = cd->w - cd->ox; 0 < x; x--) {
                    int a = (int)dst->a / 3;
                    if (0 < a) {
                        dst->y = (short)(((int)dst->y << 12) / a);
                        dst->cb = (short)(((int)dst->cb << 12) / a);
                        dst->cr = (short)(((int)dst->cr << 12) / a);
                    }
                    dst->a = (short)a;
                    dst++;
                }
            }
        }
    }

    struct PixelRGB {
        short r, g, b;
    };

    void __cdecl ColorDrift_t::filter_mt1(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        auto cd = reinterpret_cast<efColorDrift_var*>(GLOBAL::exedit_base + 0x11ed88);
        int y = efpip->scene_h * thread_id / thread_num;
        int ofs = y * efpip->scene_line * sizeof(ExEdit::PixelYC);
        y = efpip->scene_h * (thread_id + 1) / thread_num - y;
        if (0x1000 <= cd->intensity) {
            for (; 0 < y; y--) {
                auto src = (ExEdit::PixelYC*)((int)efpip->frame_edit + ofs);
                auto dst = (PixelRGB*)((int)efpip->frame_temp + ofs);
                for (int x = efpip->scene_w; 0 < x; x--) {
                    dst->r = (src->y + ((                  src->cr * 11485) >> 13)) * cd->intensity >> 12;
                    dst->g = (src->y + ((src->cb * -2818 + src->cr * -5849) >> 13)) * cd->intensity >> 12;
                    dst->b = (src->y + ((src->cb * 14516                  ) >> 13)) * cd->intensity >> 12;
                    src++; dst++;
                }
                ofs += efpip->scene_line * sizeof(ExEdit::PixelYC);
            }
        } else {
            int inv_intensity = 0x1000 - cd->intensity;
            auto ee_memory_ptr = *reinterpret_cast<void**>(GLOBAL::exedit_base + OFS::ExEdit::memory_ptr);
            for (; 0 < y; y--) {
                auto src = (ExEdit::PixelYC*)((int)efpip->frame_edit + ofs);
                auto dst = (PixelRGB*)((int)efpip->frame_temp + ofs);
                auto mem = (ExEdit::PixelYC*)((int)ee_memory_ptr + ofs);
                for (int x = efpip->scene_w; 0 < x; x--) {
                    dst->r = (src->y + ((                  src->cr * 11485) >> 13)) * cd->intensity >> 12;
                    dst->g = (src->y + ((src->cb * -2818 + src->cr * -5849) >> 13)) * cd->intensity >> 12;
                    dst->b = (src->y + ((src->cb * 14516                  ) >> 13)) * cd->intensity >> 12;

                    mem->y = src->y * inv_intensity >> 12;
                    mem->cb = src->cb * inv_intensity >> 12;
                    mem->cr = src->cr * inv_intensity >> 12;

                    src++; dst++; mem++;
                }
                ofs += efpip->scene_line * sizeof(ExEdit::PixelYC);
            }
        }
    }

    void __cdecl ColorDrift_t::filter_mt2(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        auto cd = reinterpret_cast<efColorDrift_var*>(GLOBAL::exedit_base + 0x11ed88);
        auto ee_memory_ptr = *reinterpret_cast<void**>(GLOBAL::exedit_base + OFS::ExEdit::memory_ptr);

        int r_w, g_w, b_w;
        r_w = g_w = b_w = efpip->scene_w;

        int src_r_oxy = cd->r_y * efpip->scene_line;
        int dst_r_ox;
        if (cd->r_x < 0) {
            dst_r_ox = -cd->r_x;
            r_w += cd->r_x;
        } else {
            src_r_oxy += cd->r_x;
            dst_r_ox = 0;
            r_w -= cd->r_x;
        }
        int src_g_oxy = cd->g_y * efpip->scene_line;
        int dst_g_ox;
        if (cd->g_x < 0) {
            dst_g_ox = -cd->g_x;
            g_w += cd->g_x;
        } else {
            src_g_oxy += cd->g_x;
            dst_g_ox = 0;
            g_w -= cd->g_x;
        }
        int src_b_oxy = cd->b_y * efpip->scene_line;
        int dst_b_ox;
        if (cd->b_x < 0) {
            dst_b_ox = -cd->b_x;
            b_w += cd->b_x;
        } else {
            src_b_oxy += cd->b_x;
            dst_b_ox = 0;
            b_w -= cd->b_x;
        }

        for (int y = thread_id; y < efpip->scene_h; y += thread_num) {
            int yline = y * efpip->scene_line;
            if (cd->intensity < 0x1000) {
                auto src = (ExEdit::PixelYC*)ee_memory_ptr + yline;
                auto dst = (ExEdit::PixelYC*)efpip->frame_edit + yline;
                for (int x = efpip->scene_w; 0 < x; x--) {
                    dst->y += src->y;
                    dst->cb += src->cb;
                    dst->cr += src->cr;
                    src++; dst++;
                }
            }

            int src_oy = y + cd->r_y;
            if (0 <= src_oy && src_oy < efpip->scene_h) {
                auto src = (PixelRGB*)efpip->frame_temp + yline + src_r_oxy;
                auto dst = (ExEdit::PixelYC*)efpip->frame_edit + yline + dst_r_ox;
                for (int x = r_w; 0 < x; x--) {
                    dst->y += (short)(src->r * 2449 >> 13);
                    dst->cb += (short)(src->r * -173 >> 10);
                    dst->cr += (short)(src->r >> 1);
                    src++; dst++;
                }
            }

            src_oy = y + cd->g_y;
            if (0 <= src_oy && src_oy < efpip->scene_h) {
                auto src = (PixelRGB*)efpip->frame_temp + yline + src_g_oxy;
                auto dst = (ExEdit::PixelYC*)efpip->frame_edit + yline + dst_g_ox;
                for (int x = g_w; 0 < x; x--) {
                    dst->y += (short)(src->g * 9617 >> 14);
                    dst->cb += (short)(src->g * -5423 >> 14);
                    dst->cr += (short)(src->g * -429 >> 10);
                    src++; dst++;
                }
            }

            src_oy = y + cd->b_y;
            if (0 <= src_oy && src_oy < efpip->scene_h) {
                auto src = (PixelRGB*)efpip->frame_temp + yline + src_b_oxy;
                auto dst = (ExEdit::PixelYC*)efpip->frame_edit + yline + dst_b_ox;
                for (int x = b_w; 0 < x; x--) {
                    dst->y += (short)(src->b * 1867 >> 14);
                    dst->cb += (short)(src->b >> 1);
                    dst->cr += (short)(src->b * -1327 >> 14);
                    src++; dst++;
                }
            }
        }
    }
} // namespace patch::fast
#endif // ifdef PATCH_SWITCH_FAST_COLORDRIFT
