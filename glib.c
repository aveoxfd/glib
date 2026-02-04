#include "glib.h"
#include <stdlib.h>

void widget_get_global_position(
    widget *__widget, //self rect
    point* __out
){
    if (!__widget || !__out)return;

    __out->x = 0;
    __out->y = 0;

    widget *currentw = __widget;

    while (currentw){
        __out->x += currentw->bounds.position.x;
        __out->y += currentw->bounds.position.y;
        currentw = currentw->parent;
    }
    return;
}

void widget_get_global_rect(
    widget *__widget, //self rect
    rectangle *__out
){
    if (!__widget || !__out)return;

    widget_get_global_position(__widget, &__out->position);
    __out->dimensions.width = __widget->bounds.dimensions.width;
    __out->dimensions.height = __widget->bounds.dimensions.height;
}

void widget_add_child(widget *parent, widget *child){
    if (!parent || !child)return;

    if (parent->child_capacity == 0){
        parent->child_capacity = 4;
        parent->children = calloc(parent->child_capacity, sizeof(widget*));
        if (!parent->children){
            parent->child_capacity = 0;
            return;
        }
    }
    else if(parent->child_count >= parent->child_capacity){
        int new_capacity = parent->child_capacity * 2;
        widget **tmp = realloc(parent->children, (size_t)new_capacity * sizeof(widget*));
        if (!tmp) {
            return; // realloc failed, keep old buffer
        }
        parent->children = tmp;
        parent->child_capacity = new_capacity;
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

bool point_in_widget(widget *__widget, point __p){
    if (!__widget)return false;
    rectangle global_rectangle;

    widget_get_global_rect(__widget, &global_rectangle);


    return (
        __p.x >= global_rectangle.position.x && __p.x < global_rectangle.dimensions.width + global_rectangle.position.x &&
        __p.y >= global_rectangle.position.y && __p.y < global_rectangle.dimensions.height + global_rectangle.position.y
    );
}

void widget_paint_tree(widget *__widget, gwindow *general_window){
    if (!__widget || !general_window)return;

    if (__widget->paint){
        __widget->paint(__widget, general_window);
    }

    for (int i = 0; i<__widget->child_count; ++i){
        if (__widget->children[i])widget_paint_tree(__widget->children[i], general_window);
    }
    return;
}

void widget_dispatch_event(widget *__widget, event *__e){
    if (!__widget || !__e)return;

    for (int i = __widget->child_count - 1; i>=0; --i){
        widget *ch = __widget->children[i];
         if(!ch) continue;
         if(point_in_widget(ch, __e->mouse_position)){
            widget_dispatch_event(ch, __e);
            if (__e->handled)return;
         }
    }

    if (__widget->handle_event){
        __widget->handle_event(__widget, __e);
    }
    return;
}

void widget_destroy_tree(widget *__widget){
    if(!__widget)return;

    for(int i = __widget->child_count - 1; i>=0; --i){
        if(__widget->children[i]){
            widget_destroy_tree(__widget->children[i]);
        }
    }

    if (__widget->destroy)__widget->destroy(__widget);

    if (__widget->children) {
        free(__widget->children);
        __widget->children = NULL;
    }

    if (__widget->name) {
        free(__widget->name);
        __widget->name = NULL;
    }

    if (__widget->user_data) {
        free(__widget->user_data);
        __widget->user_data = NULL;
    }

    if(__widget->a_bound.nodes){
        free(__widget->a_bound.nodes);
        __widget->a_bound.nodes = NULL;
        __widget->a_bound.nodes_count = 0;
    }

    free(__widget);
    return;
}