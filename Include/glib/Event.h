#ifndef EVENT_H
#define EVENT_H

class Widget;

class Event{
    private:
    typedef void(*EventFunction)(Widget* sender, void *data);
    EventFunction event_function;
    Widget *connection;
    void *user_data;
    public:
    Event(EventFunction function, void* data = nullptr, Widget *connected = nullptr);
    Event(EventFunction function);
    void activate(Widget *sender = nullptr);
    void* get_data() const { return user_data; }
};

#endif