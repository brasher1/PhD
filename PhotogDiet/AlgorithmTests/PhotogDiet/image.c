/*
 * Programmed by:   Robert Ainsley McLaughlin
 * E-mail:          ram@ee.uwa.edu.au
 * Date:            18 August, 1996
 * Last modified:   1 March, 1998
 * Organisation:    The Center For Intelligent Information
 *                      Processing Systems,
 *                  Dept. Electrical & Electronic Engineering,
 *                  The University of Western Australia,
 *                  Nedlands W.A. 6907,
 *                  Australia
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
#include <stdlib.h>
#include <math.h>
#include <malloc.h>
#include <string.h>
#include <float.h>     /* MAXINT */
//#include <X11/Xlib.h>
//#include <X11/Xutil.h>
/*#include <X11/Xmd.h>        /* INT8, INT16, INT32 */
#include "upwrite.h"
#include <sys/types.h>
#include <magick/api.h>

#define INT8	short
#define INT16	int
#define INT32	long int



/* Function Prototypes.
 */
/*IMAGE   *alloc_IMAGE(int x, int y);
void    free_IMAGE(IMAGE *aImage);
int     get_pixel_value(IMAGE *aImage, int x, int y);
void    set_pixel_value(IMAGE *aImage, XImage *aXImage, int x, int y, int val);
void    set_XImage_pixel_value(int x, int y, int color, XImage *aXImage);
void    set_IMAGE_pixel_value(IMAGE *aImage, int x, int y, int val);
void    set_XImage_grey_pixel_value(XImage *aXImage, int x, int y, int val);
BOOLEAN test_pixel_for_tag(IMAGE *aImage, int x, int y, int tag);
void    tag_pixel(IMAGE *aImage, int x, int y, int tag);
void    untag_pixel(IMAGE *aImage, int x, int y);
void    clear_image(IMAGE *aImage, XImage *Ximage);
void    copy_image(IMAGE *toImage, IMAGE *fromImage);
void    untag_image(IMAGE *aImage);
void    untag_subimage(IMAGE *aImage, int x_min, int x_max,
                                            int y_min, int y_max);*/
IMAGE   *alloc_IMAGE(int x, int y);
void    free_IMAGE(IMAGE *aImage);
int     get_pixel_value(IMAGE *aImage, int x, int y);
void    set_pixel_value(IMAGE *aImage, int x, int y, int val);
void    set_XImage_pixel_value(int x, int y, int color);
void    set_IMAGE_pixel_value(IMAGE *aImage, int x, int y, int val);
void    set_XImage_grey_pixel_value(int x, int y, int val);
BOOLEAN test_pixel_for_tag(IMAGE *aImage, int x, int y, int tag);
void    tag_pixel(IMAGE *aImage, int x, int y, int tag);
void    untag_pixel(IMAGE *aImage, int x, int y);
void    clear_image(IMAGE *aImage);
void    copy_image(IMAGE *toImage, IMAGE *fromImage);
void    untag_image(IMAGE *aImage);
void    untag_subimage(IMAGE *aImage, int x_min, int x_max,
                                            int y_min, int y_max);

IMAGE   *load_pbm_image(char filename[]);
int     check_for_comment(FILE *infile);




/* Allocate all of the memory required for an IMAGE structure.
 */
IMAGE *
alloc_IMAGE(int x, int y)
{
    IMAGE   *aImage;
    int     i;


    aImage = (IMAGE *)malloc(sizeof(IMAGE ));
    if (aImage == NULL)
        {
        fprintf(stderr, "malloc #1 failed in alloc_IMAGE() in image.c\n");
        exit(1);
        }

    aImage->x = x;
    aImage->y = y;

    /* aImage->data is an array of pointers.
     * aImage->data[i] points to a block of memory which contains
     * the values for the i'th column in the IMAGE.
     * The pixel (x,y) is accessed with aImage->data[x][y]
     */
    aImage->data = (unsigned char **)malloc(x * sizeof(unsigned char *) );
    if (aImage->data == NULL)
        {
        fprintf(stderr, "malloc #2 failed in alloc_IMAGE() in image.c\n");
        exit(1);
        }
    for (i=0; i < x; i++)
        {
        aImage->data[i] = (unsigned char *)malloc(y * sizeof(unsigned char) );
        if (aImage->data[i] == NULL)
            {
            fprintf(stderr, "malloc #3 failed in alloc_IMAGE() in image.c\n");
            exit(1);
            }
        }


    return(aImage);

}   /* end of alloc_IMAGE() */



