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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "geometry.h"

/* Function uses the dot product of a line between two lines
   at their closest point and each of the other two lines.  Input arguments must
   be two points on the lines in question.  The end result is the 
   function will return the mid-point of the line segment between
   the two input lines at their closest point to each other */
INTERSECT_POINT_3D * closest_point_between_2_lines(POINT_3D *p1, POINT_3D *p2,
								  POINT_3D *p3, POINT_3D *p4)
{

	double v1321, v4343, v1343, v4321, v2121;
//	double v43x, v43y, v43z;
//	double v21x, v21y, v21z;
//	double v13x, v13y, v13z;
	POINT_3D pointa, pointb;
	INTERSECT_POINT_3D *midpoint;

	double ta, tb;

	v1321 = (p1->x - p3->x)*(p2->x - p1->x) + (p1->y - p3->y)*(p2->y - p1->y) + (p1->z - p3->z)*(p2->z - p1->z);
	v4343 = (p4->x - p3->x)*(p4->x - p3->x) + (p4->y - p3->y)*(p4->y - p3->y) + (p4->z - p3->z)*(p4->z - p3->z);
	v1343 = (p1->x - p3->x)*(p4->x - p3->x) + (p1->y - p3->y)*(p4->y - p3->y) + (p1->z - p3->z)*(p4->z - p3->z);
	v4321 = (p4->x - p3->x)*(p2->x - p1->x) + (p4->y - p3->y)*(p2->y - p1->y) + (p4->z - p3->z)*(p2->z - p1->z);
	v2121 = (p2->x - p1->x)*(p2->x - p1->x) + (p2->y - p1->y)*(p2->y - p1->y) + (p2->z - p1->z)*(p2->z - p1->z);
	
	ta = ((v1321)*(v4343) - (v1343)*(v4321))/((v4321)*(v4321)- (v2121)*(v4343));
	tb = (v1343 + (ta*(v4321)))/v4343;

    pointa.x = p1->x + ta*(p2->x - p1->x);
    pointa.y = p1->y + ta*(p2->y - p1->y);
    pointa.z = p1->z + ta*(p2->z - p1->z);

    pointb.x = p3->x + tb*(p4->x - p3->x);
    pointb.y = p3->y + tb*(p4->y - p3->y);
    pointb.z = p3->z + tb*(p4->z - p3->z);

	midpoint = (INTERSECT_POINT_3D *)malloc(sizeof(INTERSECT_POINT_3D));

	midpoint->distance = sqrt(pow((pointa.x - pointb.x), 2.0) + pow((pointa.y - pointb.y), 2.0) + pow((pointa.z - pointb.z), 2.0));

	midpoint->x = (pointa.x + pointb.x)/2.0;
	midpoint->y = (pointa.y + pointb.y)/2.0;
	midpoint->z = (pointa.z + pointb.z)/2.0;

	return midpoint;


}