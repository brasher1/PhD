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
//#include <values.h>     /* MAXDOUBLE */

#include "upwrite.h"
#include "globals.h"

#define MIN_STD_DEV_FOR_NUM_POINTS  3.0

/* When modelling a neighbourhood of gaussians,
 * is it modelled as a line, or a circle.
 */
typedef enum {line, circle} MODEL_TYPE;



/* Function Prototypes
 */
int print_gaus(char *name, GAUS_LIST *gaus_list);

/*GAUS_LIST   *chunk_ellipses(GAUS_LIST *gaus_list, Window theWindow,
                            GC theGC, XImage *aXImage);*/
GAUS_LIST   *chunk_ellipses(GAUS_LIST *gaus_list);

/*GAUS_LIST   *find_chunk_based_on_gaus(GAUS_LIST *gaus_list,
                            GAUS_LIST *chunk,
                            Window theWindow,
                            GC theGC);*/
GAUS_LIST   *find_chunk_based_on_gaus(GAUS_LIST *gaus_list,
                            GAUS_LIST *chunk);
GAUS_LIST   *find_gaus_within_neighbourhood(GAUS *centre, GAUS_LIST *gaus_list,
                            double neighbourhood_size);
/*BOOLEAN     model_end_of_chunk(GAUS_LIST *chunk, int order_of_model,
                            Window theWindow, GC theGC,
                            double *x_ret, double *y_ret,
                            double *theta_ret,
                            double *x_centre_ret,
                            double *y_centre_ret,
                            double *radius_ret,
                            double *num_points_mean,
                            double *num_points_std_dev,
                            MODEL_TYPE *model_type_ret);*/
BOOLEAN     model_end_of_chunk(GAUS_LIST *chunk, int order_of_model,
                            double *x_ret, double *y_ret,
                            double *theta_ret,
                            double *x_centre_ret,
                            double *y_centre_ret,
                            double *radius_ret,
                            double *num_points_mean,
                            double *num_points_std_dev,
                            MODEL_TYPE *model_type_ret);
BOOLEAN     search_for_next_gaus(GAUS_LIST *neighbours, GAUS_LIST *chunk,
                            MODEL_TYPE model_type,
                            double x, double y, double theta,
                            double x_centre, double y_centre, double radius,
                            double num_points_mean,
                            double num_points_std_dev,
                            GAUS **gaus_ret);
void        orient_in_same_direction(GAUS *gaus, GAUS *gaus2);
/*GAUS_LIST   *look_for_occluded_GAUS(GAUS_LIST *gaus_list, GAUS_LIST *chunk,
                            Window theWindow, GC theGC,
                            BOOLEAN *status);*/
GAUS_LIST   *look_for_occluded_GAUS(GAUS_LIST *gaus_list, GAUS_LIST *chunk,
                            BOOLEAN *status);
/*GAUS_LIST   *find_chunk_based_on_gaus_in_other_direction(GAUS_LIST *gaus_list,
                            GAUS_LIST *chunk,
                            Window theWindow, GC theGC);*/
GAUS_LIST   *find_chunk_based_on_gaus_in_other_direction(GAUS_LIST *gaus_list,
                            GAUS_LIST *chunk);
/*BOOLEAN     model_end_of_chunk_in_other_direction(GAUS_LIST *chunk,
                            int order_of_model,
                            Window theWindow, GC theGC,
                            double *x_ret, double *y_ret, double *theta_ret,
                            double *x_centre_ret, double *y_centre_ret,
                            double *radius_ret,
                            double *num_points_mean,
                            double *num_points_std_dev,
                            MODEL_TYPE *model_type_ret);*/
BOOLEAN     model_end_of_chunk_in_other_direction(GAUS_LIST *chunk,
                            int order_of_model,
                            double *x_ret, double *y_ret, double *theta_ret,
                            double *x_centre_ret, double *y_centre_ret,
                            double *radius_ret,
                            double *num_points_mean,
                            double *num_points_std_dev,
                            MODEL_TYPE *model_type_ret);
BOOLEAN     search_for_next_gaus_in_other_direction(GAUS_LIST *neighbours,
                            GAUS_LIST *chunk,
                            MODEL_TYPE model_type,
                            double x, double y, double theta,
                            double x_centre, double y_centre, double radius,
                            double num_points_mean,
                            double num_points_std_dev,
                            GAUS **gaus_ret);
/*GAUS_LIST   *look_for_occluded_GAUS_in_other_direction(GAUS_LIST *gaus_list,
                            GAUS_LIST *chunk,
                            Window theWindow, GC theGC,
                            BOOLEAN *status);*/
GAUS_LIST   *look_for_occluded_GAUS_in_other_direction(GAUS_LIST *gaus_list,
                            GAUS_LIST *chunk,
                            BOOLEAN *status);





GAUS_LIST *
/*chunk_ellipses(GAUS_LIST *gaus_list, Window theWindow, GC theGC, XImage *aXImage)*/
chunk_ellipses(GAUS_LIST *gaus_list)
{
    GAUS        *initial, *gaus;
    GAUS_LIST   *chunk, *chunk_list;
    GAUS        *copy_gaus(GAUS *g);
    GAUS_LIST   *put_in_linked_list_of_gaus(GAUS_LIST *gaus_list, GAUS *g);
    void        free_linked_list_of_gaus(GAUS_LIST *gaus_list);
    GAUS_LIST   *put_in_linked_list_of_linked_list_of_gaus(
                                            GAUS_LIST *head_list,
                                            GAUS_LIST *list);



    /* No GAUS to be chunked.
     */
    if (gaus_list == NULL)
        return((GAUS_LIST *)NULL);

    /* Flag all gaussians as not used in a chunk yet.
     */
    for (gaus=gaus_list->head; gaus != NULL; gaus=gaus->next)
        gaus->flag = FALSE;


    chunk_list =(GAUS_LIST *)NULL;

    for (initial=gaus_list->head; initial != NULL; initial=initial->next)
        {
        if (initial->flag == TRUE)
            {
            /* If already used in a chunk. */
            continue;
            }

        chunk = (GAUS_LIST *)NULL;

        /* Start the chunk off */
        chunk = put_in_linked_list_of_gaus(chunk, copy_gaus(initial));
        initial->flag = TRUE;

        chunk = find_chunk_based_on_gaus(gaus_list, chunk);

        chunk = find_chunk_based_on_gaus_in_other_direction(gaus_list,
                                                chunk);


        /* Chunk too small - discard it */
        if (chunk->num_elements < 5)
            {
            for (gaus=chunk->head; gaus != NULL; gaus=gaus->next)
                gaus->original->flag = FALSE;
            free_linked_list_of_gaus(chunk);
            chunk = (GAUS_LIST *)NULL;
            }



        if (chunk != (GAUS_LIST *)NULL)
            {
            chunk_list = put_in_linked_list_of_linked_list_of_gaus(
                                                chunk_list, chunk);
            }

        }
//	fprint_gaus_list("CHUNK_ELLIPSES", chunk_list);

    return( chunk_list );

}                   /* end of chunk_ellipses() */



GAUS_LIST *
/*find_chunk_based_on_gaus(GAUS_LIST *gaus_list, GAUS_LIST *chunk,
                                                Window theWindow, GC theGC)*/
