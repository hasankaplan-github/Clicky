#include "XHelper.h"

#include <stdio.h>
#include <stdlib.h>

XErrorHandler oldHandler = NULL;
Window defaultRootWindow = 0;
Display *display = NULL;
//Time previousEventTime = 0;

int majorOpcode, firstEvent, firstError;
Bool isXI2Supported = False;

Display *
InitX ( XErrorHandler newErrorHandler )
{
	display = XOpenDisplay( NULL );
	if ( display == NULL )
	{
		//fprintf( stderr, "Clicky: Cannot open display\n" );
		printf( "Clicky: Cannot open display\n" );
		exit( 0 );
	}
	if ( newErrorHandler != NULL )
	{
		oldHandler = XSetErrorHandler( newErrorHandler );
	}
	defaultRootWindow = XDefaultRootWindow( display );
	isXI2Supported = XQueryExtension( display, "XInputExtension", &majorOpcode,
											&firstEvent,
											&firstError );
	return display;
}

void
CloseX ( void )
{
	if ( display != NULL )
	{
		XCloseDisplay( display );
		display = NULL;
	}
}

void
SubscribeNewWindowEvent ( void )
{
	XSelectInput( display, defaultRootWindow, SubstructureNotifyMask );
}

void
SubscribeClickEvent ( Window window )
{
	XIEventMask xiEventMask;
	xiEventMask.deviceid = XIAllMasterDevices;     // XIAllDevices;
	xiEventMask.mask_len = XIMaskLen( XI_LASTEVENT );
	unsigned char mask[ XIMaskLen( XI_LASTEVENT ) ] = { 0 };
	xiEventMask.mask = mask;     //calloc( xiEventMask.mask_len, sizeof(char) );     // must be initially null
	XISetMask( xiEventMask.mask, XI_ButtonPress );
	XISetMask( xiEventMask.mask, XI_ButtonRelease );

	XISelectEvents( display, window, &xiEventMask, 1 );
	// XSync( display, True );
	//free( xiEventMask.mask );

}

void
GetAllWindows ( Window **windows_out,
				long unsigned int *windowCount_out )
{
	Atom atom = XInternAtom( display, "_NET_CLIENT_LIST", False );
	Atom actualType;
	int actualFormat;
	unsigned long remain;

	XGetWindowProperty( display, defaultRootWindow, atom, 0, 1024, False, XA_WINDOW,
						&actualType, &actualFormat, windowCount_out, &remain,
						windows_out );
}

int
WaitNextEvent ( XEvent *xEvent )
{
	return XNextEvent( display, xEvent );
}

Bool
IsXiDeviceEvent ( XEvent *xEvent )
{
	if ( isXI2Supported == True &&
			xEvent->xcookie.type == GenericEvent &&
			xEvent->xcookie.extension == majorOpcode &&
			XGetEventData( display, &xEvent->xcookie ) == True )
	{
		return True;
	}

	return False;
}

void
FreeEventData ( XEvent *xEvent )
{
	XFreeEventData( display, &xEvent->xcookie );
}

Status
GetCommand ( 	Window window,
				char ***args,
				int *argCount )
{
	Status status = XGetCommand( display, window, args, argCount );

	return status;
}
