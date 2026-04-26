g++ -c glib/src/ClassWindow.cpp -o glib/bin/ClassWindow.o -I include/glib
g++ -c glib/src/Widget.cpp -o glib/bin/Widget.o -I include/glib
g++ -c glib/src/Event.cpp -o glib/bin/Event.o -I include/glib

ar rcs glib/lib/glib.lib glib/bin/ClassWindow.o glib/bin/Event.o glib/bin/Widget.o

copy glib\lib\glib.lib include\glib\

pause