find_chunk_based_on_gaus(GAUS_LIST *gaus_list, GAUS_LIST *chunk)
{
    BOOLEAN     ret_val;
    int         order_of_model;
    GAUS        *gaus_ret;
    GAUS_LIST   *neighbours;
    double      x, y, theta;
    double      x_centre, y_centre, radius;
    double      num_points_mean, num_points_std_dev;
    MODEL_TYPE  model_type;
    void        orient_in_same_direction(GAUS *gaus, GAUS *gaus2);
    GAUS_LIST   *put_in_linked_list_of_gaus(GAUS_LIST *gaus_list, GAUS *g);
    GAUS        *copy_gaus(GAUS *g);
    void        free_linked_list_of_gaus(GAUS_LIST *gaus_list);
#ifdef VERBOSE_INSTRUMENTATION
	FILE		*output;
    FILE		*output2;
	FILE		*output3;

	output = fopen("find_chunk_based_on_gaus.txt","a+");
	output2 = fopen("found_predicted_gaus.txt","a+");
	output3 = fopen("find_occluded_gaus.txt","a+");
#endif //find_chunk_based_on_gaus
    do
        {
        /* Find gaus which may be chunked
         * This reduces our search space.
         */
#ifdef DEBUG
{
XSetForeground(gDisplay, theGC, gRandomColors[1].pixel);
XFillArc(gDisplay, theWindow, theGC,
                            (int )(chunk->tail->mean[1]-5),
                            (int )(chunk->tail->mean[2]-5),
                            10, 10, 0, 23040);
XFlush(gDisplay);
}
#endif
        ret_val = FALSE;
        neighbours = find_gaus_within_neighbourhood(chunk->tail, gaus_list,
                                                    NEIGHBOURHOOD);
		
        /* Are there other GAUS nearby? */
        if (neighbours != (GAUS_LIST *)NULL)
            {
#ifdef VERBOSE_INSTRUMENTATION
			fprintf(output,"neighbours != NULL\n");
#endif //VERBOSE_INSTRUMENTATION
            order_of_model = 4;
            do
                {
/*                ret_val = model_end_of_chunk(chunk, order_of_model,
                                    theWindow, theGC,
                                    &x, &y, &theta,
                                    &x_centre, &y_centre, &radius,
                                    &num_points_mean, &num_points_std_dev,
                                    &model_type);*/
                ret_val = model_end_of_chunk(chunk, order_of_model,
                                    &x, &y, &theta,
                                    &x_centre, &y_centre, &radius,
                                    &num_points_mean, &num_points_std_dev,
                                    &model_type);

//				if(ret_val)
//					fprintf(output,"model_end_of_chunk is TRUE\n");
//				else
//					fprintf(output,"model_end_of_chunk is FALSE\n");

#ifdef DEBUG
{
int dx, dy;
int x1, y1, x2, y2;

if (ret_val == TRUE)
{
    if (model_type == line)
        {
        XSetForeground(gDisplay, theGC, gRandomColors[2].pixel);
        x1 = (int )(x - 100*cos(theta));
        y1 = (int )(y - 100*sin(theta));
        x2 = (int )(x + 100*cos(theta));
        y2 = (int )(y + 100*sin(theta));
        XDrawLine(gDisplay, theWindow, theGC, x1, y1, x2, y2);
        }
    else if (model_type == circle)
        {
        XSetForeground(gDisplay, theGC, gRandomColors[3].pixel);
        x1 = (int )(x_centre - radius);
        y1 = (int )(y_centre - radius);
        dx = (int )(2*radius);
        dy = (int )(2*radius);
        XDrawArc(gDisplay, theWindow, theGC, x1, y1, dx, dy, 0, 23040);
        }
    XFlush(gDisplay);
}
else
    printf("%d Model Failed\n", order_of_model);
sleep(1);
}
#endif
                if (ret_val == TRUE)
                    ret_val = search_for_next_gaus(neighbours, chunk,
                                        model_type,
                                        x, y, theta,
                                        x_centre, y_centre, radius,
                                        num_points_mean, num_points_std_dev,
                                        &gaus_ret);
				   
                if (ret_val == TRUE)
                    {
#ifdef VERBOSE_INSTRUMENTATION
						fprintf(output2,"Found next gaus - add it to chunk\n");
#endif //VERBOSE_INSTRUMENTATION
                    /* Found the predicted GAUS.
                     * Add it to the chunk.
                     */
                    chunk = put_in_linked_list_of_gaus(chunk,
                                        copy_gaus(gaus_ret->original));
                    orient_in_same_direction(chunk->tail->prev, chunk->tail);

                    /* Flag the GAUS as having been used in a chunk */
                    gaus_ret->original->flag = TRUE;
                    }
                else if (ret_val == FALSE)
				{
//					fprintf(output2,"DID NOT find next gaus\n");
                    order_of_model--;
				}
                if (order_of_model == 0)
				{
//					fprintf(output2,"Breaking out of do-while\n");
                    break;          /* out of do-while loop */
				}
                }
            while (ret_val == FALSE);

            /* Free memory */
            free_linked_list_of_gaus(neighbours);

            }               /* end of 'if (neighbours != (GAUS_LIST *)NULL)' */



        /* If we couldn't find the next GAUS.
         * Try looking a little further afield.
         */
        if (ret_val == FALSE)
            {
/*            chunk = look_for_occluded_GAUS(gaus_list, chunk,
                                    theWindow, theGC,
                                    &ret_val);*/
            chunk = look_for_occluded_GAUS(gaus_list, chunk,
                                    &ret_val);
//			fprintf(output3,"Looked for Occluded GAUS\n");
            }

        }
    while (ret_val == TRUE);
#ifdef VERBOSE_INSTRUMENTATION
	fclose(output);
	fclose(output2);
	fclose(output3);
#endif //VERBOSE_INSTRUMENTATION
    return(chunk);

}                   /* end of find_chunk_based_on_gaus() */



/* Return a linked list of GAUS within some neighbourhood
 * of (GAUS *)centre.
 * Only gaus with (gaus->flag==FALSE) are considered.
 * The original linked list (GAUS_LIST *)gaus_list
 * is not modified.
 * The GAUS in the returned linked list are copied
 * from(GAUS_LIST *)gaus_list using copy_gaus().
 */
GAUS_LIST *
find_gaus_within_neighbourhood(GAUS *centre, GAUS_LIST *gaus_list,
                                                double neighbourhood_size)
{
    GAUS_LIST   *neighbours;
    GAUS        *gaus, *gaus2, *g;
    double      dist;
    GAUS        *copy_gaus(GAUS *g);
    GAUS_LIST   *put_in_linked_list_of_gaus(GAUS_LIST *gaus_list, GAUS *g);


    neighbours = (GAUS_LIST *)NULL;

    /* Find all ellipses within some neighbourhood.
     */
    for (gaus=gaus_list->head; gaus != NULL; gaus=gaus->next)
        if (gaus->flag == FALSE)
            {
            dist = sqrt(SQR(centre->mean[1]-gaus->mean[1]) +
                        SQR(centre->mean[2]-gaus->mean[2]));


            if (dist < neighbourhood_size)
                {
                /* Make a copy of all GAUS that are
                 * within the neighbourhood
                 */
                gaus2 = copy_gaus(gaus);    /* gaus2 is a copy of a GAUS */
                gaus2->value = dist;        /* that is in the neighbourhood */

                /* Store these in the linked list: neighbours
                 * Store them in order from closest to furtherest away.
                 */
                if (neighbours == (GAUS_LIST *)NULL)
                    {
                    /* If linked list empty,
                     * gaus2 is put at head of list.
                     */
                    neighbours = put_in_linked_list_of_gaus(neighbours, gaus2);
                    }
                else
                    {
                    /* g is just a temporary variable for searching
                     * through the list 'neighbours'.
                     */
                    for (g=neighbours->head; g != NULL; g=g->next)
                        if (gaus2->value < g->value)
                            break;
                    if (g == neighbours->head)  /* Insert at head of list */
                        {
                        gaus2->next = neighbours->head;
                        gaus2->prev = (GAUS *)NULL;
                        (neighbours->head)->prev = gaus2;
                        neighbours->head = gaus2;
                        (neighbours->num_elements)++;
                        }
                    else if (g == NULL)         /* Insert at tail of list */
                        {
                        (neighbours->tail)->next = gaus2;
                        gaus2->next = (GAUS *)NULL;
                        gaus2->prev = neighbours->tail;
                        neighbours->tail = gaus2;
                        (neighbours->num_elements)++;
                        }
                    else                /* Insert somewhere in the list */
                        {
                        (g->prev)->next = gaus2;
                        gaus2->prev = g->prev;
                        g->prev = gaus2;
                        gaus2->next = g;
                        (neighbours->num_elements)++;

                        }
                    }   /* finished inserting gaus2 in the list 'neighbours' */
                }   /* end of 'if (dist < NEIGHBOURHOOD)' */
            }   /* end of 'if (gaus->flag == FALSE)' */


        return(neighbours);

}               /* end of find_gaus_within_neighbourhood() */



