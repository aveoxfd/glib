# Technical Documentation: nwind + glib

---

## 1. nwind — Low-Level Window Library

### Purpose
A thin wrapper over WinAPI. Creates a window, manages a bitmap, and processes system messages.

### Rendering Pipeline
```
putpixel(wnd, x, y, color)
    └── wnd->pixels[y * width + x] = color   // write directly to bitmap memory

WindowUpdate(wnd)
    └── InvalidateRect()
    └── WM_PAINT → BitBlt()                  // copy wnd->pixels to screen

ClearWindow(wnd, color)
    └── fill entire wnd->pixels with one color
```

### Event Callbacks
```c
WindowSetMouseButtonCallback(wnd, fn);
WindowSetMouseMoveCallback(wnd, fn);
WindowSetKeyCallback(wnd, fn);
```

---

## 2. File Structure

```
include/glib/
├── glib_api.h          — GLIBAPI macro only
├── glib.h              — main header, includes everything in correct order
├── ClassWindow.h/cpp   — window + event routing
├── Event.h/cpp         — event wrapper
├── structures/         — position, size, rect, WindowEntry
└── widget/
    ├── Widget.h/cpp    — base class
    ├── Button.h        — clickable widget with callbacks
    └── Frame.h         — container with background render
```

### Include Order (no circular dependencies)
```
glib_api.h          — no dependencies
structures/         — no dependencies
widget/Widget.h     → glib_api.h, structures/
widget/Button.h     → Widget.h
widget/Frame.h      → Widget.h
ClassWindow.h       → glib_api.h, structures/ (Widget via forward declaration)
glib.h              → Widget.h, ClassWindow.h, Button.h, Frame.h
```

---

## 3. ClassWindow

### Purpose
Links a `Window*` (nwind) to a widget tree. Maintains a registry of all windows.  
Routes mouse and keyboard events to the correct widget.

### Fields
```cpp
Window *window;       // nwind window
Widget *root_widget;  // root of the widget tree
Widget *focused;      // widget that receives keyboard events
```

### Event Routing

**Mouse click:**
```
mouse_button_callback()
    └── GetCursorPos() + ScreenToClient()
    └── findwindow(wnd) → ClassWindow*
    └── root->find_widget(pos) → Widget*
    └── cw->set_focus(target)               // keyboard focus follows click
    └── target->mouse_press_handler(button)
```

**Mouse move:**
```
mouse_move_callback()
    └── find_widget(pos) → target
    └── if target changed:
            last_hovered->mouse_outbound_handler()
            target->mouse_inbound_handler()
    └── last_hovered = target
```

**Keyboard:**
```
keyboardCallback()
    └── findwindow() → ClassWindow*
    └── cw->get_focused() → Widget*
    └── focused->keyboard_handler(key, pressed)
```

### Main Loop
```cpp
void ClassWindow::start_cycle() {
    while (MessageProcess()) {
        root_widget->update_tree();  // update all widgets recursively
        root_widget->render_tree();  // render all widgets recursively
        WindowUpdate(window);        // present frame to screen
    }
}
```

### API
```cpp
ClassWindow window(800, 600);
window.set_widget(&root);     // attach root, propagates Window* to whole tree
window.start_cycle();         // enter main loop

window.get_mouse_position();
window.get_mouse_button();
window.get_keyboard_key();
window.set_focus(widget);     // manually set keyboard focus
window.get_focused();
window.update();              // WindowUpdate — use outside start_cycle

findwindow(Window *native);   // find ClassWindow by nwind Window*
```

---

## 4. Widget

### Purpose
Base class of the widget tree. Stores geometry and parent-child relationships.  
Dispatches input events to virtual hooks that subclasses override.

### Fields
```cpp
Widget  *parent;
Widget **children;
int      children_count;
Window  *association;    // propagated from ClassWindow via set_associated_window
rect_t   bound;          // position and size RELATIVE TO PARENT
```

### Tree Operations
```
add_child(child)
    ├── cycle protection
    ├── detaches from old parent if any
    ├── appends to children array
    └── recursively propagates association to all descendants

remove_child(child)
    ├── removes from children array
    └── child->parent = nullptr, association = nullptr (recursive)

find_widget(pos)
    ├── if pos outside bounds → nullptr
    ├── searches children from end (last added = drawn on top)
    └── if none matched → return this
```

### Real Position
```cpp
get_real_position(widget)
    // sums positions of all ancestors
    // bound.pos is relative to parent — always use this for drawing
```

