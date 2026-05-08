#ifndef CLASSWINDOW_H
#define CLASSWINDOW_H

#include "glib_api.h"
#include "structures/structs.h"
#include <windows.h>

class Widget;
typedef Window Window;

typedef void (*on_timer_function)(void*);

struct TIMER_ENTRY {
    UINT_PTR id;
    void *user_data;
    on_timer_function function;
};

struct TIMER_ENTRY_ARRTYPE {
    TIMER_ENTRY *array = nullptr;
    int array_count = 0;
};

GLIBAPI class ClassWindow{
    private:
    Window *window;
    Widget *root_widget; //main widget
    Widget *focused = nullptr;

    TIMER_ENTRY_ARRTYPE TE;

    public:
    ClassWindow(const int width, const int height);
    ~ClassWindow();
    Widget *get_root_widget();
    void set_widget(Widget *widget);
    position get_mouse_position(void);
    int get_mouse_button(void);
    int get_keyboard_key(void);
    void update(void);
    void start_cycle();
    void set_focus(Widget *widget);
    Widget *get_focused(void);
    void register_ontimer_function(on_timer_function function, void* user_data, UINT interval_ms, TIMERPROC other); //register on_timer_function
    void delete_ontimer_function(on_timer_function function);                                                       //delete on_timer_function
    on_timer_function find_timer_function(WPARAM wParam);
    TIMER_ENTRY* find_timer_function_TE(WPARAM wParam);
};
typedef ClassWindow CWindow;

CWindow *findwindow(Window* native); //find associated classwindow with window
#endif