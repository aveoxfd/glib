# Technical Documentation: nwind + glib

---

## 1. nwind — Low-Level Window Library

### Purpose
A thin wrapper over WinAPI. Creates a window, manages a bitmap, and processes system messages.

### Window Structure
```c
struct Window {
    HWND     hwnd;           // Windows window handle
    void    *pixels;         // pointer to bitmap memory (DIB section)
    int      width;
    int      height;
    HBITMAP  handle_bitmap;  // bitmap handle
};
```

### Window Lifecycle
```
WindowCreate()
    └── RegisterClass()       // register window class in Windows
    └── CreateWindowEx()      // create the window
    └── CreateDIBSection()    // allocate pixel memory → wnd->pixels
    └── return Window*

MessageProcess()              // processes the Windows message queue (PeekMessage)
    └── returns 0 if window is closed

WindowDestroy()
    └── DestroyWindow()
    └── free memory
```

### Rendering Pipeline
```
putpixel(wnd, x, y, color)
    └── wnd->pixels[y * width + x] = color   // write directly to bitmap memory

WindowUpdate(wnd)
    └── InvalidateRect()      // mark window as "dirty"
    └── Windows sends WM_PAINT
            └── BitBlt()      // copy wnd->pixels to screen

ClearWindow(wnd, color)
    └── fill entire wnd->pixels with one color
```

### Event Callbacks
```c
WindowSetMouseButtonCallback(wnd, fn);  // mouse click
WindowSetMouseMoveCallback(wnd, fn);    // mouse movement
WindowSetKeyCallback(wnd, fn);          // keyboard
```
Callbacks are invoked inside `MessageProcess()` when the corresponding Windows messages are processed.

---

## 2. glib — Widget Library

### Purpose
Builds a widget tree on top of nwind. Routes mouse and keyboard events to the correct widget.

---

### 2.1 ClassWindow

#### Purpose
Links a `Window*` (nwind) to a widget tree. Maintains a registry of all windows.

#### Window Registry
```cpp
WindowEntry WE_array[WE_MAX]; // array of { Window*, ClassWindow* } pairs

findwindow(Window *native)    // find ClassWindow by Window*
RegWindow(...)                // register a pair on creation
UnregWindow(...)              // remove on destruction
```

#### Event Routing
```
mouse_button_callback()           // called from nwind on click
    └── GetCursorPos()            // get cursor position
    └── findwindow()              // find ClassWindow
    └── root->find_widget(pos)    // find widget under cursor
    └── target->mouse_press_handler()

mouse_move_callback()
    └── find_widget(pos)          // widget under cursor
    └── if widget changed:
            last_hovered->mouse_outbound_handler()
            target->mouse_inbound_handler()
    └── last_hovered = target     // remember current
```

#### Global Mouse State
```cpp
namespace Mouse {
    position pos;   // current position
    int button;     // last button pressed
}
```

---

### 2.2 Widget

#### Purpose
Base class of the widget tree. Stores geometry and widget relationships. Has no knowledge of events or rendering — only dispatching.

#### Fields
```cpp
Widget  *parent;          // parent widget
Widget **children;        // array of children
int      children_count;
Window  *association;     // associated nwind window
rect_t   bound;           // position and size relative to parent
```

#### Tree Operations
```
add_child(child)
    ├── checks that child is not an ancestor (cycle protection)
    ├── detaches child from old parent if any
    ├── adds to children array
    └── recursively sets association on all descendants

remove_child(child)
    ├── removes from children array
    ├── child->parent = nullptr
    └── child->association = nullptr (recursive)

find_widget(pos)
    ├── if pos is outside bounds → nullptr
    ├── searches children from the end (last added = on top)
    └── if none matched → return this
```

#### Real Position
```cpp
get_real_position(widget)
    // sums positions of all ancestors up the tree
    // needed for contains() and for drawing
```

#### Event Dispatching
```cpp
mouse_press_handler(button)   → on_press(button)     // virtual
mouse_inbound_handler()       → on_inbound()          // virtual
mouse_outbound_handler()      → on_outbound()         // virtual
keyboard_handler(key,pressed) → on_key(key, pressed)  // virtual
```
Subclasses override `on_*` methods — base implementations are empty.

