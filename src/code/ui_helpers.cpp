#include "ui_helpers.h"
#include <commdlg.h>

namespace ui {

std::wstring GetControlText(HWND control) {
    if (!control) {
        return L"";
    }

    const int length = GetWindowTextLengthW(control);
    if (length <= 0) {
        return L"";
    }

    std::wstring buffer(static_cast<size_t>(length) + 1, L'\0');
    const int written = GetWindowTextW(control, buffer.data(), length + 1);
    if (written <= 0) {
        return L"";
    }

    buffer.resize(static_cast<size_t>(written));
    return buffer;
}

void AppendLog(HWND logEdit, const std::wstring& message) {
    if (!logEdit) {
        return;
    }

    const int textLength = GetWindowTextLengthW(logEdit);
    SendMessageW(logEdit, EM_SETSEL, textLength, textLength);

    std::wstring line = message;
    if (!line.empty() && line.back() != L'\n') {
        line.append(L"\r\n");
    } else if (line.empty()) {
        line.assign(L"\r\n");
    }

    SendMessageW(logEdit, EM_REPLACESEL, FALSE, reinterpret_cast<LPARAM>(line.c_str()));
    SendMessageW(logEdit, EM_SCROLLCARET, 0, 0);
}

bool IsRunningAsAdmin() {
    BOOL isAdmin = FALSE;
    PSID adminGroup = nullptr;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;

    if (AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminGroup)) {
        if (!CheckTokenMembership(nullptr, adminGroup, &isAdmin)) {
            isAdmin = FALSE;
        }
    }

    if (adminGroup) {
        FreeSid(adminGroup);
    }

    return isAdmin == TRUE;
}

std::wstring OpenDllFileDialog(HWND owner) {
    wchar_t fileBuffer[MAX_PATH] = L"";
    OPENFILENAMEW ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = owner;
    ofn.lpstrFilter = L"Dynamic libraries (*.dll)\0*.dll\0All files (*.*)\0*.*\0";
    ofn.lpstrFile = fileBuffer;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER;
    ofn.lpstrDefExt = L"dll";

    if (GetOpenFileNameW(&ofn)) {
        return fileBuffer;
    }

    return L"";
}

} 
