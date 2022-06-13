/*
 * Programmed by:   Daivd John Brasher
 * E-mail:          brasher1@uab.edu
 * Date:            2010
 * 
 * Organization:    Dept. Electrical & Computer Engineering,
 *                  The University of Alabama at Birmingham,
 *                  Birmingham, Alabama,
 *                  USA
 *
 * Modified by:     David John Brasher
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

int Negate_Image(char *filename)
{
	Image *image;
	Image *image2;
	ExceptionInfo exception;
	ImageInfo *image_info;
	ImageInfo *image_info2;
	PixelPacket *pixels;
	int row, col, W, H;

	InitializeMagick(filename);
	GetExceptionInfo(&exception);
	image_info=CloneImageInfo((ImageInfo *) NULL);
	strcpy(image_info->filename, filename);

	image=ReadImage(image_info,&exception);

	W = image->columns; H = image->rows;
	pixels = GetImagePixels(image, 0,0,W,H);

	for (row = 1; row < H-1; row++) {
		for (col = 1; col < W-1; col++) {
			if ( pixels[row*W + col].blue < MaxRGB/8 )
			{
				pixels[row*W + col].blue =
				pixels[row*W + col].green = 
				pixels[row*W + col].red = MaxRGB;
			}
			else
			{
				pixels[row*W + col].blue =
				pixels[row*W + col].green = 
				pixels[row*W + col].red = 0;
			}
		}
	}
//	NegateImage(image, FALSE);
//	PosterizeImage(image, 2, 0);
//	NormalizeImage(image);

	SyncImagePixels(image);  
	//strcpy(image_info->filename, "c:\\temp\\pbmattempt.pbm");

    WriteImage(image_info,image);

	DestroyImageInfo(image_info);
	//DestroyImagePixels(image);
	DestroyImage(image);
	DestroyMagick();
	return 0;
}