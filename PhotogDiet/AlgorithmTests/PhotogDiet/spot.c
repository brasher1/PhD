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
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>

#include "upwrite.h"


//#define VERBOSE_INSTRUMENTATION


/* Function Prototypes
 */
GAUS_LIST   *spot_algorithm_2D(IMAGE *aImage);




/* spot_algorithm_2D() is to be used with black and white images.
 */

GAUS_LIST *
spot_algorithm_2D(IMAGE *aImage)
{
#ifdef VERBOSE_INSTRUMENTATION
	FILE *output;
	FILE *imagedata;
	FILE *output2;
#endif //VERBOSE_INSTRUMENTATION
    int     x, y;
    int     x1, y1, x2, y2, i, j;
    double  sum_x, sum_y, num_points;
    GAUS    *gaus;
    GAUS_LIST   *gaus_list;
    int     get_pixel_value(IMAGE *aImage, int x, int y);
    BOOLEAN test_pixel_for_tag(IMAGE *aImage, int x, int y, int tag);
    void    tag_pixel(IMAGE *aImage, int x, int y, int tag);
    void    untag_image(IMAGE *aImage);
    GAUS    *alloc_gaus(int dim);
    void    free_gaus(GAUS *g);
    void    find_eigenvectors_2x2_positive_semi_def_real_matrix(double **covar, double *eig_val, double **eig_vec);
    GAUS_LIST   *put_in_linked_list_of_gaus(GAUS_LIST *gaus_list, GAUS *g);
//    void    draw_image(XImage *X_im, Window theWindow, GC theGC);


    gaus_list = NULL;
#ifdef VERBOSE_INSTRUMENTATION
    imagedata = fopen("imagedata.txt", "w+");
	
    for( i=0; i<aImage->x; i++)
	{
		for( j=0; j<aImage->y; j++)
		{
			fprintf(imagedata, "%u, ", get_pixel_value(aImage,i,j));
		}
		fprintf(imagedata,"\r\n");
		fprintf(imagedata,"************************\r\n");
	}
		fclose(imagedata);
#endif //VERBOSE_INSTRUMENTATION

//    draw_image(aXImage, theWindow, theGC);
    /* Set foreground color to the first random color.
     */
//    XSetForeground(gDisplay, theGC, gRandomColors[0].pixel);

    /* Untag pixels (just in case any happen to be tagged).
     */
    untag_image(aImage);
#ifdef VERBOSE_INSTRUMENTATION
    output = fopen("instrument.txt", "a+");
#endif //VERBOSE_INSTRUMENTATION
        for (x=0; x < (int)aImage->x; x++)
			for (y=0; y < (int)aImage->y; y++)

            {

            /* Find an untagged (i.e. not yet used in a spot),
             * BLACK pixel.
             */
            if ((get_pixel_value(aImage, x, y)==BLACK) && (test_pixel_for_tag(aImage,x,y,1) == FALSE))
                {
                x1 = x - (int )gResolutionRadius;
                y1 = y - (int )gResolutionRadius;
                if (x1 < 0)
                    x1 = 0;
                if (y1 < 0)
                    y1 = 0;
                x2 = x + (int )gResolutionRadius;
                y2 = y + (int )gResolutionRadius;
                if (x2 >= (int)aImage->x)
                    x2 = (int)aImage->x-1;
                if (y2 >= (int)aImage->y)
                    y2 = (int)aImage->y-1;
#ifdef VERBOSE_INSTRUMENTATION
				fprintf(output, "#######***BEGINNING OF LOOP***#######\r\n");
				fprintf(output, "glResolutionRadius = %f\n", gResolutionRadius);
				fprintf(output, "x=%d, y=%d, pixel=%d\n", x, y, get_pixel_value(aImage, x, y));
				fprintf(output, "x1=%d, y1=%d, x2=%d, y2=%d\r\n", x1, y1, x2, y2);
#endif //VERBOSE_INSTRUMENTATION

                /* Calculate mean of points in ball
                 */
                sum_x = 0.0;
                sum_y = 0.0;
                num_points = 0.0;

                for (i=x1; i <= x2; i++)
                    for (j=y1; j <= y2; j++)
                        if (get_pixel_value(aImage,i,j) == BLACK)
                            if ((SQR(i-x) + SQR(j-y)) < SQR(gResolutionRadius))
                                {
                                sum_x += (double )i;
                                sum_y += (double )j;
                                num_points++;
                                }

				/* Need at least a total of 1 point for a gaussian
                 * or the covariance formule doesn't make sense
                 * since it has a (num_points-1) in the denominator.
                 */
                if (num_points <= 1.0)
                    continue;

//                XDrawArc(gDisplay, theWindow, theGC, x1, y1, (x2-x1), (y2-y1), 0, 23040);
//                XFlush(gDisplay);

                gaus = alloc_gaus(2);

                gaus->num_points = num_points;
                gaus->mean[1] = sum_x / (double )num_points;
                gaus->mean[2] = sum_y / (double )num_points;
#ifdef VERBOSE_INSTRUMENTATION
				fprintf(output, "x=%d, y=%d, pixel=%d\n", x, y, get_pixel_value(aImage, x, y));
				fprintf(output, "***MEAN***\n");
				fprintf(output, "gaus->mean[1]=%f\n", gaus->mean[1]);
				fprintf(output, "gaus->mean[2]=%f\n", gaus->mean[2]);
				
#endif
				
				
                /* Calculate covariance matrix
                 */
                gaus->covar[1][1] = 0.0;  gaus->covar[1][2] = 0.0;
                gaus->covar[2][1] = 0.0;  gaus->covar[2][2] = 0.0;

                for (i=x1; i <=x2; i++)
                    for (j=y1; j <=y2; j++)
                        if (get_pixel_value(aImage,i,j) == BLACK)
                            if ((SQR(i-x) + SQR(j-y)) < SQR(gResolutionRadius))
                                {
                                gaus->covar[1][1] += SQR(((double )i) - gaus->mean[1]);
                                gaus->covar[1][2] += (((double )i) - gaus->mean[1])*(((double )j) - gaus->mean[2]);
                                gaus->covar[2][2] += SQR(((double )j) - gaus->mean[2]);
                                }


                gaus->covar[1][1] /= (double )num_points-1;
                gaus->covar[1][2] /= (double )num_points-1;
                gaus->covar[2][2] /= (double )num_points-1;
                gaus->covar[2][1] = gaus->covar[1][2];

#ifdef VERBOSE_INSTRUMENTATION				
				fprintf(output, "***COVARIANCE***\n");
				fprintf(output, "gaus->covar[1][1]=%f, gaus->covar[1][2]=%f\n", gaus->covar[1][1], gaus->covar[1][2]);
				fprintf(output, "gaus->covar[2][1]=%f, gaus->covar[2][2]=%f\n", gaus->covar[1][2], gaus->covar[2][2]);
#endif //VERBOSE_INSTRUMENTATION
                find_eigenvectors_2x2_positive_semi_def_real_matrix(gaus->covar, gaus->eig_val, gaus->eig_vec);
#ifdef VERBOSE_INSTRUMENTATION
				fprintf(output, "***EIGEN-VECTOR***\n");
				fprintf(output, "gaus->eig_vec[1][1]=%f, gaus->eig_vec[1][2]=%f\n", gaus->eig_vec[1][1], gaus->eig_vec[1][2]);
				fprintf(output, "gaus->eig_vec[2][1]=%f, gaus->eig_vec[2][2]=%f\n", gaus->eig_vec[1][2], gaus->eig_vec[2][2]);
				
				fprintf(output, "***EIGEN-VALUES**\n");
				fprintf(output, "gaus->eig_val[1]=%f\n", gaus->eig_val[1]);
				fprintf(output, "gaus->eig_val[2]=%f\n", gaus->eig_val[2]);
#endif //VERBOSE_INSTRUMENTATION                
				/* If it has at least 1 non-zero eigenvalue
                 */
                if (gaus->eig_val[1] > 0.0)
                    {
                    /* Set ellipses to a standard size.
                     */
                    gaus->eig_val[1] = SQR(gResolutionRadius);
                    gaus->eig_val[2] = SQR(gResolutionRadius/3);

                    gaus_list = put_in_linked_list_of_gaus(gaus_list, gaus);

                    /* Tag all black pixels within the ball.
                     * Tagged pixels may not be the centre of
                     * a later ball, but may be included in the
                     * calculations for future balls.
                     */

                    for (i=x1; i <=x2; i++)
                        for (j=y1; j <=y2; j++)
                            if ((SQR(i-x) + SQR(j-y)) < SQR(gResolutionRadius))
                                {
//								fprintf(output,"TAGGING PIXEL i=%d, j=%d\r\n", i,j);
                                tag_pixel(aImage, i, j, 1);
                                }



                    }   /* end of 'if (gaus->eig_val[1] > 0.0)' */
                else
                    {
                    free_gaus(gaus);
                    continue;
                    }

                }   /* end of 'if ((get_pixel_value(aImage, x, y)==...'*/
            }   /* end of for (y=0; y < aImage->y; y++) */


    /* Untag pixels
     */
    untag_image(aImage);
#ifdef VERBOSE_INSTRUMENTATION
	fclose(output);                
#endif //VERBOSE_INSTRUMENTATION
    return(gaus_list);

}   /* end of spot_algorithm_2D() */
