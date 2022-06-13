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
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <math.h>
#include <values.h>     /* MAXDOUBLE */
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "../include/upwrite.h"



/* Data structures.
 */
typedef struct sum_type {
    /* A data structure containing some running totals used by
     * the E.M. slgorithm.
     * There will be one of these for each GAUS used by the
     * E.M. algorithm
     */
    int     dim;
    double  likelihood;
    double  sum_w;
    double  *sum_wx;
    double  **sum_wxy;

    struct sum_type *next;

} SUM;




/* Function Prototypes
 */
GAUS_LIST   *em_algorithm(int num_gaus, POINT_LIST *point_list, Window theWindow, GC theGC);
SUM     *alloc_sum(int dim);
void    free_sum(SUM *sum);
SUM     *put_in_linked_list_of_sum(SUM *head, SUM *sum);
void    free_linked_list_of_sum(SUM *sum_head);
SUM     *remove_from_linked_list_of_sum(SUM *sum_list, SUM *sum);
GAUS_LIST   *init_gaussians(int num_gaus, POINT_LIST *point_list);
SUM     *init_sum_list(int num_gaus, int dim);
void    set_running_totals_to_zero(SUM *sum);
void    print_gaus(GAUS *g);







GAUS_LIST *
em_algorithm(int num_gaus, POINT_LIST *point_list, Window theWindow, GC theGC)
{
    GAUS    *gaus, *gaus_to_be_removed;
    GAUS_LIST   *gaus_list;
    SUM     *sum, *sum_list, *sum_to_be_removed;
    POINT   *point;
    int     num_iterations = 300;
    int     num_gaus_remaining;
    int     iteration;
    int     i, j;
    double  total_likelihood, w, total_points, det;
    int         theScreen;

    unsigned long   theWhitePixel;
    char        str[500];
    XFontStruct *font_info; /* info on the font used. */
    int         x, y, direction, ascent, descent;
    XCharStruct overall;

    GAUS_LIST   *init_gaussians(int num_gaus, POINT_LIST *point_list);
    SUM     *init_sum_list(int num_gaus, int dim);
    double  likelihood(POINT *point, GAUS *gaus);
    GAUS_LIST   *remove_from_linked_list_of_gaus(GAUS_LIST *gaus_list, GAUS *gaus);
    SUM     *remove_from_linked_list_of_sum(SUM *sum_list, SUM *sum);
    int     find_eigenvectors_real_symmetric_matrix(double **A, double *Eig_val, double **Eig_vec, int dim);
    void    find_inverse(double **MAT, double **I, int dim);
    void    set_running_totals_to_zero(SUM *sum);
    void    free_linked_list_of_sum(SUM *sum_head);
    int     return_val;



    /* Check the arguments passed to this function.
     */
    if ((point_list == NULL) || (point_list->num_elements == 0) || (num_gaus == 0))
        return((GAUS_LIST *)NULL);


    /* Initialise font.
     * Used to display how many iterations have been completed.
     */
    theScreen = DefaultScreen(gDisplay);
    theWhitePixel = WhitePixel( gDisplay, theScreen);
    font_info = XLoadQueryFont(gDisplay, DISPLAY_FONT_NAME);
    if (font_info == NULL)
        {
        fprintf(stderr, "Can not open font %s in em_algorithm() in em_algorithm.c\n", DISPLAY_FONT_NAME);
        exit(1);
        }
    XSetFont(gDisplay, theGC, font_info->fid);


    /* Add a small amount of white noise
     * to the data.
     * This helps keep the gaussians from going degenerate.
     * (Because of the numerical method that we use
     * to compute the eigenvectors of the gaussians).
     */
    for (point=point_list->head; point != NULL; point=point->next)
        for (i=1; i <= point->dim; i++)
            point->x[i] += 0.1*(drand48() - 0.5);



    /* Create linked list of gaussians.
     * Initialise means and covariance matrices.
     */
    gaus_list = init_gaussians(num_gaus, point_list);


    /* Create an associate linked list.
     * This stores some running totals which are
     * needed for each gaus, hence there is one element
     * in this list for every gaus in gaus_list.
     */
    sum_list = init_sum_list(num_gaus, point_list->head->dim);



    for (iteration=0; iteration < num_iterations; iteration++)
        {
        /* Count the number of remaining gaussians.
         */
        if ((gaus_list == NULL) || (gaus_list->head == NULL))
            break;      /* No gaussians remaining. */

        num_gaus_remaining = 0;
        for (gaus=gaus_list->head; gaus != NULL; gaus=gaus->next) num_gaus_remaining++;



        /* Display number of iterations and gaussians remaining.
         */
        if(num_gaus_remaining == 1)
            sprintf(str, "iteration #%d of %d - 1 gaussian remaining\n", iteration, num_iterations);
        else
            sprintf(str, "iteration #%d of %d - %d gaussians remaining\n", iteration, num_iterations, num_gaus_remaining);

        /* Determine width, ascent and descent
         * of string (in pixels).
         */
        XTextExtents(font_info, str, strlen(str), &direction, &ascent, &descent, &overall);

        x = 10;
        y = 20 + ascent;
        XSetForeground(gDisplay, theGC, theWhitePixel);
        XFillRectangle(gDisplay, theWindow, theGC, x-1, y-ascent-1, overall.width+2, ascent+descent+2);
        /* Text in red
         */
        XSetForeground(gDisplay, theGC, gRandomColors[0].pixel);
        XDrawString(gDisplay, theWindow, theGC, x, y, str, strlen(str));
        XFlush(gDisplay);



        /* Do for all points
         * This 'for loop' contains most of the
         * calculations for E.M.
         */
        for (point=point_list->head; point != NULL; point=point->next)
            {
            total_likelihood = 0.0;
            for (gaus=gaus_list->head, sum=sum_list; gaus != NULL; gaus=gaus->next, sum=sum->next)
                {
                /* Calculate the point's likelihood relative
                 *  to each GAUS
                 */
                sum->likelihood = likelihood(point, gaus);
                total_likelihood += sum->likelihood;

                }
            /* Avoid divide-by-zero.
             * 'total_likelihood' is the summation of the
             * likelihood of this point relative to each gaussian.
             */
            if (total_likelihood > 0.0)
                {
                /* Calculate the point's contribution
                 * to running totals that we are
                 * keeping for each GAUS.
                 * For each GAUS in gaus_list,
                 * there is a corresponding SUM in sum_list.
                 */
                for (sum=sum_list; sum != NULL; sum=sum->next)
                    {
                    w = sum->likelihood / total_likelihood;
                    sum->sum_w += w;
                    for (i=1; i <= sum->dim; i++)
                        sum->sum_wx[i] += w*point->x[i];
                    for (i=1; i <= sum->dim; i++)
                        for (j=1; j <= sum->dim; j++)
                            sum->sum_wxy[i][j] +=  w*point->x[i]*point->x[j];
                    }
                }
            }

        /* Remove any GAUS with <= 2 point assigned
         * to them.
         * You need more than 1 point assigned to a gaussian for
         * for it to have a meaningful covariance matrix.
         * We're just being careful by insisting on 2 points.
         */
        gaus = gaus_list->head;
        sum = sum_list;
        while (gaus != NULL)
            {
            if (sum->sum_w <= 2.0)
                {
                gaus_to_be_removed = gaus;
                sum_to_be_removed = sum;
                gaus=gaus->next;
                sum=sum->next;
                gaus_list = remove_from_linked_list_of_gaus(gaus_list, gaus_to_be_removed);
                sum_list = remove_from_linked_list_of_sum(sum_list, sum_to_be_removed);
                }
            else
                {
                    gaus=gaus->next;
                sum=sum->next;
                }
            }


        /* If no gaussians remaining
         */
        if ((gaus_list == NULL) || (gaus_list->head == NULL))
            break;      /* out of iteration loop */


        /* Recalculate the gaussians.
         */
        total_points = 0;
        for (gaus=gaus_list->head, sum=sum_list; gaus != NULL; gaus=gaus->next, sum=sum->next)
            {
            /* Calculate means.
             */
            for (i=1; i <= gaus->dim; i++)
                gaus->mean[i] = sum->sum_wx[i] / sum->sum_w;

            /* Calculate covariance matrix.
             */
            for (i=1; i <= gaus->dim; i++)
                for (j=1; j <= gaus->dim; j++)
                    gaus->covar[i][j] =(sum->sum_wxy[i][j] -
                                        gaus->mean[i]*sum->sum_wx[j] -
                                        gaus->mean[j]*sum->sum_wx[i] +
                                        gaus->mean[i]*gaus->mean[j]*sum->sum_w)
                                            / (sum->sum_w - 1.0);
            /* Sum the weights.
             */
            total_points += sum->sum_w;
            }

        /* Set the number of points associated
         * with each gaus and the weight.
         * In a gaussian mixture model, there
         * is a weight associated with each gaussian
         * such that the sum of all the weights is 1.0
         */
        for (gaus=gaus_list->head, sum=sum_list; gaus != NULL; gaus=gaus->next, sum=sum->next)
            {
            gaus->num_points = sum->sum_w;
            gaus->weight = sum->sum_w / total_points;
            }


        /* Recalculate each gaussian's eigenvalues, eigenvectors,
         * sqrt_det and inverse_covar.
         */
        gaus = gaus_list->head;
        sum = sum_list;
        while (gaus != NULL)
            {
            return_val = find_eigenvectors_real_symmetric_matrix(gaus->covar, gaus->eig_val, gaus->eig_vec, gaus->dim);
            if (return_val == -1)
                {
                gaus_to_be_removed = gaus;
                sum_to_be_removed = sum;
                gaus=gaus->next;
                sum=sum->next;
                gaus_list = remove_from_linked_list_of_gaus(gaus_list, gaus_to_be_removed);
                sum_list = remove_from_linked_list_of_sum(sum_list, sum_to_be_removed);
                continue;       /* with next gaus */
                }


            /* The determinant of the covariance matrix equals
             * the product of the covariance matrix's eigenvalues.
             */
            det = 1.0;
            for (i=1; i <= gaus->dim; i++)
                det *= gaus->eig_val[i];
            if (det > 0.0)
                {
                gaus->sqrt_det = sqrt(det);
                find_inverse(gaus->covar, gaus->inverse_covar, gaus->dim);
                set_running_totals_to_zero(sum);
                gaus=gaus->next;
                sum=sum->next;
                }
            else
                {
                /* Degenerate gaussian. Remove it.
                 * We do this since otherwise we run into
                 * problems when calculating likelihoods
                 * relative to a degenerate gaussian.
                 */
                gaus_to_be_removed = gaus;
                sum_to_be_removed = sum;
                gaus=gaus->next;
                sum=sum->next;
                gaus_list = remove_from_linked_list_of_gaus(gaus_list, gaus_to_be_removed);
                sum_list = remove_from_linked_list_of_sum(sum_list, sum_to_be_removed);
                }
            }   /* end of recalculating eigenvalues etc. loop */

        }   /* end of iteration loop */


    /* Count the number of remaining gaussians.
     */
    num_gaus_remaining = 0;
    if ((gaus_list != NULL) && (gaus_list->head != NULL))
        for (gaus=gaus_list->head; gaus != NULL; gaus=gaus->next)
            num_gaus_remaining++;




    /* Tell them that we've finished.
     */
    if (num_gaus_remaining == 1)
        sprintf(str, "E.M. algorithm complete - 1 gaussian remaining\n");
    else
        sprintf(str, "E.M. algorithm complete - %d gaussians remaining\n", num_gaus_remaining);

    /* Determine width, ascent and descent
     * of string (in pixels).
     */
    XTextExtents(font_info, str, strlen(str), &direction, &ascent, &descent, &overall);

    x = 10;
    y = 20 + ascent;
    XSetForeground(gDisplay, theGC, theWhitePixel);
    XFillRectangle(gDisplay, theWindow, theGC, x-1, y-ascent-1, overall.width+2, ascent+descent+2);
    /* Text in red
     */
    XSetForeground(gDisplay, theGC, gRandomColors[0].pixel);
    XDrawString(gDisplay, theWindow, theGC, x, y, str, strlen(str));
    XFlush(gDisplay);

    free_linked_list_of_sum(sum_list);

    if (font_info != NULL)
        XFreeFont(gDisplay, font_info);

    return(gaus_list);

}   /* end of em_algorithm() */



