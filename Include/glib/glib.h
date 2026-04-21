#ifndef GLIB_H
#define GLIB_H

#define GLIBAPI

#ifdef DLLBUILD
#define GLIBAPI __attribute__(dllexport)
#endif

#if defined (USEDLL)
#define GLIBAPI __attribute__(dllimport)
#endif

#include "../nwind/nwind.h"
#include "structures/structs.h"
#include "ClassWindow.h"
#include "Widget.h"
#include "Event.h"

#endif