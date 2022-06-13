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
#include "database.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

int add_timestamp(char *buffer)
{
	char* filename = buffer;
	char cannyname[_MAX_PATH];
	char *chr = NULL;
	int result = 0;
    char tmpbuf[128], timebuf[26], ampm[] = "AM";
	char extension[_MAX_PATH];
    time_t ltime;

    struct tm gmt, xmas = { 0, 0, 12, 25, 11, 93 };
    errno_t err;


    chr = strrchr( buffer, '.' );
    result = (int)(chr - buffer + 1);
    if (  chr != NULL )
	{
		strcpy( extension, chr+1);
		strncpy(cannyname, buffer, result-1);
	}
	else
	{
		strcpy(extension, "JPG");
		strncpy(cannyname, buffer, sizeof(buffer)-1);
	} 	
	
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
	sprintf(&cannyname[result-1], "_%s.%s", tmpbuf, extension );
	strcpy(buffer, cannyname);
	return 0;
}

int save_cvImageAsPBM(char *filename)
{
	char buffer[_MAX_PATH];
	char *p;
	IplImage *image;

	if( filename == NULL)
	{
		image = cvLoadImage(get_original_filename(),1);
		strcpy(buffer, get_original_filename());
	}
	else
	{
		image = cvLoadImage(filename,1);
		strcpy(buffer, get_original_filename());
	}
	p = strrchr(buffer, '.');
	if( p != NULL )
		*p = 0;
	strcat(buffer, ".PBM\0");
	add_timestamp(buffer);
	cvSaveImage(buffer, image, 0);
	set_current_filename(buffer);
	cvRelease(&image);
	return 1;
}

int save_cvImageAsJPG()
{
	char buffer[_MAX_PATH];
	char *p;

	IplImage *image = cvLoadImage(get_current_filename(),1);
	strcpy(buffer, get_current_filename());
	p = strrchr(buffer, '.');
	if( p != NULL )
		*p = 0;
	strcat(buffer, ".JPG\0");
	add_timestamp(buffer);
	cvSaveImage(buffer, image, 0);
	set_current_filename(buffer);
	cvRelease(&image);
	return 1;

}

int save_cvSegmentationImageAsJPG(float sigma, float k, int min_size)
{
	char buffer[_MAX_PATH];
	char *p;

	IplImage *image = cvLoadImage(get_current_filename(),1);
	strcpy(buffer, get_current_filename());
	p = strrchr(buffer, '.');
	if( p != NULL )
		*p = 0;
	sprintf(buffer, "%s_sigma-%f_constant-%f_minsize-%d.JPG", buffer, sigma, k, min_size);
	//strcat(buffer, ".JPG\0");
	add_timestamp(buffer);
	cvSaveImage(buffer, image, 0);
	set_current_filename(buffer);
	cvRelease(&image);
	return 1;

}

int save_cvSegmentationImageAsPPM(float sigma, float k, int min_size)
{
	char buffer[_MAX_PATH];
	char *p;

	IplImage *image = cvLoadImage(get_current_filename(),1);
	strcpy(buffer, get_current_filename());
	p = strrchr(buffer, '.');
	if( p != NULL )
		*p = 0;
	sprintf(buffer, "%s_sigma-%f_constant-%f_minsize-%d.PPM", buffer, sigma, k, min_size);
	//strcat(buffer, ".PPM\0");
	add_timestamp(buffer);
	cvSaveImage(buffer, image, 0);
	set_current_filename(buffer);
	cvRelease(&image);
	return 1;

}