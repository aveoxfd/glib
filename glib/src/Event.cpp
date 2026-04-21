#include "Event.h"
#include "Widget.h"

Event::Event(EventFunction function, Widget *connected):event_function(function), connection(connected){
}
Event::Event(EventFunction function):event_function(function){}

void Event::activate(Widget *sender){
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