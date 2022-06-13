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

extern "C"
{

	extern int current_image, current_monumentpoint, current_meal, current_photoblob, current_volumepoint, current_blobpoint;
	extern char qhull_path[MAX_PATH];
	extern int placematmap_id;
	extern int upwrite_radius;
	extern char current_filename[MAX_PATH];
	extern char filename_base[MAX_PATH];
}