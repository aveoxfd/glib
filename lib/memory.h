#ifndef MEMORY_H
#define MEMORY_H
#define MEM_API

#if defined(MEMORY_STATIC)
    #define MEM_API
    #elif defined(_WIN32) || defined(_WIN64) || defined(_CYGWIN__)
        #ifdef MEM_BUILD_DLL
            #define MEM_API __declspec(dllexport)
        #else
            #define MEM_API __declspec(dllimport)
    #endif
    #else
        #define MEM_API
#endif

#define FREE        0b00000001
#define IS_USING    0b00000010

typedef unsigned long long size_t;

MEM_API void* cmalloc(size_t size);
MEM_API void cfree(void *block); //free with merge
MEM_API void* crealloc(void *block, size_t size);

#endif