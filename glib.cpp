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

    friend position get_real_position(Widget *_widget);

    rect_t bound;

    Event *onclick_event;
    Event *inbound_event;

    bool contains(position pos){
        position real = get_real_position(this);
        //std::cout<<"The real position of widget is <"<<real.x<<", "<<real.y<<">\n";
        //std::cout<<"bound: <"<<bound.size.width<<", "<<bound.size.height<<">\n ";
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

    void MousePressHandler(int button){
        //std::cout << "MousePressHandler: button=" << button << std::endl;
        if (button == 0 && onclick_event){
            onclick_event->activate(this);
        }
        return;
    }

    void MouseInboundHandler(){
        if(inbound_event){
            inbound_event->activate(this);
        }
        return;
    }

    //position of object in the bound of last layer of widget's tree
    Widget* search_match(position pos){
        if (!contains(pos)){
            return nullptr;
        }
        for (int i = children_count - 1; i >= 0; --i){
            Widget* found = children[i]->search_match(pos);
            if (found) return found;
        }
        return this;
    }


    //func add_event
    //func_del_event
};

position get_real_position(Widget *_widget){
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
        //WindowDestroy(window); need to be fixed
    }

    Widget* get_root_widget(){
        if (main_widget)return main_widget;
        return nullptr;
    }

    void set_widget(Widget *_widget){
        main_widget = _widget;
        return;
    }
};

void mouse_button_callback(Window* wnd, int button, char pressed){
    Mouse::button = button;
    POINT p;                        //<-----------
    GetCursorPos(&p);               //<-----------
    HWND hwnd = WindowFromPoint(p); //hwnd under cursor
    RECT rect;
    GetWindowRect(hwnd, &rect);
    Mouse::pos.x = p.x - rect.left;
    Mouse::pos.y = p.y - rect.top;
    
    //std::cout << "Mouse click at (" << Mouse::pos.x << ", " << Mouse::pos.y << "); button = " << button << std::endl;
    //go to end children (tree parsing)

    CWindow* window = findwindow(wnd);
    if (window == nullptr)return;
    Widget* root = window->get_root_widget();
    if(root == nullptr)return;
    Widget* target = root->search_match(Mouse::pos);
    if(target == nullptr)return;

    target->MousePressHandler(Mouse::button);
    return;
}

int main(){
    CWindow w1(800, 600);
    Widget wid(rect_t{position{200, 200}, size{200, 100}});
    Event *ev = new Event([](Widget *wid)->void{
        std::cout<<"Hello world!\n";
    });
    wid.on_click(ev);
    w1.set_widget(&wid);

    MSG msg = {};
    while (MessageProcess()) {

    }
    return 0;
}