/* Model the last few GAUS in the chunk.
 * The number of gaus is specified by 'order_of_model'.
 * We will model them as either a line, or a circle.
 * If we model them as a circle, the parameters of the
 * circle are recorded in (*x_centre_ret, *y_centre_ret, *radius_ret).
 * It we model them as a line, the parameters are recorded
 * in (*x_ret, *y_ret, *theta_ret).
 *
 * Return TRUE if we were able to model the GAUS.
 * Return FALSE if were not able to model the GAUS.
 */
BOOLEAN
/*model_end_of_chunk(GAUS_LIST *chunk, int order_of_model,
                        Window theWindow, GC theGC,
                        double *x_ret, double *y_ret, double *theta_ret,
                        double *x_centre_ret, double *y_centre_ret,
                        double *radius_ret,
                        double *num_points_mean, double *num_points_std_dev,
                        MODEL_TYPE *model_type_ret)*/
model_end_of_chunk(GAUS_LIST *chunk, int order_of_model,
                        double *x_ret, double *y_ret, double *theta_ret,
                        double *x_centre_ret, double *y_centre_ret,
                        double *radius_ret,
                        double *num_points_mean, double *num_points_std_dev,
                        MODEL_TYPE *model_type_ret)
{
    GAUS    *last, *gaus_from, *gaus_to, *gaus;
    double  theta, theta_to, theta_from, theta_normal;
    double  dtheta, dt, curv;
    int     i;

    if (order_of_model < 1)
        return(FALSE);

    /* There aren't enough GAUS in the chunk */
    if (order_of_model > chunk->num_elements)
        return(FALSE);


    if (order_of_model == 1)
        {
        /* Use only last GAUS in chunk to form estimate.
         * Estimated GAUS will have same theta as last GAUS in chunk.
         */
        last = chunk->tail;

        *x_ret = last->mean[1];
        *y_ret = last->mean[2];
        *theta_ret = atan2(last->eig_vec[2][1], last->eig_vec[1][1]);
        *num_points_mean = last->num_points;
        *num_points_std_dev = MIN_STD_DEV_FOR_NUM_POINTS;
        *model_type_ret = line;
        return(TRUE);
        }
    else
        {
        gaus_to = chunk->tail;
        gaus_from = chunk->tail->prev;

        /* Estimate curvature - curvature = dtheta/dt
         * First find dt (distance along curve)
         */
        dt = 0;
        for (i=1; i < order_of_model; i++)
            {
            dt += sqrt( SQR(gaus_to->mean[1] - gaus_from->mean[1]) +
                        SQR(gaus_to->mean[2] - gaus_from->mean[2]) );

            /* Don't do this the last time through the loop. */
            if (i < (order_of_model-1))
                {
                gaus_to = gaus_from;
                gaus_from = gaus_from->prev;
                }
            }

        /* Find difference in angle */
        theta_to = atan2(chunk->tail->eig_vec[2][1],
                                            chunk->tail->eig_vec[1][1]);
        theta_from = atan2(gaus_from->eig_vec[2][1],
                                            gaus_from->eig_vec[1][1]);
        dtheta = theta_to - theta_from;

        /* Make sure dtheta is in the range [-M_PI_2, M_PI_2].
         */
        if ((dtheta >= -2.0*M_PI) && (dtheta < -1.5*M_PI))
            {
                                                    /* [-2PI, -1.5PI) */
            dtheta += 2.0*M_PI;
            }
        else if ((dtheta >= -1.5*M_PI) && (dtheta < -M_PI_2))
            {
                                                    /* [-1.5PI, -.5PI) */
            dtheta += M_PI;
            }
        else if ((dtheta > M_PI_2) && (dtheta <= 1.5*M_PI))
            {
                                                    /* (.5PI, 1.5PI] */
            dtheta -= M_PI;
            }
        if ((dtheta > 1.5*M_PI) && (dtheta <= 2.0*M_PI))
            {
                                                    /* (1.5PI, 2PI] */
            dtheta -= 2.0*M_PI;
            }

        /* Estimate curvature */
        curv = dtheta / dt;


        /* Compute average number of points in each GAUS,
         * and the standard deviation.
         */
        *num_points_mean = 0;
        for (i=0, gaus=chunk->tail; i < order_of_model; i++, gaus=gaus->prev)
            {
            *num_points_mean += gaus->num_points;
            }
        *num_points_mean /= (double )order_of_model;
        *num_points_std_dev = 0;
        for (i=0, gaus=chunk->tail; i < order_of_model; i++, gaus=gaus->prev)
            {
            *num_points_std_dev += SQR(gaus->num_points - *num_points_mean);
            }
        *num_points_std_dev /= (double )(order_of_model-1);
        *num_points_std_dev = sqrt(*num_points_std_dev);
        if (*num_points_std_dev < MIN_STD_DEV_FOR_NUM_POINTS)
            *num_points_std_dev = MIN_STD_DEV_FOR_NUM_POINTS;


        /* Zero curvature - model GAUS as a straight line. */
        if (ABS(curv) < A_VERY_SMALL_POSITIVE_VALUE)
            {
            last = chunk->tail;

            *x_ret = last->mean[1];
            *y_ret = last->mean[2];
            *theta_ret = atan2(last->eig_vec[2][1], last->eig_vec[1][1]);
            *model_type_ret = line;
            return(TRUE);
            }
        else
            {
            /* Model GAUS as lying on a circle. */

            /* Find Radius */
            *radius_ret = 1.0 / ABS(curv);

            /* Find Centre.
             * The last GAUS in the chunk lies on the circle,
             * and is orientated normal to it.
             * Hence the line perpendicular to it passes through
             * the centre.
             * We know the radius, so just move along this line by
             * a distance equal to the radius, to find the circle centre.
             */
            last = chunk->tail;
            theta = atan2(last->eig_vec[2][1], last->eig_vec[1][1]);
            if (curv > 0)
                theta_normal = theta + M_PI_2;
            else
                theta_normal = theta - M_PI_2;
            *x_centre_ret = last->mean[1] + (*radius_ret) * cos(theta_normal);
            *y_centre_ret = last->mean[2] + (*radius_ret) * sin(theta_normal);
            *model_type_ret = circle;


            return(TRUE);
            }
        }

}               /* end of model_end_of_chunk() */



/* Search for which of the GAUS in neighbours best fits the
 * model of the end of the chunk.
 * The end of the chunk is either modelled as a straight line
 * (when model_type = line), whose parameters are specified
 * in (x, y, theta),
 * or as a circle (when model_type = circle), whose parameters
 * are specified in (x_centre, y_centre, radius).
 * If an appropriate GAUS is found, *gaus_ret is set to point
 * to it and the function returns TRUE.
 * Otherwise the function return FALSE.
 */
