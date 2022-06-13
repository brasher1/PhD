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
extern "C" {
#include "geometry.h"
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

void canny(char *filename);
void recognise(char *filename, char *orgfilename);
void input_classes(char *);
void ImageEdgeMagick(char *filename);
void canny2(float sigma, float tlow, float thigh, unsigned char **edge, char *filename);
void canny3(unsigned char *image, int rows, int cols, float sigma,
         float tlow, float thigh, unsigned char **edge, char *fname);

void startpgmcanny3(char *infilename, float sigma, float tlow, float thigh, char *dirfilename);
void startjpgcanny3(char *infilename, float sigma, float tlow, float thigh, char *dirfilename);

int OpenCVcanny( char *name);

int Negate_Image(char *filename);
int shade_detection(char *filename, SIMPLE_POINT *, SIMPLE_POINT *);

}