/* Free all of the memory associated with an IMAGE structure.
 */
void
free_IMAGE(IMAGE *aImage)
{
    int     x;

    if (aImage != NULL)
        {
        if (aImage->data != NULL)
            {
            for (x=0; x < (int)aImage->x; x++)
                free(aImage->data[x]);
            free(aImage->data);
            }
        free(aImage);
        }


}   /* end of free_IMAGE() */



/* Returns the value of the pixel at (x,y)
 * The return value will be in the range [0,63]
 * WHITE = 63, BLACK = 0.
 */
int
get_pixel_value(IMAGE *aImage, int x, int y)
{
    /* The lower 6 bits specify the value of the pixel.
     * The upper 2 bits are used for tagging a pixel.
     */
    return((int )(aImage->data[x][y] & (unsigned char )0x3F));

}   /* end of get_pixel_value() */



/* Set the value of the pixel at (x,y)
 * in both the IMAGE structure (i.e. the one that we work with)
 * and the XImage structure (i.e. the one we use to display).
 * val should be in the range [0,63]
 *
 * In the IMAGE structure, the lower 6 bits specify the value,
 * the highest 2 bits are used as tags.
 * Only the lower 6 bits of the pixel in the IMAGE structure are affected.
 *
 * gNumGreyLevels is the number of grey levels that we
 * have allocated. Ideally, this will be 64, but some
 * machines can't handle this, so we dont assume it.
 * The value in the XImage is automatically set to
 * an appropriate grey level.
 * WHITE = 63, BLACK = 0.
 */
void
set_pixel_value(IMAGE *aImage, int x, int y, int val)
{
    int     index;

    if ((x >= 0) && (x < (int)aImage->x) && (y >= 0) && (y < (int)aImage->y))
        {
        /* Clear the lower 6 bits. */
        aImage->data[x][y] &= (unsigned char )0xC0;

        /* Set the value. */
        aImage->data[x][y] |= (unsigned char )val;
#ifdef DUMMY
        /* num_grey_levels is the number of grey levels that we
         * have allocated. Ideally, this will be 64, but some
         * machines can't handle this, so we dont assume it.
         * These grey levels are stored in gGreyValues[]
         * (info such as what the corresponding entry in the
         * color map is).
         * `index' equals val scaled to be in the range [0,gNumGreyLevels).
         *
         * We have to do some type casting of aXImage->data,
         * depending on the depth of the visual.
         */
        index = (int )( (((double )val)/64.0) * (double )gNumGreyLevels);
        switch (gVisualDepth)
            {
            case 8:
                *(((INT8 *)(aXImage->data)) + (y*aXImage->width) + x) = gGreyValues[index].pixel;
                break;
    
            case 12:
            case 15:
            case 16:
                *(((INT16 *)(aXImage->data)) + (y*aXImage->width) + x) = gGreyValues[index].pixel;
                break;

            case 24:
            case 32:
                *(((INT32 *)(aXImage->data)) + (y*aXImage->width) + x) = gGreyValues[index].pixel;
                break;
            }
        }

#endif
	}
}   /* end of set_pixel_value() */



void
set_XImage_pixel_value(int x, int y, int color)
{
#ifdef DUMMY
    if ((x >= 0) && (x < aXImage->width) && (y >= 0) && (y < aXImage->height))
        {
        switch (gVisualDepth)
            {
            case 8:
                *(((INT8 *)(aXImage->data)) + (y*aXImage->width) + x) = gRandomColors[color%NUM_RANDOM_COLORS].pixel;
                break;
    
            case 12:
            case 15:
            case 16:
                *(((INT16 *)(aXImage->data)) + (y*aXImage->width) + x) = gRandomColors[color%NUM_RANDOM_COLORS].pixel;
                break;

            case 24:
            case 32:
                *(((INT32 *)(aXImage->data)) + (y*aXImage->width) + x) = gRandomColors[color%NUM_RANDOM_COLORS].pixel;
                break;
            }
        }
#endif
}   /* end of set_XImage_pixel_value() */



/* Set the value of the pixel at (x,y)
 * in the IMAGE structure (i.e. the one that we work with)
 * val should be in the range [0,63]
 *
 * In the IMAGE structure, the lower 6 bits specify the value,
 * the highest 2 bits are used as tags.
 * Only the lower 6 bits of the pixel in the IMAGE structure are affected.
 */
