#ifndef RECTANGLE_H
#define RECTANGLE_H

#include "position.h"
#include "size.h"

struct rectangle{
    position pos;
    size size;
};typedef rectangle rectangle, rectangle_t, rect, rect_t;

#endif