BOOLEAN
search_for_next_gaus(GAUS_LIST *neighbours, GAUS_LIST *chunk,
                            MODEL_TYPE model_type,
                            double x, double y, double theta,
                            double x_centre, double y_centre, double radius,
                            double num_points_mean, double num_points_std_dev,
                            GAUS **gaus_ret)
{
    /* PARAMETERS */
    double  tolerance_translation = gResolutionRadius/3;
    double  tolerance_angle = M_PI/12;
    double  tolerance_num_points = 3;

    GAUS    *gaus, *last;
    double  t;
    double  theta1, theta_tangent;
    double  dist, diff;
    double  x_closest, y_closest;
    double  mahal_dist, min_mahal_dist;
    double  a, b, val;


    if (neighbours == (GAUS_LIST *)NULL)
        {
        return(FALSE);
        }


    if (model_type == line)
        {
        min_mahal_dist = DBL_MAX;
        for (gaus=neighbours->head; gaus != NULL; gaus=gaus->next)
            {

            /* Test if number of points in gaus are as expected.
             */
            val = (gaus->num_points - num_points_mean) / num_points_std_dev;
            if (ABS(val) > tolerance_num_points)
                continue;


            /* We are searching for the GAUS that is closest to the
             * tail of the chunk, and which conforms to the model
             * (to within some tolerance).
             * Use the tail GAUS of the chunk to define a metric,
             * with one basis vector in the direction of the dominant
             * eigenvector of the tail gaus, with length equal to
             * the square-root of the dominant eigenvalue,
             * and the second basis vector perpendicular to this,
             * with length equal to the square-root of the smaller eigenvalue.
             */
            last = chunk->tail;
            a = (( (gaus->mean[1]-last->mean[1]) *last->eig_vec[1][1]) +
                    ( (gaus->mean[2]-last->mean[2]) *last->eig_vec[2][1])) /
                                sqrt(last->eig_val[1]);
            b = (( (gaus->mean[1]-last->mean[1]) *last->eig_vec[1][2]) +
                    ( (gaus->mean[2]-last->mean[2]) *last->eig_vec[2][2])) /
                                sqrt(last->eig_val[2]);

            /* Wrong direction */
            if (a <= 0)
                continue;

            mahal_dist = sqrt( SQR(a) + SQR(b) );
            if (mahal_dist > min_mahal_dist)
                continue;

            /* Test if GAUS lies near the line modelling the
             * end of the chunk.
             * First test how far it is from the line
             * (perpendicular distance).
             * We have a parametric representation of the line as:
             * { (x,y) : (x, y) + t(cos(theta), sin(theta)) }
             * for t a real number.
             */
            t = (gaus->mean[1]-x)*cos(theta) + (gaus->mean[2]-y)*sin(theta);
            x_closest = x + t*cos(theta);
            y_closest = y + t*sin(theta);
            dist = sqrt( SQR(gaus->mean[1] - x_closest) +
                            SQR(gaus->mean[2] - y_closest) );
            if (dist > tolerance_translation)
                continue;

            /* acos() gives angle difference in the range [0..M_PI].
             */
            val = gaus->eig_vec[1][1]*cos(theta) +
                            gaus->eig_vec[2][1]*sin(theta);
            if (ABS(val) > 0.9999)
                diff = 0;
            else
                diff = acos(val);

            /* The angle difference must be in the range [0, M_PI/2].
             * When the angle is larger than this, we consider
             * the angle difference to the negative of the eigenvector
             * (which is also a valid eigenvector).
             * So an angle difference of M_PI is mapped to 0.
             */
            if (diff > M_PI_2)
                diff = M_PI - diff;
            if ( ABS(diff) > tolerance_angle )
                {
                continue;
                }

            /* If we got this far, then we have found
             * a GAUS which agrees witht he model (to within tolerance)
             */
            *gaus_ret = gaus;
            return(TRUE);

            }                   /* end of 'for (gaus=neighbours->head...' */

        }                   /* end of 'if (model_type == line)' */
    else if (model_type == circle)
        {
        min_mahal_dist = DBL_MAX;
        for (gaus=neighbours->head; gaus != NULL; gaus=gaus->next)
            {

            /* Test if number of points in gaus are as expected.
             */
            val = (gaus->num_points - num_points_mean) / num_points_std_dev;
            if (ABS(val) > tolerance_num_points)
                continue;


            /* We are searching for the GAUS that is closest to the
             * tail of the chunk, and which conforms to the model
             * (to within some tolerance).
             * Use the tail GAUS of the chunk to define a metric,
             * with one basis vector in the direction of the dominant
             * eigenvector of the tail gaus, with length equal to
             * the square-root of the dominant eigenvalue,
             * and the second basis vector perpendicular to this,
             * with length equal to the square-root of the smaller eigenvalue.
             */
            last = chunk->tail;
            a = (( (gaus->mean[1]-last->mean[1]) *last->eig_vec[1][1]) +
                    ( (gaus->mean[2]-last->mean[2]) *last->eig_vec[2][1])) /
                                sqrt(last->eig_val[1]);
            b = (( (gaus->mean[1]-last->mean[1]) *last->eig_vec[1][2]) +
                    ( (gaus->mean[2]-last->mean[2]) *last->eig_vec[2][2])) /
                                sqrt(last->eig_val[2]);

            /* Wrong direction */
            if (a <= 0)
                continue;

            mahal_dist = sqrt( SQR(a) + SQR(b) );
            if (mahal_dist > min_mahal_dist)
                continue;


            /* Test if GAUS lies near circle.
             * First test if its distance from the centre is approx.
             * equal to the radius.
             */
            dist = sqrt( SQR(gaus->mean[1] - x_centre) +
                            SQR(gaus->mean[2] - y_centre) );
            if (ABS(dist - radius) > tolerance_translation)
                continue;

            /* Next test if the orientation of the GAUS is
             * approx. the orientation of the nearest tangent
             * to the circle.
             * First find angle of line from GAUS to circle centre.
             * Angle of tangent will be perpendicular to this.
             */
            theta1 = atan2(gaus->mean[2]-y_centre, gaus->mean[1]-x_centre);
            theta_tangent = theta1+M_PI_2;
            /* acos() gives angle difference in the range [0..M_PI].
             */
            val = gaus->eig_vec[1][1]*cos(theta_tangent) +
                            gaus->eig_vec[2][1]*sin(theta_tangent);
            if (ABS(val) > 0.9999)
                diff = 0;
            else
                diff = acos(val);

            /* The angle difference must be in the range [0, M_PI/2].
             * When the angle is larger than this, we consider
             * the angle difference to the negative of the eigenvector
             * (which is also a valid eigenvector).
             * So an angle difference of M_PI is mapped to 0.
             */
            if (diff > M_PI_2)
                diff = M_PI - diff;
            if ( ABS(diff) > tolerance_angle )
                {
                continue;
                }

            /* If we got this far, then we have found
             * a GAUS which agrees witht he model (to within tolerance)
             */
            *gaus_ret = gaus;
            return(TRUE);

            }                   /* end of 'for (gaus=neighbours->head...' */

        }                   /* end of 'else if (model_type == circle)' */

    return(FALSE);

}               /* end of search_for_next_gaus() */



void
orient_in_same_direction(GAUS *gaus, GAUS *gaus2)
{
    double  dot_prdct;

    /* Check that dominant eigenvector is pointing in
     * same direction.
     */
    dot_prdct = gaus->eig_vec[1][1]*gaus2->eig_vec[1][1] +
                gaus->eig_vec[2][1]*gaus2->eig_vec[2][1];

    if (dot_prdct < 0.0)
        {
        gaus2->eig_vec[1][1] = -gaus2->eig_vec[1][1];
        gaus2->eig_vec[2][1] = -gaus2->eig_vec[2][1];
        }


    /* Check that the other eigenvector is pointing in
     * same direction.
     */
    dot_prdct = gaus->eig_vec[1][2]*gaus2->eig_vec[1][2] +
                gaus->eig_vec[2][2]*gaus2->eig_vec[2][2];

    if (dot_prdct < 0.0)
        {
        gaus2->eig_vec[1][2] = -gaus2->eig_vec[1][2];
        gaus2->eig_vec[2][2] = -gaus2->eig_vec[2][2];
        }

}   /* end of orient_in_same_direction() */



/* Assumes we have hit an intersection or missing GAUS in the image.
 * Discards the last GAUS added to the chunk (old_tail_gaus).
 * (since if near an intersection, this GAUS is probably mis-aligned).
 * models the end of the chunk, and look for the next GAUS.
 *
 * Having found one GAUS, try to find a second one.
 * If we can't find two GAUS to add to the chunk,
 * then we conclude that we are not at an intersection,
 * add just revert back to the chunk as it was when
 * we began this function.
 *
 * If two appropriate GAUS is found (one predicted by the model of the
 * end of the chunk), then it is added to the chunk,
 * and *status is set to TRUE.
 * If none or only one GAUS is found, then old_tail_gaus is re-added
 * to the chunk, and *status is set to FALSE.
 */
GAUS_LIST *
/*look_for_occluded_GAUS(GAUS_LIST *gaus_list, GAUS_LIST *chunk,
                                    Window theWindow, GC theGC,
                                    BOOLEAN *status)*/
