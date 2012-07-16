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
pthread_t menu_thread;


int Kbhit (void)
{
  struct termios oldt, newt;
  int ch;
  int oldf;

  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

  ch = getchar();
    
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);

  if(ch != EOF)
  {
    //ungetc(ch, stdin);
    //return 1;
    return ch;
  }

  return 0;
}

// message bus for the current GSTVideoControl object, must have replicated for each created object
gboolean BusCall1 (GstBus *bus, GstMessage *msg, gpointer data)
{
  	switch (GST_MESSAGE_TYPE (msg))
	{
		case GST_MESSAGE_EOS:	g_print ("End of stream\n");
					gst_video_object->GSTSeekAbsolute(0);
					gst_video_object->GSTPause();
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


void *menu_function (void *);
void show_menu (void);


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
	
	// enable the menus
	pthread_create (&menu_thread, 0, menu_function, (void*)&thread_id);
	
	// keep the app runing until the end_of_stream is not reached.

	show_menu ();
	while (true)
	{
	}
	
	gst_video_object->GSTDeInit();
	pthread_cancel(thread_id);
	
	return 0;
}

void *menu_function (void *)
{
	int option;
	static bool flag_play_pause = 0;
	static bool flag_stop = 0;
	static float ff_value = 1;
	static int ff_key_pressed = 0;
	static float sf_value = 1;
	static int sf_key_pressed = 0;
	static int sr_key_pressed = 0;
	static float sr_value = 1;
	
	while (1)
	{
		option = Kbhit ();
		
		switch (option)
		{
			case 109: 	show_menu();
					break;
					
			case 10:	show_menu();
					break;
					
			case 112:	if (flag_play_pause)
					{
						gst_video_object->GSTPause();
						flag_play_pause = 0;
						printf ("\nVideo Paused\n");
					}
					else
					{
						if (flag_stop)
						{
							gst_video_object->GSTBuildPipeline((char *)"mfw_v4lsink", NULL, BusCall1);
							flag_stop = 0;
						}
						gst_video_object->GSTPlay();
						flag_play_pause = 1;
						
						printf ("\nVideo Playing\n");
					}
					break;
		
			case 115:	gst_video_object->GSTStop();
					printf ("\nVideo Stopped\n");
					flag_stop = 1;
					flag_play_pause = 0;
					break;
					
			case 102: 	if (ff_key_pressed == 5)
					{
						ff_key_pressed = 0;
						ff_value = 1.0;
					}
					else if (ff_key_pressed == 0)
					{
						ff_key_pressed++;
						ff_value = 1.5;
					}
					else if (ff_key_pressed == 1)
					{
						ff_key_pressed++;
						ff_value = 2.0;
					}
					else if (ff_key_pressed == 2)
					{
						ff_key_pressed++;
						ff_value = 4.0;
					}
					else if (ff_key_pressed == 3)
					{
						ff_key_pressed++;
						ff_value = 10.0;
					}
					else if (ff_key_pressed == 4)
					{
						ff_key_pressed++;
						ff_value = 20.0;
					}
					
					gst_video_object->GSTFastForward(ff_value);
					printf ("\nFast forwarding at: %.1f\n", ff_value);
					break;
					
					
			case 110: 	if (sf_key_pressed == 4)
					{
						sf_key_pressed = 0;
						sf_value = 1.0;
					}
					else if (sf_key_pressed == 0)
					{
						sf_key_pressed++;
						sf_value = 0.7;
					}
					else if (sf_key_pressed == 1)
					{
						sf_key_pressed++;
						sf_value = 0.5;
					}
					else if (sf_key_pressed == 2)
					{
						sf_key_pressed++;
						sf_value = 0.3;
					}
					else if (sf_key_pressed == 3)
					{
						sf_key_pressed++;
						sf_value = 0.1;
					}
					
					gst_video_object->GSTFastForward(sf_value);
					printf ("\nSlow Motion forward at: %.1f\n", sf_value);
					break;
					
				

			case 106:	guint64 value;
					printf ("\nDuration: %" GST_TIME_FORMAT ", jump to (in ms): ", GST_TIME_ARGS (gst_video_object->GSTQueryDuration()));
					scanf ("%lu", &value); 
					if ((value * 1000 * 1000) >  gst_video_object->GSTQueryDuration())
						printf ("\nValue out of range...\n");
					
					else
					{
						printf ("\nPlaying from: %"GST_TIME_FORMAT "\n", GST_TIME_ARGS (value*1000*1000));
						gst_video_object->GSTSeekAbsolute(value);
					}
					break;

    
			case 103:	printf ("\nVideo Position is: %" GST_TIME_FORMAT "\n", GST_TIME_ARGS (gst_video_object->GSTQueryPosition()));
					break;

			case 108:	printf ("\nVideo duration is: %" GST_TIME_FORMAT "\n", GST_TIME_ARGS (gst_video_object->GSTQueryDuration()));
					break;

			case 27:	exit (0);
					break;
					
			default :	break;
			
		};
	}

}

void show_menu (void)
{
	printf ("\n-------------------- GST Video Control Menu --------------------\n");
	printf ("\nuse the following keys to perform any control:\n");
	printf ("\nP		toogle Play/Pause"); 			// 112
	printf ("\nS		stop"); 				// 115
	printf ("\nF		fast forward (1,1.5,2,4,10,20x)"); 	// 102
	printf ("\nN		slow forward (1,0.7, 0.5, 0.3, 0.1x)"); // 110
	printf ("\nJ		seek to a position");			// 106
	printf ("\nG		get video position"); 			// 103
	printf ("\nL		get video length"); 			// 108
	printf ("\nM		show this menu"); 			// 109
	printf ("\nESC		exit\n");				// 27
	printf ("\n----------------------------------------------------------------\n");
	
	return;
}