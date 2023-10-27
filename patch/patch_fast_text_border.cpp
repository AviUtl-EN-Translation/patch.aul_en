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
#include "patch_fast_text_border.hpp"
#ifdef PATCH_SWITCH_FAST_TEXTBORDER

#include <numbers>

#include "global.hpp"
#include "offset_address.hpp"
#include "util_int.hpp"

namespace patch::fast {


	void short_font_border_create_mask1(int thread_id, int thread_num, void* n1, void* n2) {
		auto font = (TextBorder_t::CreateFontBorder*)(GLOBAL::exedit_base + OFS::ExEdit::CreateFontBorder_var_ptr);

		int border_size = font->border;
		int border_range = border_size * 2;
		int src_w = font->src_w;
		int dst_line = src_w + border_range;
		int src_line = font->src_line;
		const int maxvalue = 1024;
		auto buf = (short*)font->src;
		auto src = buf + thread_id * src_line;
		auto dst = (char*)(buf + font->src_h * src_line) + thread_id * dst_line;
		int y;
		for (y = font->src_h - thread_id; 0 < y; y -= thread_num) {
			auto src1 = src;
			auto dst1 = dst;
			memset(dst, 0, dst_line);
			int cd = 0;
			for (int i = src_w; 0 < i; i--) {
				if (*src1) {
					cd = border_range;
					if (maxvalue <= *src1) {
						dst1[border_size] = 2;
					}
					if (*dst1 == 0) {
						*dst1 = 1;
					}
				} else if (cd) {
					cd--;
					if (*dst1 == 0) {
						*dst1 = 1;
					}
				}
				src1++;
				dst1++;
			}
			for (int i = border_range; 0 < i; i--) {
				if (cd) {
					cd--;
					if (*dst1 == 0) {
						*dst1 = 1;
					}
				}
				dst1++;
			}

			src += thread_num * src_line;
			dst += thread_num * dst_line;
		}
		for (y += border_range; 0 < y; y -= thread_num) {
			memset(dst, 0, dst_line);
			dst += thread_num * dst_line;
		}
	}

	void sb_font_border_create_mask2(int thread_id, int thread_num, int src_elm, void* n2) {
		auto font = (TextBorder_t::CreateFontBorder*)(GLOBAL::exedit_base + OFS::ExEdit::CreateFontBorder_var_ptr);

		int border_range = font->border * 2;
		int src_h = font->src_h;
		int src_line = font->src_w + border_range;
		int border_ofs = font->border * src_line;
		auto src = (char*)((int)font->src + src_h * font->src_line * src_elm) + thread_id;
		for (int x = src_line - thread_id; 0 < x; x -= thread_num) {
			auto src1 = src;
			int cd = 0;
			for (int i = src_h; 0 < i; i--) {
				switch (*src1) {
				case 0:
					if (cd) {
						cd--;
						*src1 = 1;
					}
					break;
				case 1:
					cd = border_range;
					break;
				case 2:
					cd = border_range;
					*src1 = 1;
					src1[border_ofs] |= 4;
					break;
				case 4:
					if (cd) {
						cd--;
					}
					break;
				case 5:
					cd = border_range;
					break;
				case 6:
					cd = border_range;
					src1[border_ofs] |= 4;
					break;
				}
				src1 += src_line;
			}
			for (int i = border_range; 0 < i; i--) {
				if (cd) {
					cd--;
					if (*src1 == 0) {
						*src1 = 1;
					}
				}
				src1 += src_line;
			}
			src += thread_num;
		}
	}
	void short_font_border_create_mask2(int thread_id, int thread_num, void* n1, void* n2) {
		sb_font_border_create_mask2(thread_id, thread_num, sizeof(short), n2);
	}

