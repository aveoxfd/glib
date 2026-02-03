#include "glib.h"
#include <stdlib.h>

void widget_add_child(widget *parent, widget *child){
    if (!parent || !child)return;

    if (parent->child_capacity == 0){
        parent->child_capacity = 4;
        parent->children = calloc(parent->child_capacity, sizeof(widget*));
    }
    else if(parent->child_count >= parent->child_capacity){
        parent->child_capacity *= 2;
        parent->children = realloc(parent->children, sizeof(widget*) * parent->child_capacity);
    }
    parent->children[parent->child_count++] = child;
    child->parent = parent;
    return;
}

void widget_add_paintf(widget *__widget, widget_paint_func __paint_function){
    if (!__widget || !__paint_function)return;

    __widget->paint = __paint_function;
    return;
}

void widget_add_eventf(widget *__widget, widget_event_func __event_function){
    if (!__widget || !__event_function)return;

    __widget->handle_event = __event_function;
    return;
}

void widget_add_updatef(widget *__widget, widget_update_func __update_function){
    if (!__widget || !__update_function)return;

    __widget->update = __update_function;
    return;
}

void widget_add_destroyf(widget *__widget, widget_destroy_func __destroy_function){
    if (!__widget || !__destroy_function)return;

    __widget->destroy = __destroy_function;
    return;
}

bool point_in_widget(widget *__widget, point __global_position_in_window){
    
    return 0;
}