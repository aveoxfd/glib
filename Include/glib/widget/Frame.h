#ifndef FRAME_H
#define FRAME_H

#include "Widget.h"
class Frame : public Widget {
    public:
    Frame(rect_t bound, Widget *parent = nullptr): Widget(bound, parent){}
    using render_function = void(*)(Frame *); //background

    render_function render_func;

    void set_render_function(render_function function){
        render_func = function;
    }

    void render() override {
        if (render_func)render_func(this);
    }
};

#endif