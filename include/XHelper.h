#ifndef XHELPER_H_INCLUDED
#define XHELPER_H_INCLUDED


#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XInput2.h>

#include <X11/cursorfont.h>
#include <X11/Xcursor/Xcursor.h>


Display *
InitX ( XErrorHandler newErrorHandler );

void
CloseX ( void );

void
SubscribeNewWindowEvent ( void );

void
SubscribeClickEvent ( Window window );

void
GetAllWindows ( Window **windows_out,
				long unsigned int *windowCount_out );

int
WaitNextEvent ( XEvent *xEvent );

Bool
IsXiDeviceEvent ( XEvent *xEvent );

void
FreeEventData ( XEvent *xEvent );

Status
GetCommand ( 	Window window,
				char ***args,
				int *argCount );

void
ActivateWatchCursor ( Window window );

void
ActivateLeftPtrCursor ( Window window );

void
SendDummyEvent ( void );

#endif // XHELPER_H_INCLUDED