SUM *
alloc_sum(int dim)
{
    SUM *sum;
    double  **alloc_array(int row, int column);

    sum = (SUM *)malloc(sizeof(SUM));
    if (sum == NULL)
        {
        fprintf(stderr, "malloc failed in alloc_sum() in em_algorithm.c\n");
        exit(1);
        }
    sum->dim = dim;
    sum->sum_wx = alloc_vector_MACRO(dim);
    sum->sum_wxy = alloc_array(dim, dim);
    if ((sum->sum_wx == NULL) || (sum->sum_wxy == NULL))
        {
        fprintf(stderr, "malloc failed in alloc_sum() in em_algorithm.c\n");
        exit(1);
        }

    return(sum);

}   /* end of alloc_sum() */



void
free_sum(SUM *sum)
{
    void    free_array(double **A, int column);

    if (sum != NULL)
        {
        if (sum->sum_wx != NULL)
            free(sum->sum_wx);
        if (sum->sum_wxy != NULL)
            free_array(sum->sum_wxy, sum->dim);
        free(sum);
        }

}   /* end of free_sum() */


SUM *
put_in_linked_list_of_sum(SUM *head, SUM *sum)
{
    SUM *ptr;

    if (head == NULL)
        head = sum;
    else
        {
        for (ptr=head; ptr->next != NULL; ptr=ptr->next)
            ;
        ptr->next = sum;
        }

    sum->next = NULL;

    return(head);


}   /* end of put_in_linked_list_of_sum() */



