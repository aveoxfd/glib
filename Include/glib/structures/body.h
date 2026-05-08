#ifndef BODY_H
#define BODY_H

#include "position.h"

struct Body{
    position center;
    position *nodes;
    int nodes_count;
}; typedef Body Body, Body_t;

#endif