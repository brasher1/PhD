/*
 * Programmed by:   Robert Ainsley McLaughlin
 * E-mail:          ram@ee.uwa.edu.au
 * Date:            18 August, 1996
 * Last modified:   23 Feb, 1998
 * Organisation:    The Center For Intelligent Information
 *                      Processing Systems,
 *                  Dept. Electrical & Electronic Engineering,
 *                  The University of Western Australia,
 *                  Nedlands W.A. 6907,
 *                  Australia
 */
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "../include/upwrite.h"



/* Function Prototypes
 */
void    add_speckle_noise(double proportion_noise, IMAGE *aImage,
                                XImage *aXImage, Window theWindow, GC theGC);



void
add_speckle_noise(double proportion_noise, IMAGE *aImage,
                                XImage *aXImage, Window theWindow, GC theGC)
{
    int     i, num, x, y;
    int     get_pixel_value(IMAGE *aImage, int x, int y);
    void    set_pixel_value(IMAGE *aImage, XImage *aXImage,
                                                int x, int y, int val);



    num = (int )(proportion_noise * (double )aImage->x * (double )aImage->y);

    for (i=0; i < num; i++)
        {
        x = (int )(drand48() * (double )aImage->x);
        y = (int )(drand48() * (double )aImage->y);

        if (get_pixel_value(aImage, x, y) == WHITE)
            set_pixel_value(aImage, aXImage, x, y, BLACK);
        else if (get_pixel_value(aImage, x, y) == BLACK)
            set_pixel_value(aImage, aXImage, x, y, WHITE);
        }


}       /* end of add_speckle_noise() */
