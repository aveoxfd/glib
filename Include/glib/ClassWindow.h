#ifndef CLASSWINDOW_H
#define CLASSWINDOW_H

#include "glib_api.h"
#include "structures/structs.h"

class Widget;
typedef Window Window;
typedef TIMER_ENTRY TIMER_ENTRY;
typedef TIMER_ENTRY_ARRTYPE TIMER_ENTRY_ARRTYPE;
typedef void (*on_timer_function)(void*);
//typedef on_timer_function on_timer_function;

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
    void regont_function(on_timer_function function, void* user_data, UINT interval_ms);    //register on_timer_function
    void delont_function(on_timer_function function);                                       //delete on_timer_function
    on_timer_function find_timer_function(WPARAM wParam);
    TIMER_ENTRY* find_timer_function_TE(WPARAM wParam);
};
typedef ClassWindow CWindow;

CWindow *findwindow(Window* native); //find associated classwindow with window
#endif