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

#include "patch_second_cache.hpp"

#ifdef PATCH_SWITCH_SECOND_CACHE
namespace patch {



#define SECOND_CACHE_NUM 512
    struct SecondCacheInfo { // size = 272 * SECOND_CACHE_NUM
        uint32_t priority;
        char name[260];
        uint16_t _padding;
        uint16_t w, h, bitcount;
    }scinfo[SECOND_CACHE_NUM];


	void* __cdecl second_cache_t::GetCache_end(LPCSTR name, int* w, int* h, int* bitcount) {
        for (int i = 0; i < SECOND_CACHE_NUM; i++) {
            if (lstrcmpiA(name, scinfo[i].name) == 0) {
                void* ptr = reinterpret_cast<void* (__cdecl*)(int, int, int, LPCSTR)>(GLOBAL::exedit_base + OFS::ExEdit::CreateCache)(scinfo[i].w, scinfo[i].h, scinfo[i].bitcount, name);
                if (ptr == NULL) {
                    return NULL;
                }
                AviUtl::ExFunc* a_exfunc = (AviUtl::ExFunc*)(GLOBAL::aviutl_base + OFS::AviUtl::exfunc);
                void* scptr = a_exfunc->get_shared_mem((int)&scinfo[i], 0, NULL);
                if (scptr == NULL) {
                    return NULL;
                }
                scinfo[i].priority = 0;
                scinfo[i].name[0] = '\0';
                *w = scinfo[i].w;
                *h = scinfo[i].h;
                *bitcount = scinfo[i].bitcount;
                int mem_size = (((*w * *bitcount + 7) >> 3) + 3 & 0xfffffffc) * *h + 16;
                memcpy(ptr, scptr, mem_size);
                a_exfunc->delete_shared_mem((int)&scinfo[i], NULL);
                return ptr;
            }
        }
        return NULL;
	}

    void second_cache_t::init_secondcache() {
        for (int i = 0; i < SECOND_CACHE_NUM; i++) {
            scinfo[i].name[0] = '\0';
            scinfo[i].priority = 0;
        }
    }
    int second_cache_t::get_secondcache_id(char* name) {
        int old_id = 0;
        int old_priority = scinfo[0].priority;
        for (int i = 0; i < SECOND_CACHE_NUM; i++) {
            if (lstrcmpiA(name, scinfo[i].name) == 0) {
                return i;
            }
            if (scinfo[i].priority < old_priority) {
                old_id = i;
                old_priority = scinfo[i].priority;
            }
        }
        return old_id;
    }

    int __stdcall second_cache_t::lstrcmpiA_wrapcef0(int cid, LPCSTR name, LPCSTR str_cachetemp) {
        AviUtl::ExFunc* a_exfunc = (AviUtl::ExFunc*)(GLOBAL::aviutl_base + OFS::AviUtl::exfunc);
        auto cacheinfo = (ExEdit::CacheInfo*)(GLOBAL::exedit_base + OFS::ExEdit::CacheInfo);
        int* cache_priority_cnt = (int*)(GLOBAL::exedit_base + OFS::ExEdit::CachePriorityCount);

        if (cacheinfo[cid].main.name[0] != '\0') {
            int scid = get_secondcache_id(cacheinfo[cid].main.name);
            int mem_size = (((cacheinfo[cid].main.w * cacheinfo[cid].main.bitcount + 7) >> 3) + 3 & 0xfffffffc) * cacheinfo[cid].main.h + 16;
            void* scptr = a_exfunc->create_shared_mem((int)&scinfo[scid], 0, mem_size, NULL);
            if (scptr != NULL) {
                scinfo[scid].priority = *cache_priority_cnt;
                lstrcpyA(scinfo[scid].name, cacheinfo[cid].main.name);
                scinfo[scid].w = cacheinfo[cid].main.w;
                scinfo[scid].h = cacheinfo[cid].main.h;
                scinfo[scid].bitcount = cacheinfo[cid].main.bitcount;
                memcpy(scptr, cacheinfo[cid].main.ptr, mem_size);
            }
        }
        for (int sub_id = 0; sub_id < cacheinfo[cid].sub_num; sub_id++) {
            if (cacheinfo[cid].sub[sub_id].name[0] != '\0') {
                int scid = get_secondcache_id(cacheinfo[cid].sub[sub_id].name);
                int mem_size = (((cacheinfo[cid].sub[sub_id].w * cacheinfo[cid].sub[sub_id].bitcount + 7) >> 3) + 3 & 0xfffffffc) * cacheinfo[cid].sub[sub_id].h + 16;
                void* scptr = a_exfunc->create_shared_mem((int)&scinfo[scid], 0, mem_size, NULL);
                if (scptr != NULL) {
                    scinfo[scid].priority = *cache_priority_cnt;
                    lstrcpyA(scinfo[scid].name, cacheinfo[cid].sub[sub_id].name);
                    scinfo[scid].w = cacheinfo[cid].sub[sub_id].w;
                    scinfo[scid].h = cacheinfo[cid].sub[sub_id].h;
                    scinfo[scid].bitcount = cacheinfo[cid].sub[sub_id].bitcount;
                    memcpy(scptr, cacheinfo[cid].sub[sub_id].ptr, mem_size);
                }
            }
        }

        return lstrcmpiA(name, str_cachetemp);
    }



    // scene obj



