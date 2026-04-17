#include "Event.h"

Event::Event(EventFunction function = nullptr, Widget *connected = nullptr):event_function(function), connection(connected){
}
void Event::activate(Widget *sender = nullptr){
    if (!event_function)return;
    if (sender == nullptr){
        if (connection){
            event_function(connection);
        }
        else{}
    }
    else{
        event_function(sender);
    }
    return;
}