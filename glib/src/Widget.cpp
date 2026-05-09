#include "../../include/glib/widget/Widget.h"
#include "../../include/nwind/nwind.h"
#include <iostream>
#include <new>

#define POINTER 0
#define VIRTUAL 1

position get_real_position(Widget *widget){
    position real_position = widget->bound.pos;
    for (Widget *curr = widget->parent; curr != nullptr; curr = curr->parent){
        real_position.x += curr->bound.pos.x;
        real_position.y += curr->bound.pos.y;
    }
    return real_position;
}

bool point_in_polygon(position point, Body body) {
    /*
    y = kx + b;
    b = y - kx;
    k = y/x;
    x = -b/k;
    */
    bool inside = false;
    int n = body.nodes_count;
    if (n < 3) return inside;

    for (int i = 0; i < n; ++i) {
        int j = (i + 1) % n;

        double x1 = body.center.x + body.nodes[i].x;
        double y1 = body.center.y + body.nodes[i].y;
        double x2 = body.center.x + body.nodes[j].x;
        double y2 = body.center.y + body.nodes[j].y;

        double px = point.x;
        double py = point.y;
        double rx1 = x1 - px;
        double ry1 = y1 - py;
        double rx2 = x2 - px;
        double ry2 = y2 - py;

        if ((ry1 > 0) != (ry2 > 0)) {
            double x_intersect;
            if (rx2 == rx1) {
                x_intersect = rx1;
            } 
            
            else {

                double k = (ry2 - ry1) / (rx2 - rx1);
                double b = ry1 - k * rx1;

                x_intersect = -b / k;
            }
            if (x_intersect > 0) {

                inside = !inside;
            }
        }
    }
    return inside;
}

bool Widget::contains(position pos /*- already gobal*/){
    position real = get_real_position(this); //real postion of widget in depended window;

    if (pos.x < real.x || pos.x >= real.x + bound.size.width ||
        pos.y < real.y || pos.y >= real.y + bound.size.height)return false;
    //return pos.x >= real.x && pos.x < bound.size.width + real.x 
    //    && pos.y >= real.y && pos.y < bound.size.height + real.y;
    //...

    position local_point = { pos.x - real.x, pos.y - real.y };

    if (body.nodes && body.nodes_count > 0) {
        return point_in_polygon(local_point, body);
    }

    return true;
}

Widget::Widget(rect_t rectangle_bound, Widget *parent, Body rBound):
body(rBound),
parent(parent), 
children(nullptr),
children_count(0),
association(nullptr),
bound(rectangle_bound){
    if (this->parent)parent->add_child(this);
}

//Widget::~Widget() {
//    //for (int i = 0; i < children_count; ++i) {
//    //    delete children[i];
//    //}
//    //delete[] children;
//    //delete onclick_event;
//    //delete inbound_event;
//    //delete outbound_event;
//}

rect_t Widget::get_rect(){
    return bound;
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

void Widget::update_tree(){
    update();
    for (int i = 0; i < children_count; ++i){
        children[i]->update_tree();
    }
}

void Widget::render_tree(){
    render();
    for (int i = 0; i < children_count; ++i){
        children[i]->render_tree();
    }
}