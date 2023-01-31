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

#include "patch_border_only_text.hpp"

#ifdef PATCH_SWITCH_BORDER_ONLY_TEXT
namespace patch {

    void BorderOnlyText_t::cb_add_border() {
        SendMessageA(*(HWND*)(GLOBAL::exedit_base + OFS::ExEdit::efTextShadowBorderComboHWND_ptr), CB_ADDSTRING, 0, (LPARAM)border_only_str);
        SendMessageA(*(HWND*)(GLOBAL::exedit_base + OFS::ExEdit::efTextShadowBorderComboHWND_ptr), CB_ADDSTRING, 0, (LPARAM)border_thin_only_str);
    }
    
    /*

    void short_font_border_only(int thread_id, int thread_num, void* n1, void* n2) {
        int exedit_buffer_line = *(int*)(GLOBAL::exedit_base + OFS::ExEdit::exedit_buffer_line);
        auto font = (fast::TextBorder_t::CreateFontBorder*)(GLOBAL::exedit_base + OFS::ExEdit::CreateFontBorder_var_ptr);

        int mask_line = font->src_w + font->border * 2;
        auto masksrc0 = (char*)((short*)font->src + font->src_h * font->src_line);
        auto masksrc1 = masksrc0 + (font->border + font->src_oy) * mask_line + font->border + font->src_ox;
        int a_max_thres = (0x400000 - 1 + font->alpha_rate) / font->alpha_rate;

        int yline8 = thread_num * exedit_buffer_line * sizeof(ExEdit::PixelYCA);
        auto dst0 = font->dst + (thread_id + font->dst_oy) * exedit_buffer_line + font->dst_ox;

        int xx_max = font->src_w - 1;
        int yy_max = font->src_h - 1;

        for (int y = thread_id; y < font->dst_h; y += thread_num) {
            auto dst = dst0;
            dst0 = (ExEdit::PixelYCA*)((int)dst0 + yline8);
            auto mask = masksrc1 + y * mask_line;

            int srcyoy = y + font->src_oy;
            int yy_begin = max(0, srcyoy - font->border);
            int yy_end = min(srcyoy + font->border, yy_max);
            int yy_sub = yy_end - yy_begin;

            auto circle_w0 = font->circle_w + font->border - srcyoy + yy_begin;
            for (int x = 0; x < font->dst_w; x++) {
                if (*mask == 1) {
                    int sum_a = 0;
                    auto circle_w = circle_w0;

                    int srcxox = x + font->src_ox;
                    auto src0 = (short*)font->src + font->src_line * yy_begin;
                    for (int yy = yy_sub; 0 <= yy; yy--) {
                        int xx_begin = max(0, srcxox - *circle_w);
                        int xx_end = min(srcxox + *circle_w, xx_max);
                        circle_w++;
                        auto src = src0 + xx_begin;
                        for (int xx = xx_end - xx_begin; 0 <= xx; xx--) {
                            sum_a += *src;
                            src++;
                        }
                        if (a_max_thres <= sum_a)break;
                        src0 += font->src_line;
                    }
                    int aa = font->alpha_rate * sum_a >> 10;
                    int a = font->alpha;
                    if (aa < 0x1000) {
                        a = a * aa >> 12;
                    }
                    if (0 < a) {
                        if (0 <= srcyoy && srcyoy <= yy_max && 0 <= srcxox && srcxox <= xx_max) {
                            a -= (int)*((short*)font->src + font->src_line * srcyoy + srcxox) << 2;
                        }
                        if (0 < a) {
                            fast::TextBorder_t::blend_yca_normal_func((ExEdit::PixelYCA*)dst, font->color_y, font->color_cb, font->color_cr, a);
                        }
                    }
                }
                mask++;
                dst++;
            }
        }
    }

    void byte_font_border_only(int thread_id, int thread_num, void* n1, void* n2) {
        int exedit_buffer_line = *(int*)(GLOBAL::exedit_base + OFS::ExEdit::exedit_buffer_line);
        auto font = (fast::TextBorder_t::CreateFontBorder*)(GLOBAL::exedit_base + OFS::ExEdit::CreateFontBorder_var_ptr);

        int mask_line = font->src_w + font->border * 2;
        auto masksrc0 = (char*)((char*)font->src + font->src_h * font->src_line);
        auto masksrc1 = masksrc0 + (font->border + font->src_oy) * mask_line + font->border + font->src_ox;
        int a_max_thres = (0x40000 - 1 + font->alpha_rate) / font->alpha_rate;

        int yline8 = thread_num * exedit_buffer_line * sizeof(ExEdit::PixelYCA);
        auto dst0 = font->dst + (thread_id + font->dst_oy) * exedit_buffer_line + font->dst_ox;

        int xx_max = font->src_w - 1;
        int yy_max = font->src_h - 1;

        for (int y = thread_id; y < font->dst_h; y += thread_num) {
            auto dst = dst0;
            dst0 = (ExEdit::PixelYCA*)((int)dst0 + yline8);
            auto mask = masksrc1 + y * mask_line;

            int srcyoy = y + font->src_oy;
            int yy_begin = max(0, srcyoy - font->border);
            int yy_end = min(srcyoy + font->border, yy_max);
            int yy_sub = yy_end - yy_begin;

            auto circle_w0 = font->circle_w + font->border - srcyoy + yy_begin;
            for (int x = 0; x < font->dst_w; x++) {
                if (*mask == 1) {
                    int sum_a = 0;
                    auto circle_w = circle_w0;

                    int srcxox = x + font->src_ox;
                    auto src0 = (char*)font->src + font->src_line * yy_begin;
                    for (int yy = yy_sub; 0 <= yy; yy--) {
                        int xx_begin = max(0, srcxox - *circle_w);
                        int xx_end = min(srcxox + *circle_w, xx_max);
                        circle_w++;
                        auto src = src0 + xx_begin;
                        for (int xx = xx_end - xx_begin; 0 <= xx; xx--) {
                            sum_a += *src;
                            src++;
                        }
                        if (a_max_thres <= sum_a)break;
                        src0 += font->src_line;
                    }
                    int aa = font->alpha_rate * sum_a >> 6;
                    int a = font->alpha;
                    if (aa < 0x1000) {
                        a = a * aa >> 12;
                    }

                    if (0 < a) {
                        if (0 <= srcyoy && srcyoy <= yy_max && 0 <= srcxox && srcxox <= xx_max) {
                            a -= (int)*((char*)font->src + font->src_line * srcyoy + srcxox) << 6;
                        }
                        if (0 < a) {
                            fast::TextBorder_t::blend_yca_normal_func((ExEdit::PixelYCA*)dst, font->color_y, font->color_cb, font->color_cr, a);
                        }
                    }
                }
                mask++;
                dst++;
            }
        }
    }


    void __cdecl BorderOnlyText_t::draw_font_border(ExEdit::PixelYCA* dst, int ofsx, int ofsy, void* src,
        int gmBlackBoxX, int gmBlackBoxY, int y, int cb, int cr, int alpha, int flag, int type, int size) {

        auto font = (fast::TextBorder_t::CreateFontBorder*)(GLOBAL::exedit_base + OFS::ExEdit::CreateFontBorder_var_ptr);

        font->src_w = gmBlackBoxX;
        font->src_h = gmBlackBoxY;
        font->dst_w = gmBlackBoxX + size * 2;
        font->dst_h = gmBlackBoxY + size * 2;
        ofsx -= size;
        ofsy -= size;
        font->src_oy = font->src_ox = -size;
        if (ofsx < 0) {
            font->src_ox -= ofsx;
            font->dst_w += ofsx;
            ofsx = 0;
        }
        if (ofsy < 0) {
            font->src_oy -= ofsy;
            font->dst_h += ofsy;
            ofsy = 0;
        }
        font->dst_w = min(font->dst_w, *(int*)(GLOBAL::exedit_base + OFS::ExEdit::exedit_max_w) - ofsx);
        font->dst_h = min(font->dst_h, *(int*)(GLOBAL::exedit_base + OFS::ExEdit::exedit_max_h) - ofsy);

        if (font->dst_w <= 0 || font->dst_h <= 0) return;

        if (64 < size) {
            size = 64;
        }
        if (font->border != size) {
            font->border = size;
            int range = 0;
            int* circle_w = font->circle_w;
            for (int i = -size; i <= size; i++) {
                int cw = (int)(sqrt((double)(size * size - i * i)) + 0.5);
                *circle_w = cw;
                range += cw;
                circle_w++;
            }
            double border_size;
            if (type == 4) {
                border_size = (double)range * 0.25;
            } else {
                border_size = (double)range * 0.1;
            }
            font->alpha_rate = (int)((4096.0 / (border_size + 1.0)) * (double)size * 0.4);
        }
        font->src = src;
        font->dst = dst;
        font->dst_ox = ofsx;
        font->dst_oy = ofsy;
        font->alpha = alpha;
        font->color_y = y;
        font->color_cb = cb;
        font->color_cr = cr;
        BOOL mtflag = (0x3f < font->dst_h);
        if (flag & 0x20000) {
            font->src_line = gmBlackBoxX;
            fast::TextBorder_t::do_multi_thread_func((AviUtl::MultiThreadFunc*)&fast::TextBorder_t::short_font_border_pre_h_mt, mtflag);
            fast::TextBorder_t::do_multi_thread_func((AviUtl::MultiThreadFunc*)&fast::TextBorder_t::short_font_border_pre_v_mt, mtflag);
            fast::TextBorder_t::do_multi_thread_func((AviUtl::MultiThreadFunc*)&short_font_border_only, mtflag);
        } else {
            font->src_line = gmBlackBoxX + 3U & 0xfffc;
            fast::TextBorder_t::do_multi_thread_func((AviUtl::MultiThreadFunc*)&fast::TextBorder_t::byte_font_border_pre_h_mt, mtflag);
            fast::TextBorder_t::do_multi_thread_func((AviUtl::MultiThreadFunc*)&fast::TextBorder_t::byte_font_border_pre_v_mt, mtflag);
            fast::TextBorder_t::do_multi_thread_func((AviUtl::MultiThreadFunc*)&byte_font_border_only, mtflag);
        }
        
    }
    */
} // namespace patch
#endif // ifdef PATCH_SWITCH_BORDER_ONLY_TEXT