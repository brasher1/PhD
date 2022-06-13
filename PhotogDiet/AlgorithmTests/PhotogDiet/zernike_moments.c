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
 *
 * Modified by:     David John Brasher
 * E-mail:			brasher1@uab.edu
 * Date:			2010
 * Last Modified:	
 * Organization:	University of Alabama Birmingham
 *					Dept. Electrical and Computer Engineering
 *					
 */
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "upwrite.h"


/* Function Prototypes
 */
POINT   *calculate_zernike_moments(GAUS_LIST *gaus_list, int max_order);
double  factorial(int x);



/* Calculate Zernike moments of the points attatched
 * to a linked list of GAUS, up
 * to order max_order (inclusive).
 *
 * The GAUS (and hence the points) are assumed to be 2-D.
 *
 * Each point set is translated so that the centroid is
 * at the origin.
 * Because of this A(1,1) will always equal 0,
 * so we don't calculate A(1,1).
 *
 * We get scale invariance by first finding the distance
 * from the centroid to the furtherest point in the image.
 * Use this to scale the image so that it lies within the
 * unit circle (the zernike moments that we use are only
 * orthogonal in the unit circle, so we would have done this
 * anyway).
 * Then, after having calculated the zernike moments, divide
 * them by the zeroth order geometric moment of the image.
 * Because of this, A(0,0) will always equal 1/pi,
 * so we don't calculate A(0,0).
 */
POINT *
calculate_zernike_moments(GAUS_LIST *gaus_list, int max_order)
{
    POINT   *point, *pt;
    GAUS    *gaus;
    double  x_mean, y_mean, zeroth;
    double  dist, max_dist;
    int     p, q, s, num_terms;
    double  A_real, A_imaginary, Z_real, Z_imaginary, R;
    double  rho, theta, numer, denom;
    int     index;
    POINT   *alloc_point(int dim);
    double  factorial(int x);


    if (max_order < 2)
        return(NULL);


    /* How many terms will we be returning.
     */
    num_terms = ((max_order/2)+1) * (((max_order+1)/2) + 1) - 2;
    point = alloc_point(num_terms);

    for (index=1; index < point->dim; index++)
        point->x[index] = 0;


    /* Error Check.
     */
    if ((gaus_list == NULL) || (gaus_list->head == NULL))
        return(point);



    /* Count the number of points and calculate the centroid (mean)
     * of the image.
     * These are the zeroth and first order geometric moments.
     */
    x_mean = 0;
    y_mean = 0;
    zeroth = 0;
    for (gaus=gaus_list->head; gaus != NULL; gaus=gaus->next)
        if (gaus->point_list != NULL)
            for (pt=gaus->point_list->head; pt != NULL; pt=pt->next)
                {
                x_mean += (double )pt->x[1];
                y_mean += (double )pt->x[2];
                zeroth++;   /* Zeroth order moment = number of points */
                }

    x_mean /= zeroth;
    y_mean /= zeroth;


    /* Calculate the distance from the centroid
     * to the furtherest point in the image.
     * We will use this to scale the image
     * so that it lies within the unit disc.
     */
    max_dist = 0;
    for (gaus=gaus_list->head; gaus != NULL; gaus=gaus->next)
        if (gaus->point_list != NULL)
            for (pt=gaus->point_list->head; pt != NULL; pt=pt->next)
                {
                dist = sqrt( SQR(pt->x[1]-x_mean) + SQR(pt->x[2]-y_mean) );
                if (dist > max_dist)
                    max_dist = dist;
                }





    /* Notation: A(p,q) is the Zernike moment of order p
     * with repetition q.
     */
    /* Because we are building in scale invariance,
     * A(0,0) will always equal 1/pi, so don't we bother
     * calculating it.
     * Because we build in translation invariance,
     * A(1,1) will always equal 0, so we don;t both
     * calculating that either.
     * This is why we start with p=2.
     */
    index = 1;
    for (p=2; p <= max_order; p++)  /* order p */
        {
        for (q=((p%2)==0)?0:1; q <= p; q+=2)    /* repetition q */
            {
            /* We only calculate moments with positive q
             * since we will only be recording the magnitude
             * of each moment and
             * |A(p,q)| == |A(p,-q)|
             */
            A_real = 0;
            A_imaginary = 0;
            for (gaus=gaus_list->head; gaus != NULL; gaus=gaus->next)
                if (gaus->point_list != NULL)
                    for (pt=gaus->point_list->head; pt != NULL; pt=pt->next)
                        {
                        /* rho is normalised to be in the range [0, 1]
                         */
                        rho = sqrt( SQR(pt->x[1]-x_mean) + SQR(pt->x[2]-y_mean) ) / max_dist;
                        theta = atan2((double )(pt->x[2]-y_mean), (double )(pt->x[1]-x_mean));
                        /* Calculate R(p,q,rho)
                         */
                        for (s=0; s <= ((p-q)/2); s++)
                            {
                            numer = ((s%2)==0?1:-1) * factorial(p-s) * pow(rho,(double )(p-2*s));
                            denom = factorial(s)*factorial(((p+q)/2)-s)*factorial(((p-q)/2)-s);
                            R = numer / denom;

                            /* Z = R*exp(j*q*theta)
                             */
                            Z_real = R*cos(theta*(double )q);
                            Z_imaginary = R*sin(theta*(double )q);

                            /* A = sum_over_xy(f(x,y)*Z_cc)
                             * where Z_cc is the complex conjugate of Z.
                             * and f(x,y) if the characteristic function
                             * with values { 1: foreground point}
                             *             { 0: background point}
                             */
                            A_real += Z_real;
                            A_imaginary += (-Z_imaginary);
                            }
                        }   /* end of 'for (p=gaus->point_list->head;...' */
                A_real *= ((double )(p+1)) / M_PI;
                A_imaginary *= ((double )(p+1)) / M_PI;

                /* Record it
                 */
                point->x[index++] = sqrt(SQR(A_real) + SQR(A_imaginary))/zeroth;

            }   /* end of 'for (q=((p%2)==0)?0:1;...' */
        }   /* end of 'for (p=2; p <= max_order;...' */



    /* We found that the numerical values for
     * the zernike moments were too small.
     * This caused numerical rounding errors
     * in the function that finds eigenvectors
     * (called from the em_algorithm() when Learning a new class).
     * So we are making the values bigger.
     */
    for (index=1; index <= point->dim; index++)
        point->x[index] *= 100.0;


    return(point);


}   /* end of calculate_zernike_moments() */



double
factorial(int x)
{
    int i, f;

    for (i=1, f=1; i <= x; i++)
        f *=i;

    return((double )f);

}   /* end of factorial() */
