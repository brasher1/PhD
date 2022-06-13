/*
 * Programmed by:   Daivd John Brasher
 * E-mail:          brasher1@uab.edu
 * Date:            2011
 * 
 * Organization:    Dept. Electrical & Computer Engineering,
 *                  The University of Alabama at Birmingham,
 *                  Birmingham, Alabama,
 *                  USA
 */
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#ifdef _CH_
#pragma package <opencv>
#endif
#define CV_NO_BACKWARD_COMPATIBILITY

#ifndef _EiC
#include "cv.h"
#include "highgui.h"
#endif
#include "database.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

int combinex[25], combiney[25];
int combinecount;
int red[25], green[25], blue[25];

void my_mouse_callback3( int event, int x, int y, int flags, void *param)
{

	IplImage *img = (IplImage*)param;
	char buffer[MAX_PATH];
	wchar_t buff[MAX_PATH];
	uchar *p;

	switch( event)
	{
	case CV_EVENT_LBUTTONUP:
		sprintf(buffer, "(X,Y) = ( %d, %d )", x, y);
		mbstowcs(buff, buffer, sizeof(buff));
		combinex[combinecount] = x;
		combiney[combinecount] = y;

		p = img->imageData + y * img->widthStep;
		red[combinecount] = p[3*x+2];
		green[combinecount] = p[3*x+1];
		blue[combinecount] = p[3*x+0];
		
		combinecount++;
		//MessageBox(NULL,buff,L"POINT", MB_OK);
		break;
	}
}

int fixSegmentation()
{
	IplImage *image;
	char buffer[MAX_PATH];
	char name[MAX_PATH];
	wchar_t namew[MAX_PATH];
	uchar *p;
	char *p2;
	int i, x, y;

	combinecount = 0;
	strcpy(buffer, get_current_filename());
		p2 = strrchr(buffer, '\\');
	sprintf(buffer,"Moments-%s.txt", p2+1);

	image = cvLoadImage(get_current_filename(), 1);
	sprintf(namew,L"COMBINE: %s", get_current_filename());
	sprintf(name, "COMBINE: %s", get_current_filename());
	cvNamedWindow(name,0);
	cvResizeWindow(name, image->width, image->height);
	cvShowImage(name, image);		
	cvSetMouseCallback(name, my_mouse_callback3, (void *)image);
	while( 1 )
	{
		if( cvWaitKey(100) == 27 ) break;
	}
	for( i=1; i<combinecount; i++)
	{
		for( y=0; y<image->height; y++)
		{
			uchar *p = (uchar *)(image->imageData + y * image->widthStep );
			for( x=0; x<image->width; x++)
			{
				if( red[i] == p[3*x+2] &&
					green[i] == p[3*x+1] &&
					blue[i] == p[3*x+0] )
				{
					p[3*x+2] = red[0];
					p[3*x+1] = green[0];
					p[3*x+0] = blue[0];
				}
			}
		}
				
	}
	cvSaveImage(get_current_filename(), image, 0);
	cvReleaseImage(&image);
	cvDestroyWindow(name);
	return(0);
}