/*
 * SignalHelper.c
 *
 *  Created on: Apr 19, 2017
 *      Author: haskap
 */


#include "SignalHelper.h"

#include <signal.h>
#include <stddef.h>	// for NULL definition



void
SetSignalsHandler ( __sighandler_t handler )
{
	struct sigaction sigAction;
	sigAction.sa_handler = handler;
	sigemptyset( &sigAction.sa_mask );
	sigAction.sa_flags = 0;
	sigaction( SIGTERM, &sigAction, NULL );
	sigaction( SIGINT, &sigAction, NULL );
	sigaction( SIGQUIT, &sigAction, NULL );
}

