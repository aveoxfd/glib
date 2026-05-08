#include "../../include/glib/ClassWindow.h"
#include "../../include/glib/widget/Widget.h"
#include "../../include/nwind/nwind.h"

#include <stdio.h>

#ifdef DEBUG
#include <iostream>
#endif

//typedef void (*on_timer_function)(Window *window, void*);

//struct TIMER_ENTRY{
//    UINT_PTR id;
//    void* user_data;
//    on_timer_function function;
//};typedef TIMER_ENTRY TIMER_ENTRY;
//
//struct TIMER_ENTRY_ARRTYPE{
//    TIMER_ENTRY *array;
//    int array_count;
//};typedef TIMER_ENTRY_ARRTYPE TIMER_ENTRY_ARRTYPE;

namespace Mouse{
    inline position pos;
    inline int button;
}
namespace Keyboard{
    inline int key;
}

/*======*/
#define WE_MAX 10
WindowEntry WE_array[WE_MAX];
int WE_array_count = 0;
static Widget *last_hovered = nullptr;

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

CWindow *findwindow(Window* native){ //find associated ClassWindow with Window
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
/*======*/

void mouse_button_callback(Window* wnd, int button, char pressed){
    Mouse::button = button;
    POINT p;
    GetCursorPos(&p);                   //get cursor position
    HWND hwnd = WindowFromPoint(p);     //hwnd under cursor
    ScreenToClient(hwnd, &p);           //recalculate screen's coord into client's coord
    Mouse::pos.x = p.x;
    Mouse::pos.y = p.y;
    
    #ifdef DEBUG_MOUSE
    std::cout << "Mouse click at (" << Mouse::pos.x << ", " << Mouse::pos.y << "); button = " << button << std::endl;
    #endif
    //go to end children (tree parsing)
    CWindow* window = findwindow(wnd);
    if (window == nullptr)return;
    Widget* root = window->get_root_widget();
    if(root == nullptr)return;
    Widget* target = root->find_widget(Mouse::pos);
    if(target == nullptr)return;

    window->set_focus(target);
    target->mouse_press_handler(Mouse::button);
    return;
}

void mouse_move_callback(Window *wnd, int x, int y){
    POINT p;
    GetCursorPos(&p);                   //get cursor position
    HWND hwnd = WindowFromPoint(p);     //hwnd under cursor
    ScreenToClient(hwnd, &p);           //recalculate screen's coord into client's coord
    Mouse::pos.x = p.x;
    Mouse::pos.y = p.y;
    
    #ifdef DEBUG_MOUSE
    std::cout << "Mouse has been moved at (" << Mouse::pos.x << ", " << Mouse::pos.y << "); key = " << key << std::endl;
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

void keyboard_callback(Window *window, int key, char pressed){
    Keyboard::key = key;

    CWindow *classwindow = findwindow(window);
    if (!classwindow)return;

    Widget *focused = classwindow->get_focused();
    if(!focused)return;

    focused->keyboard_handler(key, pressed);
    
    return;
}

void timer_callback(Window *window, WPARAM wParam/*= id*/){ //calls by nwind

    

    CWindow *classwindow = findwindow(window);

    if(!classwindow) return;

    TIMER_ENTRY *te_ptr = classwindow->find_timer_function_TE(wParam);
    
    #ifdef TIMER_DEBUG
    printf("timer_callback(wParam): \n%d \n", (WPARAM)wParam);
    printf("te_ptr = %d\n", te_ptr);
    #endif

    if(te_ptr && te_ptr->function) te_ptr->function(te_ptr->user_data);

    return;
}

ClassWindow::ClassWindow(const int width, const int height):window(nullptr), root_widget(nullptr){
    window = WindowCreate(width, height, "glib_window");
    if(!window){}

    RegWindow({window, this});

    WindowSetMouseButtonCallback(window, mouse_button_callback);
    WindowSetMouseMoveCallback(window, mouse_move_callback);
    WindowSetKeyCallback(window, keyboard_callback);
    WindowSetTimerCallback(window, timer_callback);
}

ClassWindow::~ClassWindow(){
    UnregWindow(window);
    WindowDestroy(window);
    last_hovered = nullptr;
};

Widget* ClassWindow::get_root_widget(){
    if (root_widget)return root_widget;
    return nullptr;
};

void ClassWindow::set_widget(Widget *widget){
    root_widget = widget;
    widget->set_associated_window(window);
}

position ClassWindow::get_mouse_position(void){
    return Mouse::pos;
}

int ClassWindow::get_mouse_button(void){
    return Mouse::button;
}

int get_keyboard_key(void){
    return Keyboard::key;
}

void ClassWindow::update(void){
    WindowUpdate(window);
}

void ClassWindow::start_cycle(){
    while (MessageProcess()){
        root_widget->update_tree();
        root_widget->render_tree();

        WindowUpdate(window);
    }
}
void ClassWindow::set_focus(Widget *widget){
    focused = widget;
}

Widget* ClassWindow::get_focused(void){
    return focused;
}

//TODO!

//set window its own timer processor (WM_TIMER)
void ClassWindow::register_ontimer_function(on_timer_function function, void* user_data/*= will be in function's parameter*/, UINT interval_ms, TIMERPROC other){ //07.05.2026
    if (!function)return;

    UINT_PTR id = WindowStartTimer(window, interval_ms, nullptr);
    
    TIMER_ENTRY *temp = new TIMER_ENTRY [TE.array_count + 1]; //create buff

    for (int i = 0; i < TE.array_count; ++i){ //copy
        temp[i] = TE.array[i];
    }

    temp[TE.array_count] = {id, user_data, function}; //add new
    ++TE.array_count;

    delete[] TE.array; //delete old

    TE.array = temp; //set new

    //DEBUG
    #ifdef TIMER_DEBUG
    printf("register_on_timer_func (id):\n %d\n", id);

    for (int i = 0; i < TE.array_count; ++i){
        printf("(TE id):\n %d\n", TE.array[i].id);
    }
    #endif
    return;
}

void ClassWindow::delete_ontimer_function(on_timer_function function){ //<==== may have a bug
    if (!TE.array || !function) return;

    UINT_PTR kill_id = 0;
    for (int i = 0; i < TE.array_count; ++i)
        if (TE.array[i].function == function) { kill_id = TE.array[i].id; break; }

    if (!kill_id) return;
    WindowKillTimer(window, kill_id);

    TIMER_ENTRY *temp = new TIMER_ENTRY[TE.array_count - 1];
    for (int i = 0, j = 0; i < TE.array_count; ++i)
        if (TE.array[i].id != kill_id) temp[j++] = TE.array[i];

    delete[] TE.array;
    TE.array = temp;
    --TE.array_count;
    return;
}

on_timer_function ClassWindow::find_timer_function(WPARAM wParam){
    for (int i = 0; i < TE.array_count; ++i){
        if (TE.array[i].id == (UINT_PTR)wParam)return TE.array[i].function;
    }
    return nullptr;
}

TIMER_ENTRY* ClassWindow::find_timer_function_TE(WPARAM wParam /*= id*/){
    for (int i = 0; i < TE.array_count; ++i){
        if (TE.array[i].id == (UINT_PTR)wParam)return &TE.array[i];
    }
    return nullptr;
}