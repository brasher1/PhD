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
#include <sys/types.h>      /* time() */
#include <sys/time.h>       /* time() */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "../include/upwrite.h"



/* Function Prototypes
 */
void    init_upwrite(void);


void
init_upwrite(void)
{
    time_t  seed;           /* used to initialise random number drand48() */
    void    srand48(long seedval);
    void    input_classes(void);


    /* Initialise Resolution Radius.
     */
    gResolutionRadius = 15.0;
    gLineThickness = 2;
    gTrainingPointList = NULL;
    gOldCountTrainingPoint = 0;
    gDisplayObjectList = NULL;
    input_classes();        /* These are put in gClassList */

    /* initialise random number generator.
     */
    if (time(&seed) == -1)
        {
        seed = (time_t )1;      /* arbitary number */
        }
    srand48((long )seed);


}   /* end of init_upwrite() */
