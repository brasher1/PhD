/*
 * Programmed by:   Daivd John Brasher
 * E-mail:          brasher1@uab.edu
 * Date:            2010
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
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "database.h"

char wndname[] = "Edge";
char tbarname[] = "Threshold";
int edge_thresh = 50;

IplImage *image = 0, *cedge = 0, *gray = 0, *edge = 0;


// define a trackbar callback
void on_trackbar(int h)
{
    cvSmooth( gray, edge, CV_BLUR, 3, 3, 0, 0 );
    cvNot( gray, edge );

    // Run the edge detector on grayscale
    cvCanny(gray, edge, (float)edge_thresh, (float)edge_thresh*3, 3);
	cvSaveImage("edgeaftercanny.bmp", edge, 0);

    //cvZero( cedge );
    // copy edge points
    //cvCopy( image, cedge, edge );
	//cvSaveImage("cedge.bmp",cedge, 0);

   // cvShowImage(wndname, cedge);
}

int OpenCVcanny( char *name)
{
	char* filename = name;
	char cannyname[_MAX_PATH];
	char *chr = NULL;
	int result = 0;
    char tmpbuf[128], timebuf[26], ampm[] = "AM";
    time_t ltime;

    struct tm gmt, xmas = { 0, 0, 12, 25, 11, 93 };
    errno_t err;

    if( (image = cvLoadImage( filename, 1)) == 0 )
        return -1;

	// Create the output image
   // cedge = cvCreateImage(cvSize(image->width,image->height), IPL_DEPTH_8U, 3);

    // Convert to grayscale
    gray = cvCreateImage(cvSize(image->width,image->height), IPL_DEPTH_8U, 1);
    edge = cvCreateImage(cvSize(image->width,image->height), IPL_DEPTH_8U, 1);
    cvCvtColor(image, gray, CV_BGR2GRAY);

	cvSaveImage("gray.bmp", gray, 0);
	cvSaveImage("edge.bmp", edge, 0);

    // Create a window
//    cvNamedWindow(wndname, 1);

    // create a toolbar
//    cvCreateTrackbar(tbarname, wndname, &edge_thresh, 255, on_trackbar);

    // Show the image
    on_trackbar(0);

    chr = strrchr( name, '.' );
    result = (int)(chr - name + 1);
    if (  chr != NULL )
	{
		strncpy(cannyname, name, result-1);
	}
	else
	{
		strncpy(cannyname, name, sizeof(cannyname)-1);
	} 
	//cvSaveImage(
	    time( &ltime );
    // Display UTC. 
    err = _gmtime64_s( &gmt, &ltime );
    if (err)
    {
       printf("_gmtime64_s failed due to an invalid argument.");
    }
    err = asctime_s(timebuf, 26, &gmt);
    if (err)
    {
       printf("asctime_s failed due to an invalid argument.");
    }
    //printf( "Coordinated universal time:\t\t%s", timebuf );

	strftime( tmpbuf, 128, "%m_%d_%Y_%H_%M_%S", &gmt );
	sprintf(&cannyname[result-1], "_%s.PPM", tmpbuf );
	set_current_filename(cannyname);
	cvSaveImage(cannyname, edge, NULL );

    // Wait for a key stroke; the same function arranges events processing
 //   cvWaitKey(0);
    cvReleaseImage(&image);
    cvReleaseImage(&gray);
    cvReleaseImage(&edge);
	//cvReleaseImage(&cedge);
    cvDestroyWindow(wndname);

    return 0;
}

#ifdef _EiC
main(1,"edge.c");
#endif
