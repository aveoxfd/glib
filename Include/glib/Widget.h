#ifndef WIDGET_H
#define WIDGET_H

#include "glib.h"
#include "Event.h"
#include "Widget.h"

class ClassWindow;
//typedef struct settings settings_t;

GLIBAPI class Widget{
    private:
    typedef void(*render_function)(Widget *widget);
    typedef void(*update_function)(Widget *widget, void *user_data);
    Widget *parent;
    Widget **children;
    int children_count = 0;
    Window *association;
    rect_t bound;
    Event *onclick_event;
    Event *inbound_event;
    Event *outbound_event;
    render_function render_func;
    update_function update_func;
    void *user_data;
    char render_function_type; //0 - pointer; 1 - virtual
    char update_function_type; //0 - pointer; 1 - virtual
    friend position get_real_position(Widget *widget);
    bool contains(position pos);

    public:
    Widget(rect_t bound);
    Widget(rect_t bound, Widget *parent);
    virtual ~Widget();
    rect_t get_rect();
    void on_click(Event *e);
    void in_bound(Event *e);
    void out_bound(Event *e);
    Window* get_associated_window(void); //associated
    void set_associated_window(Window *association);
    void set_render_function(render_function function);
    void set_update_function(update_function function, void *user_data = nullptr);
    void virtual virtual_render_function();
    void virtual virtual_update_function();
    void use_virtual_render_function(){render_function_type = 1;};
    void use_pointer_render_function(){render_function_type = 0;};
    void use_virtual_update_function(){update_function_type = 1;};
    void use_pointer_update_function(){update_function_type = 0;};
    void mouse_press_handler(int button);
    void mouse_inbound_handler();
    void mouse_outbound_handler();
    Widget* find_widget(position pos);
    void render();
    void update();
    void set_position(position_t new_position){bound.pos = new_position;};
    void add_child(Widget *child_widget);
    void remove_child(Widget *child_widget);
};

#endif