void
free_linked_list_of_sum(SUM *sum_head)
{
    SUM     *sum;
    void    free_sum(SUM *sum);

    while (sum_head != NULL)
        {
        sum = sum_head;
        sum_head = sum_head->next;
        free_sum(sum);
        }

}   /* end of free_linked_list_of_sum() */



SUM *
remove_from_linked_list_of_sum(SUM *sum_list, SUM *sum)
{
    SUM *s;
    void    free_sum(SUM *sum);

    if (sum != NULL)
        {
        if (sum_list == NULL)
            {
            fprintf(stderr, "Tried to remove a SUM from an empty linked list of SUM\n");
            fprintf(stderr, "in remove_from_linked_list_of_sum() in em_algorithm.c.\n");
            exit(1);
            }
        if (sum == sum_list)
            {
            sum_list = sum_list->next;
            free_sum(sum);
            }
        else
            {
            /* Find the previous element in the linked list.
             * (i.e. s->next == sum)
             */
            for (s=sum_list; s->next != NULL; s=s->next)
                if (s->next == sum)
                    break;
            if (s->next != NULL)
                {
                s->next = sum->next;
                free_sum(sum);
                }
            else    /* couldn't find it in list */
                {
                fprintf(stderr, "Couldn't find SUM to be removed from linked list\n");
                fprintf(stderr, "in remove_from_linked_list_of_sum() in em_algorithm.c\n");
                exit(1);
                }
            }
        }

    return(sum_list);

}   /* end of remove_from_linked_list_of_sum() */



