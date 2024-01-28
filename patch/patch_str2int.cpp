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

#include "patch_str2int.hpp"

#ifdef PATCH_SWITCH_STR2INT
namespace patch {

    char* skip_space_plus(char* str) {
        while (*str != '\0') {
            if (IsDBCSLeadByte(*str)) {
                uint16_t beshort = str[0] << 8 | str[1];
                if (beshort != '　' && beshort != '＋') {
                    return str;
                }
                str++;
            } else {
                if (*str != ' ' && *str != '+') {
                    return str;
                }
            }
            str++;
        }
        return str;
    }
    void full2halfchar_numeric(char* str) {
        char* ptr = str;
        while (*str != '\0') {
            if (IsDBCSLeadByte(*str)) {
                char halfchar;
                uint16_t beshort = str[0] << 8 | str[1];
                switch (beshort) {
                case '－':
                    halfchar = '-';
                    break;
                case '．':
                    halfchar = '.';
                    break;
                case '０':case '１':case '２':case '３':case '４':case '５':case '６':case '７':case '８':case '９':
                    halfchar = beshort - '０' + '0';
                    break;
                default:
                    *ptr = '\0';
                    return;
                }
                *ptr = halfchar;
                str++;
            } else {
                switch (*str) {
                case '-':case '.':case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                    break;
                default:
                    *ptr = '\0';
                    return;
                }
                *ptr = *str;
            }
            ptr++;
            str++;
        }
        *ptr = '\0';
    }

    int __cdecl str2int_t::str2int_wrap(char* str) {
        str = skip_space_plus(str);
        full2halfchar_numeric(str);
        return reinterpret_cast<int(__cdecl*)(char*)>(GLOBAL::exedit_base + 0x91820)(str);
    }

} // namespace patch
#endif // ifdef PATCH_STR2INT