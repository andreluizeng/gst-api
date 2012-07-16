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
#include "glutils.h"
#include "glplane.h"

GLUtils *window = new GLUtils;

GLPlane *plane_video1 = new GLPlane;
GLPlane *plane_video2 = new GLPlane;
GLPlane *plane_video3 = new GLPlane;

GSTVideoControl *gst_video1_object = new GSTVideoControl;
GSTVideoControl *gst_video2_object = new GSTVideoControl;
GSTVideoControl *gst_video3_object = new GSTVideoControl;

int thread_id;

float matProj[16] = {0};
float matModel[9] = {0};


// video 1
gchar*	g_pcFrameBuffer1 = NULL;

void *WrapFunction1 (void *obj);
gboolean BusCall1 (GstBus *bus, GstMessage *msg, gpointer data);
void on_handoff1 (GstElement* pFakeSink, GstBuffer* pBuffer, GstPad* pPad);


// video 2
gchar*	g_pcFrameBuffer2 = NULL;

void *WrapFunction2 (void *obj);
gboolean BusCall2 (GstBus *bus, GstMessage *msg, gpointer data);
void on_handoff2 (GstElement* pFakeSink, GstBuffer* pBuffer, GstPad* pPad);


// video 3
gchar*	g_pcFrameBuffer3 = NULL;

void *WrapFunction3 (void *obj);
gboolean BusCall3 (GstBus *bus, GstMessage *msg, gpointer data);
void on_handoff3 (GstElement* pFakeSink, GstBuffer* pBuffer, GstPad* pPad);

#define VIDEO_W 720
#define VIDEO_H	480


void Render (void);
//--------------------------------------------------------------------------------------
// Name: main()
// Desc: main function (entry point)
//--------------------------------------------------------------------------------------
int main (int argc, char **argv)
{
	char filepath[100];
		

	window->GLInit();
	window->GenPerspectiveMatrix (0.5, 1.0, -1.0, 1.0, matProj);
	
	gst_init (&argc, &argv);
	
	gst_video1_object->GSTInit();
	gst_video2_object->GSTInit();
	gst_video3_object->GSTInit();

	plane_video1->PlaneCreate(VIDEO_W, VIDEO_H);
	plane_video1->PlaneCreateTex(VIDEO_W, VIDEO_H);
	plane_video1->PlaneSetProjMatrix(matProj);
	g_pcFrameBuffer1 = (gchar*) malloc (VIDEO_W * VIDEO_H * 2);
	snprintf(filepath, 100, "file:///home/gstcontrol/samples/videos/finalfantasy.avi");
	gst_video1_object->GSTSetURI(filepath);
	// create the pipe line using the mfw_v4lsin, no callback function and the message bus BusCall1
	gst_video1_object->GSTBuildPipeline((char *)"fakesink", (GCallback)on_handoff1, BusCall1);
	// create the thread for this GSTVideoControl (thread used for Buscall and loop)
	if (! gst_video1_object->GSTThreadCreate(WrapFunction1))
		return 0;

	plane_video2->PlaneCreate(VIDEO_W, VIDEO_H);
	plane_video2->PlaneCreateTex(VIDEO_W, VIDEO_H);
	plane_video2->PlaneSetProjMatrix(matProj);
	g_pcFrameBuffer2 = (gchar*) malloc (VIDEO_W * VIDEO_H * 2);
	snprintf(filepath, 100, "file:///home/gstcontrol/samples/videos/uncharted.avi");
	gst_video2_object->GSTSetURI(filepath);
	// create the pipe line using the mfw_v4lsin, no callback function and the message bus BusCall2
	gst_video2_object->GSTBuildPipeline((char *)"fakesink",  (GCallback)on_handoff2, BusCall2);
	// create the thread for this GSTVideoControl (thread used for Buscall and loop)
	if (! gst_video2_object->GSTThreadCreate(WrapFunction2))
		return 0;
	
	plane_video3->PlaneCreate(VIDEO_W, VIDEO_H);
	plane_video3->PlaneCreateTex(VIDEO_W, VIDEO_H);
	plane_video3->PlaneSetProjMatrix(matProj);
	g_pcFrameBuffer3 = (gchar*) malloc (VIDEO_W * VIDEO_H * 2);
	snprintf(filepath, 100, "file:///home/gstcontrol/samples/videos/Up.mp4");
	gst_video3_object->GSTSetURI(filepath);
	// create the pipe line using the mfw_v4lsin, no callback function and the message bus BusCall3
	gst_video3_object->GSTBuildPipeline((char *)"fakesink",  (GCallback)on_handoff3, BusCall3);
	// create the thread for this GSTVideoControl (thread used for Buscall and loop)
	if (! gst_video3_object->GSTThreadCreate(WrapFunction3))
		return 0;


	// Change the GSTcontrol to playing state
	gst_video1_object->GSTPlay();
	gst_video2_object->GSTPlay();
	gst_video3_object->GSTPlay();
	
	
	// keep the app runing until the end_of_stream is not reached.
	while (!window->Kbhit ())
	{
		Render ();	
	}
	
	gst_video1_object->GSTStop();
	gst_video2_object->GSTStop();
	gst_video3_object->GSTStop();
	
	free (g_pcFrameBuffer1);
	free (g_pcFrameBuffer2);
	free (g_pcFrameBuffer3);
	
	window->GLEnd();
	
	plane_video1->PlaneDestroy();
	plane_video2->PlaneDestroy();
	plane_video3->PlaneDestroy();
	
	gst_video1_object->GSTDeInit();
	gst_video2_object->GSTDeInit();
	gst_video3_object->GSTDeInit();

	return 0;
}



