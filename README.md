# glib

A lightweight C++ widget library for Windows built on top of **nwind** — a minimal WinAPI wrapper.  
Provides a widget tree, event routing, and a simple rendering pipeline via direct pixel access.

---

## Architecture

```
nwind                    — low-level window, bitmap, WinAPI events
  └── glib
        ├── ClassWindow  — links nwind window to a widget tree, routes events
        ├── Widget       — base class: tree, geometry, event dispatching
        └── Button       — widget with click / hover / render callbacks
```

---

## Getting Started

```cpp
#include "include/glib/glib.h"
#include "include/nwind/nwind.h"

int main() {
    ClassWindow window(800, 600);

    Widget root(rect_t{{0, 0}, {800, 600}});
    Button btn(rect_t{{50, 50}, {150, 50}});

    btn.set_render_function([](Widget *wid) {
        Window *win   = wid->get_associated_window();
        position real = get_real_position(wid);
        rect_t   b    = wid->get_rect();
        for (int dy = 0; dy < b.size.height; ++dy)
            for (int dx = 0; dx < b.size.width; ++dx)
                putpixel(win, real.x + dx, real.y + dy, 0xFFFFFFFF);
    });

    btn.on_click([](Widget *wid, void *) {
        // handle click
    });

    root.add_child(&btn);
    window.set_widget(&root);
    Window *win = root.get_associated_window();

    while (MessageProcess()) {
        ClearWindow(win, 0xFF222222);
        btn.update();
        btn.render();
        WindowUpdate(win);
    }
}
```

---

## Main Loop

The correct order every frame:

```cpp
while (MessageProcess()) {
    ClearWindow(win, background);  // 1. erase previous frame
    widget.update();               // 2. update state / position
    widget.render();               // 3. draw
    WindowUpdate(win);             // 4. present to screen
}
```

> ⚠️ Never call `WindowUpdate` inside `update_func` — it will cause a visual trail  
> because the frame is presented before `ClearWindow` runs.

---

## Widget

Base class for all UI elements. Manages the widget tree and dispatches input events.

```cpp
Widget root(rect_t{{0, 0}, {800, 600}});

// Tree
root.add_child(&child);
root.remove_child(&child);
Widget *hit = root.find_widget(mouse_pos); // deepest widget at position

// Geometry
root.get_rect();
root.set_position({x, y});
get_real_position(&root); // absolute position (sum of all ancestors)

// Render / update — override in subclasses or set via Button
virtual void render() {}
virtual void update() {}
```

### Event hooks (override in subclasses)

```cpp
virtual void on_press(int button) {}
virtual void on_inbound()         {}  // mouse entered
virtual void on_outbound()        {}  // mouse left
virtual void on_key(int key, char pressed) {}
```

---

## Button

Subclass of `Widget` with callback-based event handling and runtime-swappable render/update functions.

```cpp
Button btn(rect_t{{x, y}, {w, h}});
Button btn(rect_t{{x, y}, {w, h}}, &parent_widget); // with parent
```

### Render

```cpp
btn.set_render_function([](Widget *wid) {
    // draw pixels via putpixel()
});
```

### Update (runs every frame)

```cpp
btn.set_update_function([](Widget *wid, void *data) {
    // update position, state, etc.
    // do NOT call WindowUpdate here
}, user_data);
```

### Events

```cpp
btn.on_click([](Widget *wid, void *data) {
    // left mouse button click
}, user_data);

btn.set_on_inbound_event([](Widget *wid, void *data) {
    // mouse entered the widget bounds
}, user_data);

btn.set_on_outbound_event([](Widget *wid, void *data) {
    // mouse left the widget bounds
}, user_data);
```

### Hover color change example

```cpp
btn.set_render_function([](Widget *wid) { fill_rect(wid, 0xFFFFFFFF); });

btn.set_on_inbound_event([](Widget *wid, void *) {
    wid->set_render_function([](Widget *wid) { fill_rect(wid, 0xFFFF0000); });
});
btn.set_on_outbound_event([](Widget *wid, void *) {
    wid->set_render_function([](Widget *wid) { fill_rect(wid, 0xFFFFFFFF); });
});
```

### Drag example

```cpp
btn.on_click([](Widget *wid, void *) {
    ClassWindow *cw = findwindow(wid->get_associated_window());
    if (!cw) return;
    wid->set_update_function([](Widget *wid, void *data) {
        position mouse = ((ClassWindow *)data)->get_mouse_position();
        rect_t b = wid->get_rect();
        wid->set_position({mouse.x - b.size.width / 2,
                           mouse.y - b.size.height / 2});
    }, cw);
});
```

---

## ClassWindow

```cpp
ClassWindow window(800, 600);   // creates window 800×600
window.set_widget(&root);       // attach root widget

window.get_mouse_position();    // current cursor position
window.get_mouse_button();      // last pressed button
window.get_keyboard_key();      // last pressed key
window.update();                // calls WindowUpdate (use only if needed outside main loop)

findwindow(Window *native);     // find ClassWindow by nwind Window*
```

---

## nwind API

Low-level functions used directly for rendering:

```c
putpixel(Window *wnd, int x, int y, unsigned int color); // write pixel (ARGB)
ClearWindow(Window *wnd, int color);                     // fill entire window
WindowUpdate(Window *wnd);                               // present frame to screen
MessageProcess();                                        // process event queue, returns 0 on close
```

---

## Widget Tree

Widgets form a parent-child tree. Position is relative to the parent.  
`get_real_position()` returns the absolute screen position by summing all ancestors.

```cpp
Widget root(rect_t{{0, 0}, {800, 600}});
Button panel(rect_t{{100, 100}, {300, 200}});
Button btn(rect_t{{10, 10}, {80, 30}});  // at (110, 110) on screen

root.add_child(&panel);
panel.add_child(&btn);

window.set_widget(&root);
// set_widget recursively propagates the Window* to all children
```

`find_widget(pos)` searches children from last to first (last added = drawn on top).

---

## Extending Widget

Create your own widget type by inheriting from `Widget`:

```cpp
class MyWidget : public Widget {
    bool active = false;

public:
    MyWidget(rect_t bound, Widget *parent = nullptr)
        : Widget(bound, parent) {}

    void render() override {
        fill_rect(this, active ? 0xFF00FF00 : 0xFF333333);
    }

protected:
    void on_press(int button) override {
        if (button == 0) active = !active;
    }
    void on_inbound()  override { /* mouse entered */ }
    void on_outbound() override { /* mouse left    */ }
};
```

---

## Platform

- **Windows only** (uses WinAPI, DIB sections, `GetCursorPos`)
- C++17 or later