void
set_IMAGE_pixel_value(IMAGE *aImage, int x, int y, int val)
{

    if ((x >= 0) && (x < (int)aImage->x) && (y >= 0) && (y < (int)aImage->y))
        {
        /* Clear the lower 6 bits. */
        aImage->data[x][y] &= (unsigned char )0xC0;

        /* Set the value. */
        aImage->data[x][y] |= (unsigned char )val;

        }

}   /* end of set_IMAGE_pixel_value() */


void
set_XImage_grey_pixel_value(int x, int y, int val)
{
    int     index;
#ifdef DUMMY
    if ((x >= 0) && (x < aXImage->width) && (y >= 0) && (y < aXImage->height))
        {

        /* num_grey_levels is the number of grey levels that we
         * have allocated. Ideally, this will be 64, but some
         * machines can't handle this, so we dont assume it.
         * These grey levels are stored in gGreyValues[]
         * (info such as what the corresponding entry in the
         * color map is).
         * `index' equals val scaled to be in the range [0,gNumGreyLevels).
         */
        index = (int )( (((double )val)/64.0) * (double )gNumGreyLevels);
        switch (gVisualDepth)
            {
            case 8:
                *(((INT8 *)(aXImage->data)) + (y*aXImage->width) + x) = gGreyValues[index].pixel;
                break;
    
            case 12:
            case 15:
            case 16:
                *(((INT16 *)(aXImage->data)) + (y*aXImage->width) + x) = gGreyValues[index].pixel;
                break;

            case 24:
            case 32:
                *(((INT32 *)(aXImage->data)) + (y*aXImage->width) + x) = gGreyValues[index].pixel;
                break;
            }
        }
#endif
}   /* end of set_XImage_grey_pixel_value() */



/* Test to see if a pixel is tagged.
 * There are 4 possible tags.
 * `tag' specifies the tag that we are looking for.
 * Valid values are 0,1,2,3.
 * Value 0 is usually used to denote an untagged pixel.
 */
BOOLEAN
test_pixel_for_tag(IMAGE *aImage, int x, int y, int tag)
{
    unsigned char   val;

    /* Move the tag into the lower 2 bits.
     */
    val = (aImage->data[x][y]) >> (unsigned )6;

    if (val == (unsigned char )tag)
        return(TRUE);
    else
        return(FALSE);

}   /* end of test_pixel_for_tag() */


/* Tag the pixel (x,y)
 * Valid values for `tag' are 1,2,3.
 * A value of 0 means that the pixel is untagged.
 * The tag is stored in the most significant
 * 2 bits of the pixel.
 */
void
tag_pixel(IMAGE *aImage, int x, int y, int tag)
{
    unsigned char   val;

    aImage->data[x][y] &= (unsigned char )0x3F; /* Clear the m.s. 2 bits. */

    val = ((unsigned char )tag) << (unsigned )6;
    aImage->data[x][y] |= (unsigned char )val;  /* Set the tag. */

}   /* end of tag_pixel() */



/* Clear the Most Significant 2 bits of the pixel (x,y),
 * which store the tag.
 */
void
untag_pixel(IMAGE *aImage, int x, int y)
{
    aImage->data[x][y] &= (unsigned char )0x3F;

}   /* end of untag_pixel() */



/* Set all pixels in the image to white.
 * WHITE = 63, BLACK = 0.
 */
void
clear_image(IMAGE *aImage)
{
    int x, y;

    void    set_pixel_value(IMAGE *aImage, int x, int y, int val);

    for (x=0; x < (int)aImage->x; x++)
        for (y=0; y < (int)aImage->y; y++)
            {
            set_pixel_value(aImage, x, y, WHITE);
            }
}   /* end of clear_image() */



void
copy_image(IMAGE *toImage, IMAGE *fromImage)
{
    int     x, y;


    if ((fromImage == (IMAGE *)NULL) ||
                (toImage == (IMAGE *)NULL) ||
                (fromImage->x != toImage->x) ||
                (fromImage->y != toImage->y))
        {
        fprintf(stderr, "bad arguements passed to copy_image() in image.c\n");
        exit(1);
        }

    for (x=0; x < (int)fromImage->x; x++)
        for (y=0; y < (int)fromImage->y; y++)
            toImage->data[x][y] = fromImage->data[x][y];


}   /* end of copy_image() */



/* Clear the Most Significant 2 bits of every pixel
 * in the image. These are the two bits which store the tag.
 */