    void __cdecl second_cache_t::sceneobj_blend_wrap(ExEdit::FilterProcInfo* efpip, int dst_x, int dst_y, void* src, int src_x, int src_y, int w, int h, int alpha, int flag) {
        reinterpret_cast<void(__cdecl*)(void*, int, int, void*, int, int, int, int, int, int)>(GLOBAL::exedit_base + OFS::ExEdit::exfunc_44)(efpip->obj_edit, dst_x, dst_y, src, src_x, src_y, w, h, alpha, flag);
        if (efpip->flag & 0x1000) {
            i32 func_offset;
            if (efpip->flag & 0x100) {
                func_offset = OFS::ExEdit::GetOrCreateSceneBufYCA;
            } else {
                func_offset = OFS::ExEdit::GetOrCreateSceneBufYC;
            }
            // efpip->frame_edit = reinterpret_cast<void* (__cdecl*)(ExEdit::ObjectFilterIndex, int, int, int, int)>(GLOBAL::exedit_base + func_offset)(efpip->scene_ofi, efpip->scene_w, efpip->scene_h, efpip->video_func_idx, 1);; // flag 1:create cache if not found.
            efpip->frame_edit = reinterpret_cast<void* (__cdecl*)(ExEdit::ObjectFilterIndex, int, int, int, int)>(GLOBAL::exedit_base + func_offset)((ExEdit::ObjectFilterIndex)efpip->unknown6[1], efpip->scene_w, efpip->scene_h, efpip->unknown_camera_idx, 1); // flag 1:create cache if not found.
        }
    }

    /*

    BOOL __cdecl second_cache_t::video_func_main_wrap4cf00(ExEdit::FilterProcInfo* efpip, ExEdit::SubFilterProcInfo* sfpip, int end_layer, int frame, int subframe, int scene_idx, ExEdit::ObjectFilterIndex ofi) {
        int v_func_id = *(int*)(GLOBAL::exedit_base + OFS::ExEdit::VideoFuncCount);

        auto scene_setting = (ExEdit::SceneSetting*)(GLOBAL::exedit_base + OFS::ExEdit::SceneSetting);

        i32 func_offset;
        if (*(byte*)&scene_setting[scene_idx].flag & 2) {
            func_offset = OFS::ExEdit::GetOrCreateSceneBufYCA;
        } else {
            func_offset = OFS::ExEdit::GetOrCreateSceneBufYC;
        }

        BOOL ret = reinterpret_cast<BOOL (__cdecl*)(AviUtl::FilterPlugin*, ExEdit::SubFilterProcInfo*, int, int, int, int, ExEdit::ObjectFilterIndex)>(GLOBAL::exedit_base + OFS::ExEdit::video_func_main)(NULL, sfpip, end_layer, frame, subframe, scene_idx, ofi);
        
        sfpip->buf_edit = reinterpret_cast<void*(__cdecl*)(ExEdit::ObjectFilterIndex, int, int, int, int)>(GLOBAL::exedit_base + func_offset)(ofi, sfpip->w, sfpip->h, v_func_id, 0);
        
        
        if (0 < v_func_id) {
            efpip->frame_edit = reinterpret_cast<void* (__cdecl*)(ExEdit::ObjectFilterIndex, int, int, int, int)>(GLOBAL::exedit_base + func_offset)(ofi, efpip->scene_w, efpip->scene_h, v_func_id - 1, 0);
        }

        return ret;
    }


    /*
    
    BOOL __cdecl second_cache_t::video_func_main_wrap4cf00(AviUtl::FilterPlugin* fp, ExEdit::SubFilterProcInfo* sfpip, int end_layer, int frame, int subframe, int scene_idx, ExEdit::ObjectFilterIndex ofi) {
        BOOL ret = reinterpret_cast<BOOL (__cdecl*)(AviUtl::FilterPlugin*, ExEdit::SubFilterProcInfo*, int, int, int, int, ExEdit::ObjectFilterIndex)>(GLOBAL::exedit_base + OFS::ExEdit::video_func_main)(fp, sfpip, end_layer, frame, subframe, scene_idx, ofi);
        
        auto scene_setting = (ExEdit::SceneSetting*)(GLOBAL::exedit_base + OFS::ExEdit::SceneSetting);
        int* v_func_cnt = (int*)(GLOBAL::exedit_base + OFS::ExEdit::VideoFuncCount);
        
        int w,h,bitcount;
        if ((*(byte*)scene_setting[scene_idx].flag & 2)) {
            w = *(int*)(GLOBAL::exedit_base + OFS::ExEdit::exedit_buffer_line);
            h = *(int*)(GLOBAL::exedit_base + OFS::ExEdit::exedit_max_h_add8);
            bitcount = 0x40;
        } else {
            bitcount = 0x30;
            w = *(int*)(GLOBAL::exedit_base + OFS::ExEdit::exedit_YC_vram_w);
            h = *(int*)(GLOBAL::exedit_base + OFS::ExEdit::exedit_YC_vram_h) + 2;
        }
        BOOL tmp;
        sfpip->buf_edit = reinterpret_cast<void*(__cdecl*)(ExEdit::ObjectFilterIndex, int, int, int, int, BOOL*)>(GLOBAL::exedit_base + OFS::ExEdit::GetOrCreateCache)(ofi, w, h, bitcount, *v_func_cnt, &tmp);
        
        return ret;
    }

    
    */



} // namespace patch
#endif // ifdef PATCH_SWITCH_SECOND_CACHE
