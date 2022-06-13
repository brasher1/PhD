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
#pragma once

#include "resource.h"

//  algorithm points inside the photogrammetry setup and segmentation portion of the code.
#define WHOLE				0
#define CANNY				1
#define NEGATEIMAGE			2
#define UPWRITE				3
#define ASSIGNCOORDDINATES	4
#define LOCATECAMERA		5
#define SAVEIMAGEPBM		6
#define SEGMENTATION		7
#define SAVEIMAGEPPM		8
#define FINDFOOD			9
#define REVIEWPOINTASSIGNMENT 10


extern "C"
{
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

HANDLE executeFullProcess(void *);
int doBatchProcess(char *filename);


}
