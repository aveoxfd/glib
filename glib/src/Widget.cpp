#include "Widget.h"
#include "../../include/nwind/nwind.h"

position get_real_position(Widget *widget);
position get_real_position(Widget *widget){
    position real_position = widget->bound.pos;
    for (Widget *curr = widget; curr->parent != nullptr; curr = curr->parent){
        real_position.x += curr->bound.pos.x;
        real_position.y += curr->bound.pos.y;
    }
    return real_position;
}

bool Widget::contains(position pos){
    position real = get_real_position(this);
    return pos.x >= real.x && pos.x < bound.size.width + real.x 
        && pos.y >= real.y && pos.y < bound.size.height + real.y;
}

Widget::Widget(rect_t rectangle_bound):
parent(nullptr), 
children(nullptr), 
association(nullptr),
bound(rectangle_bound), 
onclick_event(nullptr), 
inbound_event(nullptr), 
outbound_event(nullptr), 
render_func(nullptr)
{
    //none
}

Widget::~Widget(){}

rect_t Widget::get_rect(){
    return bound;
}

void Widget::on_click(Event *event){
    if (event)onclick_event = event;
}

void Widget::in_bound(Event *event){
    if(event)inbound_event = event;
}

void Widget::out_bound(Event *event){
    if(event)outbound_event = event;
}

Window* Widget::get_associated_window(){
    if (association)return association;
    return nullptr;
}

void Widget::set_associated_window(Window *association){
    if (association)this->association = association;
}

void Widget::set_render_function(render_function function){
    if (function)render_func = function;
}

void Widget::mouse_press_handler(int button){
    if (onclick_event && button == 0){
        onclick_event->activate(this);
    } 
}

void Widget::mouse_inbound_handler(){
    if (inbound_event){
        inbound_event->activate(this);
    }
}

void Widget::mouse_outbound_handler(){
    if (outbound_event)outbound_event->activate(this);
}

void Widget::render(){
    if (render_func)render_func(this);
}

Widget* Widget::find_widget(position pos){
    if (!contains(pos)){
        return nullptr;
    }
    for (int i = children_count - 1; i >= 0; --i){
        Widget* found = children[i]->find_widget(pos);
        if (found) return found;
    }
    return this;
}