	void short_font_border(int thread_id, int thread_num, void* n1, void* n2) {
		int yca_vram_w = *(int*)(GLOBAL::exedit_base + OFS::ExEdit::yca_vram_w);
		auto font = (TextBorder_t::CreateFontBorder*)(GLOBAL::exedit_base + OFS::ExEdit::CreateFontBorder_var_ptr);

		int mask_line = font->src_w + font->border * 2;
		auto masksrc0 = (char*)((short*)font->src + font->src_h * font->src_line);
		auto masksrc1 = masksrc0 + (font->border + font->src_oy) * mask_line + font->border + font->src_ox;
		int a_max_thres = (0x400000 - 1 + font->alpha_rate) / font->alpha_rate;

		int yline8 = thread_num * yca_vram_w * sizeof(ExEdit::PixelYCA);
		auto dst0 = font->dst + (thread_id + font->dst_oy) * yca_vram_w + font->dst_ox;

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
						TextBorder_t::blend_yca_normal_func((ExEdit::PixelYCA*)dst, font->color_y, font->color_cb, font->color_cr, a);
					}
				}
				mask++;
				dst++;
			}
		}
	}



	void byte_font_border_create_mask1(int thread_id, int thread_num, void* n1, void* n2) {
		auto font = (TextBorder_t::CreateFontBorder*)(GLOBAL::exedit_base + OFS::ExEdit::CreateFontBorder_var_ptr);

		int border_size = font->border;
		int border_range = border_size * 2;
		int src_w = font->src_w;
		int dst_line = src_w + border_range;
		int src_line = font->src_line;
		const int maxvalue = 64;
		auto buf = (char*)font->src;
		auto src = buf + thread_id * src_line;
		auto dst = (char*)(buf + font->src_h * src_line) + thread_id * dst_line;
		int y;
		for (y = font->src_h - thread_id; 0 < y; y -= thread_num) {
			auto src1 = src;
			auto dst1 = dst;
			memset(dst, 0, dst_line);
			int cd = 0;
			for (int i = src_w; 0 < i; i--) {
				if (*src1) {
					cd = border_range;
					if (maxvalue <= *src1) {
						dst1[border_size] = 2;
					}
					if (*dst1 == 0) {
						*dst1 = 1;
					}
				} else if (cd) {
					cd--;
					if (*dst1 == 0) {
						*dst1 = 1;
					}
				}
				src1++;
				dst1++;
			}
			for (int i = border_range; 0 < i; i--) {
				if (cd) {
					cd--;
					if (*dst1 == 0) {
						*dst1 = 1;
					}
				}
				dst1++;
			}

			src += thread_num * src_line;
			dst += thread_num * dst_line;
		}
		for (y += border_range; 0 < y; y -= thread_num) {
			memset(dst, 0, dst_line);
			dst += thread_num * dst_line;
		}
	}

	void byte_font_border_create_mask2(int thread_id, int thread_num, void* n1, void* n2) {
		sb_font_border_create_mask2(thread_id, thread_num, sizeof(char), n2);
	}

	void byte_font_border(int thread_id, int thread_num, void* n1, void* n2) {
		int yca_vram_w = *(int*)(GLOBAL::exedit_base + OFS::ExEdit::yca_vram_w);
		auto font = (TextBorder_t::CreateFontBorder*)(GLOBAL::exedit_base + OFS::ExEdit::CreateFontBorder_var_ptr);

		int mask_line = font->src_w + font->border * 2;
		auto masksrc0 = (char*)((char*)font->src + font->src_h * font->src_line);
		auto masksrc1 = masksrc0 + (font->border + font->src_oy) * mask_line + font->border + font->src_ox;
		int a_max_thres = (0x40000 - 1 + font->alpha_rate) / font->alpha_rate;

		int yline8 = thread_num * yca_vram_w * sizeof(ExEdit::PixelYCA);
		auto dst0 = font->dst + (thread_id + font->dst_oy) * yca_vram_w + font->dst_ox;

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
						TextBorder_t::blend_yca_normal_func((ExEdit::PixelYCA*)dst, font->color_y, font->color_cb, font->color_cr, a);
					}
				}
				mask++;
				dst++;
			}
		}
	}