#### Render and Update
```cpp
// function pointers — can be swapped per instance at runtime
render_function render_func;     // void(*)(Widget*)
update_function update_func;     // void(*)(Widget*, void*)
void           *user_data_update;

virtual void render() {
    if (render_func) render_func(this);
}
virtual void update() {
    if (update_func) update_func(this, user_data_update);
}
```

---

### 2.3 Button : public Widget

#### Purpose
Adds event handling via callbacks on top of Widget. A polymorphic subclass — stored in the tree as `Widget*`.

#### Additional Fields
```cpp
event_function on_click_function;   // void(*)(Widget*, void*)
void          *user_data_on_click;
event_function inbound_func;
void          *user_data_on_inbound;
event_function outbound_func;
void          *user_data_on_outbound;
```

#### Overridden Hooks
```cpp
on_press(button)  → on_click_function(this, user_data_on_click)
on_inbound()      → inbound_func(this, user_data_on_inbound)
on_outbound()     → outbound_func(this, user_data_on_outbound)
```

---

## 3. Correct Main Loop

```cpp
while (MessageProcess()) {
    ClearWindow(main_win, background_color); // 1. erase previous frame

    // 2. update state (positions, animations)
    btn.update();

    // 3. redraw everything
    btn.render();

    // 4. present frame to screen
    WindowUpdate(main_win);
}
```

**Important:** `update_func` should only change widget state.  
`WindowUpdate` must be called only once per frame — otherwise you get a trail.

---

## 4. Full Chain: Click to Screen

```
User clicks mouse
    └── Windows generates WM_LBUTTONDOWN
    └── MessageProcess() handles the message
    └── mouse_button_callback(wnd, button, pressed)
            └── GetCursorPos() + ScreenToClient()
            └── findwindow(wnd) → ClassWindow*
            └── root->find_widget(Mouse::pos) → Widget* target
            └── target->mouse_press_handler(0)
                    └── on_press(0)              // virtual dispatch
                            └── Button::on_press
                                    └── on_click_function(this, data)
                                            └── your lambda
```

---

## 5. How vtable Works (Polymorphism)

```
Button in memory:
┌─────────────────────────┐
│ vptr ───────────────────┼──► Button vtable
│ parent                  │    ┌──────────────────────┐
│ children                │    │ Button::render        │
│ bound                   │    │ Button::update        │
│ association             │    │ Button::on_press      │
├─────────────────────────┤    │ Button::on_inbound    │
│ on_click_function       │    │ Button::on_outbound   │
│ inbound_func            │    └──────────────────────┘
│ outbound_func           │
└─────────────────────────┘

Widget *w = &button; // w points to the start of the object
w->render();
    // 1. read vptr from object → Button vtable
    // 2. get render from table → Button::render
    // 3. call with this = object address
    // Button::render sees ALL fields of the object through this
```

---

## 6. Runtime Behavior Change Per Instance

Virtual functions are per-class — you cannot override them per object.  
Use function pointers for per-instance runtime behavior:

```cpp
// set default render
btn.set_render_function([](Widget *wid) { fill_rect(wid, 0xFFFFFFFF); });

// change on hover
btn.set_on_inbound_event([](Widget *wid, void *) {
    wid->set_render_function([](Widget *wid) { fill_rect(wid, 0xFFFF0000); });
});
```

If behaviors are few and known in advance → prefer **state**:
```cpp
enum class State { Normal, Hovered };
State state = State::Normal;

void render() override {
    fill_rect(this, state == State::Hovered ? 0xFFFF0000 : 0xFFFFFFFF);
}
```

---

## 7. Public vs Private Inheritance

```cpp
class Button : Widget { }        // private — Button* cannot be used as Widget*
class Button : public Widget { } // public  — Button is a Widget, polymorphism works
```

Use `public` when the subclass **is a** base class (Liskov Substitution Principle).
