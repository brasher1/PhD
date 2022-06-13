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
#include <float.h>

#include "upwrite.h"


/* MACROS and #define
 */
#define DOT(a,b) (a[1]*b[1] + a[2]*b[2])
#define GAUS_USED   10      /* a value that is not TRUE or FALSE */



/* Function Prototypes
 */
/*void    measure_curve(GAUS_LIST *gaus_list, Window theWindow, GC theGC);
GAUS    *find_beginning_of_curve(GAUS *current_gaus, GAUS_LIST *gaus_list, Window theWindow, GC theGC);
void    trace_out_curve(GAUS *current_gaus, GAUS_LIST *gaus_list, Window theWindow, GC theGC);
void    first_order_stats(GAUS *current_gaus, GAUS_LIST *gaus_list, FILE *outfile);
void    second_order_stats(GAUS *current_gaus, GAUS_LIST *gaus_list, FILE *outfile);
GAUS    *find_next_gaus(GAUS *current_gaus, GAUS_LIST *gaus_list);
double  distance_to_gaus(GAUS *g1, GAUS *g2, double vec[][3]);
void    reverse_gaus_orientation(GAUS *gaus);
void    mean_relative_to_gaus(GAUS *g1, GAUS *g2, double *a, double *b);*/

void    measure_curve(GAUS_LIST *gaus_list);
GAUS    *find_beginning_of_curve(GAUS *current_gaus, GAUS_LIST *gaus_list);
void    trace_out_curve(GAUS *current_gaus, GAUS_LIST *gaus_list);
void    first_order_stats(GAUS *current_gaus, GAUS_LIST *gaus_list, FILE *outfile);
void    second_order_stats(GAUS *current_gaus, GAUS_LIST *gaus_list, FILE *outfile);
GAUS    *find_next_gaus(GAUS *current_gaus, GAUS_LIST *gaus_list);
double  distance_to_gaus(GAUS *g1, GAUS *g2, double vec[][3]);
void    reverse_gaus_orientation(GAUS *gaus);
void    mean_relative_to_gaus(GAUS *g1, GAUS *g2, double *a, double *b);

void
measure_curve(GAUS_LIST *gaus_list)
{
    GAUS    *gaus, *beginning_gaus;
/*    GAUS    *find_beginning_of_curve(GAUS *current_gaus, GAUS_LIST *gaus_list, Window theWindow, GC theGC);
    void    trace_out_curve(GAUS *current_gaus, GAUS_LIST *gaus_list, Window theWindow, GC theGC);*/
    GAUS    *find_beginning_of_curve(GAUS *current_gaus, GAUS_LIST *gaus_list);
    void    trace_out_curve(GAUS *current_gaus, GAUS_LIST *gaus_list);

    if (gaus_list == NULL)
        return;

    /* Set all flags to FALSE.
     */
    for (gaus=gaus_list->head; gaus != NULL; gaus=gaus->next)
        gaus->flag = FALSE;



    
    for (gaus=gaus_list->head; gaus != NULL; gaus=gaus->next)
        {
        if (gaus->flag == TRUE)
            continue;

//        XSetForeground(gDisplay, theGC, gRandomColors[1].pixel);
        beginning_gaus = find_beginning_of_curve(gaus, gaus_list);

//        XSetForeground(gDisplay, theGC, gRandomColors[2].pixel);
        trace_out_curve(beginning_gaus, gaus_list);
        }

}   /* end of measure_curve() */


/* Given (GAUS *)current_gaus, finds the first GAUS
 * in the same curve.
 * Oritentates the dominant eigenvector of the GAUS to
 * point back along the curve.
 * Only searches through GAUS with flag == FALSE.
 */