look_for_occluded_GAUS(GAUS_LIST *gaus_list, GAUS_LIST *chunk,
                                    BOOLEAN *status)
{
    BOOLEAN     ret_val;
    int         order_of_model, num_gaus;
    GAUS        *gaus_ret, *old_tail_gaus, *gaus_discard;
    GAUS_LIST   *neighbours;
    double      x, y, theta;
    double      x_centre, y_centre, radius;
    double      num_points_mean, num_points_std_dev;
    MODEL_TYPE  model_type;
    GAUS_LIST   *disconnect_tail_from_linked_list_of_GAUS(GAUS_LIST *gaus_list);
    void        orient_in_same_direction(GAUS *gaus, GAUS *gaus2);
    GAUS_LIST   *put_in_linked_list_of_gaus(GAUS_LIST *gaus_list, GAUS *g);
    GAUS        *copy_gaus(GAUS *g);
    void        free_gaus(GAUS *g);
    void        free_linked_list_of_gaus(GAUS_LIST *gaus_list);




    /* Remove the last GAUS that was added to the chunk.
     * This GAUS may have been misaligned, explaining why
     * we couldn't find the next GAUS in the chunk.
     */
    old_tail_gaus = chunk->tail;
    chunk = disconnect_tail_from_linked_list_of_GAUS(chunk);

    if ((chunk == (GAUS_LIST *)NULL) || (chunk->head == (GAUS *)NULL))
        {
        *status = FALSE;
        /* Since we couldn't find anything,
         * add the old tail gaus back to the chunk.
         */
        chunk = put_in_linked_list_of_gaus(chunk, old_tail_gaus);

        return(chunk);
        }



#ifdef DEBUG
{
XSetForeground(gDisplay, theGC, gRandomColors[4].pixel);
XFillArc(gDisplay, theWindow, theGC,
                            (int )(chunk->tail->mean[1]-5),
                            (int )(chunk->tail->mean[2]-5),
                            10, 10, 0, 23040);
XFlush(gDisplay);
}
#endif

    for (num_gaus=0; num_gaus < 2; num_gaus++)
        {
        ret_val = FALSE;

        /* Find gaus which may be chunked
         * This reduces our search space.
         */
        if (num_gaus == 0)
            {
            /* When looking for first gaus, check within a
             * larger neighbourhood.
             */
            neighbours = find_gaus_within_neighbourhood(chunk->tail, gaus_list,
                                                3*NEIGHBOURHOOD);
            }
        else
            {
            /* When looking for second GAUS, go back
             * to searching within the standard neighbourhood.
             */
            neighbours = find_gaus_within_neighbourhood(chunk->tail, gaus_list,
                                                NEIGHBOURHOOD);
            }

        /* Are there other GAUS nearby? */
        if (neighbours != (GAUS_LIST *)NULL)
            {
            order_of_model = 4;
            do
                {
/*                ret_val = model_end_of_chunk(chunk, order_of_model,
                                        theWindow, theGC,
                                        &x, &y, &theta,
                                        &x_centre, &y_centre, &radius,
                                        &num_points_mean, &num_points_std_dev,
                                        &model_type);*/
                ret_val = model_end_of_chunk(chunk, order_of_model,
                                        &x, &y, &theta,
                                        &x_centre, &y_centre, &radius,
                                        &num_points_mean, &num_points_std_dev,
                                        &model_type);
#ifdef DEBUG
{
int dx, dy;
int x1, y1, x2, y2;

if (ret_val == TRUE)
{
    if (model_type == line)
        {
        XSetForeground(gDisplay, theGC, gRandomColors[5].pixel);
        x1 = (int )(x - 100*cos(theta));
        y1 = (int )(y - 100*sin(theta));
        x2 = (int )(x + 100*cos(theta));
        y2 = (int )(y + 100*sin(theta));
        XDrawLine(gDisplay, theWindow, theGC, x1, y1, x2, y2);
        }
    else if (model_type == circle)
        {
        XSetForeground(gDisplay, theGC, gRandomColors[5].pixel);
        x1 = (int )(x_centre - radius);
        y1 = (int )(y_centre - radius);
        dx = (int )(2*radius);
        dy = (int )(2*radius);
        XDrawArc(gDisplay, theWindow, theGC, x1, y1, dx, dy, 0, 23040);
        }
    XFlush(gDisplay);
}
else
    printf("%d Occlusion Model Failed\n", order_of_model);
sleep(1);
}
#endif
                if (ret_val == TRUE)
                    ret_val = search_for_next_gaus(neighbours, chunk,
                                        model_type,
                                        x, y, theta,
                                        x_centre, y_centre, radius,
                                        num_points_mean, num_points_std_dev,
                                        &gaus_ret);
                if (ret_val == TRUE)
                    {
                    /* Found the predicted GAUS.
                     * Add it to the chunk.
                     */
                    chunk = put_in_linked_list_of_gaus(chunk,
                                        copy_gaus(gaus_ret->original));
                    orient_in_same_direction(chunk->tail->prev, chunk->tail);

                    /* Flag the GAUS as having been used in a chunk */
                    gaus_ret->original->flag = TRUE;
                    }
                else if (ret_val == FALSE)
                    order_of_model--;
                if (order_of_model == 0)
                    break;          /* out of do-while loop */
                }
            while (ret_val == FALSE);

            /* Free memory */
            free_linked_list_of_gaus(neighbours);

            }           /* end of 'if (neighbours != (GAUS_LIST *)NULL)' */

        if (ret_val == FALSE)
            break;
        }               /* end of 'for (num_gaus=0;...' */


    if (ret_val == TRUE)                /* Found the next GAUS */
        {
        *status = TRUE;
        /* Keep this out of chunk */
        (old_tail_gaus->original)->flag = FALSE;
        free_gaus(old_tail_gaus);
        }
    else                                /* Did not find next two GAUS */
        {
        *status = FALSE;
        /* Since we could not add 2 gaus to the chunk,
         * we conclude that we were not at an occlusion
         * or an intersection.
         * Remove from the chunk any GAUS that we added whilst
         * in this function, and add the old tail gaus
         * back to the chunk.
         */
        if (num_gaus == 1)
            {
            gaus_discard = chunk->tail;
            chunk = disconnect_tail_from_linked_list_of_GAUS(chunk);
            (gaus_discard->original)->flag = FALSE;
            free_gaus(gaus_discard);
            }
        chunk = put_in_linked_list_of_gaus(chunk, old_tail_gaus);
        }

    return(chunk);

}       /* end of look_for_occluded_GAUS() */



GAUS_LIST *
/*find_chunk_based_on_gaus_in_other_direction(GAUS_LIST *gaus_list,
                                                GAUS_LIST *chunk,
                                                Window theWindow, GC theGC)*/