void
untag_image(IMAGE *aImage)
{
    int x, y;

    for (x=0; x < (int)aImage->x; x++)
        for (y=0; y < (int)aImage->y; y++)
            {
            aImage->data[x][y] &= (unsigned char )0x3F;
            }

}   /* end of untag_image() */



/* Clear the Most Significant 2 bits of every pixel
 * in the image. These are the two bits which store the tag.
 */
void
untag_subimage(IMAGE *aImage, int x_min, int x_max, int y_min, int y_max)
{
    int x, y;

    if ((x_min > x_max) || (y_min > y_max))
        return;
    if ((x_min < 0) || (x_max >= (int)aImage->x) ||
                    (y_min < 0) || (y_max >= (int)aImage->y))
        return;


    for (x=x_min; x <= x_max; x++)
        for (y=y_min; y < y_max; y++)
            {
            aImage->data[x][y] &= (unsigned char )0x3F;
            }

}   /* end of untag_subimage() */



IMAGE * load_image(char filename[])
{
    IMAGE   *aImage;
    FILE    *infile;
    char    magic_num[10];
    int     width, height, x, y, data, i;
    ExceptionInfo exception;
    ImageInfo *image_info;
    ImageInfo *image_info2;

	Image *image;
	PixelPacket *pixels;
	int row, col;

	unsigned char   data_raw, value;
    int     check_for_comment(FILE *infile);
    IMAGE   *alloc_IMAGE(int x, int y);
    void    set_IMAGE_pixel_value(IMAGE *aImage, int x, int y, int val);

	InitializeMagick(filename);
    GetExceptionInfo(&exception);
    image_info=CloneImageInfo((ImageInfo *) NULL);
    strcpy(image_info->filename, filename);
	
	image_info->antialias = FALSE;
	image_info->dither = FALSE;

	image_info->sampling_factor = "4:4:4";
    image=ReadImage(image_info,&exception);
    GetImageInfo(image_info);

//	image_info2=CloneImageInfo((ImageInfo *) NULL);
//	strcpy(image_info2->filename, "program_output.pbm");
//	WriteImage(image_info2, image);

    if (exception.severity != UndefinedException)
      CatchException(&exception);

    height = image->rows;  width = image->columns;	

	aImage = alloc_IMAGE(width, height);

	pixels = GetAuthenticPixels(image, 0,0, width, height, &exception);
#ifdef VERBOSE_INSTRUMENTATION
	infile = fopen("RawImagePixels.txt", "w+");
	
    for (x=0; x < 40; x++)
	{
        for (y=0; y < 40; y++)
            {
				fprintf(infile, " %hu, ", pixels[x*width + y].blue);
            }
		fprintf(infile, "--->>\n");
	}
	fclose(infile);
#endif //VERBOSE_INSTRUMENTATION
	for(row=0; row<height; row++)
	{
		for( col=0; col<width; col++)
		{
			if( pixels[row*width + col].red <= MaxRGB/2)
				set_IMAGE_pixel_value(aImage, col, row, BLACK);
			else
				set_IMAGE_pixel_value(aImage, col, row, WHITE);
		}
	}
#ifdef VERBOSE_INSTRUMENTATION
	infile = fopen("UpWritePixels.txt", "w+");
    for (x=1066; x < 1157; x++)
	{
        for (y=773; y < 872; y++)
            {
				fprintf(infile, " %d, ", get_pixel_value(aImage,x,y));
            }
		fprintf(infile, "--->\n");
	}
	fclose(infile);
#endif //VERBOSE_INSTRUMENTATION
	//DestroyExceptionInfo(&exception);
	DestroyImageInfo(image_info);
	//DestroyImagePixels(image);
	DestroyImage(image);
	DestroyMagick();

	return(aImage);
}
#ifdef DUMMY
/* Load in a PBM image (ascii or binary)
 */
