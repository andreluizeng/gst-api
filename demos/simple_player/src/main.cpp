/************************-***************************************************
 *   Copyright (C) 2011 by Andre L. V. da Silva   									*
 *   andreluizeng@yahoo.com.br   														*
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

//--------------------------------------------------------------------------------------
// File: main.cpp
//--------------------------------------------------------------------------------------
#include <iostream>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <fcntl.h>
#include <malloc.h>
#include <math.h>
#include <stdlib.h>

#include <termios.h>
#include <unistd.h>
#include <time.h>

#include <pthread.h>

#include "gstcontrol.h"

GSTVideoControl *gst_video_object = new GSTVideoControl;

int thread_id;

// message bus for the current GSTVideoControl object, must have replicated for each created object
gboolean BusCall1 (GstBus *bus, GstMessage *msg, gpointer data)
{
  	switch (GST_MESSAGE_TYPE (msg))
	{
		case GST_MESSAGE_EOS:	g_print ("End of stream\n");
					exit (0);
					break;

    		case GST_MESSAGE_ERROR:{
						gchar  *debug;
      						GError *error;
						gst_message_parse_error (msg, &error, &debug);
      						g_free (debug);
	
      						g_printerr ("Error: %s\n", error->message);
      						g_error_free (error);
	
					}
		      		
    		default:		break;
  	}

  return TRUE;

}

// just to create the pthread inside the GSTVideoControl class, this wrap function must be replicate for every object
void *WrapFunction1 (void *obj)
{
	GSTVideoControl *aux = reinterpret_cast <GSTVideoControl *> (obj);
	aux->GSTLoopFunction();
	
	return NULL;

}


//--------------------------------------------------------------------------------------
// Name: main()
// Desc: main function (entry point)
//--------------------------------------------------------------------------------------
int main (int argc, char **argv)
{
	char filepath[100];
	// set the file path
	if (argc < 2) 
	{
		printf ("\n-----------------------------------\n");
		printf ("\nUsage: %s <file complete path>\n", argv[0]);
		printf ("\n-----------------------------------\n\n");
		return 0;
	}
		
	// initialize the GSTVideoControl object
	gst_init (&argc, &argv);
	gst_video_object->GSTInit();

	snprintf(filepath, 100, "file://%s", argv[1]);
	gst_video_object->GSTSetURI(filepath);
	
	// create the pipe line using the mfw_v4lsin, no callback function and the message bus BusCall1
	gst_video_object->GSTBuildPipeline((char *)"mfw_v4lsink", NULL, BusCall1);
	
	// create the thread for this GSTVideoControl (thread used for Buscall and loop)
	if (! gst_video_object->GSTThreadCreate(WrapFunction1))
		return 0;

	
	// Change the GSTcontrol to playing state
	gst_video_object->GSTPlay();
	
	
	// keep the app runing until the end_of_stream is not reached.
	while (true);
	
	return 0;
}