find_chunk_based_on_gaus_in_other_direction(GAUS_LIST *gaus_list,
                                                GAUS_LIST *chunk)
{
    BOOLEAN     ret_val;
    int         order_of_model;
    GAUS        *gaus_ret;
    GAUS_LIST   *neighbours;
    double      x, y, theta;
    double      x_centre, y_centre, radius;
    double      num_points_mean, num_points_std_dev;
    MODEL_TYPE  model_type;
    void        orient_in_same_direction(GAUS *gaus, GAUS *gaus2);
    GAUS_LIST   *insert_at_head_of_linked_list_of_gaus(GAUS_LIST *gaus_list,
                                            GAUS *g);
    GAUS        *copy_gaus(GAUS *g);
    void        free_linked_list_of_gaus(GAUS_LIST *gaus_list);


    do
        {
        /* Find gaus which may be chunked
         * This reduces our search space.
         */
#ifdef DEBUG
{
XSetForeground(gDisplay, theGC, gRandomColors[1].pixel);
XFillArc(gDisplay, theWindow, theGC,
                            (int )(chunk->head->mean[1]-5),
                            (int )(chunk->head->mean[2]-5),
                            10, 10, 0, 23040);
XFlush(gDisplay);
}
#endif
        ret_val = FALSE;
        neighbours = find_gaus_within_neighbourhood(chunk->head, gaus_list,
                                                    NEIGHBOURHOOD);

        /* Are there other GAUS nearby? */
        if (neighbours != (GAUS_LIST *)NULL)
            {
            order_of_model = 4;
            do
                {
/*                ret_val = model_end_of_chunk_in_other_direction(chunk,
                                    order_of_model,
                                    theWindow, theGC,
                                    &x, &y, &theta,
                                    &x_centre, &y_centre, &radius,
                                    &num_points_mean, &num_points_std_dev,
                                    &model_type);*/
                ret_val = model_end_of_chunk_in_other_direction(chunk,
                                    order_of_model,
                                    &x, &y, &theta,
                                    &x_centre, &y_centre, &radius,
                                    &num_points_mean, &num_points_std_dev,
                                    &model_type);
#ifdef DEBUG
{
int dx, dy;
int x1, y1, x2, y2;

if (ret_val == TRUE)
{
    if (model_type == line)
        {
        XSetForeground(gDisplay, theGC, gRandomColors[2].pixel);
        x1 = (int )(x - 100*cos(theta));
        y1 = (int )(y - 100*sin(theta));
        x2 = (int )(x + 100*cos(theta));
        y2 = (int )(y + 100*sin(theta));
        XDrawLine(gDisplay, theWindow, theGC, x1, y1, x2, y2);
        }
    else if (model_type == circle)
        {
        XSetForeground(gDisplay, theGC, gRandomColors[3].pixel);
        x1 = (int )(x_centre - radius);
        y1 = (int )(y_centre - radius);
        dx = (int )(2*radius);
        dy = (int )(2*radius);
        XDrawArc(gDisplay, theWindow, theGC, x1, y1, dx, dy, 0, 23040);
        }
    XFlush(gDisplay);
}
else
    printf("%d Model Failed\n", order_of_model);
sleep(1);
}
#endif
                if (ret_val == TRUE)
                    ret_val = search_for_next_gaus_in_other_direction(
                                        neighbours,
                                        chunk,
                                        model_type,
                                        x, y, theta,
                                        x_centre, y_centre, radius,
                                        num_points_mean, num_points_std_dev,
                                        &gaus_ret);
                if (ret_val == TRUE)
                    {
                    /* Found the predicted GAUS.
                     * Add it to the chunk.
                     */
                    chunk = insert_at_head_of_linked_list_of_gaus(chunk,
                                        copy_gaus(gaus_ret->original));
                    orient_in_same_direction(chunk->head->next, chunk->head);

                    /* Flag the GAUS as having been used in a chunk */
                    gaus_ret->original->flag = TRUE;
                    }
                else if (ret_val == FALSE)
                    order_of_model--;
                if (order_of_model == 0)
                    break;          /* out of do-while loop */
                }
            while (ret_val == FALSE);


            /* Free memory */
            free_linked_list_of_gaus(neighbours);

            }               /* end of 'if (neighbours != (GAUS_LIST *)NULL)' */



            /* If we couldn't find the next GAUS.
             * Try looking a little further afield.
             */
            if (ret_val == FALSE)
                {
/*                chunk = look_for_occluded_GAUS_in_other_direction(gaus_list,
                                        chunk,
                                        theWindow, theGC,
                                        &ret_val);*/
                chunk = look_for_occluded_GAUS_in_other_direction(gaus_list,
                                        chunk,
                                        &ret_val);
                }

        }
    while (ret_val == TRUE);


    return(chunk);

}                   /* end of find_chunk_based_on_gaus_in_other_direction() */



/* Model the last few GAUS in the chunk.
 * The number of gaus is specified by 'order_of_model'.
 * We will model them as either a line, or a circle.
 * If we model them as a circle, the parameters of the
 * circle are recorded in (*x_centre_ret, *y_centre_ret, *radius_ret).
 * It we model them as a line, the parameters are recorded
 * in (*x_ret, *y_ret, *theta_ret).
 *
 * Return TRUE if we were able to model the GAUS.
 * Return FALSE if were not able to model the GAUS.
 */
BOOLEAN
/*model_end_of_chunk_in_other_direction(GAUS_LIST *chunk, int order_of_model,
                        Window theWindow, GC theGC,
                        double *x_ret, double *y_ret, double *theta_ret,
                        double *x_centre_ret, double *y_centre_ret,
                        double *radius_ret,
                        double *num_points_mean, double *num_points_std_dev,
                        MODEL_TYPE *model_type_ret)*/
model_end_of_chunk_in_other_direction(GAUS_LIST *chunk, int order_of_model,
                        double *x_ret, double *y_ret, double *theta_ret,
                        double *x_centre_ret, double *y_centre_ret,
                        double *radius_ret,
                        double *num_points_mean, double *num_points_std_dev,
                        MODEL_TYPE *model_type_ret)
{
    GAUS    *first, *gaus_from, *gaus_to, *gaus;
    double  theta, theta_to, theta_from, theta_normal;
    double  dtheta, dt, curv;
    int     i;

    if (order_of_model < 1)
        return(FALSE);

    /* There aren't enough GAUS in the chunk */
    if (order_of_model > chunk->num_elements)
        return(FALSE);


    if (order_of_model == 1)
        {
        /* Use only first GAUS in chunk to form estimate.
         * Estimated GAUS will have same theta as first GAUS in chunk.
         */
        first = chunk->head;

        *x_ret = first->mean[1];
        *y_ret = first->mean[2];
        *theta_ret = atan2(first->eig_vec[2][1], first->eig_vec[1][1]);
        *num_points_mean = first->num_points;
        *num_points_std_dev = MIN_STD_DEV_FOR_NUM_POINTS;
        *model_type_ret = line;
        return(TRUE);
        }
    else
        {
        gaus_to = chunk->head->next;
        gaus_from = chunk->head;

        /* Estimate curvature - curvature = dtheta/dt
         * First find dt (distance along curve)
         */
        dt = 0;
        for (i=1; i < order_of_model; i++)
            {
            dt += sqrt( SQR(gaus_to->mean[1] - gaus_from->mean[1]) +
                        SQR(gaus_to->mean[2] - gaus_from->mean[2]) );

            /* Don't do this the last time through the loop. */
            if (i < (order_of_model-1))
                {
                gaus_from = gaus_to;
                gaus_to = gaus_to->next;
                }
            }

        /* Find difference in angle */
        theta_to = atan2(gaus_to->eig_vec[2][1],
                                            gaus_to->eig_vec[1][1]);
        theta_from = atan2(chunk->head->eig_vec[2][1],
                                            chunk->head->eig_vec[1][1]);
        dtheta = theta_to - theta_from;

        /* Make sure dtheta is in the range [-M_PI_2, M_PI_2].
         */
        if ((dtheta >= -2.0*M_PI) && (dtheta < -1.5*M_PI))
            {
                                                    /* [-2PI, -1.5PI) */
            dtheta += 2.0*M_PI;
            }
        else if ((dtheta >= -1.5*M_PI) && (dtheta < -M_PI_2))
            {
                                                    /* [-1.5PI, -.5PI) */
            dtheta += M_PI;
            }
        else if ((dtheta > M_PI_2) && (dtheta <= 1.5*M_PI))
            {
                                                    /* (.5PI, 1.5PI] */
            dtheta -= M_PI;
            }
        if ((dtheta > 1.5*M_PI) && (dtheta <= 2.0*M_PI))
            {
                                                    /* (1.5PI, 2PI] */
            dtheta -= 2.0*M_PI;
            }

        /* Estimate curvature */
        curv = dtheta / dt;



        /* Compute average number of points in each GAUS,
         * and the standard deviation.
         */
        *num_points_mean = 0;
        for (i=0, gaus=chunk->head; i < order_of_model; i++, gaus=gaus->next)
            {
            *num_points_mean += gaus->num_points;
            }
        *num_points_mean /= (double )order_of_model;
        *num_points_std_dev = 0;
        for (i=0, gaus=chunk->head; i < order_of_model; i++, gaus=gaus->next)
            {
            *num_points_std_dev += SQR(gaus->num_points - *num_points_mean);
            }
        *num_points_std_dev /= (double )(order_of_model-1);
        *num_points_std_dev = sqrt(*num_points_std_dev);
        if (*num_points_std_dev < MIN_STD_DEV_FOR_NUM_POINTS)
            *num_points_std_dev = MIN_STD_DEV_FOR_NUM_POINTS;


        /* Zero curvature - model GAUS as a straight line. */
        if (ABS(curv) < A_VERY_SMALL_POSITIVE_VALUE)
            {
            first = chunk->head;

            *x_ret = first->mean[1];
            *y_ret = first->mean[2];
            *theta_ret = atan2(first->eig_vec[2][1], first->eig_vec[1][1]);
            *model_type_ret = line;
            return(TRUE);
            }
        else
            {
            /* Model GAUS as lying on a circle. */

            /* Find Radius */
            *radius_ret = 1.0 / ABS(curv);

            /* Find Centre.
             * The last GAUS in the chunk lies on the circle,
             * and is orientated normal to it.
             * Hence the line perpendicular to it passes through
             * the centre.
             * We know the radius, so just move along this line by
             * a distance equal to the radius, to find the circle centre.
             */
            first = chunk->head;
            theta = atan2(first->eig_vec[2][1], first->eig_vec[1][1]);
            if (curv > 0)
                theta_normal = theta + M_PI_2;
            else
                theta_normal = theta - M_PI_2;
            *x_centre_ret = first->mean[1] + (*radius_ret) * cos(theta_normal);
            *y_centre_ret = first->mean[2] + (*radius_ret) * sin(theta_normal);
            *model_type_ret = circle;

            return(TRUE);
            }
        }

}               /* end of model_end_of_chunk_in_other_direction() */