IMAGE *
load_pbm_image(char filename[])
{
    IMAGE   *aImage;
    FILE    *infile;
    char    magic_num[10];
    int     width, height, x, y, data, i;
    unsigned char   data_raw, value;
    int     check_for_comment(FILE *infile);
    IMAGE   *alloc_IMAGE(int x, int y);
    void    set_IMAGE_pixel_value(IMAGE *aImage, int x, int y, int val);


    infile = fopen(filename, "r");
    if (infile == NULL)
        {
        fprintf(stderr, "Error opening file %s in load_pbm_image() in image.c\n", filename);
        exit(1);
        }


    if (check_for_comment(infile) == EOF)
        {
        fprintf(stderr, "Error reading file %s in load_pbm_image() in image.c\n", filename);
        exit(1);
        }

    if (fscanf(infile, "%[P14]", magic_num) != 1)
        {
        fprintf(stderr, "Error reading magic number of file %s in load_pbm_image() in image.c\n", filename);
        exit(1);
        }

    if ((strlen(magic_num) != 2) ||
                            ((magic_num[1] != '1') && (magic_num[1] != '4')))
        {
        fprintf(stderr, "Error reading magic number of file %s in load_pbm_image() in image.c\n", filename);
        exit(1);
        }

    if (check_for_comment(infile) == EOF)
        {
        fprintf(stderr, "Error reading file %s in load_pbm_image() in image.c\n", filename);
        exit(1);
        }

    if (fscanf(infile, "%d", &width) != 1)
        {
        fprintf(stderr, "Error reading width in file %s in load_pbm_image() in image.c\n", filename);
        exit(1);
        }

    if (check_for_comment(infile) == EOF)
        {
        fprintf(stderr, "Error reading file %s in load_pbm_image() in image.c\n", filename);
        exit(1);
        }

    if (fscanf(infile, "%d", &height) != 1)
        {
        fprintf(stderr, "Error reading height in file %s in load_pbm_image() in image.c\n", filename);
        exit(1);
        }

    aImage = alloc_IMAGE(width, height);

    /* Read in the data
     */
    if (magic_num[1] == '1')
        {
        if (check_for_comment(infile) == EOF)
            return((IMAGE *)NULL);
        for (y=0; y < height; y++)
            {
            /* Read in a line of data.
             */
            for (x=0; x < width; x++)
                {
                if (fscanf(infile, "%d", &data) != 1)
                    {
                    fprintf(stderr, "Error reading data from file %s in load_pbm_image() in image.c\n", filename);
                    exit(1);
                    }
                if (data == 1)
                    set_IMAGE_pixel_value(aImage, x, y, BLACK);
                else if (data == 0)
                    set_IMAGE_pixel_value(aImage, x, y, WHITE);
                check_for_comment(infile);
                }
            }
        }
    else if (magic_num[1] == '4')
        {
        /* Discard white space after height.
         */
        fscanf(infile, "%*c");

        for (y=0; y < height; y++)
            {
            /* Read in a line of data.
             */
            for (x=0; x < width; x+=8)
                {
                if (fscanf(infile, "%c", &data_raw) != 1)
                    {
                    fprintf(stderr, "Error reading data from file %s in load_pbm_image() in image.c\n", filename);
                    exit(1);
                    }

                /* Extract each bit as the value for a pixel.
                 */
                for (i=0; i < 8; i++)
                    {
                    value = data_raw >> (7-i);
                    value = value & 0x01;       /* Mask 00000001 */
                    if (value == (unsigned char )1)
                        set_IMAGE_pixel_value(aImage, x+i, y, BLACK);
                    else if (value == (unsigned char )0)
                        set_IMAGE_pixel_value(aImage, x+i, y, WHITE);
                    }
                }
            }
        }

    return(aImage);


}   /* end of load_pbm_image() */
#endif //DUMMY


/* Read in (possibly multiple) lines of comments.
 * Comments begin with #
 * and continue until the next end-of-line character.
 *
 * If we read in EOF, then return EOF
 * otherwise return not EOF.
 */
int
check_for_comment(FILE *infile)
{
    unsigned char   c;


    /* Remove trailing white characters.
     */
    do
        c = getc(infile);
    while ((c == '\n') || (c == ' '));


    /* If line begins with #
     * discard it - it is a comment.
     */
    while (c == '#')
        {
        /* Read in the rest of the line as comment
         */
        do
            {
            c = (unsigned char )getc(infile);
            }
        while ((c != (unsigned char )'\n') && (c != (unsigned char )EOF));
        c = getc(infile);   /* to check if next line begins with # */
        }

    /* When testing to see if the next line began with a #,
     * we read one too many characters.
     */
    if (c != (unsigned char )EOF)
        {
        if (ungetc(c, infile) == EOF)
            {
            fprintf(stderr, "Error reading pbm file in check_for_comment() in image.c\n");
            exit(1);
            }
        }


    if (c == (unsigned char )EOF)
        return(EOF);
    else
        return(!EOF);

}   /* end of check_for_comment() */
