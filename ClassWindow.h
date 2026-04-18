#ifndef CLASSWINDOW_H
#define CLASSWINDOW_H

#include "Widget.h"
#include "Include/nwind/nwind.h"
#include "structures/structs.h"
#include "glib.h"

GLIBAPI class ClassWindow{
    private:
    Window *window;
    Widget *root_widget; //main widget
    public:
    ClassWindow(const int width, const int height);
    ~ClassWindow();
    Widget *get_root_widget();
    void set_widget(Widget *widget);
};
typedef ClassWindow CWindow;
#endif