#ifndef WIDGET_H
#define WIDGET_H


#include "../glib_api.h"
#include "../structures/structs.h"

GLIBAPI class Widget{

    private:
    Widget *parent;
    Widget **children;
    int children_count = 0;
    Window *association;
    rect_t bound;

    friend position get_real_position(Widget *widget);
    bool contains(position pos);

    public:
    Widget();
    Widget(rect_t bound, Widget *parent = nullptr);
    virtual ~Widget() = default;
    Widget* find_widget(position pos);
    void add_child(Widget *child_widget);
    void remove_child(Widget *child_widget);
    Window* get_associated_window(void); //associated
    void set_associated_window(Window *association);
    rect_t get_rect();
    void set_position(position_t new_position){bound.pos = new_position;}
    void update_tree();
    void render_tree();

    virtual void render() {
        
    }
    virtual void update() {
        
    }
    
    void mouse_press_handler(int button)            {on_press(button); }
    void mouse_inbound_handler()                    {on_inbound();     }
    void mouse_outbound_handler()                   {on_outbound();    }
    void keyboard_handler(int key, char pressed)    {on_key(key, pressed);}

    protected:

    virtual void on_press(int button)           {}
    virtual void on_inbound()                   {}
    virtual void on_outbound()                  {}
    virtual void on_key(int key, char pressed)  {}

};

#endif