#ifdef PATCH_SWITCH_BORDER_ONLY_TEXT
	void short_font_border_only(int thread_id, int thread_num, void* n1, void* n2) {
		int yca_vram_w = *(int*)(GLOBAL::exedit_base + OFS::ExEdit::yca_vram_w);
		auto font = (TextBorder_t::CreateFontBorder*)(GLOBAL::exedit_base + OFS::ExEdit::CreateFontBorder_var_ptr);

		int mask_line = font->src_w + font->border * 2;
		auto masksrc0 = (char*)((short*)font->src + font->src_h * font->src_line);
		auto masksrc1 = masksrc0 + (font->border + font->src_oy) * mask_line + font->border + font->src_ox;
		int a_max_thres = (0x400000 - 1 + font->alpha_rate) / font->alpha_rate;

		int yline8 = thread_num * yca_vram_w * sizeof(ExEdit::PixelYCA);
		auto dst0 = font->dst + (thread_id + font->dst_oy) * yca_vram_w + font->dst_ox;

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
							// 正確な式は違うけどこれでほぼ問題無いはず。正しい式はたぶん右のような感じ a = (0x1000 - src_a) * a * 0x1000 / (0x1000000 - (0x1000 - a) * (0x1000 - src_a));
							a -= (int)*((short*)font->src + font->src_line * srcyoy + srcxox) << 2;
						}
						if (0 < a) {
							TextBorder_t::blend_yca_normal_func((ExEdit::PixelYCA*)dst, font->color_y, font->color_cb, font->color_cr, a);
						}
					}
				}
				mask++;
				dst++;
			}
		}
	}

	void byte_font_border_only(int thread_id, int thread_num, void* n1, void* n2) {
		int yca_vram_w = *(int*)(GLOBAL::exedit_base + OFS::ExEdit::yca_vram_w);
		auto font = (TextBorder_t::CreateFontBorder*)(GLOBAL::exedit_base + OFS::ExEdit::CreateFontBorder_var_ptr);

		int mask_line = font->src_w + font->border * 2;
		auto masksrc0 = (char*)((char*)font->src + font->src_h * font->src_line);
		auto masksrc1 = masksrc0 + (font->border + font->src_oy) * mask_line + font->border + font->src_ox;
		int a_max_thres = (0x40000 - 1 + font->alpha_rate) / font->alpha_rate;

		int yline8 = thread_num * yca_vram_w * sizeof(ExEdit::PixelYCA);
		auto dst0 = font->dst + (thread_id + font->dst_oy) * yca_vram_w + font->dst_ox;

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
							// 正確な式は違うけどこれでほぼ問題無いはず。正しい式はたぶん右のような感じ a = (0x1000 - src_a) * a * 0x1000 / (0x1000000 - (0x1000 - a) * (0x1000 - src_a));
							a -= (int)*((char*)font->src + font->src_line * srcyoy + srcxox) << 6;
						}
						if (0 < a) {
							TextBorder_t::blend_yca_normal_func((ExEdit::PixelYCA*)dst, font->color_y, font->color_cb, font->color_cr, a);
						}
					}
				}
				mask++;
				dst++;
			}
		}
	}
#endif



	void __cdecl TextBorder_t::create_font_border(ExEdit::PixelYCA* dst, int ofsx, int ofsy, void* src,
		int gmBlackBoxX, int gmBlackBoxY, int y, int cb, int cr, int alpha, int flag, int type, int size) {

		auto font = (CreateFontBorder*)(GLOBAL::exedit_base + OFS::ExEdit::CreateFontBorder_var_ptr);

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
		font->dst_w = min(font->dst_w, *(int*)(GLOBAL::exedit_base + OFS::ExEdit::yca_max_w) - ofsx);
		font->dst_h = min(font->dst_h, *(int*)(GLOBAL::exedit_base + OFS::ExEdit::yca_max_h) - ofsy);

		if (font->dst_w <= 0 || font->dst_h <= 0) return;

		if (64 < size) {
			size = 64;
		}
		if (font->border != size) {
			font->border = size;
			int range = 0;
			int* circle_w = font->circle_w;
			int* circle_we = circle_w + size * 2;
			int cw;
			for (int i = size; 0 <= i; i--) {
				cw = (int)(sqrt((double)(size * size - i * i)) + 0.5);
				*circle_we = *circle_w = cw;
				range += cw * 2;
				circle_w++;
				circle_we--;
			}
			range -= cw;
			pre_range = range;
		}
		double range = pre_range;
		if ((type & 1) == 0) {
			range *= 2.5;
		}
		font->alpha_rate = (int)(16384 / (range + 10) * (double)size);

		font->src = src;
		font->dst = dst;
		font->dst_ox = ofsx;
		font->dst_oy = ofsy;
		font->alpha = alpha;
		font->color_y = y;
		font->color_cb = cb;
		font->color_cr = cr;
		BOOL mtflag = (0x1f < font->dst_h);

		if (flag & 0x20000) {
			font->src_line = gmBlackBoxX;
			do_multi_thread_func((AviUtl::MultiThreadFunc*)&short_font_border_create_mask1, mtflag);
			do_multi_thread_func((AviUtl::MultiThreadFunc*)&short_font_border_create_mask2, mtflag);

#ifdef PATCH_SWITCH_BORDER_ONLY_TEXT
			if (4 < type) {
				do_multi_thread_func((AviUtl::MultiThreadFunc*)&short_font_border_only, mtflag);
				return;
			}
#endif
			do_multi_thread_func((AviUtl::MultiThreadFunc*)&short_font_border, mtflag);
		} else {
			font->src_line = gmBlackBoxX + 3U & 0xfffc;
			do_multi_thread_func((AviUtl::MultiThreadFunc*)&byte_font_border_create_mask1, mtflag);
			do_multi_thread_func((AviUtl::MultiThreadFunc*)&byte_font_border_create_mask2, mtflag);

#ifdef PATCH_SWITCH_BORDER_ONLY_TEXT
			if (4 < type) {
				do_multi_thread_func((AviUtl::MultiThreadFunc*)&byte_font_border_only, mtflag);
				return;
			}
#endif
			do_multi_thread_func((AviUtl::MultiThreadFunc*)&byte_font_border, mtflag);
		}


	}



}
#endif
