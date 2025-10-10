#include <stdlib.h>
#include <windows.h>
#include <stdio.h>

// Global hook handle
HHOOK keyboardHook;

// hook procedure
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            KBDLLHOOKSTRUCT* p = (KBDLLHOOKSTRUCT*)lParam;
            char keyName[50] = { 0 };
            int isSpecialKey = 1; // Flag to handle non-printable keys

            // Manually handling common non-printable keys (special keys)
            switch (p->vkCode) {
            case VK_BACK:   strcpy(keyName, "[BACKSPACE]"); break;
            case VK_RETURN: strcpy(keyName, "[ENTER]"); break;
            case VK_TAB:    strcpy(keyName, "[TAB]"); break;
            case VK_SHIFT:
            case VK_LSHIFT:
            case VK_RSHIFT: strcpy(keyName, "[SHIFT]"); break;
            case VK_CONTROL:
            case VK_LCONTROL:
            case VK_RCONTROL: strcpy(keyName, "[CTRL]"); break;
            case VK_MENU:
            case VK_LMENU:
            case VK_RMENU:  strcpy(keyName, "[ALT]"); break;
            case VK_LWIN:
            case VK_RWIN:   strcpy(keyName, "[WINDOWS]"); break;
            case VK_CAPITAL:strcpy(keyName, "[CAPS LOCK]"); break;
            case VK_ESCAPE: strcpy(keyName, "[ESCAPE]"); break;
            default:
                isSpecialKey = 0;
                break;
            }

            if (isSpecialKey) {
                //printf("%s\n", keyName);
                printf("vkCode: 0x%02lX, Name: %s\n", p->vkCode, keyName);
            }
            else {
                // Try to translate to unicode character
                BYTE keyboardState[256];
                if (GetKeyboardState(keyboardState) == FALSE) {
                    fprintf(stderr, "Error: Failed to get keyboard state. GetLastError = %lu\n", GetLastError());
                    return EXIT_FAILURE;
                }
                WCHAR unicodeChar[5];
                int result = ToUnicode(p->vkCode, p->scanCode, keyboardState, unicodeChar, _countof(unicodeChar), 0);

                if (result > 0) {
                    printf("vkCode: 0x%02lX, Translated: '%ls'\n", p->vkCode, unicodeChar);
                }
                else {
                    // Fallback to GetKeyNameText for other keys (like F1-F12, Home, etc.)
                    // Bit 24 indicates an extended key, such as the right-hand ALT and CTRL keys.
                    LONG lParam_for_getkey = (p->scanCode << 16) | ((p->flags & LLKHF_EXTENDED) << 24);
                    GetKeyNameTextA(lParam_for_getkey, keyName, sizeof(keyName));
                    printf("vkCode: 0x%02lX, Name: [%s]\n", p->vkCode, keyName);
                }
            }
        }
    }
    return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
}

int main() {
    keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle(NULL), 0);
    if (!keyboardHook) {
        fprintf(stderr, "Error: Failed to set hook. GetLastError = %lu\n", GetLastError());
        return EXIT_FAILURE;
    }
    printf("Hook set successfully. Press keys to see output. Press Ctrl+C in this console to exit.\n");
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    UnhookWindowsHookEx(keyboardHook);
    return EXIT_SUCCESS;
}
