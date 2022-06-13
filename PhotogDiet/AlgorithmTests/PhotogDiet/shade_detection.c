/* Programmed by:     David John Brasher
 * E-mail:			brasher1@uab.edu
 * Date:			2010
 * Last Modified:	
 * Organization:	University of Alabama Birmingham
 *					Dept. Electrical and Computer Engineering
 *					
 */
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>     /* strlen() */
#include <sys/types.h>
#include <magick/api.h>
#include "upwrite.h"
#include "geometry.h"

typedef struct SHADING {
	int	black_count;
	int white_count;
	int num_elements;
} Shading;

int shade_detection(char *filename, SIMPLE_POINT *, SIMPLE_POINT *);
double sample_ellipse(Image *image, SIMPLE_POINT *center, SIMPLE_POINT *edge, Shading *shading);




/* detect the shading factor of each
 * detected ellipse
 */
int shade_detection(char *filename, SIMPLE_POINT *center, SIMPLE_POINT *points)
{
//	GAUS *gaus;
//	POINT *point;
	
//	SIMPLE_POINT edge;
	int i;

	Shading shading;
	Image *image;
//	Image *image2;
	ExceptionInfo exception;
	ImageInfo *image_info;
//	ImageInfo *image_info2;

	double shadingFactor;

	InitializeMagick(filename);
	GetExceptionInfo(&exception);
	image_info=CloneImageInfo((ImageInfo *) NULL);
	strcpy(image_info->filename, filename);

	image=ReadImage(image_info,&exception);

	shading.black_count = 0;
	shading.white_count = 0;
	shading.num_elements = 0;
	for (i=0; i<8; i++)
	{
		sample_ellipse(image, center, &points[i], &shading);
		//fprintf(output6,"EDGE POINT: x=%f, y=%f\n", point->x[1], point->x[2]);
	}
	shadingFactor = ((double)shading.black_count / (double)(shading.black_count+shading.white_count))*100.0;
	if( shadingFactor > 50.0 )
		shadingFactor = 100.0;
	else
		shadingFactor = 0.0; 
	
	//DestroyExceptionInfo(&exception);
	DestroyImageInfo(image_info);
	DestroyImage(image);
	DestroyMagick();

	return((int)shadingFactor);
}

double sample_ellipse(Image *image, SIMPLE_POINT *center, SIMPLE_POINT *edge, Shading *shading)
{
	double slope;	// slope of line between pt1 and pt2
	double b;		//y intercept
    PixelPacket *pixels;
	ExceptionInfo exception;
#ifdef VERBOSE_INSTRUMENTATION
	FILE *output;
#endif //VERBOSE_INSTRUMENTATION
	int new_x, new_y;

#ifdef VERBOSE_INSTRUMENTATION	
	output = fopen("output shading data.txt", "a+");
#endif //VERBOSE_INSTRUMENTATION
	slope = (double)(edge->y - center->y)/(double)(edge->x - center->x );

	b = edge->y - (slope*edge->x);
	new_x = edge->x;  new_y = edge->y;

	do
	{
		if( center->y > new_y )
		{
			new_y = new_y + 2;
			new_x = (int)((b - new_y)/(-slope));
		}
		else if( new_y > center->y )
		{
			new_y = new_y - 2;
			new_x = (int)((b - new_y)/(-slope));
		}
		else
		{
			if( new_x > center->x )
			{
				new_x = new_x - 2;
				new_y = (int)(slope*new_x + b);
			}
			else if(center->x > edge->x)
			{
				new_x = new_x + 2;
				new_y = (int)(slope*new_x + b);
			}
			else
			{
				return;
#ifdef VERBOSE_INSTRUMENTATION
				fclose(output);
#endif //VERBOSE_INSTRUMENTATION
			}
		}
		
		pixels = GetVirtualPixels(image,new_x, new_y, 1, 1, &exception);
#ifdef VERBOSE_INSTRUMENTATION
		fprintf(output, "CenterX,CenterY(%d,%d): NewX,NewY(%d,%d):  Pixel(%d)\n",
			center->x, center->y, new_x, new_y, pixels->blue);
#endif //VERBOSE_INSTRUMENTATION
		if( pixels->blue > MaxRGB/4 )
			shading->white_count++;
		else
			shading->black_count++;
		shading->num_elements++;
		
	} while(abs(new_x-center->x)>=2 && abs(new_y-center->y)>=2);
#ifdef VERBOSE_INSTRUMENTATION	
	fclose(output);
#endif //VERBOSE_INSTRUMENTATION
	return(0.0);
}