GAUS_LIST *
init_gaussians(int num_gaus, POINT_LIST *point_list)
{
    GAUS    *gaus;
    GAUS_LIST   *gaus_list;
    int     i, j, dim;
    int     num_points, index;
    POINT   *point;
    GAUS    *alloc_gaus(int dim);
    GAUS_LIST   *put_in_linked_list_of_gaus(GAUS_LIST *gaus_list, GAUS *g);

    dim = point_list->head->dim;
    gaus_list = NULL;

    for (index=0; index < num_gaus; index++)
        {
        gaus = alloc_gaus(dim);
        gaus_list = put_in_linked_list_of_gaus(gaus_list, gaus);
        }


    /* Initialise weight, covar[][] mean[][],
     * inverse_covar[][], eig_val[], eig_vec[][]
     * and det
     * of the gaussians.
     */

    num_points = point_list->num_elements;

    for (gaus=gaus_list->head; gaus != NULL; gaus=gaus->next)
        {
        /* Initialise num_points, weights, det.
         */
        gaus->num_points = ((double )num_points) / (double )num_gaus;
        gaus->weight = 1.0 / (double )num_gaus;
        gaus->sqrt_det = 1.0;

        /* Initialise covar[][], inverse_covar[][], eig_vec[][] and eig_val[].
         */
        for (i=1; i <= dim; i++)
            {
            for (j=1; j <= dim; j++)
                {
                gaus->covar[i][j] = (i==j)?1.0:0.0;
                gaus->inverse_covar[i][j] = (i==j)?1.0:0.0;
                gaus->eig_vec[i][j] = (i==j)?1.0:0.0;
                }
            gaus->eig_val[i] = 1.0;
            }


        /* Initialise mean[] to co-ords of a randomly
         * chosen point.
         */
        index = (int )(drand48()*(double )num_points);
        for (i=0, point=point_list->head; i != index; i++, point=point->next)
            ;

        for (i=1; i <= dim; i++)
            {
            gaus->mean[i] = point->x[i];
            }
        }


    return(gaus_list);

}   /* end of init_gaussians() */