### Tree Traversal
```cpp
void render_tree() {
    render();                               // render self first
    for each child: child->render_tree();   // then children
}
void update_tree() {
    update();
    for each child: child->update_tree();
}
```

### Event Dispatching
```cpp
// called by ClassWindow — do not override
mouse_press_handler(button)  → on_press(button)
mouse_inbound_handler()      → on_inbound()
mouse_outbound_handler()     → on_outbound()
keyboard_handler(key,pressed)→ on_key(key, pressed)

// override these in subclasses
virtual void on_press(int button)          {}
virtual void on_inbound()                  {}
virtual void on_outbound()                 {}
virtual void on_key(int key, char pressed) {}

// override to draw / update
virtual void render() {}
virtual void update() {}
```

---

## 5. Button

### Purpose
Widget with callback-based rendering, update, and event handling.

### Callback Types
```cpp
using render_function = void(*)(Button *);
using update_function = void(*)(Button *, void *);
using event_function  = void(*)(Button *, void *);
```

### API
```cpp
Button btn(rect_t{{x, y}, {w, h}});

btn.set_render_function([](Button *btn) { /* draw */ });
btn.set_update_function([](Button *btn, void *data) { /* update */ }, data);

btn.on_click([](Button *btn, void *data) { /* left click */ }, data);
btn.set_on_inbound_event([](Button *btn, void *data) { /* hover in  */ }, data);
btn.set_on_outbound_event([](Button *btn, void *data) { /* hover out */ }, data);
```

### Hover example
```cpp
btn.set_render_function([](Button *btn) { fill_rect(btn, 0xFFFFFFFF); });

btn.set_on_inbound_event([](Button *btn, void *) {
    btn->set_render_function([](Button *btn) { fill_rect(btn, 0xFFFF0000); });
});
btn.set_on_outbound_event([](Button *btn, void *) {
    btn->set_render_function([](Button *btn) { fill_rect(btn, 0xFFFFFFFF); });
});
```

### Drag example
```cpp
btn.on_click([](Button *btn, void *) {
    ClassWindow *cw = findwindow(btn->get_associated_window());
    if (!cw) return;
    btn->set_update_function([](Button *btn, void *data) {
        position mouse = ((ClassWindow *)data)->get_mouse_position();
        rect_t b = btn->get_rect();
        btn->set_position({mouse.x - b.size.width  / 2,
                           mouse.y - b.size.height / 2});
    }, cw);
});
```

---

## 6. Frame

### Purpose
Container widget with a background render function.  
Groups child widgets, does not handle click or hover by itself.

```cpp
Frame page(rect_t{{0, 0}, {800, 600}});

page.set_render_function([](Frame *frame) {
    Window   *win  = frame->get_associated_window();
    position  real = get_real_position(frame);
    rect_t    b    = frame->get_rect();
    for (int dy = 0; dy < b.size.height; ++dy)
        for (int dx = 0; dx < b.size.width; ++dx)
            putpixel(win, real.x + dx, real.y + dy, 0xFF808080);
});

page.add_child(&btn);
window.set_widget(&page);
window.start_cycle();       // render_tree draws frame first, then btn
```

---

## 7. Extending Widget

```cpp
class TextInput : public Widget {
    std::string text;
public:
    TextInput(rect_t bound, Widget *parent = nullptr)
        : Widget(bound, parent) {}

    void render() override { /* draw text */ }

protected:
    void on_key(int key, char pressed) override {
        if (pressed) text += (char)key; // receives keys when focused
    }
};
```

Focus is set automatically by ClassWindow when the user clicks any widget.

---

## 8. Full Rendering Chain

```
start_cycle()
    └── root->render_tree()
            └── Frame::render() → putpixel() → wnd->pixels
            └── Button::render() → putpixel() → wnd->pixels
    └── WindowUpdate()
            └── WM_PAINT → BitBlt() → screen
```

`update_func` must only change state — never call `WindowUpdate` inside it.

---

## 9. Minimal Example

```cpp
#include "include/glib/glib.h"

int main() {
    ClassWindow window(800, 600);

    Frame page(rect_t{{0, 0}, {800, 600}});
    page.set_render_function([](Frame *f) {
        // draw background
    });

    Button btn(rect_t{{50, 50}, {150, 50}});
    btn.set_render_function([](Button *b) {
        // draw button
    });
    btn.on_click([](Button *b, void *) {
        // handle click
    });

    page.add_child(&btn);
    window.set_widget(&page);
    window.start_cycle();
}
```
