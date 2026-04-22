#include "Event.h"
#include "Widget.h"

Event::Event(EventFunction function, void* data, Widget *connected):
event_function(function),
user_data(data),
connection(connected){
}
Event::Event(EventFunction function):
event_function(function),
user_data(nullptr),
connection(nullptr){
}

void Event::activate(Widget *sender){
    if (!event_function)return;
    event_function(sender ? sender : connection, user_data);
    return;
}

Event::~Event(){
}