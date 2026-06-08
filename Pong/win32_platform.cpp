#include <windows.h>
#include "utils.h"

struct Render_State
{
    int width, height;
    void* memory;
    BITMAPINFO bitmap_info;
};

global Render_State render_state;
global bool running = true;

#include "renderer.cpp"
#include "platform_common.cpp"
#include "game.cpp"
global Bitmap font;
LRESULT CALLBACK Wndproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;
    switch (uMsg)
    {
    case WM_CLOSE:
    case WM_DESTROY:
    {
        running = false;
       
    } break;

    case WM_SIZE:
    {
        RECT rect;
        GetClientRect(hwnd, &rect);
        render_state.width = rect.right - rect.left;
        render_state.height = rect.bottom - rect.top;

        int render_state_size = render_state.width * render_state.height * sizeof(unsigned int);
        if (render_state.memory) VirtualFree(render_state.memory, 0, MEM_RELEASE);
        render_state.memory = VirtualAlloc(0, render_state_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

        render_state.bitmap_info.bmiHeader.biSize = sizeof(render_state.bitmap_info.bmiHeader);
        render_state.bitmap_info.bmiHeader.biWidth = render_state.width;
        render_state.bitmap_info.bmiHeader.biHeight = render_state.height;
        render_state.bitmap_info.bmiHeader.biBitCount = 32;
        render_state.bitmap_info.bmiHeader.biCompression = BI_RGB;
        render_state.bitmap_info.bmiHeader.biPlanes = 1;

    }
    break;
    default:
        result = DefWindowProc(hwnd, uMsg, wParam, lParam);

    }
    return result;
}

int WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nShowCmd)
{
    WNDCLASSW window_class = {};

    window_class.style = CS_HREDRAW | CS_VREDRAW;
    window_class.lpszClassName = L"Game Window Class";
    window_class.lpfnWndProc = Wndproc;
    window_class.hInstance = hInstance;

    RegisterClassW(&window_class);

    HWND window = CreateWindowW(
        window_class.lpszClassName,
        L"Pong by Vandan",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        1280,
        720,
        0,
        0,
        hInstance,
        0
    );
    
    
    HDC hdc = GetDC(window);
     font = load_bitmap("assets/thick_8x8.bmp");
    
     
    Input input = {};

    float delta_time = 0.016666f;
    LARGE_INTEGER frame_begin_time;
    QueryPerformanceCounter(&frame_begin_time);

    float performance_frequency;
    {
        LARGE_INTEGER perf;
        QueryPerformanceFrequency(&perf);
        performance_frequency = (float)perf.QuadPart;
    }
    while (running)
    {
        // Input
        MSG message;

        for (int i = 0; i < BUTTON_COUNT; i++)
        {
            input.buttons[i].changed = false;
        }

        while (PeekMessage(&message, window, 0, 0, PM_REMOVE))
        {
            switch (message.message)
            {
            case WM_KEYUP:
            case WM_KEYDOWN:
            {
                u32 vk_code = (u32)message.wParam;
                bool is_down = (message.lParam & (1 << 31)) == 0;

#define process_button(b, vk) \
                case vk: \
                { \
                    input.buttons[b].changed = is_down!= input.buttons[b].is_down; \
                    input.buttons[b].is_down = is_down; \
                } break;

                switch (vk_code)
                {
                    process_button(BUTTON_UP, VK_UP);
                    process_button(BUTTON_DOWN, VK_DOWN);
                    process_button(BUTTON_SPACE, VK_SPACE);

                }
            } break;

            default:
            {
                TranslateMessage(&message);
                DispatchMessage(&message);
            } break;
            }
        }

        if (!render_state.memory)
            continue;

        clear_screen(0xffa500);

        if (input.buttons[BUTTON_UP].is_down)
            draw_rect(0, 0, 5, 10, 0xffff00);

        //draw_rect(5, 1, 5, 3, 0xff0000);
        //draw_rect(50, 10, 5, 30, 0xffffff);

        // Simulate
        simulate_game(&input, delta_time);
        // Render
        
        StretchDIBits(
            hdc,
            0,
            0,
            render_state.width,
            render_state.height,
            0,
            0,
            render_state.width,
            render_state.height,
            render_state.memory,
            &render_state.bitmap_info,
            DIB_RGB_COLORS,
            SRCCOPY
        );
        LARGE_INTEGER frame_end_time;
        QueryPerformanceCounter(&frame_end_time);
        delta_time = (float)(frame_end_time.QuadPart - frame_begin_time.QuadPart) / performance_frequency;
        frame_begin_time = frame_end_time;
    }
    return 0;
}