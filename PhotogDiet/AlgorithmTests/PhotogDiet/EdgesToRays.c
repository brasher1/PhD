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
//#define WIN32_LEAN_AND_MEAN
//#include <windows.h>
//#undef WIN32_LEAN_AND_MEAN
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
#include <math.h>
#include "linklist.h"
#include "database.h"

#include "geometry.h"



int convertEdgePointsToRays(int photoBlobID)
{
	//TODO: Get Camera Matrix for photo

	//TODO: For each edge point
	//TODO:		Transform photo point to a 3-D world point on placemat (Z=0)

	return 0;
}