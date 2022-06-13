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
#ifndef IMAGES_H
#define IMAGES_H
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

int shade_detection(char *filename, SIMPLE_POINT *center, SIMPLE_POINT *points);

int save_cvImageAsPBM(char *filename);
int save_cvImageAsJPG();
int save_cvSegmentationImageAsJPG(float sigma, float k, int min_size);
int save_cvSegmentationImageAsPPM(float sigma, float k, int min_size);

int openCVsegmentation(char *filename);

typedef struct imagecontent {
	float entropy;
	float energy;
	float intertia;
} IMAGECONTENT;

#endif //IMAGES_H
