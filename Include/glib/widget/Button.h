#ifndef BUTTON_H
#define BUTTON_H

#include "Widget.h"

class Button : public Widget{
    using render_function = void(*)(Button *);
    using update_function = void(*)(Button *, void *);
    using event_function = void(*)(Button *, void *);


    event_function on_click_function = nullptr;
    void *user_data_on_click = nullptr;

    event_function inbound_func = nullptr;
    void *user_data_on_inbound = nullptr;
    event_function outbound_func = nullptr;
    void *user_data_on_outbound = nullptr;

    render_function render_func = nullptr;
    update_function update_func = nullptr;
    void *user_data_update = nullptr;

    public:
    Button(rect_t bound, Widget *parent = nullptr) : Widget(bound, parent){}

    void on_click(event_function e = nullptr, void *user_data = nullptr){
        on_click_function = e;
        user_data_on_click = user_data;
    }

    void set_render_function(render_function fn) { 
        render_func = fn;
    }

    void set_update_function(update_function fn, void *data = nullptr) {
        update_func      = fn;
        user_data_update = data;
    }

    void set_on_inbound_event(event_function fn, void *user_data = nullptr)  { 
        inbound_func  = fn; 
        user_data_on_inbound = user_data;
    }
    void set_on_outbound_event(event_function fn, void *user_data = nullptr) { 
        outbound_func = fn;
        user_data_on_outbound = user_data;
    }

    void render() override {
        if (render_func) render_func(this);
    }

    void update() override {
        if (update_func) update_func(this, user_data_update);
    }

    protected:
    void on_press(int button) override {
        if (button == 0 && on_click_function) on_click_function(this, user_data_on_click);
    }
    void on_inbound() override {
        if (inbound_func) inbound_func(this, user_data_on_inbound);
    }
    void on_outbound() override {
        if (outbound_func) outbound_func(this, user_data_on_outbound);
    }
};

#endif