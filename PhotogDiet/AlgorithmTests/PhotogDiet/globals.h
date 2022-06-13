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
/* GLOBAL DECLARATIONS.
 */
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

double          gResolutionRadius;
//Display         *gDisplay;
//Colormap        gColormap;
int             gLineThickness;
//XColor          *gGreyValues;
//XColor          *gRandomColors;
int             gNumGreyLevels; /* The number of grey levels
                                 * that will be displayed
                                 * in the image.
                                 */
long int        gTime;      /* Used in coloring algorithm */
BOOLEAN         gGraphics;
POINT_LIST      *gTrainingPointList;
int             gOldCountTrainingPoint;
GAUS_LIST       *gClassList;
DISPLAY_OBJECT_LIST *gDisplayObjectList;
int             gVisualDepth;
double   sqrarg;    /* Used in the macro SQR() */
double   cubarg;    /* Used in the macro CUBE() */
double   absarg;    /* Used in the macro ABS() */