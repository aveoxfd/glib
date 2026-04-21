
#include "ClassWindow.h"
#include <windows.h>
#include "../../include/nwind/nwind.h"

#ifdef DEBUG
#include <iostream>
#endif

namespace Mouse{
    inline position pos;
    inline int button;
}

#define WE_MAX 10
WindowEntry WE_array[WE_MAX];
int WE_array_count = 0;

void RegWindow(WindowEntry both){ //window registration
    if (WE_array_count < WE_MAX){
        WE_array[WE_array_count++] = both;
    }
    return;
}

void RegWindow(Window *native, ClassWindow *owner){ //window registration
    if (WE_array_count < WE_MAX){
        WE_array[WE_array_count++] = {native, owner};
    }
    return;
}

CWindow *findwindow(Window* native){
    for (int i = 0; i<WE_MAX; i++){
        if (WE_array[i].native == native) return WE_array[i].owner;
    }
    return nullptr;
}

void UnregWindow(Window *native){
    for (int i = 0; i<WE_MAX; i++){
        if (WE_array[i].native == native) WE_array[i] = {nullptr, nullptr};
    }
    return;
}

void mouse_button_callback(Window* wnd, int button, char pressed){
    Mouse::button = button;
    POINT p;
    GetCursorPos(&p);                   //get cursor position
    HWND hwnd = WindowFromPoint(p);     //hwnd under cursor
    ScreenToClient(hwnd, &p);           //recalculate screen's coord into client's coord
    Mouse::pos.x = p.x;
    Mouse::pos.y = p.y;
    
    #ifdef DEBUG
    std::cout << "Mouse click at (" << Mouse::pos.x << ", " << Mouse::pos.y << "); button = " << button << std::endl;
    #endif
    //go to end children (tree parsing)
    CWindow* window = findwindow(wnd);
    if (window == nullptr)return;
    Widget* root = window->get_root_widget();
    if(root == nullptr)return;
    Widget* target = root->find_widget(Mouse::pos);
    if(target == nullptr)return;

    target->mouse_press_handler(Mouse::button);
    return;
}

void mouse_move_callback(Window *wnd, int key, char pressed){
    static Widget *last_hovered = nullptr;
    POINT p;
    GetCursorPos(&p);                   //get cursor position
    HWND hwnd = WindowFromPoint(p);     //hwnd under cursor
    ScreenToClient(hwnd, &p);           //recalculate screen's coord into client's coord
    Mouse::pos.x = p.x;
    Mouse::pos.y = p.y;
    
    #ifdef DEBUG
    std::cout << "Mouse click at (" << Mouse::pos.x << ", " << Mouse::pos.y << "); button = " << button << std::endl;
    #endif
    //go to end children (tree parsing)

    CWindow* class_window = findwindow(wnd);
    if (class_window == nullptr)return;
    Widget* root = class_window->get_root_widget();
    if(root == nullptr)return;
    Widget* target = root->find_widget(Mouse::pos);
    //if(target == nullptr)return;

    if(target != last_hovered || target == nullptr){
        if (last_hovered) last_hovered->mouse_outbound_handler();
        if (target) target->mouse_inbound_handler();
        last_hovered = target;
    }
    return;
}


ClassWindow::ClassWindow(const int width, const int height):window(nullptr), root_widget(nullptr){
    window = WindowCreate(width, height, "glib_window");
    if(!window){}

    RegWindow({window, this});

}

ClassWindow::~ClassWindow(){
    UnregWindow(window);
};

Widget* ClassWindow::get_root_widget(){
    if (root_widget)return root_widget;
    return nullptr;
};

void ClassWindow::set_widget(Widget *widget){
    root_widget = widget;
    widget->set_associated_window(window);
}