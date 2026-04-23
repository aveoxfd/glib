#include "Widget.h"
#include "../../include/nwind/nwind.h"
#include <iostream>
#include <new>

#define POINTER 0
#define VIRTUAL 1

position get_real_position(Widget *widget);

position get_real_position(Widget *widget){
    position real_position = widget->bound.pos;
    for (Widget *curr = widget->parent; curr != nullptr; curr = curr->parent){
        real_position.x += curr->bound.pos.x;
        real_position.y += curr->bound.pos.y;
    }
    return real_position;
}

bool Widget::contains(position pos){
    position real = get_real_position(this); //real postion of widget in depended window;
    return pos.x >= real.x && pos.x < bound.size.width + real.x 
        && pos.y >= real.y && pos.y < bound.size.height + real.y;
}

void Widget::update(){
    switch(update_function_type){
        case POINTER:
        if (update_func)update_func(this, user_data);
        break;

        case VIRTUAL:
        virtual_update_function();
        break;
    }
    return;
}

void Widget::set_update_function(update_function function, void *user_data){
    this->user_data = user_data;
    if (function)update_func = function;
}

Widget::Widget(rect_t rectangle_bound, Widget *parent):
parent(parent), 
children(nullptr),
children_count(0),
association(nullptr),
bound(rectangle_bound), 
onclick_event(nullptr), 
inbound_event(nullptr), 
outbound_event(nullptr),
render_func(nullptr),
update_func(nullptr),
user_data(nullptr){
    use_pointer_render_function();
    use_pointer_update_function();
    if (this->parent)parent->add_child(this);
}

Widget::~Widget() {
    //for (int i = 0; i < children_count; ++i) {
    //    delete children[i];
    //}
    //delete[] children;
    //delete onclick_event;
    //delete inbound_event;
    //delete outbound_event;
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
    this->association = association;

    for (int i = 0; i < children_count; ++i){
        children[i]->set_associated_window(association);
    }
}

void Widget::set_render_function(render_function function){
    if (function)render_func = function;
}

void Widget::mouse_press_handler(int button){
    if (onclick_event && button == 0){ //left button
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
    switch(render_function_type){
        case POINTER:
        if (render_func)render_func(this);
        break;
        case VIRTUAL:
        virtual_render_function();
        break;

        default:
        break;
    }
}

void Widget::virtual_update_function(){}

void Widget::virtual_render_function(){}

Widget* Widget::find_widget(position pos){
    if (!contains(pos)){
        #ifdef DEBUG
        std::cout<<"Not found.\n";
        #endif
        return nullptr;
    }
    for (int i = children_count - 1; i >= 0; --i){ //for (int i = 0; i < children_count - 1; ++i)
        Widget* found = children[i]->find_widget(pos);

        #ifdef DEBUG
        std::cout<<"Found: "<<found<<"\n";
        #endif
        if (found) return found;
    }
    #ifdef DEBUG
    std::cout<<"It's root_widget.\n";
    #endif
    return this;
}

void Widget::add_child(Widget *child_widget){ //!!!
    if (!child_widget)return;

    for (Widget* curr = this; curr; curr = curr->parent){ //if child is a parent
        if (curr == child_widget)return;
    }

    if (child_widget->parent){
        child_widget->parent->remove_child(child_widget);
    }
    Widget **temp = nullptr;
    try{
       temp = new Widget* [++children_count]; // gives throw
    } catch(std::bad_alloc&){
        --children_count;
        return;
    }

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

void Widget::remove_child(Widget* child_widget){ //removes a child element from the list of child elements of the current parent, which uses child_widget | !!!
    if (!child_widget || child_widget->parent != this)return;
    
    Widget **temp = nullptr;
    bool found = false;

    if (children_count > 1){
        temp = new Widget*[children_count-1];
    }

    for (int i = 0, j = 0; i < children_count; ++i){
        if (child_widget == *(children + i)){
            found = true; 
            continue;
        }
        temp[j++] = children[i];
    }

    if (!found){ delete[] temp; return; }

    delete[] children;
    children = temp;
    --children_count;

    child_widget->parent = nullptr;
    child_widget->set_associated_window(nullptr);
    return;
}