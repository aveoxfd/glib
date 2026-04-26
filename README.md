# glib

A lightweight C++ widget library for Windows built on top of **nwind** — a minimal WinAPI wrapper.  
Provides a widget tree, automatic event routing, and a pixel-based rendering pipeline.

---

## Architecture

```
nwind                    — low-level window, bitmap, WinAPI events
  └── glib
        ├── ClassWindow  — links nwind window to widget tree, routes events
        ├── Widget       — base class: tree, geometry, event dispatching
        ├── Button       — widget with click / hover / render callbacks
        └── Frame        — container with background render
```

---

## Getting Started

```cpp
#include "include/glib/glib.h"

int main() {
    ClassWindow window(800, 600);

    Frame page(rect_t{{0, 0}, {800, 600}});
    page.set_render_function([](Frame *f) {
        Window   *win  = f->get_associated_window();
        position  real = get_real_position(f);
        rect_t    b    = f->get_rect();
        for (int dy = 0; dy < b.size.height; ++dy)
            for (int dx = 0; dx < b.size.width; ++dx)
                putpixel(win, real.x + dx, real.y + dy, 0xFF222222);
    });

    Button btn(rect_t{{50, 50}, {150, 50}});
    btn.set_render_function([](Button *b) {
        Window   *win  = b->get_associated_window();
        position  real = get_real_position(b);
        rect_t    bd   = b->get_rect();
        for (int dy = 0; dy < bd.size.height; ++dy)
            for (int dx = 0; dx < bd.size.width; ++dx)
                putpixel(win, real.x + dx, real.y + dy, 0xFFFFFFFF);
    });
    btn.on_click([](Button *b, void *) {
        // handle click
    });

    page.add_child(&btn);
    window.set_widget(&page);
    window.start_cycle();           // handles everything automatically
}
```

---

## Main Loop

`start_cycle()` handles the loop automatically:

```cpp
window.start_cycle();
// internally:
// while (MessageProcess()) {
//     root->update_tree();   // update all widgets recursively
//     root->render_tree();   // render all widgets recursively
//     WindowUpdate(window);  // present frame to screen
// }
```

> ⚠️ Never call `WindowUpdate` inside `update_func` — it presents the frame before the tree  
> has finished updating, which causes a visual trail.

---

## Widget

Base class for all UI elements. Manages the widget tree and dispatches input events.  
Not visible by itself — subclasses define how they look and behave.

```cpp
// Tree
widget.add_child(&child);
widget.remove_child(&child);
Widget *hit = widget.find_widget(pos); // deepest widget at position

// Geometry
widget.get_rect();
widget.set_position({x, y});
get_real_position(&widget);  // absolute screen position (sums all ancestors)
                             // bound.pos is relative to parent — always use this for drawing
```

### Tree traversal

```cpp
widget.render_tree();   // render self, then all children recursively
widget.update_tree();   // update self, then all children recursively
```

`start_cycle()` calls both automatically every frame.

### Event hooks — override in subclasses

```cpp
virtual void on_press(int button)          {}  // left click: button == 0
virtual void on_inbound()                  {}  // mouse entered
virtual void on_outbound()                 {}  // mouse left
virtual void on_key(int key, char pressed) {}  // keyboard (when focused)
```

Keyboard focus is set automatically when the user clicks a widget.

---

## Button

Subclass of `Widget`. Callbacks accept `Button*` — all Button methods are available inside them.

```cpp
Button btn(rect_t{{x, y}, {w, h}});
Button btn(rect_t{{x, y}, {w, h}}, &parent_widget);
```

### Render

```cpp
btn.set_render_function([](Button *btn) {
    // draw via putpixel()
    // use get_real_position(btn) for absolute coordinates
});
```

### Update (runs every frame)

```cpp
btn.set_update_function([](Button *btn, void *data) {
    // update position, state, etc.
    // do NOT call WindowUpdate here
}, user_data);
```

### Events

```cpp
btn.on_click([](Button *btn, void *data) {
    // left mouse button
}, user_data);

btn.set_on_inbound_event([](Button *btn, void *data) {
    // mouse entered
}, user_data);

btn.set_on_outbound_event([](Button *btn, void *data) {
    // mouse left
}, user_data);
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
        ClassWindow *cw = (ClassWindow *)data;
        position mouse  = cw->get_mouse_position();
        rect_t b        = btn->get_rect();
        btn->set_position({mouse.x - b.size.width  / 2,
                           mouse.y - b.size.height / 2});
    }, cw);
});
```

---

## Frame

Container widget with a background render function. Groups child widgets.  
Does not handle click or hover by itself.

```cpp
Frame page(rect_t{{0, 0}, {800, 600}});

page.set_render_function([](Frame *f) {
    // draw background
});

page.add_child(&btn1);
page.add_child(&btn2);

window.set_widget(&page);
window.start_cycle();   // Frame renders first, then children via render_tree
```

---

## ClassWindow

```cpp
ClassWindow window(800, 600);
window.set_widget(&root);       // attach root, propagates Window* to whole tree
window.start_cycle();           // enter main loop

window.get_mouse_position();    // current cursor position
window.get_mouse_button();      // last pressed button
window.get_keyboard_key();      // last pressed key
window.set_focus(widget);       // manually set keyboard focus
window.get_focused();           // get currently focused widget
window.update();                // WindowUpdate — use outside start_cycle if needed

findwindow(Window *native);     // find ClassWindow by nwind Window*
```

---

## nwind API

Low-level functions used directly for rendering:

```c
putpixel(Window *wnd, int x, int y, unsigned int color); // write pixel (ARGB)
ClearWindow(Window *wnd, int color);                     // fill entire window
WindowUpdate(Window *wnd);                               // present frame to screen
MessageProcess();                                        // process event queue, 0 on close
```

---

## Widget Tree

Widgets form a parent-child tree. `bound.pos` is relative to the parent.  
Use `get_real_position()` to get the absolute screen coordinates for drawing.

```cpp
Frame page(rect_t{{0, 0}, {800, 600}});
Frame panel(rect_t{{100, 100}, {300, 200}});
Button btn(rect_t{{10, 10}, {80, 30}});  // drawn at (110, 110) on screen

page.add_child(&panel);
panel.add_child(&btn);

window.set_widget(&page);
// set_widget recursively propagates Window* to all children
```

`find_widget(pos)` searches children from last to first — last added is drawn on top.

---

## Custom Widgets

Inherit from `Widget` and override virtual methods:

```cpp
class Toggle : public Widget {
    bool active = false;

public:
    Toggle(rect_t bound, Widget *parent = nullptr)
        : Widget(bound, parent) {}

    void render() override {
        Window   *win  = get_associated_window();
        position  real = get_real_position(this);
        rect_t    b    = get_rect();
        unsigned int color = active ? 0xFF00FF00 : 0xFF333333;
        for (int dy = 0; dy < b.size.height; ++dy)
            for (int dx = 0; dx < b.size.width; ++dx)
                putpixel(win, real.x + dx, real.y + dy, color);
    }

protected:
    void on_press(int button) override {
        if (button == 0) active = !active;
    }
    void on_key(int key, char pressed) override {
        // receives keyboard input when focused
    }
};
```

---

## Platform

- **Windows only** (WinAPI, DIB sections, `GetCursorPos`)
- C++17 or later
