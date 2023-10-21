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

#include "patch_fast_audio_spectrum.hpp"
#include <immintrin.h>
#ifdef PATCH_SWITCH_FAST_AUDIO_SPECTRUM


//#define PATCH_STOPWATCH

namespace patch::fast {
    void __cdecl AudioSpectrum_t::fft_wrap() {
        // reinterpret_cast<void(__cdecl*)()>(GLOBAL::exedit_base + 0x8dc20)(); original_func
        for (int i = 0; i < 1000; i++) {
            int n = 0;
            auto a_exfunc = (AviUtl::ExFunc*)(GLOBAL::aviutl_base + OFS::AviUtl::exfunc);
            double* data = (double*)a_exfunc->get_shared_mem((int)&smi, 0, smi);
            if (data == nullptr) {
                data = (double*)a_exfunc->create_shared_mem((int)&smi, 0, 0x400 * 2 * sizeof(double), &smi);
                if (data == nullptr) {
                    reinterpret_cast<void(__cdecl*)()>(GLOBAL::exedit_base + 0x8dc20)();
                    return;
                }
                n = 0;

                for (int i = 0; i < 0x400 * 2; i++) {
                    double r = n * 0.003067961575771282;
                    data[i] = sin(r);
                    i++;
                    data[i] = cos(r);
                    int bit = 0x200;
                    n ^= 0x200;
                    while (n < bit) {
                        bit >>= 1;
                        n ^= bit;
                    }
                }
            }
            AudioSpectrum_var* as = reinterpret_cast<AudioSpectrum_var*>(GLOBAL::exedit_base + 0x244e30);
            n = 0;
            for (int i = 1; i < 2047; i++) {
                int bit = 0x400;
                n ^= 0x400;
                while (n < bit) {
                    bit >>= 1;
                    n ^= bit;
                }
                if (i < n) {
                    std::swap(as->buf4[i], as->buf4[n]);
                    // std::swap(as->buf3[i], as->buf3[n]);
                }
            }
            memset(as->buf3, 0, 2048 * sizeof(double));
            for (int i = 1; i <= 0x400; i <<= 1) {
                for (int j = n = 0; j < 0x800; j += i * 2) {
                    double dsin = data[n];
                    n++;
                    double dcos = data[n];
                    n++;
                    for (int k = j; k < i + j; k++) {
                        double d1 = as->buf4[k] - as->buf4[k + i];
                        double d2 = as->buf3[k] - as->buf3[k + i];
                        as->buf4[k] += as->buf4[k + i];
                        as->buf3[k] += as->buf3[k + i];
                        as->buf4[k + i] = dcos * d1 - dsin * d2;
                        as->buf3[k + i] = dsin * d1 + dcos * d2;
                    }
                }
            }
        }
    }

} // namespace patch::fast
#endif // ifdef PATCH_SWITCH_FAST_AUDIO_SPECTRUM