SUM *
init_sum_list(int num_gaus, int dim)
{
    SUM     *sum, *sum_list;
    SUM     *alloc_sum(int dim);
    SUM     *put_in_linked_list_of_sum(SUM *head, SUM *sum);
    int     i, j, k;


    sum_list = NULL;

    for (i=0; i < num_gaus; i++)
        {
        sum = alloc_sum(dim);
        sum->likelihood = 0.0;
        sum->sum_w = 0.0;
        for (j=1; j <= dim; j++)
            {
            sum->sum_wx[j] = 0.0;
            for (k=1; k <= dim; k++)
                sum->sum_wxy[j][k] = 0.0;
            }
        sum_list = put_in_linked_list_of_sum(sum_list, sum);
        }


    return(sum_list);

}   /* end of init_sum_list() */


void
set_running_totals_to_zero(SUM *sum)
{
    int i, j;

    sum->likelihood = 0.0;
    sum->sum_w = 0.0;
    for (i=1; i <= sum->dim; i++)
        {
        sum->sum_wx[i] = 0.0;
        for (j=1; j <= sum->dim; j++)
            sum->sum_wxy[i][j] = 0.0;
        }

}   /* end of set_sum_to_zero() */


GAUS_LIST *
fill_out_gaus_list(GAUS_LIST *gaus_list)
{
    GAUS    *gaus, *gaus_to_be_removed;
    double  det;
    double  num_gaus;
    int     find_eigenvectors_real_symmetric_matrix(double **A, double *Eig_val, double **Eig_vec, int dim);
    void    find_inverse(double **MAT, double **I, int dim);
    GAUS_LIST   *remove_from_linked_list_of_gaus(GAUS_LIST *gaus_list, GAUS *gaus);
    int     return_val;


    num_gaus = 0.0;
    gaus = gaus_list->head;
    while (gaus != NULL)
        {
        return_val = find_eigenvectors_real_symmetric_matrix(gaus->covar, gaus->eig_val, gaus->eig_vec, gaus->dim);
        if (return_val == -1)
            {
            fprintf(stderr, "QR algorithm failed to converge in find_eigenvectors_real_symmetric_matrix()\n");
            fprintf(stderr, "called from fill_out_gaus_list() in em_algorithm.c\n");
            fprintf(stderr, "Removing gaussian\n");
            gaus_to_be_removed = gaus;
            gaus=gaus->next;
            gaus_list = remove_from_linked_list_of_gaus(gaus_list, gaus_to_be_removed);
            continue;       /* with next gaus */
            }

        /* The determinant of the covariance matrix equals
         * the product of the covariance matrix's eigenvalues.
         */
        det = gaus->eig_val[1]*gaus->eig_val[2];
        if (det > 0.0)
            {
            num_gaus++;
            gaus->sqrt_det = sqrt(det);
            find_inverse(gaus->covar, gaus->inverse_covar, gaus->dim);
            gaus=gaus->next;
            }
        else
            {
            /* Degenerate gaussian. Remove it.
             * We do this since otherwise we run into
             * problems when calculating likelihoods
             * relative to a degenerate gaussian.
             */
            gaus_to_be_removed = gaus;
            gaus=gaus->next;
            gaus_list = remove_from_linked_list_of_gaus(gaus_list, gaus_to_be_removed);
            }
        }   /* end of 'while (gaus != NULL)' */


    /* Assign an equal weight to each gaussian.
     */
    for (gaus=gaus_list->head; gaus != NULL; gaus=gaus->next)
        gaus->weight = 1.0/num_gaus;


    return(gaus_list);

}   /* end of fill_out_gaus_list() */


void
print_gaus(GAUS *gaus)
{
    int     i, j;


    printf("num_points: %lf\n", gaus->num_points);
    printf("mean: ");
    for (i=1; i <= gaus->dim; i++)
        printf("%lf ", gaus->mean[i]);
    printf("\n");
    printf("eig_val: ");
    for (i=1; i <= gaus->dim; i++)
        printf("%lf ", gaus->eig_val[i]);
    printf("\n");
    printf("---\n");
    printf("covar: ");
    for (i=1; i <= gaus->dim; i++)
        {
        for (j=1; j <= gaus->dim; j++)
            printf("%lf ", gaus->covar[i][j]);
        printf("\n");
        }
    printf("---\n");

    printf("---\n\n\n\n");


}   /* end of print_gaus() */
