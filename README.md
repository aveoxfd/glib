# glib

A lightweight C++ widget library for Windows built on top of **nwind** — a minimal WinAPI wrapper.  
Provides a widget tree, automatic event routing, and a pixel-based rendering pipeline.

> **Platform:** Windows only (WinAPI, DIB sections) · **Language:** C++17 or later

---

## Table of Contents

- [Architecture](#architecture)
- [Prerequisites](#prerequisites)
- [Building the Library](#building-the-library)
- [Linking in Your Project](#linking-in-your-project)
- [Quick Start](#quick-start)
- [Main Loop](#main-loop)
- [Widget Tree](#widget-tree)
- [ClassWindow](#classwindow)
- [Widget](#widget)
- [Button](#button)
- [Frame](#frame)
- [Custom Widgets](#custom-widgets)
- [Data Structures](#data-structures)
- [nwind API Reference](#nwind-api-reference)
- [Known Limitations](#known-limitations)

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

**Rendering pipeline:**
```
putpixel(wnd, x, y, color)
    └── wnd->pixels[y * width + x]   // write directly to bitmap memory

WindowUpdate(wnd)
    └── InvalidateRect()
    └── WM_PAINT → BitBlt()          // copy bitmap to screen

start_cycle()
    └── root->update_tree()
    └── root->render_tree()
    └── WindowUpdate()
```

---

## Prerequisites

- Windows (x86 or x86-64)
- MinGW-w64 (g++) or MSVC
- CMake 3.15 or later (optional — a manual build script is also provided)

---

## Building the Library

### Option A — CMake (recommended)

```bat
cmakerun.bat
```

Internally runs:

```bat
cmake -B build -S . -G "MinGW Makefiles"
cmake --build build
```

Output: `glib/lib/glib.lib` and a copy at `include/glib/glib.lib`.

### Option B — Manual (g++ + ar)

```bat
buildlib.bat
```

```bat
g++ -c glib/src/ClassWindow.cpp -o glib/bin/ClassWindow.o -I include/glib
g++ -c glib/src/Widget.cpp     -o glib/bin/Widget.o     -I include/glib
g++ -c glib/src/Event.cpp      -o glib/bin/Event.cpp    -I include/glib

ar rcs glib/lib/glib.lib glib/bin/ClassWindow.o glib/bin/Event.o glib/bin/Widget.o

copy glib\lib\glib.lib include\glib\
```

### Debug Build

```bat
libdebug.bat
```

Same as Option B but with `-DDEBUG`. Enables diagnostic output in `Widget::find_widget`.

---

## Linking in Your Project

```
-L include/nwind include/nwind/nwind.lib
-L include/glib  include/glib/glib.lib
```

Full g++ example:

```bat
g++ main.cpp -I path/to/glib/include -L path/to/glib/include/glib -lglib ^
             -L path/to/glib/include/nwind -lnwind -o myapp.exe
```

Include the single top-level header in your source:

```cpp
#include "include/glib/glib.h"
```

---

## Quick Start

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
    window.start_cycle();
}
```

---

## Main Loop

`start_cycle()` runs the message loop and drives the widget tree automatically:

```cpp
// Internally:
while (MessageProcess()) {
    root->update_tree();   // update all widgets recursively
    root->render_tree();   // render all widgets recursively
    WindowUpdate(window);  // present frame to screen
}
```

> ⚠️ Never call `WindowUpdate` inside an `update_func` — it presents the frame before the tree
> has finished updating, which causes a visual trail.

---

## Widget Tree

Widgets form a parent-child tree. `bound.pos` is always relative to the parent.  
Use `get_real_position()` to obtain absolute screen coordinates for drawing.

```cpp
Frame page(rect_t{{0, 0}, {800, 600}});
Frame panel(rect_t{{100, 100}, {300, 200}});
Button btn(rect_t{{10, 10}, {80, 30}});  // drawn at (110, 110) on screen

page.add_child(&panel);
panel.add_child(&btn);

window.set_widget(&page);
// set_widget recursively propagates Window* to all children
```

`find_widget(pos)` searches children from last to first — the last added child is drawn on top
and receives events first.

---

## ClassWindow

Links an nwind `Window*` to the widget tree. Maintains a global registry of all windows and
routes mouse and keyboard events to the correct widget.

```cpp
ClassWindow window(800, 600);
window.set_widget(&root);       // attach root; propagates Window* to whole tree
window.start_cycle();           // enter main loop

window.get_mouse_position();    // current cursor position
window.get_mouse_button();      // last pressed button index
window.get_keyboard_key();      // last pressed key code
window.set_focus(widget);       // manually set keyboard focus
window.get_focused();           // get currently focused widget
window.update();                // call WindowUpdate manually (outside start_cycle)

findwindow(Window *native);     // find ClassWindow by nwind Window*
```

**Event routing — mouse click:**
```
mouse_button_callback()
    └── GetCursorPos() + ScreenToClient()
    └── findwindow(wnd)  → ClassWindow*
    └── root->find_widget(pos) → Widget*
    └── set_focus(target)           // keyboard focus follows click
    └── target->mouse_press_handler(button)
```

**Event routing — mouse move:**
```
mouse_move_callback()
    └── find_widget(pos) → target
    └── if target changed:
            last_hovered->mouse_outbound_handler()
            target->mouse_inbound_handler()
```

---

## Widget

Base class for all UI elements. Manages geometry, the parent-child tree, and event dispatching.
Not visible by itself — subclasses define appearance and behavior.

### Construction

```cpp
// Rectangular widget — bound.pos is relative to parent
Widget(rect_t bound, Widget *parent = nullptr);

// Polygon widget — bounding box is computed automatically from vertices
Widget(Body body, Widget *parent = nullptr);
```

### Tree operations

```cpp
widget.add_child(&child);             // attach child (detaches from previous parent)
widget.remove_child(&child);          // detach child
Widget *hit = widget.find_widget(pos);// deepest widget at screen position pos
```

### Geometry

```cpp
widget.get_rect();                    // returns bound (relative to parent)
widget.set_position({x, y});          // move widget within parent
get_real_position(&widget);           // absolute screen position — always use for drawing
```

### Tree traversal

```cpp
widget.render_tree();   // render self, then all children recursively
widget.update_tree();   // update self, then all children recursively
```

### Virtual hooks — override in subclasses

```cpp
virtual void render() {}                        // called each frame to draw
virtual void update() {}                        // called each frame to update state

virtual void on_press(int button)          {}   // left click: button == 0
virtual void on_inbound()                  {}   // mouse entered
virtual void on_outbound()                 {}   // mouse left
virtual void on_key(int key, char pressed) {}   // keyboard (when focused)
```

Keyboard focus is set automatically when the user clicks a widget.

---

## Button

Subclass of `Widget` with callback-based rendering, update, and event handling.  
All callbacks receive `Button*` — all Button methods are accessible inside them.

### Construction

```cpp
Button btn(rect_t{{x, y}, {w, h}});
Button btn(rect_t{{x, y}, {w, h}}, &parent_widget);
Button btn(hexBody);              // polygon shape — bounding box computed from vertices
```

### Render

```cpp
btn.set_render_function([](Button *btn) {
    // draw with putpixel()
    // always use get_real_position(btn) for absolute coordinates
});
```

### Update (called every frame)

```cpp
btn.set_update_function([](Button *btn, void *data) {
    // update position, state, etc.
    // do NOT call WindowUpdate here
}, user_data);
```

### Events

```cpp
btn.on_click([](Button *btn, void *data) {
    // left mouse button released
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
btn.set_render_function([](Button *btn) { /* draw normal */ });

btn.set_on_inbound_event([](Button *btn, void *) {
    btn->set_render_function([](Button *btn) { /* draw highlighted */ });
});
btn.set_on_outbound_event([](Button *btn, void *) {
    btn->set_render_function([](Button *btn) { /* draw normal */ });
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
Does not handle click or hover events by itself.

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

## Custom Widgets

Inherit from `Widget` and override the virtual methods:

```cpp
class Toggle : public Widget {
    bool active = false;

public:
    Toggle(rect_t bound, Widget *parent = nullptr)
        : Widget(bound, parent) {}

    void render() override {
        Window   *win   = get_associated_window();
        position  real  = get_real_position(this);
        rect_t    b     = get_rect();
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

## Data Structures

```cpp
struct position { int x, y; };
// aliases: point, point_t, position_t

struct size     { int width, height; };

struct rectangle {
    position pos;   // top-left corner, relative to parent
    size     size;
};
// aliases: rect_t, rectangle_t, rect

struct Body {
    position  center;       // screen position of the polygon's center
    position *nodes;        // vertices relative to center
    int       nodes_count;
};
```

Colors passed to `putpixel` are 32-bit **ARGB**: `0xAARRGGBB`.  
Alpha is currently ignored by nwind — use `0xFF` for the high byte.

---

## nwind API Reference

Low-level functions used directly for rendering and window management:

```c
// Rendering
putpixel(Window *wnd, int x, int y, unsigned int color); // write pixel (0xAARRGGBB)
ClearWindow(Window *wnd, int color);                     // fill entire window
WindowUpdate(Window *wnd);                               // present frame to screen

// Message loop
unsigned char MessageProcess(void);                      // process queue; returns 0 on close

// Window lifecycle
Window* WindowCreate(int width, int height, const char *name);
void    WindowDestroy(Window *wnd);

// Timers
unsigned long long WindowStartTimer(Window *wnd, unsigned int interval_ms, void *proc);
int                WindowKillTimer(Window *wnd, unsigned long long id);

// Event callbacks
void WindowSetKeyCallback(Window *wnd, WindowKeyCallback cb);
void WindowSetMouseButtonCallback(Window *wnd, WindowMouseButtonCallback cb);
void WindowSetMouseMoveCallback(Window *wnd, WindowMouseMoveCallback cb);
void WindowSetTimerCallback(Window *wnd, WindowTimerCallback cb);
```

---

## File Structure

```
include/
├── glib/
│   ├── glib.h              — single include for consumers
│   ├── glib_api.h          — GLIBAPI export macro
│   ├── glib.lib            — import library (copied here after build)
│   ├── ClassWindow.h
│   ├── Event.h
│   ├── structures/
│   │   ├── position.h
│   │   ├── size.h
│   │   ├── rectangle.h
│   │   ├── body.h
│   │   └── structs.h       — includes all structures
│   └── widget/
│       ├── Widget.h
│       ├── Button.h
│       └── Frame.h
└── nwind/
    ├── nwind.h
    ├── nwind.dll
    └── nwind.lib

glib/
├── src/
│   ├── ClassWindow.cpp
│   ├── Widget.cpp
│   ├── Event.cpp
│   ├── Thread.cpp
│   └── Timer.cpp
├── bin/                    — intermediate .o files
└── lib/                    — output glib.lib
```

---

## Known Limitations

- Windows only — depends on WinAPI (`GetCursorPos`, `BitBlt`, DIB sections).
- `putpixel` writes to a CPU bitmap; no GPU acceleration.
- No built-in text rendering — draw text through GDI or a third-party rasterizer.
- Timer and Thread implementations are pending (see `TODO.md`).
