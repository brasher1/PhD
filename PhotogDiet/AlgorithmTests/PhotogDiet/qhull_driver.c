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
#include <windows.h>

#include "database.h"

	extern char qhull_path[MAX_PATH];

int call_qhull(int fooditem_id)
{
	char command_line[2*MAX_PATH];
	FILE *inputfile;
	WORLD_POINT *wp;
	int count;
	int i;

	wp = get_volume_points(fooditem_id);
	count = get_volume_point_count();

	/* TODO: put the points in a file for Qhull */
	inputfile = fopen("qhullinput.txt", "w+");
	if( inputfile == NULL )
		return -1;
	fprintf(inputfile, "3\n%d\n", count);

	for(i=0; i<count; i++)
	{
		fprintf(inputfile, "%d\t%d\t%d\n", wp[i].x, wp[i].y, wp[i].z);
	}
	fclose(inputfile);

	/* TODO: Call Qhull */
	if( qhull_path[strlen(qhull_path)-1] != "\\" )
		sprintf(command_line, "%s\\qconvex o FA s <qhullinput.txt", qhull_path);
	else
		sprintf(command_line, "%sqconvex o FA s <qhullinput.txt", qhull_path);


	/* TODO: Process QHull results */

	
	return(0);
}