GAUS *
/*find_beginning_of_curve(GAUS *current_gaus, GAUS_LIST *gaus_list, Window theWindow, GC theGC)*/
find_beginning_of_curve(GAUS *current_gaus, GAUS_LIST *gaus_list)
{
    GAUS    *gaus, *next_gaus;
    double  min_dist, dist;
    double  theta;
    double  vec[3][3];
    void    reverse_gaus_orientation(GAUS *gaus);

//    void    draw_ellipse( Window theWindow, GC theGC, double x, double y, double r1, double r2, double theta);
    void    draw_ellipse( double x, double y, double r1, double r2, double theta);
    double  distance_to_gaus(GAUS *g1, GAUS *g2, double vec[][3]);
    void    orient_in_same_direction(GAUS *gaus1, GAUS *gaus2);
    /* orient_in_same_direction() is in the file chunk_ellipses.c
     */


    /* Error check arguements.
     */
    if ((current_gaus == NULL) || (gaus_list == NULL) || (gaus_list->head == NULL))
        return(NULL);



    do
        {
        if (current_gaus->flag == FALSE)
            current_gaus->flag = GAUS_USED;

        min_dist = 3.0;
        next_gaus = NULL;
        /* Set up a basis,
         * to be used when computing the distance
         * from current_gaus to gaus.
         */
        theta = atan2(current_gaus->eig_vec[2][1], current_gaus->eig_vec[1][1]);
        vec[1][1] = gResolutionRadius * cos(theta);
        vec[2][1] = gResolutionRadius * sin(theta);
        vec[1][2] = (gResolutionRadius/3.0) * cos(theta+M_PI_2);
        vec[2][2] = (gResolutionRadius/3.0) * sin(theta+M_PI_2);

        for (gaus=gaus_list->head; gaus != NULL; gaus=gaus->next)
            {
            /* Check that gaus->flag is FALSE.
             */
            if (gaus->flag == FALSE)
                {
                dist = distance_to_gaus(current_gaus, gaus, vec);
                if (dist < min_dist)
                    {
                    next_gaus = gaus;
                    min_dist = dist;
                    }
                }
            }

        if (next_gaus != NULL)
            {
            orient_in_same_direction(current_gaus, next_gaus);
/*            draw_ellipse(theWindow, theGC,
                next_gaus->mean[1], next_gaus->mean[2],
                sqrt(next_gaus->eig_val[1]), sqrt(next_gaus->eig_val[2]),
                atan2(next_gaus->eig_vec[2][1], next_gaus->eig_vec[1][1]) );*/
/*            draw_ellipse(next_gaus->mean[1], next_gaus->mean[2],
                sqrt(next_gaus->eig_val[1]), sqrt(next_gaus->eig_val[2]),
                atan2(next_gaus->eig_vec[2][1], next_gaus->eig_vec[1][1]) );*/
//            XFlush(gDisplay);
            current_gaus = next_gaus;
            }
        }
    while (next_gaus != NULL);

    reverse_gaus_orientation(current_gaus);


    /* Clean up.
     * Remove all trace of GAUS_USED.
     */
    for (gaus=gaus_list->head; gaus != NULL; gaus=gaus->next)
        if (gaus->flag == GAUS_USED)
            gaus->flag = FALSE;


    return(current_gaus);

}   /* end of find_beginning_of_curve() */


void
//trace_out_curve(GAUS *current_gaus, GAUS_LIST *gaus_list, Window theWindow, GC theGC)
trace_out_curve(GAUS *current_gaus, GAUS_LIST *gaus_list)
{
    GAUS    *next_gaus;
    FILE    *outfile;
    int     count;
    GAUS    *find_next_gaus(GAUS *current_gaus, GAUS_LIST *gaus_list);
//    void    draw_ellipse( Window theWindow, GC theGC, double x, double y, double r1, double r2, double theta);
    void    draw_ellipse( double x, double y, double r1, double r2, double theta);
    void    first_order_stats(GAUS *current_gaus, GAUS_LIST *gaus_list, FILE *outfile);
    void    second_order_stats(GAUS *current_gaus, GAUS_LIST *gaus_list, FILE *outfile);

    outfile = fopen("stat.dat", "w");
    if (outfile == NULL)
        {
        fprintf(stderr, "Could not open stat.dat for output\n");
        exit(1);
        }

    count = 0;

    while (current_gaus != NULL)
        {
/*        draw_ellipse(theWindow, theGC,
                current_gaus->mean[1], current_gaus->mean[2],
                sqrt(current_gaus->eig_val[1]), sqrt(current_gaus->eig_val[2]),
                atan2(current_gaus->eig_vec[2][1], current_gaus->eig_vec[1][1]) );*/
/*        draw_ellipse(current_gaus->mean[1], current_gaus->mean[2],
                sqrt(current_gaus->eig_val[1]), sqrt(current_gaus->eig_val[2]),
                atan2(current_gaus->eig_vec[2][1], current_gaus->eig_vec[1][1]) );*/
//        XFlush(gDisplay);

        /* Delay
         */
        {
        int t;
        for (t=0; t < 10000000; t++)
            ;
        }

        current_gaus->flag = TRUE;

        fprintf(outfile, "%d ", count);
        first_order_stats(current_gaus, gaus_list, outfile);
        second_order_stats(current_gaus, gaus_list, outfile);
        fprintf(outfile, "\n");

        next_gaus = find_next_gaus(current_gaus, gaus_list);

        current_gaus = next_gaus;
        count++;
        }


    fclose(outfile);

}   /* end of trace_out_curve() */


