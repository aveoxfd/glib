#include "Event.h"
#include "Widget.h"

Event::Event(EventFunction function, void* user_data, Widget *connected):
event_function(function),
user_data(user_data),
connection(connected){
}

void Event::activate(Widget *sender){
    if (!event_function)return;
    event_function(sender ? sender : connection, user_data);
    return;
}

Event::~Event(){
}