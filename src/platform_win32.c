#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include "platform.h"

static HWND       g_hwnd = NULL;
static BITMAPINFO g_bmi;
static uint32_t   g_pixels[CHIP8_WIDTH * CHIP8_HEIGHT];
static uint8_t    g_key_state[CHIP8_NUM_KEYS];
static bool       g_running = true;
static int        g_scale = 15;

/* CHIP-8 Hex Keypad Layout -> Modern QWERTY Mapping:
 *
 *   CHIP-8 Keypad        Keyboard Keys
 *   1  2  3  C           1  2  3  4
 *   4  5  6  D    ==>    Q  W  E  R
 *   7  8  9  E           A  S  D  F
 *   A  0  B  F           Z  X  C  V
 */
static int map_vk_to_chip8(WPARAM vk) {
    switch (vk) {
        case '1': return 0x1;
        case '2': return 0x2;
        case '3': return 0x3;
        case '4': return 0xC;

        case 'Q': return 0x4;
        case 'W': return 0x5;
        case 'E': return 0x6;
        case 'R': return 0xD;

        case 'A': return 0x7;
        case 'S': return 0x8;
        case 'D': return 0x9;
        case 'F': return 0xE;

        case 'Z': return 0xA;
        case 'X': return 0x0;
        case 'C': return 0xB;
        case 'V': return 0xF;
        default:  return -1;
    }
}

static LRESULT CALLBACK chip8_wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    switch (msg) {
        case WM_CLOSE:
        case WM_DESTROY:
            g_running = false;
            PostQuitMessage(0);
            return 0;

        case WM_KEYDOWN:
            if (wparam == VK_ESCAPE) {
                g_running = false;
                PostQuitMessage(0);
                return 0;
            }
            {
                int k = map_vk_to_chip8(wparam);
                if (k >= 0) g_key_state[k] = 1;
            }
            return 0;

        case WM_KEYUP:
            {
                int k = map_vk_to_chip8(wparam);
                if (k >= 0) g_key_state[k] = 0;
            }
            return 0;

        case WM_PAINT:
            {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hwnd, &ps);
                RECT rc;
                GetClientRect(hwnd, &rc);
                StretchDIBits(
                    hdc,
                    0, 0, rc.right - rc.left, rc.bottom - rc.top,
                    0, 0, CHIP8_WIDTH, CHIP8_HEIGHT,
                    g_pixels, &g_bmi, DIB_RGB_COLORS, SRCCOPY
                );
                EndPaint(hwnd, &ps);
            }
            return 0;

        default:
            return DefWindowProcA(hwnd, msg, wparam, lparam);
    }
}

bool platform_init(const char *title, int scale) {
    g_scale = (scale > 0) ? scale : 15;
    HINSTANCE hinst = GetModuleHandleA(NULL);

    WNDCLASSA wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.lpfnWndProc   = chip8_wndproc;
    wc.hInstance     = hinst;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = "Chip8EmulatorWindowClass";

    if (!RegisterClassA(&wc)) {
        return false;
    }

    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    RECT rect = { 0, 0, CHIP8_WIDTH * g_scale, CHIP8_HEIGHT * g_scale };
    AdjustWindowRect(&rect, style, FALSE);

    g_hwnd = CreateWindowExA(
        0,
        wc.lpszClassName,
        title,
        style,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left,
        rect.bottom - rect.top,
        NULL, NULL, hinst, NULL
    );

    if (!g_hwnd) {
        return false;
    }

    ZeroMemory(&g_bmi, sizeof(g_bmi));
    g_bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    g_bmi.bmiHeader.biWidth       = CHIP8_WIDTH;
    g_bmi.bmiHeader.biHeight      = -CHIP8_HEIGHT; /* Top-down DIB */
    g_bmi.bmiHeader.biPlanes      = 1;
    g_bmi.bmiHeader.biBitCount    = 32;
    g_bmi.bmiHeader.biCompression = BI_RGB;

    ShowWindow(g_hwnd, SW_SHOW);
    UpdateWindow(g_hwnd);
    return true;
}

bool platform_process_input(uint8_t keys[CHIP8_NUM_KEYS]) {
    MSG msg;
    while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            g_running = false;
        }
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    for (int i = 0; i < CHIP8_NUM_KEYS; i++) {
        keys[i] = g_key_state[i];
    }
    return g_running;
}

void platform_render(const uint8_t display[CHIP8_WIDTH * CHIP8_HEIGHT]) {
    const uint32_t COLOR_ON  = 0xFF89B4FAu; /* Soft cyan-blue foreground */
    const uint32_t COLOR_OFF = 0xFF1E1E2Eu; /* Dark slate background */

    for (int i = 0; i < CHIP8_WIDTH * CHIP8_HEIGHT; i++) {
        g_pixels[i] = display[i] ? COLOR_ON : COLOR_OFF;
    }

    HDC hdc = GetDC(g_hwnd);
    if (hdc) {
        RECT rc;
        GetClientRect(g_hwnd, &rc);
        StretchDIBits(
            hdc,
            0, 0, rc.right - rc.left, rc.bottom - rc.top,
            0, 0, CHIP8_WIDTH, CHIP8_HEIGHT,
            g_pixels, &g_bmi, DIB_RGB_COLORS, SRCCOPY
        );
        ReleaseDC(g_hwnd, hdc);
    }
}

void platform_beep(void) {
    MessageBeep(MB_OK);
}

uint32_t platform_get_ticks(void) {
    return (uint32_t)GetTickCount();
}

void platform_sleep(uint32_t ms) {
    Sleep(ms);
}

void platform_shutdown(void) {
    if (g_hwnd) {
        DestroyWindow(g_hwnd);
        g_hwnd = NULL;
    }
}
