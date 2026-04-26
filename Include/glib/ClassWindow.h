#ifndef CLASSWINDOW_H
#define CLASSWINDOW_H

#include "glib_api.h"
#include "structures/structs.h"

class Widget;
typedef Window Window;


GLIBAPI class ClassWindow{
    private:
    Window *window;
    Widget *root_widget; //main widget
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
};
typedef ClassWindow CWindow;

CWindow *findwindow(Window* native); //find associated classwindow with window
#endif