void
first_order_stats(GAUS *current_gaus, GAUS_LIST *gaus_list, FILE *outfile)
{
    double  dx, dy, dtheta;
    GAUS    *next_gaus;
    GAUS    *find_next_gaus(GAUS *current_gaus, GAUS_LIST *gaus_list);
    void    mean_relative_to_gaus(GAUS *g1, GAUS *g2, double *a, double *b);


    next_gaus = find_next_gaus(current_gaus, gaus_list);

    if (next_gaus != NULL)
        {
        mean_relative_to_gaus(next_gaus, current_gaus, &dx, &dy);
        dtheta = atan2(next_gaus->eig_vec[2][1], next_gaus->eig_vec[1][1]) -
            atan2(current_gaus->eig_vec[2][1], current_gaus->eig_vec[1][1]);
        fprintf(outfile, "%f %f %f ", (float )dx, (float )dy, (float )dtheta);
        }
    else
        fprintf(outfile, "0 0 0 ");


}   /* end of first_order_stats() */



void
second_order_stats(GAUS *current_gaus, GAUS_LIST *gaus_list, FILE *outfile)
{
    double  dx1, dy1, dtheta1, dx2, dy2, dtheta2;
    GAUS    *next1_gaus, *next2_gaus;
    GAUS    *find_next_gaus(GAUS *current_gaus, GAUS_LIST *gaus_list);
    void    mean_relative_to_gaus(GAUS *g1, GAUS *g2, double *a, double *b);


    /* Initialisation.
     */
    next1_gaus = NULL;
    next2_gaus = NULL;

    next1_gaus = find_next_gaus(current_gaus, gaus_list);
    if (next1_gaus != NULL)
        {
        next1_gaus->flag = TRUE;
        next2_gaus = find_next_gaus(next1_gaus, gaus_list);
        next1_gaus->flag = FALSE;
        }

    if ((next1_gaus != NULL) && (next2_gaus != NULL))
        {
        mean_relative_to_gaus(next1_gaus, current_gaus, &dx1, &dy1);
        dtheta1 = atan2(next1_gaus->eig_vec[2][1], next1_gaus->eig_vec[1][1]) -
            atan2(current_gaus->eig_vec[2][1], current_gaus->eig_vec[1][1]);

        mean_relative_to_gaus(next2_gaus, next1_gaus, &dx2, &dy2);
        dtheta2 = atan2(next2_gaus->eig_vec[2][1], next2_gaus->eig_vec[1][1]) -
            atan2(next1_gaus->eig_vec[2][1], next1_gaus->eig_vec[1][1]);

        fprintf(outfile, "%f %f %f ", (float )(dx2-dx1),
            (float )(dy2-dy1), (float)(dtheta2-dtheta1));
        }
    else
        fprintf(outfile, "0 0 0 ");


}   /* end of second_order_stats() */



/* Find the next GAUS along the line segment.
 * Orient its dominant eigenvector to be in the same
 * direction as that of (GAUS *)current_gaus.
 *
 * Only search through those GAUS with flag set to FALSE.
 * Assumes that the (GAUS *)eigenvectors of current_gaus
 * have been set.
 */
