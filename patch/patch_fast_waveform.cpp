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

#include "patch_fast_waveform.hpp"
#ifdef PATCH_SWITCH_FAST_WAVEFORM


//#define PATCH_STOPWATCH

namespace patch::fast {

    void __cdecl Waveform_t::normal_wrap(wf_var* wf, ExEdit::FilterProcInfo* efpip) {
        short* audio_data = (short*)wf->audio_data;
        for (int i = wf->audio_n; 0 < i; i--) {
            audio_data[i] = audio_data[i - 1] * wf->res_h / 65536 + wf->res_h / 2;
        }
        audio_data[0] = audio_data[1];
        audio_data[wf->audio_n + 1] = audio_data[wf->audio_n + 2] = audio_data[wf->audio_n];
        auto a_exfunc = reinterpret_cast<AviUtl::ExFunc*>(GLOBAL::aviutl_base + OFS::AviUtl::exfunc);
        a_exfunc->exec_multi_thread_func(&mt, wf, efpip);
    }
    void __cdecl Waveform_t::mt(int thread_id, int thread_num, void* param1, void* param2) {
        auto wf = static_cast<wf_var*>(param1);
        auto efpip = static_cast<ExEdit::FilterProcInfo*>(param2);
        short* audio_data = (short*)wf->audio_data;
        short* temp_data = audio_data + wf->audio_n + 3 + wf->res_h * thread_id;
        double rate = (double)wf->obj_w / (double)wf->audio_n;

        int x_end = wf->audio_n * wf->obj_w / wf->audio_n;
        int x_begin = x_end * thread_id / thread_num;
        x_end = x_end * (thread_id + 1) / thread_num;

        int fra_w = 0;
        int x = 0;
        int p;
        for (p = 0; p < wf->audio_n && x < x_begin; p++) {
            if (0x1000 < fra_w) {
                x++;
                fra_w -= 0x1000;
            } else {
                double xf = (double)p * rate;
                x = (int)xf;
                fra_w = wf->sample_w + (int)round((xf - (double)x) * 4096.0);
            }
            if (0x1000 < fra_w) {
                p--;
            }
        }

        for (; p < wf->audio_n && x < x_end; p++) {
            int pp = p;
            int w;
            if (0x1000 < fra_w) {
                x++;
                fra_w -= 0x1000;
                w = fra_w;
            } else {
                double xf = (double)p * rate;
                x = (int)xf;
                w = wf->sample_w;
                fra_w = w + (int)round((xf - (double)x) * 4096.0);
            }
            if (0x1000 < fra_w) {
                w += 0x1000 - fra_w;
                p--;
            }
            if (0 <= calc_pos(x, wf->obj_w, wf->res_w, wf->pad_w)) {
                memset(temp_data, 0, wf->res_h * sizeof(short));
                short* ptr;
                int step;
                int asub = audio_data[pp] - audio_data[pp + 1];
                if (asub != 0) {
                    step = (int)(0 <= asub) * 2 - 1;
                    /*
                    if(0 < asub){
                      step = 1;
                    } else {
                      step = -1;
                    }
                    */
                    asub += step;
                    ptr = temp_data + audio_data[pp + 1] + step;
                    for (int i = step; i != asub; i += step) {
                        *ptr += w - w * i / asub;
                        ptr += step;
                    }
                }
                asub = audio_data[pp + 2] - audio_data[pp + 1];
                step = (int)(0 <= asub) * 2 - 1;
                asub += step;
                ptr = temp_data + audio_data[pp + 1];
                for (int i = 0; i != asub; i += step) {
                    *ptr += w;
                    ptr += step;
                }
                asub = audio_data[pp + 3] - audio_data[pp + 2];
                if (asub != 0) {
                    step = (int)(0 <= asub) * 2 - 1;
                    asub += step;
                    ptr = temp_data + audio_data[pp + 2] + step;
                    for (int i = step; i != asub; i += step) {
                        *ptr += w - w * i / asub;
                        ptr += step;
                    }
                }
                short* edit_a = &((ExEdit::PixelYCA*)efpip->obj_edit + x)->a;
                for (int y = 0; y < wf->obj_h; y++) {
                    int ofs = calc_pos(y, wf->obj_h, wf->res_h, wf->pad_h);
                    if (0 <= ofs) {
                        *edit_a += temp_data[ofs];
                    }
                    edit_a += efpip->obj_line * 4;
                }
            }
        }
    }

} // namespace patch::fast
#endif // ifdef PATCH_SWITCH_FAST_WAVEFORM
