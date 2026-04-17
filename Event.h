#ifndef EVENT_H
#define EVENT_H

#include "Widget.h"

class Event{
    private:
    typedef void(*EventFunction)(Widget* sender);
    EventFunction event_function;
    Widget *connection;
    public:
    Event(EventFunction function = nullptr, Widget *connected = nullptr);
    void activate(Widget *sender = nullptr);
};

#endif