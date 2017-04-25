/*
 ============================================================================
 Name        : Clicky.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "XHelper.h"
#include "SignalHelper.h"

#define CONFIG_DIR_PATH "~/.config/Clicky"
#define DISPATCHER_DIR_PATH CONFIG_DIR_PATH
#define DISPATCHER_FILE_PATH DISPATCHER_DIR_PATH "/EventDispatcher.sh"
#define EVENT_HANDLER_FILE_EXTENSION ".EventHandler.sh"
#define DEFAULT_EVENT_HANDLER_FILE_PATH CONFIG_DIR_PATH "/Default" EVENT_HANDLER_FILE_EXTENSION

#define PREPARE_DISPATCHER_FULL_COMMAND(fullCommand, format, ...)	\
	int fullCommandSize = snprintf( NULL, 0, format, __VA_ARGS__ );	\
	fullCommand = malloc( fullCommandSize + 3 );	\
	sprintf( fullCommand, format "", __VA_ARGS__ );

Bool Continue = True;

int
XCustomErrorHandler ( 	Display *display,
						XErrorEvent *xErrorEvent )
{
	printf( "\nErrorCode: %d\nMinorCode: %d\nRequestCode: %d\nResourceID: %ld\nSerial: %ld\nType: %d\n",
			xErrorEvent->error_code,
			xErrorEvent->minor_code,
			xErrorEvent->request_code,
			xErrorEvent->resourceid,
			xErrorEvent->serial,
			xErrorEvent->type );

	return 1;
}

Bool
IsConkyWindow ( Window window )
{
	Bool isConkyWindow = False;
	char **args;
	int argCount;
	int status = GetCommand( window, &args, &argCount );
	if ( status != 0 )
	{
		if ( argCount > 0 && strcmp( args[ 0 ], "conky" ) == 0 )
		{
			isConkyWindow = True;
		}
		XFreeStringList( args );
	}

	return isConkyWindow;
}

void
NewWindowEventHandler ( Window window )
{
	if ( IsConkyWindow( window ) )
	{
		SubscribeClickEvent( window );
	}
}

void
HandleOpenedWindows ( void )
{
	Window *windows = NULL;
	unsigned long int windowCount = 0;
	GetAllWindows( &windows, &windowCount );

	for ( int i = 0; i < windowCount; ++i )
	{
		NewWindowEventHandler( windows[ i ] );
	}
	free( windows );
}

char *
PrepareEventHandlerFilePath ( Window window )
{
	char *eventHandlerFilePath = NULL;

	char **args;
	int argCount;
	Status status = GetCommand( window, &args, &argCount );
	if ( status != 0 )
	{
		for ( int i = 1; i < argCount; ++i )
		{
			int found = strcmp( args[ i ], "-c" );
			if ( found == 0 )     // "-c" style arg was found process it
			{
				int len = strlen( args[ i + 1 ] ) + strlen( EVENT_HANDLER_FILE_EXTENSION )
						+ 1;     // +1 for null termination
				eventHandlerFilePath = malloc( len );
				if ( eventHandlerFilePath != NULL )
				{
					eventHandlerFilePath[ 0 ] = '\0';
					strcpy( eventHandlerFilePath, args[ i + 1 ] );
					strcat( eventHandlerFilePath, EVENT_HANDLER_FILE_EXTENSION );
				}
				break;
			}
			else     // search for "--config" style arg
			{
				found = strncmp( args[ i ], "--config=", 9 );
				if ( found == 0 )     // "--config" style arg was found process it
				{
					int len = strlen( args[ i ] ) - 9     // -9 for "--config="
					+ strlen( EVENT_HANDLER_FILE_EXTENSION ) + 1;     // +1 for null termination
					eventHandlerFilePath = malloc( len );
					if ( eventHandlerFilePath != NULL )
					{
						eventHandlerFilePath[ 0 ] = '\0';
						strcpy( eventHandlerFilePath, args[ i ] + 9 );
						strcat( eventHandlerFilePath, EVENT_HANDLER_FILE_EXTENSION );
					}
					break;
				}
			}
		}
		XFreeStringList( args );
	}

	if ( eventHandlerFilePath == NULL )
	{
		eventHandlerFilePath = strdup( DEFAULT_EVENT_HANDLER_FILE_PATH );
	}

	return eventHandlerFilePath;
}

void
ClickEventHandler ( XEvent *xEvent )
{
	XGenericEventCookie *cookie = &xEvent->xcookie;
	XIDeviceEvent *xiDeviceEvent = cookie->data;
	Window window = xiDeviceEvent->event;
	ActivateWatchCursor( window );

	char *dispatcherFullCommand;
	char *dispatcherCommand = "sh " DISPATCHER_FILE_PATH;
	char *eventHandlerFilePath = PrepareEventHandlerFilePath( window );
	PREPARE_DISPATCHER_FULL_COMMAND( dispatcherFullCommand,
										"%s %ld %d %d %f %f %f %f \"%s\"",
									dispatcherCommand,
										window,/*window in which event occurs*/
										cookie->evtype, /*button_press, release, key_press ...*/
										xiDeviceEvent->detail, /*button number or keycode */
										xiDeviceEvent->event_x,
										xiDeviceEvent->event_y,
										xiDeviceEvent->root_x,
										xiDeviceEvent->root_y,
										eventHandlerFilePath );
	system( dispatcherFullCommand );
	free( eventHandlerFilePath );
	free( dispatcherFullCommand );
	ActivateLeftPtrCursor( window );
}

void
ExitSignalsHandler ( int sigNum )
{
	// needs some change to reflect signals immediately.
	Continue = False;
}

void
EventLoop ( void )
{
	XEvent xEvent;
	while ( WaitNextEvent( &xEvent ) == 0 && Continue )
	{
		switch ( xEvent.type )
		{
			case CreateNotify:     // new window created event
				NewWindowEventHandler( xEvent.xcreatewindow.window );
				break;
			default:     // check if xi event occurred
				if ( IsXiDeviceEvent( &xEvent ) )
				{
					switch ( xEvent.xcookie.evtype )
					{
						case XI_ButtonPress:
							case XI_ButtonRelease:
							ClickEventHandler( &xEvent );
							break;
					}
					FreeEventData( &xEvent );
				}
				break;
		}
	}
}

int
main ( void )
{
	/* it is unlikely that a signal comes before EventLoop,
	 * so setting a new handler at the beginning
	 * should not cause performance issues
	 */
	SetSignalsHandler( ExitSignalsHandler );
	InitX( XCustomErrorHandler );
	SubscribeNewWindowEvent( );
	HandleOpenedWindows( );
	EventLoop( );
	printf( "...Exiting Clicky!...\n" );
	CloseX( );

	return 1;
}
