#pragma once

#include <algorithm>
#include <cwchar>
#include <cwctype>
#include <string>
#include <windows.h>

namespace injector {
    namespace utils {

    inline std::wstring Trim(const std::wstring& value) {
        const auto first = value.find_first_not_of(L" \t\r\n");
        if (first == std::wstring::npos) {
            return L"";
        }
        const auto last = value.find_last_not_of(L" \t\r\n");
        return value.substr(first, last - first + 1);
    }

    inline std::wstring ToLower(const std::wstring& value) {
        std::wstring result = value;
        std::transform(result.begin(), result.end(), result.begin(), [](wchar_t c) {
            return static_cast<wchar_t>(std::towlower(c));
        });
        return result;
    }

    inline bool TryParseProcessId(const std::wstring& text, DWORD& processId) {
        if (text.empty()) {
            return false;
        }

        for (wchar_t ch : text) {
            if (!std::iswdigit(static_cast<unsigned short>(ch))) {
                return false;
            }
        }

        const unsigned long value = std::wcstoul(text.c_str(), nullptr, 10);
        if (value == 0) {
            return false;
        }

        processId = static_cast<DWORD>(value);
        return true;
    }

    inline std::wstring ExtractFileName(const std::wstring& path) {
        const size_t pos = path.find_last_of(L"\\/");
        if (pos == std::wstring::npos) {
            return path;
        }
        return path.substr(pos + 1);
    }
    } 
}