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

#include "patch_fast_yc_filter_effect.hpp"
#ifdef PATCH_SWITCH_FAST_YC_FILTER_EFFECT


//#define PATCH_STOPWATCH
#include "patch_fast_blur.hpp"
#include "patch_fast_directionalblur.hpp"
#include "patch_fast_shadow.hpp"
#include "patch_exfilter_specialcolorconv.hpp"
#include "patch_exfilter_flash.hpp"
#include "patch_exfilter_gradation.hpp"
#include "patch_exfilter_noise.hpp"


namespace patch::fast {
    int get_check_value(int oid, int fid, int check_id) {
        auto ObjectArrayPointer = *(ExEdit::Object**)(GLOBAL::exedit_base + OFS::ExEdit::ObjectArrayPointer);
        auto obj = ObjectArrayPointer + oid;
        if (0 <= obj->index_midpt_leader) {
            obj = &ObjectArrayPointer[obj->index_midpt_leader];
        }
        return obj->check_value[obj->filter_param[fid].check_begin + check_id];
    }
    BOOL __cdecl check0(int oid, int fid, ExEdit::FilterProcInfo* efpip) {
        return get_check_value(oid, fid, 0) != 0;
    }
    BOOL __cdecl check4_not(int oid, int fid, ExEdit::FilterProcInfo* efpip) {
        return get_check_value(oid, fid, 4) == 0;
    }
    BOOL __cdecl func_check_flash(int oid, int fid, ExEdit::FilterProcInfo* efpip) {
        int* exdata = (int*)reinterpret_cast<void* (__cdecl*)(ExEdit::ObjectFilterIndex)>(GLOBAL::exedit_base + OFS::ExEdit::get_exdata_ptr)((ExEdit::ObjectFilterIndex)(oid + 1 + (fid << 16)));
        return (exdata[1] == 0 || exdata[1] == 1) && get_check_value(oid, fid, 2) != 0;
    }
    BOOL __cdecl func_check_noise(int oid, int fid, ExEdit::FilterProcInfo* efpip) {
        int* exdata = (int*)reinterpret_cast<void* (__cdecl*)(ExEdit::ObjectFilterIndex)>(GLOBAL::exedit_base + OFS::ExEdit::get_exdata_ptr)((ExEdit::ObjectFilterIndex)(oid + 1 + (fid << 16)));
        return exdata[1] == 1;
    }
    BOOL __cdecl func_check_fade(int oid, int fid, ExEdit::FilterProcInfo* efpip) {
        ExEdit::Filter* efp = reinterpret_cast<ExEdit::Filter*>(GLOBAL::exedit_base + OFS::ExEdit::efFade_ptr);
        auto ObjectArrayPointer = *(ExEdit::Object**)(GLOBAL::exedit_base + OFS::ExEdit::ObjectArrayPointer);
        int track[2];
        reinterpret_cast<void(__cdecl*)(ExEdit::Object*, int, int*, int*, void*, ExEdit::FilterProcInfo*)>(GLOBAL::exedit_base + OFS::ExEdit::store_filter)(ObjectArrayPointer + oid, fid, track, NULL, NULL, efpip);
        double fps = (double)efpip->framerate_nu * 0.01 / (double)efpip->framerate_de;
        if (efpip->frame_num - efp->frame_start_chain - efpip->object_start_frame < (int)((double)track[0] * fps)) {
            return FALSE;
        }
        if (efp->frame_end_chain - efpip->frame_num < (int)((double)track[1] * fps)) {
            return FALSE;
        }
        return TRUE;
    }
    BOOL __cdecl func_check_wipe(int oid, int fid, ExEdit::FilterProcInfo* efpip) {
        ExEdit::Filter* efp = reinterpret_cast<ExEdit::Filter*>(GLOBAL::exedit_base + OFS::ExEdit::efWipe_ptr);
        auto ObjectArrayPointer = *(ExEdit::Object**)(GLOBAL::exedit_base + OFS::ExEdit::ObjectArrayPointer);
        int track[3];
        reinterpret_cast<void(__cdecl*)(ExEdit::Object*, int, int*, int*, void*, ExEdit::FilterProcInfo*)>(GLOBAL::exedit_base + OFS::ExEdit::store_filter)(ObjectArrayPointer + oid, fid, track, NULL, NULL, efpip);
        double fps = (double)efpip->framerate_nu * 0.01 / (double)efpip->framerate_de;
        if (efpip->frame_num - efp->frame_start_chain < (int)((double)track[0] * fps)) {
            return FALSE;
        }
        if (efp->frame_end_chain - efpip->frame_num < (int)((double)track[1] * fps)) {
            return FALSE;
        }
        return TRUE;
    }
    void yc_filter_effect_t::func_check_listing() {
        auto LoadedFilterTable = (ExEdit::Filter**)(GLOBAL::exedit_base + OFS::ExEdit::LoadedFilterTable);
        for (int i = 0; i < exedit_check_filter_num; i++) {
            switch ((int)LoadedFilterTable[i] - GLOBAL::exedit_base) {
            case OFS::ExEdit::efBlur_ptr:
                if (Blur.is_enabled_i()) {
                    (func_check[i]) = (check0);
                }
                break;
            case OFS::ExEdit::efDirectionalBlur_ptr:
                if (DirectionalBlur.is_enabled_i()) {
                    (func_check[i]) = (check0);
                }
                break;
            case OFS::ExEdit::efShadow_ptr:
                if (Shadow.is_enabled_i()) {
                    (func_check[i]) = (check0);
                }
                break;
            case OFS::ExEdit::efDiffuseLight_ptr:
            case OFS::ExEdit::efLightEmission_ptr:
            case OFS::ExEdit::efLensBlur_ptr:
                (func_check[i]) = (check0);
                break;
            case OFS::ExEdit::efFlip_ptr:
                (func_check[i]) = (check4_not);
                break;
            case OFS::ExEdit::efFlash_ptr:
                (func_check[i]) = (func_check_flash);
                break;
            case OFS::ExEdit::efNoise_ptr:
                (func_check[i]) = (func_check_noise);
                break;
            case OFS::ExEdit::efFade_ptr:
                (func_check[i]) = (func_check_fade);
                break;
            case OFS::ExEdit::efWipe_ptr:
                (func_check[i]) = (func_check_wipe);
                break;
            }
        }
    }
    void __stdcall yc_filter_effect_t::yc_check(ExEdit::FilterProcInfo* efpip, ExEdit::Object** objpp, int poid, int on, int fid) {
        if (first) {
            func_check_listing();
            first = FALSE;
        }

        auto LoadedFilterTable = (ExEdit::Filter**)(GLOBAL::exedit_base + OFS::ExEdit::LoadedFilterTable);
        auto ObjectArrayPointer = *(ExEdit::Object**)(GLOBAL::exedit_base + OFS::ExEdit::ObjectArrayPointer);
        efpip->xf4 = 0;
        //fid++;
        for (; poid < on; poid++) {
            for (; fid < 12; fid++) {
                auto obj = objpp[poid];
                int oid = ((int)obj - (int)ObjectArrayPointer) / sizeof(ExEdit::Object);
                int filter_idx = obj->filter_param[fid].id;
                if (filter_idx < 0) {
                    break;
                }
                auto efp = LoadedFilterTable[filter_idx];
                if (!has_flag(efp->flag, ExEdit::Filter::Flag::Output)) {
                    if (0 <= obj->index_midpt_leader) {
                        obj = &ObjectArrayPointer[obj->index_midpt_leader];
                    }
                    if (has_flag(obj->filter_status[fid], ExEdit::Object::FilterStatus::Active)) {
                        if (((byte)efp->flag & 0x40) == 0) {
                            if (exedit_check_filter_num <= filter_idx || func_check[filter_idx] == NULL || !func_check[filter_idx](oid, fid, efpip)) {
                                return;
                            }
                        }
                    }
                }
            }
            fid = 0;
        }
        *(byte*)&efpip->flag |= 0x40;
    }

