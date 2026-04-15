#include "Include/nwind/nwind.h"
#include <iostream>
#include <string>

#define MAX_EVENT_NUM 10

struct position{
    int x, y;
};
typedef position point, point_t, position, position_t;

struct size{
    int width, height;
};
typedef size size;

struct rectangle{
    position pos;
    size size;
};
typedef rectangle rectangle, rectangle_t, rect, rect_t;

/*
event might be activated by clicking on widget (mouse event);
our goal is create an universally event activator 

widget:
event on click -> 
*/
//==================================================================================================

class Event{ //Event func
    private:
    typedef bool(*EventFunction)(Widget* sender, void* user_data);

    EventFunction function;
    void* user_data;

    public:
    Event(EventFunction function = nullptr, void* ud = nullptr): function(function), user_data(ud){};
    void Activate(Widget *sender){
        if (function){
            function(sender, user_data);
        }
    }
};
typedef Event Event_t, E, E_t, event, event_t, e, e_t;

class Widget{
    private:
    Widget *parent;
    Widget **children;

    rect_t bound;

    Event *e_array[MAX_EVENT_NUM];
    int e_count; //number of events in e_array

    //render widget
    //event func
    //event_handler
    
    public:
    Widget(rect_t bound): parent(nullptr), children(nullptr), bound(bound), e_count(0){};



    //func add_event
    //func_del_event
};

class СWindow{
    private:
    Window *window;
    Widget *main_widget;

    public:
    СWindow(const int width, const int height){
        window = WindowCreate(width, height, "glib_window");
        if (!window){
            std::cout<<"WindowCreate: error of creating window.\n";
        }
    }
    ~СWindow(){
        WindowDestroy(window);
    }
};
int main(){
    Widget wid(rect_t{position{0, 0}, size{100, 100}});
    return 0;
}