// VIDEO 1
// message bus for the current GSTVideoControl object, must have replicated for each created object
gboolean BusCall1 (GstBus *bus, GstMessage *msg, gpointer data)
{
  	switch (GST_MESSAGE_TYPE (msg))
	{
		case GST_MESSAGE_EOS:	g_print ("End of stream\n");
					gst_video1_object->GSTSeekAbsolute(0);
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

//handoff function, called every frame
void on_handoff1 (GstElement* pFakeSink, GstBuffer* pBuffer, GstPad* pPad)
{
	int video_w;
	int video_h;
	
	video_w = gst_video1_object->GSTGetPadWidth (pPad);
	video_h = gst_video1_object->GSTGetPadHeight (pPad);

	gst_buffer_ref (pBuffer);
	memmove (g_pcFrameBuffer1, GST_BUFFER_DATA (pBuffer), video_w * video_h * 2);
	gst_buffer_unref (pBuffer);
}

// VIDEO 2
// message bus for the current GSTVideoControl object, must have replicated for each created object
gboolean BusCall2 (GstBus *bus, GstMessage *msg, gpointer data)
{
  	switch (GST_MESSAGE_TYPE (msg))
	{
		case GST_MESSAGE_EOS:	g_print ("End of stream\n");
					gst_video2_object->GSTSeekAbsolute(0);
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
void *WrapFunction2 (void *obj)
{
	GSTVideoControl *aux = reinterpret_cast <GSTVideoControl *> (obj);
	aux->GSTLoopFunction();
	
	return NULL;

}

//handoff function, called every frame
void on_handoff2 (GstElement* pFakeSink, GstBuffer* pBuffer, GstPad* pPad)
{
	int video_w;
	int video_h;
	
	video_w = gst_video2_object->GSTGetPadWidth (pPad);
	video_h = gst_video2_object->GSTGetPadHeight (pPad);

	gst_buffer_ref (pBuffer);
	memmove (g_pcFrameBuffer2, GST_BUFFER_DATA (pBuffer), video_w * video_h * 2);
	gst_buffer_unref (pBuffer);
}


// VIDEO 1
// message bus for the current GSTVideoControl object, must have replicated for each created object
gboolean BusCall3 (GstBus *bus, GstMessage *msg, gpointer data)
{
  	switch (GST_MESSAGE_TYPE (msg))
	{
		case GST_MESSAGE_EOS:	g_print ("End of stream\n");
					gst_video3_object->GSTSeekAbsolute(0);
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
void *WrapFunction3 (void *obj)
{
	GSTVideoControl *aux = reinterpret_cast <GSTVideoControl *> (obj);
	aux->GSTLoopFunction();
	
	return NULL;

}

//handoff function, called every frame
void on_handoff3 (GstElement* pFakeSink, GstBuffer* pBuffer, GstPad* pPad)
{
	int video_w;
	int video_h;
	
	video_w = gst_video3_object->GSTGetPadWidth (pPad);
	video_h = gst_video3_object->GSTGetPadHeight (pPad);

	gst_buffer_ref (pBuffer);
	memmove (g_pcFrameBuffer3, GST_BUFFER_DATA (pBuffer), video_w * video_h * 2);
	gst_buffer_unref (pBuffer);
}


void Render (void)
{
	
	static float r = 0;
	static float g = 0;
	static float b = 0;
	static int cnt = 0;
	
	
	static float rotationx1 = 0;
	static float rotationy1 = 0;
	static float rotationz1 = 0;
	
	static float rotationx2 = 0;
	static float rotationy2 = 0;
	static float rotationz2 = 0;
	
	static float rotationx3 = 0;
	static float rotationy3 = 0;
	static float rotationz3 = 0;
	
	// Clear the colorbuffer and depth-buffer -- just playing with some colors
	if (cnt == 0)
	{
		r = 0;
		g = 0;
		
		if (b >= 1.0)
		{
			cnt = 1;
		}
		else
		{
			b = b + 0.01;
			
		}
	}
	else if (cnt == 1)
	{
		r = 0;
		g = 0;
		
		if (b <= 0)
		{
			cnt = 2;
		}
		else
		{
			b = b - 0.01;
		}
	}

	else if (cnt == 2)
	{
		r = 0;
		b = 0;
		
		if (g >= 1.0)
		{
			cnt = 3;
		}
		else
		{
			g = g + 0.01;
		}
	}


	else if (cnt == 3)
	{
		r = 0;
		b = 0;
		
		if (g <= 0)
		{
			cnt = 4;
		}
		else
		{
			g = g - 0.01;
		}
	}
	else if (cnt == 4)
	{
		g = 0;
		b = 0;
		
		if (r >= 1.0)
		{
			cnt = 5;
		}
		else
		{
			r = r + 0.01;
		}
	}


	else if (cnt == 5)
	{
		g = 0;
		b = 0;
		
		if (r <= 0)
		{
			cnt = 6;
		}
		else
		{
			r = r - 0.01;
		}
	}

	
	if (cnt == 6)
	{
		r = 0;
		g = 0;
		
		if (b >= 1.0)
		{
			cnt = 7;
		}
		else
		{
			b = b + 0.01;
			
		}
	}

	if (cnt == 7)
	{
		r = 0;
		
		if (g >= 1.0)
		{
			cnt = 8;
		}
		else
		{
			g = g + 0.01;
			
		}
	}
	
	if (cnt == 8)
	{
		if (r >= 1.0)
		{
			cnt = 9;
		}
		else
		{
			r = r + 0.01;
			
		}
	}
	
	if (cnt == 9)
	{
		if (b <= 0)
		{
			cnt = 10;
		}
		else
		{
			b = b - 0.01;
			
		}
	}

	if (cnt == 10)
	{
		
		if (g <= 0.0)
		{
			cnt = 11;
		}
		else
		{
			g = g - 0.01;
			
		}
	}
	
	if (cnt == 11)
	{
		if (r <= 0.0)
		{
			cnt = 0;
		}
		else
		{
			r = r - 0.01;
			
		}
	}

		
	glClearColor (r, g, b, 1.0f);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (rotationx1 == 360) rotationx1 = 0;
	else rotationx1 = rotationx1 + 0.3;
	if (rotationy1 == 360) rotationy1 = 0;
	else rotationy1 = rotationy1 + 0.7;
	if (rotationz1 == 360) rotationz1 = 0;
	else rotationz1 = rotationz1 + 0.5;

	if (rotationx2 <= 0) rotationx2 = 360;
	else rotationx2 = rotationx2 - 0.5;
	if (rotationy2 == 360) rotationy2 = 0;
	else rotationy2 = rotationy2 + 0.5;
	if (rotationz2 <= 0) rotationz2 = 360;
	else rotationz2 = rotationz2 - 0.3;

	if (rotationx3 == 360) rotationx3 = 0;
	else rotationx3 = rotationx3 + 0.7;
	if (rotationy3 <= 0) rotationy3 = 360;
	else rotationy3 = rotationy3 - 0.2;
	if (rotationz3 <= 0) rotationz3 = 360;
	else rotationz3 = rotationz3 - 0.4;

	plane_video1->PlaneMove(PLANE_X_AXIS, -350);
	plane_video1->PlaneMove(PLANE_Y_AXIS, -350);
	plane_video1->PlaneMove(PLANE_Z_AXIS, -3000);
	plane_video1->PlaneRotate(PLANE_X_AXIS, rotationx1);
	plane_video1->PlaneRotate(PLANE_Y_AXIS, rotationy1);
	plane_video1->PlaneRotate(PLANE_Z_AXIS, rotationz1);
	plane_video1->PlaneSetTexBuf(g_pcFrameBuffer1, VIDEO_W, VIDEO_H);
	plane_video1->PlaneDraw();
	
	plane_video2->PlaneMove(PLANE_Y_AXIS, -350);
	plane_video2->PlaneMove(PLANE_X_AXIS, 350);
	plane_video2->PlaneMove(PLANE_Z_AXIS, -3000);
	plane_video2->PlaneRotate(PLANE_X_AXIS, rotationx2);
	plane_video2->PlaneRotate(PLANE_Y_AXIS, rotationy2);
	plane_video2->PlaneRotate(PLANE_Z_AXIS, rotationz2);
	plane_video2->PlaneSetTexBuf(g_pcFrameBuffer2, VIDEO_W, VIDEO_H);
	plane_video2->PlaneDraw();
		
	
	plane_video3->PlaneMove(PLANE_Y_AXIS, 350);
	plane_video3->PlaneMove(PLANE_Z_AXIS, -3000);
	plane_video3->PlaneRotate(PLANE_X_AXIS, rotationx3);
	plane_video3->PlaneRotate(PLANE_Y_AXIS, rotationy3);
	plane_video3->PlaneRotate(PLANE_Z_AXIS, rotationz3);
	plane_video3->PlaneSetTexBuf(g_pcFrameBuffer3, VIDEO_W, VIDEO_H);
	plane_video3->PlaneDraw();
	
	// Swap Buffers.
	// Brings to the native display the current render surface.
	eglSwapBuffers (window->egldisplay, window->eglsurface);
	assert (eglGetError () == EGL_SUCCESS);
	return;
}