    BOOL __cdecl yc_filter_effect_t::func_proc(BOOL(__cdecl*func_proc)(ExEdit::Filter*, ExEdit::FilterProcInfo*), BOOL(__cdecl*filter_func_proc)(ExEdit::Filter*, ExEdit::FilterProcInfo*), ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        if (efpip->xf4) {
            auto scene_w = efpip->scene_w;
            auto scene_h = efpip->scene_h;
            auto frame_edit = efpip->frame_edit;
            auto frame_temp = efpip->frame_temp;
            efpip->scene_w = efpip->obj_w;
            efpip->scene_h = efpip->obj_h;
            efpip->frame_edit = reinterpret_cast<decltype(efpip->frame_edit)>(efpip->obj_edit);
            efpip->frame_temp = reinterpret_cast<decltype(efpip->frame_temp)>(efpip->obj_temp);
            
            efp->flag ^= static_cast<decltype(efp->flag)>(0x20);
            BOOL ret = filter_func_proc(efp, efpip);
            efp->flag ^= static_cast<decltype(efp->flag)>(0x20);

            efpip->obj_edit = reinterpret_cast<decltype(efpip->obj_edit)>(efpip->frame_edit);
            efpip->obj_temp = reinterpret_cast<decltype(efpip->obj_temp)>(efpip->frame_temp);
            efpip->scene_w = scene_w;
            efpip->scene_h = scene_h;
            efpip->frame_edit = frame_edit;
            efpip->frame_temp = frame_temp;
            return ret;
        } else {
            return func_proc(efp, efpip);
        }
    }

