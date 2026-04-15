#ifndef NWIND_H
#define NWIND_H

#define NWINDAPI

#if defined(DLLBUILD)
#if defined (_WIN32) || defined (_WIN64)
#define NWINDAPI __declspec(dllexport)
#endif
#endif

#if defined (DLLUSE)
#if defined (_WIN32) || defined (_WIN64)
#define NWINDAPI __declspec(dlimport)
#endif
#endif

#if defined (cplusplus) || defined (c_plusplus) || defined (__cplusplus)
extern "C" {
#endif

typedef struct Window Window, Window_t, Wnd, Wnd_t;

typedef void (*WindowKeyCallback)(Window *wnd, int key, char pressed);
typedef void (*WindowMouseButtonCallback)(Window *wnd, int key, char pressed);
typedef void (*WindowMouseMoveCallback)(Window *wnd, int key, char pressed);

NWINDAPI void WindowSetKeyCallback(Window* wnd, WindowKeyCallback cb_func);
NWINDAPI void WindowSetMouseButtonCallback(Window* wnd, WindowMouseButtonCallback cb_func);
NWINDAPI void WindowSetMouseMoveCallback(Window* wnd, WindowMouseMoveCallback cb_func);

NWINDAPI Window* WindowCreate(const int width, const int height, const char* window_name);
NWINDAPI void WindowDestroy(Window* wnd);
NWINDAPI unsigned char MessageProcess(void);
NWINDAPI void putpixel(Window* wnd, int x, int y, unsigned int color);

#if defined (cplusplus) || defined (c_plusplus) || defined (__cplusplus)
}
#endif

#endif