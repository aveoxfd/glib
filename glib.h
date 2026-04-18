#ifndef GLIB_G
#define GLIB_H

#define GLIBAPI

#ifdef DLLBUILD
#define GLIBAPI __attribute__(dllexport)
#endif

#if defined (USEDLL)
#define GLIBAPI __attribute__(dllimport)
#endif

#endif