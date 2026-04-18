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

Widget::~Widget() {
    for (int i = 0; i < children_count; ++i) {
        delete children[i];
    }
    delete[] children;
}

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

void Widget::add_child(Widget *child_widget){
    if (!child_widget)return;

    for (Widget* curr = this; curr; curr = curr->parent){ //if child is a parent
        if (curr == child_widget)return;
    }

    if (child_widget->parent){
        child_widget->parent->remove_child(child_widget);
    }

    Widget **temp = new Widget* [++children_count];

    for (int i = 0; i < children_count - 1; ++i){
        temp[i] = children[i];
    }

    temp[children_count - 1] = child_widget;
    delete[] children;

    children = temp;
    child_widget->parent = this;
    child_widget->set_associated_window(this->association); //child_widget->association = this->association; //

    return;
}

void Widget::remove_child(Widget* child_widget){ //removes a child element from the list of child elements of the current parent, which uses child_widget
    if (!child_widget || child_widget->parent != this) return;

    int index = -1;
    for (int i = 0; i < children_count; ++i){
        if (children[i] == child_widget){
            index = i;
            break;
        }
    }
    if(index == -1)return;

    Widget **temp = nullptr;

    if (children_count > 1){
        temp = new Widget* [children_count - 1];
        for (int i = 0, j = 0; i<children_count; i++){
            if (i != index) temp[j++] = children[i]; 
        }
    }

    delete[] children;

    children = temp;

    --children_count;

    child_widget->parent = nullptr;

    child_widget->set_associated_window(nullptr);

    //delete child_widget
    return;
}