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

#include "geometry.h"
//cvMinAreaRect2(

int draw_monument_point(char *filename, int x, int y, int shading_factor, SIMPLE_POINT *pts, int count)
{
	CvPoint pt1, pt2;
	CvFont font;
	char coordinates[256];

	int i;

	IplImage *image = 0, *cedge = 0, *gray = 0, *edge = 0;

	if( (image = cvLoadImage( filename, 1)) == 0 )
        return -1;

	pt1.x = x-5; pt1.y = y;
	pt2.x = x+5; pt2.y = y;
	cvLine(image, pt1, pt2, CV_RGB(0,255,0), 1,8,0);
	pt1.x = x; pt1.y = y+5;
	pt2.x = x; pt2.y = y-5;
	cvLine(image, pt1, pt2, CV_RGB(0,255,0), 1,8,0);
	//write the coordinates of the center point on the image
	cvInitFont(&font, CV_FONT_HERSHEY_DUPLEX, 0.5, 0.25,0,1,8);
	pt1.x = x; pt1.y = y;
	sprintf(coordinates, "%d, %d", x,y);
	cvPutText(image,coordinates,pt1,&font, CV_RGB(0,64,0));

	//Draw the edge of the monument point
	for(i=0; i<count; i++)
	{
		pt1.x = pts[i].x; pt1.y = pts[i].y;
		if( i == count - 1 )
		{
			pt2.x = pts[0].x; pt2.y = pts[0].y;
		}
		else
		{
			pt2.x = pts[i+1].x; pt2.y = pts[i+1].y;
		}
		cvLine(image, pt1, pt2, CV_RGB(255,0,0),1,8,0);

	}
	pt1.x = x-3; pt1.y = y+3;

	if( shading_factor == 4.787 )
		cvFloodFill(image, pt1, CV_RGB(255,255,0), cvScalarAll(2), cvScalarAll(2), 0, 4, 0);

	cvSaveImage(filename, image,0);

	cvReleaseImage(&image);
	return 1;
}

struct pt_colors
{
	int x,y;
	uchar red, green, blue;
	uchar newRed, newGreen, newBlue;	
	int count;
};
uchar colors[][3] = 
{
	{0, 0, 0}, // is black
	{255, 255, 255}, // is white
	{255, 0, 0}, // is red
	{0, 255, 0}, // is green
	{0, 0, 255}, // is blue
	{255, 255, 0}, // is yellow
	{0, 255, 255}, // is cyan
	{255, 0, 255}, // is magenta
	{0, 0, 127},
	{0, 127, 0},
	{0, 127, 127},
	{127, 0, 0},
	{127, 0, 127},
	{127, 127, 0},
	{127, 127, 127},

	{255, 255, 127},
	{255, 127, 255},
	{255, 127, 127},
	{127, 255, 255},
	{127, 255, 127},
	{127, 127, 255},
	{127, 127, 127},

	{255, 255, 63},
	{255, 63, 255},
	{255, 63, 63},
	{63, 255, 255},
	{63, 255, 63},
	{63, 63, 255},
	{63, 63, 63},

	{127, 127, 63},
	{127, 63, 127},
	{127, 63, 63},
	{63, 127, 127},
	{63, 127, 63},
	{63, 63, 127},
	{63, 63, 127}
};

int compare_colors(void *p1, void *p2)
{
	if( ((struct pt_colors *)p1)->count < ((struct pt_colors *)p2)->count )
		return(-1);
	else if( ((struct pt_colors *)p1)->count == ((struct pt_colors *)p2)->count )
		return(0);
	else
		return(1);
}
			   

int inc_count_colors(struct pt_colors ptc[], uchar red, uchar green, uchar blue, int size)
{
	int i;
	for(i=0; i<size; i++)
	{
		if( ptc[i].blue == blue && ptc[i].green == green && ptc[i].red == red )
		{
			ptc[i].count++;
			i=size;
			break;
		}
		if( ptc[i].blue == 0 && ptc[i].green == 0 && ptc[i].red == 0 && i<size )
		{
			ptc[i].blue = blue;
			ptc[i].green = green;
			ptc[i].red = red;
			ptc[i].count++;
			ptc[i].newRed = colors[i][0];
			ptc[i].newGreen = colors[i][1];
			ptc[i].newBlue = colors[i][2];
			i=size;
		}
	}
}