/* Search for which of the GAUS in neighbours best fits the
 * model of the end of the chunk.
 * The end of the chunk is either modelled as a straight line
 * (when model_type = line), whose parameters are specified
 * in (x, y, theta),
 * or as a circle (when model_type = circle), whose parameters
 * are specified in (x_centre, y_centre, radius).
 * If an appropriate GAUS is found, *gaus_ret is set to point
 * to it and the function returns TRUE.
 * Otherwise the function return FALSE.
 */
BOOLEAN
search_for_next_gaus_in_other_direction(GAUS_LIST *neighbours,
                            GAUS_LIST *chunk,
                            MODEL_TYPE model_type,
                            double x, double y, double theta,
                            double x_centre, double y_centre, double radius,
                            double num_points_mean, double num_points_std_dev,
                            GAUS **gaus_ret)
{
    /* PARAMETERS */
    double  tolerance_translation = gResolutionRadius/3;
    double  tolerance_angle = M_PI/12;
    double  tolerance_num_points = 3;

    GAUS    *gaus, *first;
    double  t;
    double  theta1, theta_tangent;
    double  dist, diff;
    double  x_closest, y_closest;
    double  mahal_dist, min_mahal_dist;
    double  a, b, val;


    if (neighbours == (GAUS_LIST *)NULL)
        {
        return(FALSE);
        }


    if (model_type == line)
        {
        min_mahal_dist = DBL_MAX;
        for (gaus=neighbours->head; gaus != NULL; gaus=gaus->next)
            {

            /* Test if number of points in gaus are as expected.
             */
            val = (gaus->num_points - num_points_mean) / num_points_std_dev;
            if (ABS(val) > tolerance_num_points)
                continue;


            /* We are searching for the GAUS that is closest to the
             * head of the chunk, and which conforms to the model
             * (to within some tolerance).
             * Use the head GAUS of the chunk to define a metric,
             * with one basis vector in the direction of the dominant
             * eigenvector of the head gaus, with length equal to
             * the square-root of the dominant eigenvalue,
             * and the second basis vector perpendicular to this,
             * with length equal to the square-root of the smaller eigenvalue.
             */
            first = chunk->head;
            a = (( (gaus->mean[1]-first->mean[1]) *first->eig_vec[1][1]) +
                    ( (gaus->mean[2]-first->mean[2]) *first->eig_vec[2][1])) /
                                sqrt(first->eig_val[1]);
            b = (( (gaus->mean[1]-first->mean[1]) *first->eig_vec[1][2]) +
                    ( (gaus->mean[2]-first->mean[2]) *first->eig_vec[2][2])) /
                                sqrt(first->eig_val[2]);

            /* Wrong direction */
            if (a >= 0)
                continue;

            mahal_dist = sqrt( SQR(a) + SQR(b) );
            if (mahal_dist > min_mahal_dist)
                continue;


            /* Test if GAUS lies near the line modelling the
             * end of the chunk.
             * First test how far it is from the line
             * (perpendicular distance).
             * We have a parametric representation of the line as:
             * { (x,y) : (x, y) + t(cos(theta), sin(theta)) }
             * for t a real number.
             */
            t = (gaus->mean[1]-x)*cos(theta) + (gaus->mean[2]-y)*sin(theta);
            x_closest = x + t*cos(theta);
            y_closest = y + t*sin(theta);
            dist = sqrt( SQR(gaus->mean[1] - x_closest) +
                            SQR(gaus->mean[2] - y_closest) );
            if (dist > tolerance_translation)
                continue;

            /* acos() gives angle difference in the range [0..M_PI].
             */
            val = gaus->eig_vec[1][1]*cos(theta) +
                            gaus->eig_vec[2][1]*sin(theta);
            if (ABS(val) > 0.9999)
                diff = 0;
            else
                diff = acos(val);

            /* The angle difference must be in the range [0, M_PI/2].
             * When the angle is larger than this, we consider
             * the angle difference to the negative of the eigenvector
             * (which is also a valid eigenvector).
             * So an angle difference of M_PI is mapped to 0.
             */
            if (diff > M_PI_2)
                diff = M_PI - diff;
            if ( ABS(diff) > tolerance_angle )
                {
                continue;
                }

            /* If we got this far, then we have found
             * a GAUS which agrees witht he model (to within tolerance)
             */
            *gaus_ret = gaus;
            return(TRUE);

            }                   /* end of 'for (gaus=neighbours->head...' */

        }                   /* end of 'if (model_type == line)' */
    else if (model_type == circle)
        {
        min_mahal_dist = DBL_MAX;
        for (gaus=neighbours->head; gaus != NULL; gaus=gaus->next)
            {

            /* Test if number of points in gaus are as expected.
             */
            val = (gaus->num_points - num_points_mean) / num_points_std_dev;
            if (ABS(val) > tolerance_num_points)
                continue;


            /* We are searching for the GAUS that is closest to the
             * head of the chunk, and which conforms to the model
             * (to within some tolerance).
             * Use the head GAUS of the chunk to define a metric,
             * with one basis vector in the direction of the dominant
             * eigenvector of the head gaus, with length equal to
             * the square-root of the dominant eigenvalue,
             * and the second basis vector perpendicular to this,
             * with length equal to the square-root of the smaller eigenvalue.
             */
            first = chunk->head;
            a = (( (gaus->mean[1]-first->mean[1]) *first->eig_vec[1][1]) +
                    ( (gaus->mean[2]-first->mean[2]) *first->eig_vec[2][1])) /
                                sqrt(first->eig_val[1]);
            b = (( (gaus->mean[1]-first->mean[1]) *first->eig_vec[1][2]) +
                    ( (gaus->mean[2]-first->mean[2]) *first->eig_vec[2][2])) /
                                sqrt(first->eig_val[2]);

            /* Wrong direction */
            if (a >= 0)
                continue;

            mahal_dist = sqrt( SQR(a) + SQR(b) );
            if (mahal_dist > min_mahal_dist)
                continue;


            /* Test if GAUS lies near circle.
             * First test if its distance from the centre is approx.
             * equal to the radius.
             */
            dist = sqrt( SQR(gaus->mean[1] - x_centre) +
                            SQR(gaus->mean[2] - y_centre) );
            if (ABS(dist - radius) > tolerance_translation)
                continue;

            /* Next test if the orientation of the GAUS is
             * approx. the orientation of the nearest tangent
             * to the circle.
             * First find angle of line from GAUS to circle centre.
             * Angle of tangent will be perpendicular to this.
             */
            theta1 = atan2(gaus->mean[2]-y_centre, gaus->mean[1]-x_centre);
            theta_tangent = theta1+M_PI_2;
            /* acos() gives angle difference in the range [0..M_PI].
             */
            val = gaus->eig_vec[1][1]*cos(theta_tangent) +
                            gaus->eig_vec[2][1]*sin(theta_tangent);
            if (ABS(val) > 0.9999)
                diff = 0;
            else
                diff = acos(val);

            /* The angle difference must be in the range [0, M_PI/2].
             * When the angle is larger than this, we consider
             * the angle difference to the negative of the eigenvector
             * (which is also a valid eigenvector).
             * So an angle difference of M_PI is mapped to 0.
             */
            if (diff > M_PI_2)
                diff = M_PI - diff;
            if ( ABS(diff) > tolerance_angle )
                {
                continue;
                }

            /* If we got this far, then we have found
             * a GAUS which agrees witht he model (to within tolerance)
             */
            *gaus_ret = gaus;
            return(TRUE);

            }                   /* end of 'for (gaus=neighbours->head...' */

        }                   /* end of 'else if (model_type == circle)' */

    return(FALSE);

}               /* end of search_for_next_gaus_in_other_direction() */



