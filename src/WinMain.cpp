#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN

#include "glad.h"
#include "DebugRenderer.h"
#include "SampleRenderer.h"

#undef APIENTRY
#include <windows.h>
#include <iostream>

int WINAPI WinMain(HINSTANCE, HINSTANCE, PSTR, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

#if _DEBUG
#pragma comment(linker, "/subsystem:console")
int main(int argc, char** argv)
{
    return WinMain(GetModuleHandle(NULL), NULL, GetCommandLineA(), SW_SHOWDEFAULT);
}
#else
#pragma comment(linker, "/subsystem:windows")
#endif

#pragma comment(lib, "opengl32.lib")

#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define WGL_CONTEXT_FLAGS_ARB 0x2094
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126

typedef HGLRC(WINAPI* PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC, HGLRC, const int*);
typedef const char* (WINAPI* PFNWGLGETEXTENSIONSSTRINGEXTPROC)(void);
typedef BOOL(WINAPI* PFNWGLSWAPINTERVALEXTPROC)(int);
typedef int(WINAPI* PFNWGLGETSWAPINTERVALEXTPROC)(void);

Application* gApp = nullptr;
GLuint gVAO = GL_ZERO;

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    switch (iMsg)
    {
    case WM_CLOSE:
    {
        if (gApp)
        {
            gApp->Shutdown();
            delete gApp;
            gApp = nullptr;
            DestroyWindow(hwnd);
        }
        break;
    }

    case WM_DESTROY:
    {
        if (gVAO)
        {
            HDC hdc = GetDC(hwnd);
            HGLRC hglrc = wglGetCurrentContext();
            glBindVertexArray(0);
            glDeleteVertexArrays(1, &gVAO);
            gVAO = NULL;
            
            wglMakeCurrent(NULL, NULL);
            wglDeleteContext(hglrc);
            ReleaseDC(hwnd, hdc);
            PostQuitMessage(0);
        }
        else
        {
            std::cout << "Multiple destroy messages\n";
        }
        break;
    }

    case WM_PAINT:
    case WM_ERASEBKGND:
        return 0;
    }

    return DefWindowProc(hwnd, iMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
    gApp = new DebugRenderer();

    WNDCLASSEX wndClass;
    wndClass.cbSize = sizeof(WNDCLASSEX);
    wndClass.style = CS_VREDRAW | CS_HREDRAW;
    wndClass.lpfnWndProc = WndProc;
    wndClass.cbClsExtra = 0;
    wndClass.cbWndExtra = 0;
    wndClass.hInstance = hInstance;
    wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndClass.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wndClass.lpszMenuName = 0;
    wndClass.lpszClassName = L"Win32 Game Window";
    wndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    RegisterClassEx(&wndClass);

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    int clientWidth = 800;
    int clientHeight = 600;

    RECT windowRect;
    SetRect(&windowRect,
        (screenWidth / 2) - (clientWidth / 2),
        (screenHeight / 2) - (clientHeight / 2),
        (screenWidth / 2) + (clientWidth / 2),
        (screenHeight / 2) + (clientHeight / 2));

    DWORD style = (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX); //WS_THICKFRAME
    AdjustWindowRectEx(&windowRect, style, FALSE, 0);

    HWND hwnd = CreateWindowEx(0, wndClass.lpszClassName, L"Game Window", style, windowRect.left, windowRect.top, windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top, NULL, NULL, hInstance, szCmdLine);

    HDC hdc = GetDC(hwnd);

    PIXELFORMATDESCRIPTOR pfd;
    memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 32;
    pfd.cStencilBits = 8;
    pfd.iLayerType = PFD_MAIN_PLANE;
    int pixelFormat = ChoosePixelFormat(hdc, &pfd);
    SetPixelFormat(hdc, pixelFormat, &pfd);

    HGLRC tempRC = wglCreateContext(hdc);
    wglMakeCurrent(hdc, tempRC);

    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = NULL;
    wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");

    const int attribList[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
        WGL_CONTEXT_MINOR_VERSION_ARB, 6,
        WGL_CONTEXT_FLAGS_ARB, 0,
        WGL_CONTEXT_PROFILE_MASK_ARB,
        WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0};

    HGLRC hglrc = wglCreateContextAttribsARB(hdc, 0, attribList);
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(tempRC);
    wglMakeCurrent(hdc, hglrc);

    if (!gladLoadGL())
    {
        std::cout << "Could not initialize GLAD\n";
    }
    else
    {
        std::cout << "OpenGL Version: " << GLVersion.major << "." << GLVersion.minor << "\n";
    }

    // Query for extension support for vsync
    PFNWGLGETEXTENSIONSSTRINGEXTPROC wglGetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC)wglGetProcAddress("wglGetExtensionsStringEXT");
    bool swapControlSupported = strstr(wglGetExtensionsStringEXT(), "WGL_EXT_swap_control") != 0;

    int vsync = 0;
    if (swapControlSupported)
    {
        PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
        PFNWGLGETSWAPINTERVALEXTPROC wglGetSwapIntervalEXT = (PFNWGLGETSWAPINTERVALEXTPROC)wglGetProcAddress("wglGetSwapIntervalEXT");

        if (wglSwapIntervalEXT(1))
        {
            std::cout << "Enabled Vsync\n";
            vsync = wglGetSwapIntervalEXT();
        }
        else
        {
            std::cout << "Could not enable vsync\n";
        }
    }
    else
    {
        std::cout << "WGL_EXT_swap_control not supported\n";
    }

    glGenVertexArrays(1, &gVAO);
    glBindVertexArray(gVAO);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    gApp->Initialize();

    DWORD lastTick = GetTickCount();
    MSG msg;
    while (true)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                break;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);                    
        }

        DWORD thisTick = GetTickCount();
        float dt = float(thisTick - lastTick) * 0.001f;
        lastTick = thisTick;
        if (gApp)
        {
            // Update
            gApp->Update(dt);

            // Render
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            clientWidth = clientRect.right - clientRect.left;
            clientHeight = clientRect.bottom - clientRect.top;

            glViewport(0, 0, clientWidth, clientHeight);
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
            glPointSize(5.0f);

            glBindVertexArray(gVAO);
            glClearColor(0.5f, 0.6f, 0.7f, 1.0f);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

            float aspectRatio = float(clientWidth) / float(clientHeight);
            gApp->Render(aspectRatio);

            SwapBuffers(hdc);

            if (vsync != 0)
            {
                glFinish();
            }
        }
    } // end of game loop

    if (gApp)
    {
        std::cout << "Exiting application\n";
        gApp->Shutdown();
        delete gApp;
    }

    return (int)msg.wParam;
}