int fill_plate_white(char *filename)
{
	struct pt_colors ptc[256];
	CvPoint pt1, pt2;
	char coordinates[256];
	int x,y;
	int count;
	int i,j,k,maxbuffer;
	FILE *output;
	CvLineIterator iterator;
	int unique_colors;

	IplImage *image = 0, *cedge = 0, *gray = 0, *edge = 0;

	memset(ptc, 0, sizeof(ptc));

	if( (image = cvLoadImage( filename, 1)) == 0 )
        return -1;
	y = image->height / 2;
	x = image->width / 2;
	count = 8;

	//Get the color of the plate
	pt1.x = x; pt1.y = y;	
	output = fopen("platecolors.txt","w+");
	count = 0;
//	for(i=1; i<=1; i++)
//	{
///////////////////////////////////////////////////////
		pt2.x = x + (image->width/3);
		pt2.y = y;
//		cvDrawLine(image, pt1, pt2, cvScalar(0,0,0,0), 2, 8, 0);
		maxbuffer = cvInitLineIterator(image, pt1, pt2, &iterator, 8, 0);
		for(j=0; j<maxbuffer; j++)
		{
			fprintf(output, "X+,Y:%x-%x-%x\n", iterator.ptr[2],iterator.ptr[1],iterator.ptr[0]);
			inc_count_colors(ptc, iterator.ptr[2], iterator.ptr[1], iterator.ptr[0], sizeof(ptc));
			CV_NEXT_LINE_POINT(iterator);
		}
		pt2.x = x - (image->width/3);
		pt2.y = y;
//		cvDrawLine(image, pt1, pt2, cvScalar(0,0,0,0), 2, 8, 0);
		maxbuffer = cvInitLineIterator(image, pt1, pt2, &iterator, 8, 0);
		for(j=0; j<maxbuffer; j++)
		{
			fprintf(output, "X-,Y:%x-%x-%x\n", iterator.ptr[2],iterator.ptr[1],iterator.ptr[0]);
			inc_count_colors(ptc, iterator.ptr[2], iterator.ptr[1], iterator.ptr[0], sizeof(ptc));
			CV_NEXT_LINE_POINT(iterator);
		}
		pt2.x = x;
		pt2.y = y + (image->height/3);
//		cvDrawLine(image, pt1, pt2, cvScalar(0,0,0,0), 2, 8, 0);
		maxbuffer = cvInitLineIterator(image, pt1, pt2, &iterator, 8, 0);
		for(j=0; j<maxbuffer; j++)
		{
			fprintf(output, "X,Y+:%x-%x-%x\n", iterator.ptr[2],iterator.ptr[1],iterator.ptr[0]);
			inc_count_colors(ptc, iterator.ptr[2], iterator.ptr[1], iterator.ptr[0], sizeof(ptc));
			CV_NEXT_LINE_POINT(iterator);
		}
		pt2.x = x;
		pt2.y = y - (image->height/3);
//		cvDrawLine(image, pt1, pt2, cvScalar(0,0,0,0), 2, 8, 0);
		maxbuffer = cvInitLineIterator(image, pt1, pt2, &iterator, 8, 0);
		for(j=0; j<maxbuffer; j++)
		{
			fprintf(output, "X,Y-:%x-%x-%x\n", iterator.ptr[2],iterator.ptr[1],iterator.ptr[0]);
			inc_count_colors(ptc, iterator.ptr[2], iterator.ptr[1], iterator.ptr[0], sizeof(ptc));
			CV_NEXT_LINE_POINT(iterator);
		}
/////////////////////////////////////////////////////////
		pt2.x = x + (image->width/3);
		pt2.y = y + (image->height/3);
//		cvDrawLine(image, pt1, pt2, cvScalar(0,0,0,0), 2, 8, 0);
		maxbuffer = cvInitLineIterator(image, pt1, pt2, &iterator, 8, 0);
		for(j=0; j<maxbuffer; j++)
		{
			fprintf(output, "X+,Y:%x-%x-%x\n", iterator.ptr[2],iterator.ptr[1],iterator.ptr[0]);
			inc_count_colors(ptc, iterator.ptr[2], iterator.ptr[1], iterator.ptr[0], sizeof(ptc));
			CV_NEXT_LINE_POINT(iterator);
		}
		pt2.x = x - (image->width/3);
		pt2.y = y + (image->height/3);
//		cvDrawLine(image, pt1, pt2, cvScalar(0,0,0,0), 2, 8, 0);
		maxbuffer = cvInitLineIterator(image, pt1, pt2, &iterator, 8, 0);
		for(j=0; j<maxbuffer; j++)
		{
			fprintf(output, "X-,Y:%x-%x-%x\n", iterator.ptr[2],iterator.ptr[1],iterator.ptr[0]);
			inc_count_colors(ptc, iterator.ptr[2], iterator.ptr[1], iterator.ptr[0], sizeof(ptc));
			CV_NEXT_LINE_POINT(iterator);
		}
		pt2.x = x + (image->width/3);
		pt2.y = y - (image->height/3);
//		cvDrawLine(image, pt1, pt2, cvScalar(0,0,0,0), 2, 8, 0);
		maxbuffer = cvInitLineIterator(image, pt1, pt2, &iterator, 8, 0);
		for(j=0; j<maxbuffer; j++)
		{
			fprintf(output, "X,Y+:%x-%x-%x\n", iterator.ptr[2],iterator.ptr[1],iterator.ptr[0]);
			inc_count_colors(ptc, iterator.ptr[2], iterator.ptr[1], iterator.ptr[0], sizeof(ptc));
			CV_NEXT_LINE_POINT(iterator);
		}
		pt2.x = x - (image->width/3);
		pt2.y = y - (image->height/3);
//		cvDrawLine(image, pt1, pt2,cvScalar(0,0,0,0), 2, 8, 0);
		maxbuffer = cvInitLineIterator(image, pt1, pt2, &iterator, 8, 0);
		for(j=0; j<maxbuffer; j++)
		{
			fprintf(output, "X,Y-:%x-%x-%x\n", iterator.ptr[2],iterator.ptr[1],iterator.ptr[0]);
			inc_count_colors(ptc, iterator.ptr[2], iterator.ptr[1], iterator.ptr[0], sizeof(ptc));
			CV_NEXT_LINE_POINT(iterator);
		}
//	}
	fclose(output);


	output = fopen("platecolorcount.txt","w+");
	unique_colors = 0;
	for( i=0; i<256; i++)
	{
		if( ptc[i].count == 0 )
			i = 256;
		else
		{
			fprintf(output,"Count:%d, R:%02x, G:%02x, B:%02x, NR:%02x, NG:%02x, NB:%02x\n", ptc[i].count, ptc[i].red, ptc[i].green, ptc[i].blue, ptc[i].newRed, ptc[i].newGreen, ptc[i].newBlue);
			unique_colors++;
		}
	}
	qsort(ptc, unique_colors, sizeof(ptc[0]), compare_colors);

	fclose(output);
//	output = fopen("FloodFill.txt","w+");

	for( i=0; i<image->height; i++)
	{
		uchar *p = (uchar *)(image->imageData + i * image->widthStep );
		for( j=0; j<image->width; j++)
		{
//			fprintf(output, "X:%d, Y:%d, R:%02x, G:%02x, B:%02x\n", j,i,p[3*j+2], p[3*j+1], p[3*j+0]);
			for( k=0; k<unique_colors; k++)
			{
				if( ptc[k].red == p[3*j+2] && ptc[k].green == p[3*j+1] && ptc[k].blue == p[3*j+0] )
				{
					CvPoint pt1;
//					fprintf(output,"FLOODFILL: X:%d, Y:%d, R:%02x, G:%02x, B:%02x, NR:%02x, NG:%02x, NB:%02x\n",j,i,ptc[k].red, ptc[k].green, ptc[k].blue, ptc[k].newRed, ptc[k].newGreen, ptc[k].newBlue);
					pt1.x = j;
					pt1.y = i;
					if( i != 0 && j != 0 )
						cvFloodFill(image, pt1, CV_RGB(ptc[k].newRed, ptc[k].newGreen, ptc[k].newBlue), cvScalarAll(0), cvScalarAll(0), 0, 4, 0);
				}
			}
		}
	}
//	fclose(output);
//	if( shading_factor == 4.787 )
//		cvFloodFill(image, pt1, CV_RGB(255,255,0), cvScalarAll(2), cvScalarAll(2), 0, 4, 0);

	cvSaveImage(filename, image,0);

	cvReleaseImage(&image);
	return 1;
}