GAUS *
find_next_gaus(GAUS *current_gaus, GAUS_LIST *gaus_list)
{
    GAUS    *gaus, *next_gaus;
    double  min_dist, dist;
    double  theta;
    double  vec[3][3];

    double  distance_to_gaus(GAUS *g1, GAUS *g2, double vec[][3]);
    void    orient_in_same_direction(GAUS *gaus1, GAUS *gaus2);
    /* orient_in_same_direction() is in the file chunk_ellipses.c
     */


    min_dist = 3.0;
    next_gaus = NULL;

    /* Set up a basis,
     * to be used when computing the distance
     * from current_gaus to gaus.
     */
    theta = atan2(current_gaus->eig_vec[2][1], current_gaus->eig_vec[1][1]);
    vec[1][1] = gResolutionRadius * cos(theta);
    vec[2][1] = gResolutionRadius * sin(theta);

    vec[1][2] = (gResolutionRadius/3.0) * cos(theta+M_PI_2);
    vec[2][2] = (gResolutionRadius/3.0) * sin(theta+M_PI_2);

    for (gaus=gaus_list->head; gaus != NULL; gaus=gaus->next)
        {
        /* Check that gaus->flag is FALSE.
         */
        if (gaus->flag == FALSE)
            {
            dist = distance_to_gaus(current_gaus, gaus, vec);
            if (dist < min_dist)
                {
                next_gaus = gaus;
                min_dist = dist;
                }
            }
        }

    if (next_gaus != NULL)
        {
        orient_in_same_direction(current_gaus, next_gaus);
        }

    return(next_gaus);

}   /* end of find_next_gaus() */



/* Find distance from mean of (GAUS *)g1,
 * to mean of (GAUS *)g2, using the basis
 * specified in vec[][].
 *
 * Assumes vec is a 2-D basis, and g1, g2 are
 * 2-D GAUS
 */
double
distance_to_gaus(GAUS *g1, GAUS *g2, double vec[][3])
{
    double  dist, a, b;
    double  point[3];


    point[1] = g2->mean[1] - g1->mean[1];
    point[2] = g2->mean[2] - g1->mean[2];

    dist = 0.0;
    a = (point[1]*vec[1][1] + point[2]*vec[2][1]) / SQR(gResolutionRadius);
    b = (point[1]*vec[1][2] + point[2]*vec[2][2]) / SQR(gResolutionRadius/3.0);

    /* Is g2 in the positive direction from g1.
     */
    if (a > 0)
        dist = sqrt(SQR(a) + SQR(b));
    else
        dist = DBL_MAX;


    return(dist);

}   /* end of distance_to_gaus() */


void
reverse_gaus_orientation(GAUS *gaus)
{
    int i, j;

    for (i=1; i <= gaus->dim; i++)
        for (j=1; j <= gaus->dim; j++)
            gaus->eig_vec[j][i] = -gaus->eig_vec[j][i];

}   /* end of reverse_gaus_orientation() */



/* Returns the position of the mean of (GAUS *)g1
 * relative to the eigenbasis of (GAUS *)g2.
 *
 * The co-ords of the mean of g1 relative to the eigenbasis g2
 * are set in (*a, *b).
 */
void
mean_relative_to_gaus(GAUS *g1, GAUS *g2, double *a, double *b)
{
    double  theta;
    double  point[3], vec[3][3];


    point[1] = g1->mean[1] - g2->mean[1];
    point[2] = g1->mean[2] - g2->mean[2];

    /* Set up a basis,
     * to be used when computing the distance
     * from current_gaus to gaus.
     */
    theta = atan2(g2->eig_vec[2][1], g2->eig_vec[1][1]);
    vec[1][1] = gResolutionRadius * cos(theta);
    vec[2][1] = gResolutionRadius * sin(theta);
    vec[1][2] = (gResolutionRadius/3.0) * cos(theta+M_PI_2);
    vec[2][2] = (gResolutionRadius/3.0) * sin(theta+M_PI_2);


    *a = (point[1]*vec[1][1] + point[2]*vec[2][1]) / SQR(gResolutionRadius);
    *b = (point[1]*vec[1][2] + point[2]*vec[2][2]) / SQR(gResolutionRadius/3.0);

}   /* end of mean_relative_to_gaus() */
