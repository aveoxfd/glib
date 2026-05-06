cmake_minimum_required(VERSION 3.15)
project(glib)

add_library(glib STATIC
    glib/src/ClassWindow.cpp
    glib/src/Widget.cpp
    glib/src/Event.cpp
)

target_include_directories(glib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set_target_properties(glib PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/glib/lib"
    OUTPUT_NAME "glib"
    PREFIX ""        
    SUFFIX ".lib"    
)

add_custom_command(TARGET glib POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        "$<TARGET_FILE:glib>"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/glib/glib.lib"
)