#include "Include/nwind/nwind.h"
#include <iostream>
#include <string>

struct position{
    int x, y;
};
typedef position point, point_t, position, position_t;

struct rectangle{
    position pos;
    int width, height;
};
typedef rectangle rectangle, rectangle_t, rect, rect_t;

/*
event might be activated by clicking on widget (mouse event);
our goal is create an universally event activator 
*/
//==================================================================================================

class Event{ //it might be deleted in future
    private:
    typedef void(*event_function)(void); //event behaviour
    event_function *e_functon;
    std::string e_id;

    public:
    Event(void){};
    Event(event_function *e): e_functon(e){};
};
typedef Event Event_t, E, E_t, event, event_t, e, e_t;

class Widget{
    private:
    Widget *parent;
    Widget **children;

    rect_t bound;

    Event **e_array;

    //render widget
    //event func
    //event_handler
    
    public:
    Widget(rect_t bound): bound(bound){
    }

    //func add_event
    //func_del_event
};

class Window{
    private:
    Window *window;
    Widget *main_widget;

    public:
    Window(const int width, const int height){
        window = WindowCreate(width, height, "glib_window");
        if (!window){
            std::cout<<"WindowCreate: error of creating window.\n";
        }
    }
    ~Window(){
        WindowDestroy(window);
    }
};