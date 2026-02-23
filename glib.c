#include "glib.h"
#include "memory.h"
#include <stdlib.h>

//========================================

typedef unsigned int state;
typedef unsigned char bool;

typedef struct point{
    int x, y;
}position, point;

typedef struct size{
    int width;
    int height;
}size, size_tp;

typedef struct rectangle{
    point position;
    size dimensions;
}rectangle, rectangle_t;

typedef struct event{
    point mouse_position;
    int mouse_button;   
    int key_code;
    int modifiers;      
    void* source;       
    int handled;        
}event, event_t;

typedef struct arbitrary_bound{
    point *nodes;
    int nodes_count
}arbitrary_bound, arbitrary_bound;

typedef struct widget;
typedef struct gwindow;
typedef struct event;
typedef struct rectangle;
typedef struct size;
typedef struct point;

typedef void (*widget_paint_func)(widget*, gwindow*);
typedef void (*widget_event_func)(widget*, event*);
typedef void (*widget_update_func)(widget*);
typedef void (*widget_destroy_func)(widget*);
typedef bool (*widget_point_in_widget)(widget*, point); //if point in widget bounds

typedef struct widget{
    rectangle bounds;       //bound
    arbitrary_bound a_bound;//arbitrary bound
    state state_flags;      //flag var


    widget** children;  //array of children
    widget* parent;     //parent
    int child_count;    //ptr
    int child_capacity; //ptr

    widget_paint_func paint;        //render function
    widget_event_func handle_event; //handle function
    widget_update_func update;      //update function
    widget_destroy_func destroy;    //destroy func
    widget_point_in_widget check;   //check if point in bound

    void* user_data;                //user data memory
    char* name;                     //name (id) of widget
}widget, widget_t;


typedef struct gwindow{
    Window* native_window;  //pointer to general window

    widget *root_widget;    //general widget

    int is_running;

    widget* focused_widget; //active widget
    widget* hovered_widget; //

    point last_mouse;       //position if mouse
}gwindow, gwindow_t; //global | general (root, source) widget
//========================================

void widget_get_global_position(
    widget *__widget, //self rect
    point* _out
){
    if (!__widget || !_out)return;

    _out->x = 0;
    _out->y = 0;

    widget *currentw = __widget;

    while (currentw){
        _out->x += currentw->bounds.position.x;
        _out->y += currentw->bounds.position.y;
        currentw = currentw->parent;
    }
    return;
}

void widget_get_global_rect(
    widget *__widget, //self rect
    rectangle *_out
){
    if (!__widget || !_out)return;

    widget_get_global_position(__widget, &_out->position);
    _out->dimensions.width = __widget->bounds.dimensions.width;
    _out->dimensions.height = __widget->bounds.dimensions.height;
}

void widget_add_child(widget *parent, widget *child){ //create child in parent
    if (!parent || !child)return;

    if (parent->child_capacity == 0){
        parent->child_capacity = 4;
        parent->children = calloc(parent->child_capacity, sizeof(widget*));
        if (!parent->children){
            parent->child_capacity = 0;
            //error msg
            return;
        }
    }
    else if(parent->child_count >= parent->child_capacity){
        int new_capacity = parent->child_capacity * 2;
        widget **tmp = realloc(parent->children, (size_t)new_capacity * sizeof(widget*));
        if (!tmp) {
            //error msg
            return; // realloc failed, keep old buffer
        }
        parent->children = tmp;
        parent->child_capacity = new_capacity;
    }
    *(parent->children + parent->child_count++) = child;
    //parent->children[parent->child_count++] = child; // *(parent->children+parent->child_count++) = child
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

bool point_in_widget(widget *__widget, point __p){ //point in box bound
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

bool point_in_arbitrary_bound(widget *__widget, point __p){
    static int step_count = 100;
    unsigned char touch_count;
    if (!__widget->a_bound.nodes)return false;

    point widget_global_position;
    widget_get_global_position(__widget, &widget_global_position);

    point __local_p = {
        __p.x - widget_global_position.x,
        __p.y - widget_global_position.y
    };

    for (int i = 0, j = __widget->a_bound.nodes_count - 1; i < __widget->a_bound.nodes_count; j = i++){ //<--
        point p1 = __widget->a_bound.nodes[j]; //end
        point p2 = __widget->a_bound.nodes[i]; //head

        double k = (p2.y - p1.y)/(p2.x - p2.x);

        //if (
        //    
        //)
    }


    if (touch_count % 2 != 0)return true;
    else return false;
}