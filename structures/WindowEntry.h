#ifndef WINDOWENTRY_H
#define WINDOWENTRY_H

typedef struct Window Window;
class ClassWindow;

struct WindowEntry{
    Window *native;
    ClassWindow *owner;
};

#endif