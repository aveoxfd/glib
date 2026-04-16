#include "Include/nwind/nwind.h"
#include <iostream>
#include <string>
#include <windows.h>

class Widget;
class CWindow;

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

struct WindowEntry{
    Window *native;
    CWindow *owner;
};
typedef WindowEntry WindowEntry, WE;
void mouse_button_callback(Window* wnd, int button, char pressed);
//==================================================================================================

#define WINDOWS_ARRAY_COUNT_MAX 3
WindowEntry windows_array[WINDOWS_ARRAY_COUNT_MAX];
int windows_array_count = 0;

void registerwindow(Window *native, CWindow *owner){
    if (windows_array_count < WINDOWS_ARRAY_COUNT_MAX && native && owner)windows_array[windows_array_count++] = (WindowEntry){.native = native, .owner = owner};
    return;
}
CWindow *findwindow(Window* native){
    for (int i = 0; i<WINDOWS_ARRAY_COUNT_MAX; i++){
        if (windows_array[i].native == native) return windows_array[i].owner;
    }
    return nullptr;
}
void unregwindow(Window *native){
    for (int i = 0; i<WINDOWS_ARRAY_COUNT_MAX; i++){
        if (windows_array[i].native == native) windows_array[i] = {nullptr, nullptr};
    }
    return;
}

class Event{ //Event type
    private:
    typedef void(*EventFunction)(Widget* sender);

    EventFunction event_function;

    public:
    Event(EventFunction function = nullptr): event_function(function){};
    void activate(Widget *sender){
        if (event_function){
            event_function(sender);
        }
    }
};
typedef Event Event_t, E, E_t, event, event_t, e, e_t;

class Widget{
    private:
    Widget *parent;
    Widget **children;
    int children_count = 0;

    friend position get_real_postion(Widget *_widget);

    rect_t bound;

    Event *onclick_event;
    Event *inbound_event;

    bool contains(position pos){
        position real = get_real_postion(this);
        return pos.x >= real.x && pos.x < bound.size.width + real.x 
        && pos.y >= real.y && pos.y < bound.size.height + real.y;
    }
    //int e_count; //number of events in e_array

    //render widget
    //event func
    //event_handler
    
    public:
    Widget(rect_t bound): parent(nullptr), children(nullptr), bound(bound), onclick_event(nullptr), inbound_event(nullptr){};
    ~Widget(){}

    void on_click(Event *e){
        if (e)onclick_event = e;
    }

    void in_bound(Event *e){
        if (e)inbound_event = e;
    }

    int get_children_count(){
        return children_count;
    }

    //position of object in the bound of last layer of wodget's tree
    Widget* search_match(position pos){
        if (!contains(pos))return nullptr;
        Widget *children_curr = children[0]; //*children
        for (int i = children_count - 1; i >= 0; --i){
            Widget* found = children[i]->search_match(pos);
            if (found) return found;
        }
        return this;
    }


    //func add_event
    //func_del_event
};

position get_real_postion(Widget *_widget){
    position real_position = _widget->bound.pos;
    for (Widget *curr = _widget; curr->parent != nullptr; curr = curr->parent){
        real_position.x += curr->bound.pos.x;
        real_position.y += curr->bound.pos.y;
    }
    return real_position;
}

class CWindow{
    private:
    Window *window;
    Widget *main_widget;

    public:
    CWindow(const int width, const int height): main_widget(nullptr), window(nullptr){
        window = WindowCreate(width, height, "glib_window");
        if (!window){
            std::cout<<"WindowCreate: error of creating window.\n";
        }
        registerwindow(window, (CWindow*)this);
        WindowSetMouseButtonCallback(window, mouse_button_callback);
    }
    ~CWindow(){
        WindowDestroy(window);
    }

    Widget* get_root_widget(){
        if (main_widget)return main_widget;
        return nullptr;
    }

    void set_widget(Widget *_widget){
    }
};

void mouse_button_callback(Window* wnd, int button, char pressed){
    Mouse::button = button;
    POINT p;                        //<-----------
    GetCursorPos(&p);               //<-----------
    ScreenToClient((HWND)wnd, &p);  //<-----------
    Mouse::pos.x = p.x;
    Mouse::pos.y = p.y;
    //got to end children (tree parsing)

    CWindow* window = findwindow(wnd);
    Widget* root = window->get_root_widget();
    Widget* target = root->search_match(Mouse::pos);
    return;
}

int main(){
    Widget wid(rect_t{position{0, 0}, size{100, 100}});
    return 0;
}