/* Assumes we have hit an intersection or missing GAUS in the image.
 * Discards the most recent GAUS added to the chunk (old_head_gaus).
 * (since if near an intersection, this GAUS is probably mis-aligned).
 * models the end of the chunk, and look for the next GAUS.
 *
 * Having found one GAUS, try to find a second one.
 * If we can't find two GAUS to add to the chunk,
 * then we conclude that we are not at an intersection,
 * add just revert back to the chunk as it was when
 * we began this function.
 *
 * If two appropriate GAUS is found (one predicted by the model of the
 * end of the chunk), then it is added to the chunk,
 * and *status is set to TRUE.
 * If none or only one GAUS is found, then old_head_gaus is re-added
 * to the chunk, and *status is set to FALSE.
 */
GAUS_LIST *
/*look_for_occluded_GAUS_in_other_direction(GAUS_LIST *gaus_list,
                                    GAUS_LIST *chunk,
                                    Window theWindow, GC theGC,
                                    BOOLEAN *status)*/
look_for_occluded_GAUS_in_other_direction(GAUS_LIST *gaus_list,
                                    GAUS_LIST *chunk,
                                    BOOLEAN *status)
{
    BOOLEAN     ret_val;
    int         order_of_model, num_gaus;
    GAUS        *gaus_ret, *old_head_gaus, *gaus_discard;
    GAUS_LIST   *neighbours;
    double      x, y, theta;
    double      x_centre, y_centre, radius;
    double      num_points_mean, num_points_std_dev;
    MODEL_TYPE  model_type;
    GAUS_LIST   *disconnect_head_from_linked_list_of_GAUS(GAUS_LIST *gaus_list);
    void        orient_in_same_direction(GAUS *gaus, GAUS *gaus2);
    GAUS_LIST   *insert_at_head_of_linked_list_of_gaus(GAUS_LIST *gaus_list,
                                            GAUS *g);
    GAUS        *copy_gaus(GAUS *g);
    void        free_gaus(GAUS *g);
    void        free_linked_list_of_gaus(GAUS_LIST *gaus_list);



    ret_val = FALSE;

    /* Remove the last GAUS that was added to the chunk.
     * This GAUS may have been misaligned, explaining why
     * we couldn't find the next GAUS in the chunk.
     */
    old_head_gaus = chunk->head;
    chunk = disconnect_head_from_linked_list_of_GAUS(chunk);

    if ((chunk == (GAUS_LIST *)NULL) || (chunk->head == (GAUS *)NULL))
        {
        *status = FALSE;
        /* Since we couldn't find anything,
         * add the old head gaus back to the chunk.
         */
        chunk = insert_at_head_of_linked_list_of_gaus(chunk, old_head_gaus);

        return(chunk);
        }



#ifdef DEBUG
{
XSetForeground(gDisplay, theGC, gRandomColors[4].pixel);
XFillArc(gDisplay, theWindow, theGC,
                            (int )(chunk->head->mean[1]-5),
                            (int )(chunk->head->mean[2]-5),
                            10, 10, 0, 23040);
XFlush(gDisplay);
}
#endif
    for (num_gaus=0; num_gaus < 2; num_gaus++)
        {
        ret_val = FALSE;

        /* Find gaus which may be chunked
         * This reduces our search space.
         */
        if (num_gaus == 0)
            {
            /* When looking for first gaus, check within a
             * larger neighbourhood.
             */
            neighbours = find_gaus_within_neighbourhood(chunk->head, gaus_list,
                                                3*NEIGHBOURHOOD);
            }
        else
            {
            /* When looking for second GAUS, go back
             * to searching within the standard neighbourhood.
             */
            neighbours = find_gaus_within_neighbourhood(chunk->head, gaus_list,
                                                NEIGHBOURHOOD);
            }


        /* Are there other GAUS nearby? */
        if (neighbours != (GAUS_LIST *)NULL)
            {
            order_of_model = 4;
            do
                {
/*                ret_val = model_end_of_chunk_in_other_direction(chunk,
                                        order_of_model,
                                        theWindow, theGC,
                                        &x, &y, &theta,
                                        &x_centre, &y_centre, &radius,
                                        &num_points_mean, &num_points_std_dev,
                                        &model_type);*/
                ret_val = model_end_of_chunk_in_other_direction(chunk,
                                        order_of_model,
                                        &x, &y, &theta,
                                        &x_centre, &y_centre, &radius,
                                        &num_points_mean, &num_points_std_dev,
                                        &model_type);
#ifdef DEBUG
{
int dx, dy;
int x1, y1, x2, y2;

if (ret_val == TRUE)
{
    if (model_type == line)
        {
        XSetForeground(gDisplay, theGC, gRandomColors[5].pixel);
        x1 = (int )(x - 100*cos(theta));
        y1 = (int )(y - 100*sin(theta));
        x2 = (int )(x + 100*cos(theta));
        y2 = (int )(y + 100*sin(theta));
        XDrawLine(gDisplay, theWindow, theGC, x1, y1, x2, y2);
        }
    else if (model_type == circle)
        {
        XSetForeground(gDisplay, theGC, gRandomColors[5].pixel);
        x1 = (int )(x_centre - radius);
        y1 = (int )(y_centre - radius);
        dx = (int )(2*radius);
        dy = (int )(2*radius);
        XDrawArc(gDisplay, theWindow, theGC, x1, y1, dx, dy, 0, 23040);
        }
    XFlush(gDisplay);
}
else
    printf("%d Occlusion Model Failed\n", order_of_model);
sleep(1);
}
#endif
                if (ret_val == TRUE)
                    ret_val = search_for_next_gaus_in_other_direction(
                                        neighbours,
                                        chunk,
                                        model_type,
                                        x, y, theta,
                                        x_centre, y_centre, radius,
                                        num_points_mean, num_points_std_dev,
                                        &gaus_ret);
                if (ret_val == TRUE)
                    {
                    /* Found the predicted GAUS.
                     * Add it to the chunk.
                     */
                    chunk = insert_at_head_of_linked_list_of_gaus(chunk,
                                        copy_gaus(gaus_ret->original));
                    orient_in_same_direction(chunk->head->next, chunk->head);

                    /* Flag the GAUS as having been used in a chunk */
                    gaus_ret->original->flag = TRUE;
                    }
                else if (ret_val == FALSE)
                    order_of_model--;
                if (order_of_model == 0)
                    break;          /* out of do-while loop */
                }
            while (ret_val == FALSE);

            /* Free memory */
            free_linked_list_of_gaus(neighbours);

            }           /* end of 'if (neighbours != (GAUS_LIST *)NULL)' */

        if (ret_val == FALSE)
            break;
        }                   /* end of 'for (num_gaus=0;...' */


    if (ret_val == TRUE)                /* Found the next GAUS */
        {
        *status = TRUE;
        /* Keep this out of chunk */
        (old_head_gaus->original)->flag = FALSE;
        free_gaus(old_head_gaus);
        }
    else                                /* Did not find the next GAUS */
        {
        *status = FALSE;
        /* Since we could not add 2 gaus to the chunk,
         * we conclude that we were not at an occlusion
         * or an intersection.
         * Remove from the chunk any GAUS that we added whilst
         * in this function, and add the old tail gaus
         * back to the chunk.
         */
        if (num_gaus == 1)
            {
            gaus_discard = chunk->head;
            chunk = disconnect_head_from_linked_list_of_GAUS(chunk);
            (gaus_discard->original)->flag = FALSE;
            free_gaus(gaus_discard);
            }
        chunk = insert_at_head_of_linked_list_of_gaus(chunk, old_head_gaus);
        }

    return(chunk);

}       /* end of look_for_occluded_GAUS_in_other_direction() */
