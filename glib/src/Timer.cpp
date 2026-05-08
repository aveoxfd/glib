#include <Windows.h>
/*
https://learn.microsoft.com/en-us/windows/win32/api/winuser/nc-winuser-timerproc


If the function succeeds and the hWnd parameter is NULL, the return value is an integer identifying the new timer. 
An application can pass this value to the KillTimer function to destroy the timer.

If the function succeeds and the hWnd parameter is not NULL, then the return value is a nonzero integer. 
An application can pass the value of the nIDEvent parameter to the KillTimer function to destroy the timer.

If the function fails to create a timer, the return value is zero. To get extended error information, call GetLastError.
*/


#include "../../include/glib/ClassWindow.h"

static void CALLBACK GlobalTimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime){
    //...
}