/*
00000000 - b
*/
#ifndef GLIB_H
#define GLIB_H

#include "winwindow.h"
#define false 0x0
#define true 0x1
#define isVisible_macro 0b00000001
#define isResizable     0b00000010

typedef unsigned int state;
typedef unsigned char bool;

typedef struct point{
    int x, y;
}position, point;

typedef struct{
    int width;
    int height;
}size;

typedef struct{
    point position;
    size dimensions;
}rectangle;

typedef struct{
    point mouse_position;
    int mouse_button;   
    int key_code;
    int modifiers;      
    void* source;       
    int handled;        
}event;

typedef struct widget;
typedef struct gwindow;

typedef void (*widget_paint_func)(widget*, gwindow*);
typedef void (*widget_event_func)(widget*, event*);
typedef void (*widget_update_func)(widget*);
typedef void (*widget_destroy_func)(widget*);

typedef struct widget{
    rectangle bounds;
    state state_flags;

    widget** children;
    widget* parent;
    int child_count;
    int child_capacity;

    widget_paint_func paint;
    widget_event_func handle_event;
    widget_update_func update;
    widget_destroy_func destroy;

    void* user_data;
    char* name;
}widget;

typedef struct gwindow{
    Window* native_window;

    widget *root_widget;

    int is_running;

    widget* focused_widget;
    widget* hovered_widget;

    point last_mouse;
}gwindow; //global | general (root, source) widget

#endif