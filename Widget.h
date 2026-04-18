#ifndef WIDGET_H
#define WIDGET_H

#include "ClassWindow.h"
#include "structs.h"
#include "Event.h"
#include "glib.h"

GLIBAPI class Widget{
    private:
    Widget *parent;
    Widget **children;
    int children_count = 0;
    Window *association;
    friend position get_real_position(Widget *widget);
    typedef void(*render_function)(Widget *widget);
    rect_t bound;
    Event *onclick_event;
    Event *inbound_event;
    Event *outbound_event;
    render_function render_func;
    bool contains(position pos);

    public:
    Widget(rect_t bound);
    ~Widget(){};
    rect_t get_rect();
    void on_click(Event *e);
    void in_bound(Event *e);
    void out_bound(Event *e);
    Window* get_associated_window(void); //associated
    void set_associated_window(Window *association);
    void set_render_function(render_function function);
    void mouse_press_handler(int button);
    void mouse_inbound_handler();
    void mouse_outbound_handler();
    Widget* find_widget(position pos);
    void render();

};

#endif