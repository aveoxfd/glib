#ifndef EVENT_H
#define EVENT_H

class Widget;

class Event{
    private:
    typedef void(*EventFunction)(Widget* sender);
    EventFunction event_function;
    Widget *connection;
    public:
    Event(EventFunction function = nullptr, Widget *connected = nullptr);
    Event(EventFunction function);
    void activate(Widget *sender = nullptr);
};

#endif