#include "Include/nwind/nwind.h"
#include <iostream>
#include <string>
#include <windows.h>

#define MAX_EVENT_NUM 10

struct position{
    int x, y;
};
typedef position point, point_t, position, position_t;

namespace Mouse{
    inline position pos;
    inline int button;
}

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

*/
//==================================================================================================

class Event{ //Event func
    private:
    typedef void(*EventFunction)(Widget* sender, void* user_data);

    EventFunction e_function;
    void* user_data;

    public:
    Event(EventFunction function = nullptr, void* ud = nullptr): e_function(function), user_data(ud){};
    void Activate(Widget *sender){
        if (e_function){
            e_function(sender, user_data);
        }
    }
};
typedef Event Event_t, E, E_t, event, event_t, e, e_t;

class Widget{
    private:
    Widget *parent;
    Widget **children;

    friend position get_real_postion(Widget *_widget);

    rect_t bound;

    Event *onclick_event;
    Event *inbound_event;

    bool contains(position pos){
        position real_pos = get_real_postion(this);
        return pos.x > real_pos.x && pos.x <= bound.size.width + real_pos.x 
        && pos.y > real_pos.y && pos.y <= bound.size.height;
    }
    //int e_count; //number of events in e_array

    //render widget
    //event func
    //event_handler
    
    public:
    Widget(rect_t bound): parent(nullptr), children(nullptr), bound(bound), onclick_event(nullptr), inbound_event(nullptr){};

    void on_click(Event *e){
        if (e)onclick_event = e;
    }

    void in_bound(Event *e){
        if (e)inbound_event = e;
    }

    //func add_event
    //func_del_event
};

position get_real_postion(Widget *_widget){
    Widget *curr = _widget;
    position real_position = {0};
    for (; curr->parent != nullptr; curr = curr->parent){
        real_position.x += curr->bound.pos.x;
        real_position.y += curr->bound.pos.y;
    }
    return real_position;
}

class СWindow{
    private:
    Window *window;
    Widget *main_widget;

    static void mouse_button_callback(Window* wnd, int button, char pressed){
        Mouse::button = button;
        POINT p;                        //<-----------
        GetCursorPos(&p);               //<-----------
        ScreenToClient((HWND)wnd, &p);  //<-----------
        Mouse::pos.x = p.x;
        Mouse::pos.y = p.y;

        //got to end children (tree parsing)
    }

    public:
    СWindow(const int width, const int height): main_widget(nullptr), window(nullptr){
        window = WindowCreate(width, height, "glib_window");
        if (!window){
            std::cout<<"WindowCreate: error of creating window.\n";
        }
        WindowSetMouseButtonCallback(window, mouse_button_callback);
    }
    ~СWindow(){
        WindowDestroy(window);
    }

    void set_widget(Widget *_widget){
        if(_widget){
            if (main_widget){
                //TODO!!!
            }
            main_widget = _widget;
        }
    }
};
int main(){
    Widget wid(rect_t{position{0, 0}, size{100, 100}});
    return 0;
}
