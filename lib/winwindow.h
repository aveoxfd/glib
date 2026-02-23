#ifndef WINWINDOW_H
#define WINWINDOW_H

#define W_STATIC

#ifdef __cplusplus
extern "C"{
#endif

#define ERR_CODE -1

#if defined(W_STATIC)
  #define W_API
#elif defined(_WIN32) || defined(__CYGWIN__)
  #if defined(BUILDING_WINDOW)
    #define W_API __declspec(dllexport)
  #else
    #define W_API __declspec(dllimport)
  #endif
#elif defined(__GNUC__) && __GNUC__ >= 4
  #define W_API __attribute__((visibility("default")))
#else
  #define W_API
#endif

typedef struct Window Window;

W_API Window* WindowCreate(int w, int h);
W_API void WindowDestroy(Window* wnd);
W_API void ClearWindow(Window*, unsigned int color);
W_API void PutPixel(Window*, int x, int y, unsigned int color);
W_API int WindowProcessMessage(void);
W_API void WindowUpdate(Window*);
W_API void WSleep(int);
W_API unsigned int ARGB(unsigned char a, unsigned char r, unsigned char g, unsigned char b);
W_API int get_width(Window* wnd);
W_API int get_height(Window* wnd);
W_API void set_name(Window* wnd, const char *window_name);

typedef void (*WindowKeyCallback)(Window* wnd, int key, int pressed); // type
typedef void (*WindowMouseButtonCallback)(Window* wnd, int button, int pressed);
typedef void (*WindowMouseMoveCallback)(Window* wnd, int x, int y);

W_API void WindowSetKeyCallback(Window* wnd, WindowKeyCallback cb);
W_API void WindowSetMouseButtonCallback(Window* wnd, WindowMouseButtonCallback cb);
W_API void WindowSetMouseMoveCallback(Window* wnd, WindowMouseMoveCallback cb);


#ifdef __cplusplus
}
#endif
#endif