    BOOL __cdecl yc_filter_effect_t::efColorCorrection_func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        return func_proc(efColorCorrection_func_proc_org, efColorCorrection_func_proc_org, efp, efpip);
    }
    BOOL __cdecl yc_filter_effect_t::efMonochromatic_func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        return func_proc(efMonochromatic_func_proc_org, efMonochromatic_func_proc_org, efp, efpip);
    }
    BOOL __cdecl yc_filter_effect_t::efMosaic_func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        return func_proc(efMosaic_func_proc_org, efMosaic_func_proc_org, efp, efpip);
    }
    BOOL __cdecl yc_filter_effect_t::efGradation_func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        return func_proc(efGradation_func_proc_org, exfilter::Gradation_t::func_proc, efp, efpip);
    }
    BOOL __cdecl yc_filter_effect_t::efSpecialColorConv_func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        return func_proc(efSpecialColorConv_func_proc_org, exfilter::SpecialColorConv_t::func_proc, efp, efpip);
    }
    BOOL __cdecl yc_filter_effect_t::efDiffuseLight_func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        return func_proc(efDiffuseLight_func_proc_org, efDiffuseLight_func_proc_org, efp, efpip);
    }
    BOOL __cdecl yc_filter_effect_t::efLightEmission_func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        return func_proc(efLightEmission_func_proc_org, efLightEmission_func_proc_org, efp, efpip);
    }
    BOOL __cdecl yc_filter_effect_t::efLensBlur_func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        return func_proc(efLensBlur_func_proc_org, efLensBlur_func_proc_org, efp, efpip);
    }
    BOOL __cdecl yc_filter_effect_t::efFlip_func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        return func_proc(efFlip_func_proc_org, efFlip_func_proc_org, efp, efpip);
    }
    BOOL __cdecl yc_filter_effect_t::efFlash_func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        if (efpip->xf4) { // サイズ固定 && (前方に合成 || 後方に合成)
            if (((int*)efp->exdata_ptr)[1] == 1) { // 後方に合成
                return TRUE;
            }
            efp->track[3] = 750;
        }
        return func_proc(efFlash_func_proc_org, exfilter::Flash_t::func_proc, efp, efpip);
    }
    BOOL __cdecl yc_filter_effect_t::efNoise_func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        return func_proc(efNoise_func_proc_org, exfilter::Noise_t::func_proc, efp, efpip);
    }


    struct cache_var { // 0x0d75f8
        char instant_name[260];
        void* instant_ptr;
        int alloc_size;
        int instant_bitcount;
        int instant_size;
        int instant_w;
        ExEdit::CacheInfo info[32];
        int instant_h;
        int max_n;
        int n;
        int priority_count;
    };
    ExEdit::CacheDataInfo* __cdecl GetCacheInfo(LPCSTR name, uint32_t** offset) { // サイズ調整が可能ならoffsetを返す
        auto cache = reinterpret_cast<cache_var*>(GLOBAL::exedit_base + OFS::ExEdit::Cache_var_ptr);
        for (int cid = 0; cid < cache->n; cid++) {
            if (lstrcmpiA(name, cache->info[cid].main.name) == 0) {
                cache->priority_count++;
                cache->info[cid].priority = cache->priority_count;
                if (offset != nullptr && cache->info[cid].sub_num == 0) {
                    *offset = &cache->info[cid].offset;
                }
                return &cache->info[cid].main;
            }
            for (uint32_t sub_id = 0; sub_id < cache->info[cid].sub_num; sub_id++) {
                if (lstrcmpiA(name, cache->info[cid].sub[sub_id].name) == 0) {
                    cache->priority_count++;
                    cache->info[cid].priority = cache->priority_count;
                    if (offset != nullptr && cache->info[cid].sub_num == sub_id + 1) {
                        *offset = &cache->info[cid].offset;
                    }
                    return &cache->info[cid].sub[sub_id];
                }
            }
        }
        return nullptr;
    }
    BOOL __cdecl yc_filter_effect_t::load_image_cache_1st(ExEdit::PixelYCA* buf, int* w, int* h, char* path, int* load_pos) {
        uint32_t* offset;
        auto cacheinfo = GetCacheInfo(path, &offset);
        if (cacheinfo != nullptr && cacheinfo->bitcount == 32) {
            // 透明度情報が必要かを調べる
            byte* src = (byte*)cacheinfo->ptr + 3;
            for (int i = cacheinfo->h * cacheinfo->w; 0 < i; i--) {
                if (*src != 0xff) {
                    return reinterpret_cast<BOOL(__cdecl*)(ExEdit::PixelYCA*, int*, int*, char*, int*)>(GLOBAL::exedit_base + OFS::ExEdit::LoadImageCache)(buf, w, h, path, load_pos);
                }
                src += 4;
            }
            byte* dst = src = (byte*)cacheinfo->ptr;
            int dline = ((cacheinfo->w + 1) * 3 & 0xfffffffc) - cacheinfo->w * 3;
            for (int y = cacheinfo->h; 0 < y; y--) {
                for (int x = cacheinfo->w; 0 < x; x--) {
                    *dst = *src; dst++; src++;
                    *dst = *src; dst++; src++;
                    *dst = *src; dst++; src += 2;
                }
                dst += dline;
            }
            cacheinfo->bitcount = 24;
            if (offset != nullptr) {
                *offset -= (((cacheinfo->w << 2) * cacheinfo->h + 16 + 15) & 0xfffffff0) - ((((cacheinfo->w + 1) * 3 & 0xfffffffc) * cacheinfo->h + 16 + 15) & 0xfffffff0);
            }
        }
        return reinterpret_cast<BOOL(__cdecl*)(ExEdit::PixelYCA*, int*, int*, char*, int*)>(GLOBAL::exedit_base + OFS::ExEdit::LoadImageCache)(buf, w, h, path, load_pos);
    }
    static int rgb2yc_flag = 0;
    BOOL __stdcall yc_filter_effect_t::exfunc_load_image_wrap(ExEdit::FilterProcInfo* efpip, ExEdit::PixelYCA* buf, char* path, int* w, int* h, int* load_pos, int flag) {
        rgb2yc_flag = 0;
        efpip->xf4 = 0;
        auto e_exfunc = reinterpret_cast<ExEdit::Exfunc*>(GLOBAL::exedit_base + OFS::ExEdit::exfunc);
        BOOL ret, ran = FALSE;
        if (efpip->flag & 0x40) {
            int ww, hh, bitcount;
            if (nullptr == reinterpret_cast<void* (__cdecl*)(char*, int*, int*, int*)>(GLOBAL::exedit_base + OFS::ExEdit::GetCache)(path, &ww, &hh, &bitcount)) {
                ret = e_exfunc->load_image(buf, path, w, h, (int)load_pos, flag);
                ran = TRUE;
                if (nullptr == reinterpret_cast<void* (__cdecl*)(char*, int*, int*, int*)>(GLOBAL::exedit_base + OFS::ExEdit::GetCache)(path, &ww, &hh, &bitcount)) {
                    return ret;
                }
            }
            if (bitcount == 24) {
                int yc_wh = *reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::yc_max_w);
                if (ww <= yc_wh || yc_wh == *reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::yca_max_w)) {
                    yc_wh = *reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::yc_max_h);
                    if (hh <= yc_wh || yc_wh == *reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::yca_max_h)) {
                        rgb2yc_flag = 1;
                        efpip->xf4 = 1;
                        ran = FALSE;
                    }
                }
            }
        }
        if (!ran) {
            ret = e_exfunc->load_image(buf, path, w, h, (int)load_pos, flag);
        }
        rgb2yc_flag = 0;
        return ret;
    }

    struct bgr2yca_var{ // 0x1e42d0
        ExEdit::PixelBGRA* rgba;
        int _padding1[8];
        ExEdit::PixelYCA* yca;
        int _padding2[2];
        int w;
        int rgba_line_size;
        int _padding3[8];
        int h;
    };
    void __cdecl bgr2yc_mt(int thread_id, int thread_num, bgr2yca_var* by, int vram_line_size) {
        int y = thread_id * by->h / thread_num;
        byte* src0 = (byte*)by->rgba + by->rgba_line_size * y;
        byte* dst0 = (byte*)by->yca - vram_line_size * y;
        for (y = (thread_id + 1) * by->h / thread_num - y; 0 < y; y--) {
            auto src = src0;
            src0 += by->rgba_line_size;
            dst0 -= vram_line_size;
            auto dst = reinterpret_cast<short*>(dst0);
            for (int x = by->w; 0 < x; x--) {
                int b = (*src << 5) + 9; src++;
                int g = (*src << 5) + 9; src++;
                int r = (*src << 5) + 9; src++;
                *dst = (r * 4918 >> 15) + (g * 9655 >> 15) + (b * 1875 >> 15) - 3; dst++;
                *dst = (r * -2775 >> 15) + (g * -5449 >> 15) + (b * 8224 >> 15) - 3; dst++;
                *dst = (r * 8224 >> 15) + (g * -6887 >> 15) + (b * -1337 >> 15) - 3; dst++;
            }
        }
    }
    void __cdecl bgr2yc(void* dst, void* src, int w, int h, int srcline) {
        auto by = reinterpret_cast<bgr2yca_var*>(GLOBAL::exedit_base + 0x1e42d0);
        int vram_line_size = *reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::yc_vram_line_size);
        by->yca = reinterpret_cast<decltype(by->yca)>((int)dst + vram_line_size * h);
        by->rgba = reinterpret_cast<decltype(by->rgba)>(src);
        by->h = h;
        if (srcline == 0) {
            srcline = (w * 3 + 3) & 0xfffffffc;
        }
        by->w = w;
        by->rgba_line_size = srcline;
        auto a_exfunc = (AviUtl::ExFunc*)(GLOBAL::aviutl_base + OFS::AviUtl::exfunc);
        a_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)&bgr2yc_mt, by, (void*)vram_line_size);
    }
    void __cdecl yc_filter_effect_t::exfunc_bgr2yca_wrap(void* dst, void* src, int w, int h, int bitcount, int srcline) {
        if (rgb2yc_flag) {
            bgr2yc(dst, src, w, h, srcline);
        } else {
            auto e_exfunc = reinterpret_cast<ExEdit::Exfunc*>(GLOBAL::exedit_base + OFS::ExEdit::exfunc);
            e_exfunc->yc2rgb(dst, src, w, h, bitcount, srcline);
        }
    }
} // namespace patch::fast
#endif // ifdef PATCH_SWITCH_FAST_YC_FILTER_EFFECT
