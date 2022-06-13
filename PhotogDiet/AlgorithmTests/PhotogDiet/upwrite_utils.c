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
#include <malloc.h>
#include <float.h>     /* MAXDOUBLE */
/*#include <X11/Xlib.h>
#include <X11/Xutil.h>*/
#include "upwrite.h"



/* Structure definitions.
 */
struct gaus_dist {
    GAUS    *gaus;
    double  dist;
};


typedef struct {
    int x, y;   /* size of the array. */
    int **val;  /* 2-D array of int */
} TAG_ARRAY;





/* Function Prototypes
 */
POINT   *upwrite_point_list_to_a_point(POINT_LIST *point_list);
GAUS    *fit_gaussian_to_a_point_list(POINT_LIST *point_list);
POINT   *convert_gaussian_to_a_point(GAUS *gaus);
GAUS    *convert_point_to_a_gaussian(POINT *point);
POINT_LIST  *convert_gaus_list_to_points_list(GAUS_LIST *gaus_list);
void    calculate_covariance_matrix_of_points(GAUS *gaus, POINT_LIST *point_list);
GAUS    *fit_a_single_gaus_to_list_of_gaus_means(GAUS_LIST *gaus_list);
double  *find_third_order_moments_of_list_of_2D_gaus_means(GAUS_LIST *gaus_list);

int     find_eigenvectors_real_symmetric_matrix(double **A, double *Eig_val, double **Eig_vec, int dim);
int     QR_algorithm(double **A, double **eig_vec, int dim);
void    tridiagonalize_symmetric_matrix(double **A, double **Q, int dim);
void    descending_order_eig(double *Eig_val, double **Eig_vec, int n);
void    find_eigenvectors_2x2_positive_semi_def_real_matrix(double **covar, double *eig_val, double **eig_vec);
void    find_biggest_entry_2x2(double m[][3], int *i, int *j);
GAUS_LIST   *threshold_eigenvalues(GAUS_LIST *gaus_list);
void    assign_pixels_to_gaus_2D(IMAGE *aImage, GAUS_LIST *gaus_list);
POINT_LIST  *model_chunks(GAUS_LIST *chunk_list);
//GAUS_ARRAY  *model_image_locally(IMAGE *aImage, Window theWindow, GC theGC);
GAUS_ARRAY  *model_image_locally(IMAGE *aImage);
GAUS    *compute_local_model(int x, int y, double radius, IMAGE *aImage);
//void    estimate_curvature(GAUS_ARRAY *gaus_array, double radius, IMAGE *aImage, Window theWindow, GC theGC);
void    estimate_curvature(GAUS_ARRAY *gaus_array, double radius, IMAGE *aImage);
double  curvature_at_a_gaus(int x, int y, GAUS_ARRAY *gaus_array, double radius);
double  angle_difference(double theta1, double theta2);
double  position_difference(double x, double y, GAUS *gaus);
//void    chunk_local_models(GAUS_ARRAY *gaus_array, XImage *aXImage);
void    chunk_local_models(GAUS_ARRAY *gaus_array);
BOOLEAN gaus_similar(GAUS *g1, GAUS *g2, double *d);
TAG_ARRAY   *alloc_tag_array(int x, int y);
void    free_tag_array(TAG_ARRAY *tag);
void    tag_chunk(int x, int y, int tag, GAUS_ARRAY *gaus_array, TAG_ARRAY *tag_array, int *x_stack, int *y_stack);
void    compute_mean_of_chunk(GAUS_LIST *chunk, int *x, int *y);
POINT   *compute_up_to_second_order_geometric_moments(GAUS_LIST *gaus_list);
CHUNK_LIST  *group_chunks(CHUNK_LIST **ptr_chunk_list);
BASIS       *compute_local_metric_of_nearby_chunks(CHUNK *chunk,
                                        CHUNK_LIST *chunk_list);



POINT *
upwrite_point_list_to_a_point(POINT_LIST *point_list)
{
    POINT   *point, *p;
    int     dim1, dim2, i, j, k;
    double  num_points;
    double  *mean;
    double  **covar, **sqrt_covar;

    double  **alloc_array(int row, int column);
    void    free_array(double **A, int dim);
    POINT   *alloc_point(int dim);
    void    sqrt_of_square_matrix(double **A, double **Sqrt_A, int dim);


    /* Empty list.
     */
    if ((point_list == NULL) || (point_list->num_elements == 0))
        return(NULL);


    /* Calculate mean and covariance matrix of points
     * in linked list.
     */

    /* Allocate memory
     */
    mean = alloc_vector_MACRO(point_list->head->dim);
    covar = alloc_array(point_list->head->dim, point_list->head->dim);
    sqrt_covar = alloc_array(point_list->head->dim, point_list->head->dim);
    if ((mean == NULL) || (covar == NULL) || (sqrt_covar == NULL))
        {
        fprintf(stderr, "Malloc failed in upwrite_point_list_to_a_point() in upwrite_utils.c\n"); 
        exit(1);
        }

    /* Initialise num_points, mean and covariance matrix.
     */
    for (i=1; i <= point_list->head->dim; i++)
        mean[i] = 0.0;
    for (i=1; i <= point_list->head->dim; i++)
        for (j=1; j <= point_list->head->dim; j++)
            covar[i][j] = 0.0;
        
    /* Calculate num_points, mean and covariance matrix.
     */
    for (p=point_list->head; p != NULL; p=p->next)
        {
        for (i=1; i <= p->dim; i++)
            mean[i] += p->x[i];
        }
    num_points = (double )(point_list->num_elements);

    for (i=1; i <= point_list->head->dim; i++)
        mean[i] /= num_points;

    if (point_list->num_elements > 1)
        {
        for (p=point_list->head; p != NULL; p=p->next)
            for (i=1; i <= p->dim; i++)
                for (j=1; j <= p->dim; j++)
                    covar[i][j] += (p->x[i]-mean[i])*(p->x[j]-mean[j]);
        for (i=1; i <= point_list->head->dim; i++)
            for (j=1; j <= point_list->head->dim; j++)
                covar[i][j] /= (num_points - 1.0);
        }
    sqrt_of_square_matrix(covar, sqrt_covar, point_list->head->dim);



    /* Calculate required dimension of point.
     *   number of points (1 number)
     *                              0th order moment
     *      +
     *   mean of points (dim1 numbers)
     *                              1st order moment
     *      +
     *   square root of covariance matrix of points
     *                    (number of entries in a dim1 x dim1
     *                    symmetric matrix. We will record the upper right
     *                    triangle of the matrix i.e.  __     __
     *                                                |* * * * *|
     *                                                |  * * * *|
     *                                                |    * * *|
     *                                                |      * *|
     *                                                |        *|
     *                                                 --     --
     *                              2nd order moment
     */

    dim1 = point_list->head->dim;

    dim2 = 1 + dim1 + (dim1 * (dim1+1))/2;


    point = alloc_point(dim2);


    /* Store number of points in list as first entry
     */
    point->x[1] = num_points;

    /* Store mean as next dim1 entries.
     */
    for (i=1; i <= dim1; i++)
        point->x[i+1] = mean[i];

    /* Store covariance matrix entries as the
     * remaining co-ords.
     */
    i = dim1+2;
    for (j=1; j <= dim1; j++)        /* column */
        for (k=j; k <= dim1; k++)    /* row */
            point->x[i++] = sqrt_covar[j][k];



    free(mean);
    free_array(covar, dim1);
    free_array(sqrt_covar, dim1);


    return(point);

}   /* end of upwrite_point_list_to_a_point() */



GAUS *
fit_gaussian_to_a_point_list(POINT_LIST *point_list)
{
    POINT   *p;
    int     i, j;
    double  num_points;
    GAUS    *gaus;
    GAUS    *alloc_gaus(int dim);


    /* Empty list.
     */
    if ((point_list == NULL) || (point_list->num_elements == 0))
        return(NULL);


    /* Calculate num_points in linked list
     * and mean and covariance matrix of points
     * in linked list.
     */

    /* Allocate memory
     */
    gaus = alloc_gaus(point_list->head->dim);

    /* Initialise num_points, mean and covariance matrix.
     */
    for (i=1; i <= point_list->head->dim; i++)
        gaus->mean[i] = 0.0;
    for (i=1; i <= point_list->head->dim; i++)
        for (j=1; j <= point_list->head->dim; j++)
            gaus->covar[i][j] = 0.0;
        
    /* Calculate num_points, mean and covariance matrix.
     */
    for (p=point_list->head; p != NULL; p=p->next)
        {
        for (i=1; i <= p->dim; i++)
            gaus->mean[i] += p->x[i];
        }

        num_points = (double )(point_list->num_elements);

        for (i=1; i <= point_list->head->dim; i++)
            gaus->mean[i] /= num_points;


    gaus->num_points = num_points;


    if (point_list->num_elements > 1)
        {
        for (p=point_list->head; p != NULL; p=p->next)
            for (i=1; i <= p->dim; i++)
                for (j=1; j <= p->dim; j++)
                    gaus->covar[i][j] += (p->x[i]-gaus->mean[i]) * (p->x[j]-gaus->mean[j]);
        for (i=1; i <= point_list->head->dim; i++)
            for (j=1; j <= point_list->head->dim; j++)
                gaus->covar[i][j] /= (num_points - 1.0);
        }


    return(gaus);

}   /* end of fit_gaussian_to_a_point_list() */


POINT *
convert_gaussian_to_a_point(GAUS *gaus)
{
    POINT   *point;
    int     dim1, dim2, i, j, k;
    double  **sqrt_covar;

    double  **alloc_array(int row, int column);
    void    free_array(double **A, int dim);
    POINT   *alloc_point(int dim);
    void    sqrt_of_square_matrix(double **A, double **Sqrt_A, int dim);


    if (gaus == NULL)
        return(NULL);


    /* Allocate memory
     */
    sqrt_covar = alloc_array(gaus->dim, gaus->dim);
    if (sqrt_covar == NULL)
        {
        fprintf(stderr, "Malloc failed in convert_gaussian_to_a_point() in upwrite_utils.c\n"); 
        exit(1);
        }


    sqrt_of_square_matrix(gaus->covar, sqrt_covar, gaus->dim);



    /* Calculate required dimension of point.
     *   number of points (1 number)
     *                              0th order moment
     *      +
     *   mean of gaussian (dim1 numbers)
     *                              1st order moment
     *      +
     *   square root of covariance matrix of gaussian
     *                    (number of entries in a dim1 x dim1
     *                    symmetric matrix. We will record the upper right
     *                    triangle of the matrix i.e.  __     __
     *                                                |* * * * *|
     *                                                |  * * * *|
     *                                                |    * * *|
     *                                                |      * *|
     *                                                |        *|
     *                                                 --     --
     *                              2nd order moment
     */

    dim1 = gaus->dim;

    dim2 = 1 + dim1 + (dim1 * (dim1+1))/2;


    point = alloc_point(dim2);


    /* Store number of points in list as first entry
     */
    point->x[1] = gaus->num_points;

    /* Store mean as next dim1 entries.
     */
    for (i=1; i <= dim1; i++)
        point->x[i+1] = gaus->mean[i];

    /* Store covariance matrix entries as the
     * remaining co-ords.
     */
    i = dim1+2;
    for (j=1; j <= dim1; j++)        /* column */
        for (k=j; k <= dim1; k++)    /* row */
            point->x[i++] = sqrt_covar[j][k];



    free_array(sqrt_covar, dim1);


    return(point);

}   /* end of convert_gaussian_to_a_point() */


GAUS *
convert_point_to_a_gaussian(POINT *point)
{
    GAUS    *gaus;
    double  dp, dg;
    int     i, j, k;
    double  **sqrt_covar;

    double  **alloc_array(int row, int column);
    void    free_array(double **A, int dim);
    GAUS    *alloc_gaus(int dim);
    void    square_matrix_multiply_square_matrix(double **A, double **B, double **RESULT, int dim);


    if (point == NULL)
        return(NULL);


    /* Calculate dim of required gaussian
     * Note: Look at convert_gaussian_to_a_point()
     *       to see how the gaussian is written to a point.
     *
     * Note: I'll use the abbreviations:
     *             dp = dim_of_point
     *             dg = dim_of_gaus
     *
     * The point contains info. about the gaussian in the following form:
     * The first co-ord is the number of points the gaussian is modelling
     * (i.e. zeroth order moment).
     * The next dg co-ords contain the mean.
     *
     * The rest of the co-ords contain the entries in the
     * square-root of the covariance matrix.
     * There are [dg * (dg+1)] / 2 of these entries.
     *
     * so in total dp = 1 + dg + [dg*(dg+1)/2]
     * i.e. dp =  1 * (dg^2)  +  3 * dg  +  1
     *           ---            ---
     *            2              2
     *
     * i.e.  1 * (dg^2)  +  3 * dg  +  (1-dp) = 0
     *      ---            ---
     *       2              2
     *
     * Solve this quadratic and choose the positive root,
     * will give us the value fo dg.
     */


    dp = (double )point->dim;

    /*    -b + sqrt(b^2 - 4ac) / (2a)
     * =  -3/2 + sqrt[ (3/2)^2 - 4 * 0.5 * (1-dp)] / (2*0.5)
     * =  -1.5 + sqrt[  2.25   - 2 * (1-dp) ] / 1
     */
    dg = (-1.5 + sqrt(2.25 - 2.0*(1-dp)));

    gaus = alloc_gaus((int )dg);


    /* Number of points.
     */
    gaus->num_points = point->x[1];


    /* Mean
     */
    for (i=1; i <= gaus->dim; i++)
        gaus->mean[i] = point->x[i+1];



    sqrt_covar = alloc_array(gaus->dim, gaus->dim);
    if (sqrt_covar == NULL)
        {
        fprintf(stderr, "Malloc failed in convert_point_to_a_gaussian() in upwrite_utils.c\n"); 
        exit(1);
        }

    /* Fill out upper triangle of square root of covariance matrix.
     *                                                 --     --
     *                                                |* * * * *|
     *                                                |  * * * *|
     *                                                |    * * *|
     *                                                |      * *|
     *                                                |        *|
     *                                                 --     --
     */
    i = gaus->dim+1;
    for (j=1; j <= gaus->dim; j++)
        for (k=j; k <= gaus->dim; k++)
            sqrt_covar[j][k] = point->x[i++];

    /* Fill out the rest of the matrix.
     */
    for (j=1; j <= gaus->dim; j++)
        for (k=1; k < j; k++)
            sqrt_covar[j][k] = sqrt_covar[k][j];

    /* Calculate covariance matrix of gaussian.
     */
    square_matrix_multiply_square_matrix(sqrt_covar, sqrt_covar, gaus->covar, gaus->dim);


    free_array(sqrt_covar, gaus->dim);


    return(gaus);

}   /* end of convert_point_to_a_gaussian() */



POINT_LIST *
convert_gaus_list_to_points_list(GAUS_LIST *gaus_list)
{
    GAUS    *gaus;
    POINT   *point;
    POINT_LIST  *point_list;
    POINT   *convert_gaussian_to_a_point(GAUS *gaus);
    POINT_LIST  *put_in_linked_list_of_point(POINT_LIST *point_list, POINT *p);


    point_list = NULL;

    for (gaus=gaus_list->head; gaus != NULL; gaus=gaus->next)
        {
        point = convert_gaussian_to_a_point(gaus);
        point_list = put_in_linked_list_of_point(point_list, point);
        }


    return(point_list);

}   /* end of convert_gaus_list_to_points_list() */



void
calculate_covariance_matrix_of_points(GAUS *gaus, POINT_LIST *point_list)
{
    int     i, j;
    double  num_points;
    POINT   *p;

    if (gaus == NULL)
        {
        fprintf(stderr, "passed a null pointer for arguement GAUS *gaus in calculate_covariance_matrix_of_points() in upwrite_utils.c.\n");
        exit(1);
        }

    for (i=1; i <= point_list->head->dim; i++)
        for (j=1; j <= point_list->head->dim; j++)
            gaus->covar[i][j] = 0.0;


    /* If no data points then return with zero matrix as
     * covariance matrix.
     */
    if (point_list->num_elements == 0)
        return;


    num_points = (double )(point_list->num_elements);

    for (p=point_list->head; p != NULL; p=p->next)
        {
        for (i=1; i <= p->dim; i++)
            for (j=1; j <= p->dim; j++)
                gaus->covar[i][j] += (p->x[i]-gaus->mean[i]) * (p->x[j]-gaus->mean[j]);
        }

    for (i=1; i <= point_list->head->dim; i++)
        for (j=1; j <= point_list->head->dim; j++)
            gaus->covar[i][j] /= (num_points-1.0);

    gaus->num_points = num_points;


}   /* end of calculate_covariance_matrix_of_points() */



GAUS *
fit_a_single_gaus_to_list_of_gaus_means(GAUS_LIST *gaus_list)
{
    GAUS    *gaus, *g;
    int     i, j;
    GAUS    *alloc_gaus(int dim);


    gaus = alloc_gaus(gaus_list->head->dim);


    /* Initialise
     */
    gaus->num_points = 0.0;
    for (i=1; i <= gaus->dim; i++)
        gaus->mean[i] = 0.0;
    for (i=1; i <= gaus->dim; i++)
        for (j=1; j <= gaus->dim; j++)
            gaus->covar[i][j] = 0.0;


    /* Calculate mean
     */
    for (g=gaus_list->head; g != NULL; g=g->next)
        {
        for (i=1; i <= gaus->dim; i++)
            gaus->mean[i] += g->mean[i];

        }
    gaus->num_points = (double )(gaus_list->num_elements);

    if (gaus->num_points > 0.0)
        for (i=1; i <= gaus->dim; i++)
            gaus->mean[i] /= gaus->num_points;


    /* Calculate covariance matrix
     */
    if (gaus->num_points > 1.0)
        {
        for (g=gaus_list->head; g != NULL; g=g->next)
            for (i=1; i <= gaus->dim; i++)
                for (j=1; j <= gaus->dim; j++)
                    gaus->covar[i][j] += (g->mean[i] - gaus->mean[i])*(g->mean[j] - gaus->mean[j]);

        for (i=1; i <= gaus->dim; i++)
            for (j=1; j <= gaus->dim; j++)
                gaus->covar[i][j] /= (gaus->num_points-1.0);
        }


    return(gaus);

}   /* end of fit_a_single_gaus_to_list_of_gaus_means() */



double *
find_third_order_moments_of_list_of_2D_gaus_means(GAUS_LIST *gaus_list)
{
    int     i;
    double  num_gaus;
    double  *mean, *thrd_ordr_mmnts;
    GAUS    *gaus;


    /* Allocate memory, initialise stuff to zero.
     */
    mean = alloc_vector_MACRO(2);
    thrd_ordr_mmnts = alloc_vector_MACRO(4);
    num_gaus = 0.0;
    mean[1] = 0.0;
    mean[2] = 0.0;

    /* Calculate mean
     */
    for (gaus=gaus_list->head; gaus != NULL; gaus=gaus->next)
        {
        mean[1] += gaus->mean[1];
        mean[2] += gaus->mean[2];
        }
    num_gaus = (double )(gaus_list->num_elements);
    mean[1] /= num_gaus;
    mean[2] /= num_gaus;

    for (i=1; i <= 4; i++)
        thrd_ordr_mmnts[i] = 0;

    for (gaus=gaus_list->head; gaus != NULL; gaus=gaus->next)
        {
        thrd_ordr_mmnts[1] += (gaus->mean[1]-mean[1])*
                                (gaus->mean[1]-mean[1])*
                                (gaus->mean[1]-mean[1]);
        thrd_ordr_mmnts[2] += (gaus->mean[1]-mean[1])*
                                (gaus->mean[1]-mean[1])*
                                (gaus->mean[2]-mean[2]);
        thrd_ordr_mmnts[3] += (gaus->mean[1]-mean[1])*
                                (gaus->mean[2]-mean[2])*
                                (gaus->mean[2]-mean[2]);
        thrd_ordr_mmnts[4] += (gaus->mean[2]-mean[2])*
                                (gaus->mean[2]-mean[2])*
                                (gaus->mean[2]-mean[2]);
        }

    for (i=1; i <= 4; i++)
        thrd_ordr_mmnts[i] /= (num_gaus-1.0);


    return(thrd_ordr_mmnts);

}   /* end of find_third_order_moments_of_list_of_gaus_means() */



int
find_eigenvectors_real_symmetric_matrix(double **A, double *Eig_val, double **Eig_vec, int dim)
{
    int     i, j;
    double  **e_value;
    double  **alloc_array(int row, int column);
    void    free_array(double **A, int row);
    void    tridiagonalize_symmetric_matrix(double **A, double **Q, int dim);
    int     QR_algorithm(double **A, double **eig_vec, int dim);
    void    descending_order_eig(double *Eig_val, double **Eig_vec, int n);
    int     return_val;



    e_value = alloc_array(dim, dim);
    if (e_value == NULL)
        {
        fprintf(stderr, "malloc() failed in find_eigenvectors_real_symmetric_matrix() in upwrite_utils.c\n");
        exit(1);
        }

    /* Copy A into e_value
     */
    for (i=1; i <= dim; i++)
        for (j=1; j <= dim; j++)
            e_value[i][j] = A[i][j];

    /* Let A be a matrix
     * Let A' be the tridiagonal form of A
     * such that A' = Q_t . A . Q   (Q_t is the transpose of Q)
     * n.b. Q is orthogonal, therefore Q_t equals inverse of Q.
     *
     * Note: In my comments, I am explaining things in terms
     * of A and Q, but I am passing e_value Eig_vec to the functions.
     * Remember that e_value was just a copy of A.
     * and Eig_vec takes the place of Q.
     */

    tridiagonalize_symmetric_matrix(e_value, Eig_vec, dim);


    /* The eigenvalues of A' are the eigenvalues of A.
     * We will find the eigenvectors of A'
     * Let x' be an eigenvector of A'
     * then Q.x' will be an eigenvector of A.
     *
     * Let lambda be the eigenvalue associated with x'
     * Let x = Q.x'     ( i.e. x' = Q_t.x)
     * then
     *                A'. x' = lambda.x'
     * i.e.     A' . (Q_t.x) = lambda.(Q_t.x)
     * i.e.  Q. A' . (Q_t.x) = Q.lambda.(Q_t.x)
     * i.e.   (Q.A'.Q_t) . x = lambda.x
     * i.e.                A = lambda.x
     * Hence x is the eigenvector associated
     * with the eigenvalue lambda.
     */

    return_val = QR_algorithm(e_value, Eig_vec, dim);

    if (return_val == 0)    /* Successfully converged */
        {
        /* At completion of QR_algorithm()
         * the eigenvalues are the diagonal elements of A[][]
         * and the eigenvectors are the column vectors of Q[][]
         */

        for (i=1; i <= dim; i++)
            Eig_val[i] = e_value[i][i];

        descending_order_eig(Eig_val, Eig_vec, dim);



        /* If eigenvalue is very close to zero,
         * set it to zero
         * set eigenvector to the zero vector.
         *
         */
        for (i=1; i <= dim; i++)
            if (fabs(Eig_val[i]) < A_VERY_SMALL_POSITIVE_VALUE)
                {
                Eig_val[i] = (double )0;

                /* Set the corresponding column vector to zero */
                for (j=1; j <= dim; j++)
                    Eig_vec[j][i] = (double )0;
                }
        }
    else    /* QR algorithm failed to converge. */
        {
        for (i=1; i <= dim; i++)
            {
            Eig_val[i] = (double )0;

            /* Set the corresponding column vector to zero */
            for (j=1; j <= dim; j++)
                Eig_vec[j][i] = (double )0;
            }
        }


    free_array(e_value, dim);

    return(return_val);

}   /* end of find_eigenvectors_real_symmetric_matrix() */



/* This function assumes that A is symmetric.
 * On completion, A contains the tridiagonalized matrix.
 * Q contains the transformation matrix.
 *
 * Tridiagonalization is achieved through a series of
 * transformations
 * P(1), P(2), ... P(dim-2)
 * where P(i) tridiagonalizes the ith row and column.
 * n.b. P is an orthogonal and symmetric matrix, thus:
 *      P = transpose of P = inverse of P
 *
 * Let    A = original matrix passed to function.
 *        A' = tridiagonal matrix
 *        P(n) = the nth transformation applied to A
 *        and Q = P(1).P(2)...P(dim-2)
 * then A' = P(dim-2).P(dim-1)...P(2).P(1). A .P(1).P(2)...P(dim-2)
 *                                (using the fact that P equals its transpose)
 * i.e. A' = Q_t . A . Q    (Q_t = transpose of Q)
 *
 * Alternatively, A = Q . A' . Q_t
 *
 * At the completion of this function, A' is left in A
 * and Q is left in the arguement Q.
 *
 */
void
tridiagonalize_symmetric_matrix(double **A, double **Q, int dim)
{
    double  **B, **P, **Q1, **Q2;
    double  *x, *u, sum, H, scale;
    int     i, j, k, flag, row, col;
    double  **alloc_array(int row, int column);
    void    free_array(double **A, int row);


    x = (double *)malloc(dim * sizeof(double));
    u = (double *)malloc(dim * sizeof(double));
    P = alloc_array(dim, dim);
    B = alloc_array(dim, dim);
    Q1 = alloc_array(dim, dim);
    Q2 = alloc_array(dim, dim);
    if ((x == NULL) || (u == NULL) || (P == NULL) || (B == NULL) || (Q1 == NULL) || (Q2 == NULL))
        {
        fprintf(stderr, "malloc failed in tridiagonalize_symmetric_matrix() in upwrite_utils.c\n");
        exit(1);
        }

    /* Initialise Q1 to the identity matrix.
     */
    for (j=1; j <= dim; j++)
        for (k=1; k <= dim; k++)
            if (j == k)
                Q1[j][k] = 1.0;
            else
                Q1[j][k] = 0.0;
    flag = 1;


    /* Each iteration of this loop
     * will tridiagonalize one column/row.
     */
    for (i=1; i < (dim-1); i++)
        {
        scale = 0.0;
        for (j=1; j <= (dim-i); j++)
            scale += fabs(A[(j+i)][i]);

        /* If column is all zeros,
         * then we can skip the transformation.
         */
        if (scale != 0.0)
            {

            /* Calculate u
             * We can scale the elements of u[] since
             * a Householder transformation only depends
             * upon the ratio of the elements.
             */
            sum = 0.0;
            for (j=1; j <= (dim-i); j++)
                {
                u[j] = A[(j+i)][i] / scale;
                sum += SQR(u[j]);
                }


                        /* make (+-sqrt(sum)) the same sign as u[1] */
            if (u[1] < 0.0)
                u[1] -= sqrt(sum);
            else
                u[1] += sqrt(sum);

            /* Calculate H */
            H = 0.0;
            for (j=1; j <= (dim-i); j++)
                H += SQR(u[j]);
            H /= 2.0;

            /* Calculate P
             * Note that the first i rows/columns of P have
             * 1.0 on the main diagonal and 0.0 off the diagonal.
             * e.g.  if i == 3
             *        ----          ----
             *       | 1  0  0  0  0  0 |
             *       | 0  1  0  0  0  0 |
             *       | 0  0  1  0  0  0 |
             *       | 0  0  0  x  x  x |   where x is some value
             *       | 0  0  0  x  x  x |
             *       | 0  0  0  x  x  x |
             *        ----          ----
             * Becaue of this, we can make some simplifications
             * when doing matrix multiplication with P.
             * Also, we don't bother actually setting the first
             * few columns/rows to 1.0 and 0.0 since we wont
             * actually use these in calculations.
             */
            for (j=i+1; j <= dim; j++)
                for (k=i+1; k <= dim; k++)
                    if (j==k)
                        P[j][k] = 1.0 - u[j-i]*u[k-i]/H;
                    else
                        P[j][k] = -u[j-i]*u[k-i]/H;

            /* Calculate
             *      A' = P.A.P
             * where A' is the new version of A.
             * In the matrix multiplications below, we have
             * made a few optimizations because of the
             * form of P and because the answer is copied
             * back into A.
             *
             *
             * First, matrix multiply P.A = B
             */
            for (row=1; row <= i; row++)
                for (col=i+1; col <= dim; col++)
                    B[row][col] = A[row][col];
            for (row=i+1; row <= dim; row++)
                for (col=1; col <= dim; col++)
                    {
                    B[row][col] = (double )0;
                    for (k=i+1; k <= dim; k++)
                        B[row][col] += P[row][k] * A[k][col];
                    }

            /* Second, matrix multiply B.P = A
             * We are using the fact that P = transpose of P
             */
            for (row=i+1; row <= dim; row++)
                for (col=1; col <= i; col++)
                    A[row][col] = B[row][col];
            for (row=1; row <= dim; row++)
                for (col=i+1; col <= dim; col++)
                    {
                    A[row][col] = (double )0;
                    for (k=i+1; k <= dim; k++)
                        A[row][col] += B[row][k] * P[k][col];
                    }

            /* Accumulate transformations in Q
             */
            if (i==1)
                {
                /* First time through, Q1 equals the identity matrix I.
                 *             I.P = P
                 * so just set Q1 to P and we save ourselves
                 * a matrix multiplication.
                 */
                for (j=i+1; j <= dim; j++)
                    for (k=i+1; k <= dim; k++)
                        Q1[j][k] = P[j][k];
                flag = 1;
                }
            else if (flag == 1)
                {
                /* Matrix multiply Q1.P = Q2
                 * We have used the form of P to save a few
                 * iterations in the matrix multiplication.
                 */
                for (row=1; row <= dim; row++)
                    for (col=1; col <= i; col++)
                        Q2[row][col] = Q1[row][col];
                for (row=1; row <= dim; row++)
                    for (col=i+1; col <= dim; col++)
                        {
                        Q2[row][col] = (double )0;
                        for (k=i+1; k <= dim; k++)
                            Q2[row][col] += Q1[row][k] * P[k][col];
                        }
                flag = 2;
                }
            else if (flag == 2)
                {
                /* Matrix multiply Q2.P = Q1
                 * We have used the form of P to save a few
                 * iterations in the matrix multiplication.
                 */
                for (row=1; row <= dim; row++)
                    for (col=1; col <= i; col++)
                        Q1[row][col] = Q2[row][col];
                for (row=1; row <= dim; row++)
                    for (col=i+1; col <= dim; col++)
                        {
                        Q1[row][col] = (double )0;
                        for (k=i+1; k <= dim; k++)
                            Q1[row][col] += Q2[row][k] * P[k][col];
                        }
                flag = 1;
                }
            }        /* end of if (scale != 0.0) */
        }        /* end of for (i=1; i < (dim-1); i++) */


    /* Copy the final transformation matrix into Q
     */
    if (flag == 1)
        {
        /* Q1 = P1.P2.P3...P(dim-2)
         */
        for (i=1; i <= dim; i++)
            for (j=1; j <= dim; j++)
                Q[i][j] = Q1[i][j];
        }
    else        /* (flag == 2) */
        {
        /* Q2 = P1.P2.P3...P(dim-2)
         */
        for (i=1; i <= dim; i++)
            for (j=1; j <= dim; j++)
                Q[i][j] = Q2[i][j];
        }

    free_array(B, dim);
    free_array(P, dim);
    free_array(Q1, dim);
    free_array(Q2, dim);
    free(x);
    free(u);

}   /* end of tridiagonalize_symmetric_matrix() */



/* This function assumes that A is a symmetric, tridiagonal matrix.
 * If A[][] was originally tridiagonal, then eig_vec[][] should be
 * set to the identity matrix when it is passed to QR_algorithm().
 *
 * If A was tridiagonalized by tridiagonalize_symmetric_matrix(),
 * then eig_vec[][] is the transformation matrix returned in the
 * arguement double **Q of 
 * tridiagonalize_symmetric_matrix(double **A, double **Q, int dim)
 *
 * At completion, A[][] is a diagonal matrix whose diagonal entries
 * are the eigenvalues of the original A[][].
 * The column vectors of eig_vec[][] are the eigenvectors of the
 * original A[][].
 */
int
QR_algorithm(double **A, double **eig_vec, int dim)
{
    double  *shift, **Q, **R, **P;
    double  e1, e2, a1, a2, a3;
    double  s, c, t, tmp1, tmp2;
    int     i, j, k, row, col, converge, count, p;
    double  **alloc_array(int row, int column);
    void    free_array(double **A, int row);


    shift = alloc_vector_MACRO(dim);
    R = alloc_array(dim, dim);
    Q = alloc_array(dim, dim);
    P = alloc_array(dim, dim);
    if ((shift == NULL) || (R == NULL) || (Q == NULL))
        {
        fprintf(stderr, "malloc failed in QR_algorithm() in upwrite_utils.c\n");
        exit(1);
        }

    for (i=dim; i > 1; i--)
        {
        if (fabs(A[i][i-1]) <= A_VERY_SMALL_POSITIVE_VALUE)
            {
            shift[i] = 0;       /* This element is already an eigenvalue
                                 * so skip this iteration of the loop.
                                 */
            }
        else
            {
            count=0;
            /* Find eigenvalues of bottom right 2x2 square.
             */
            a1 = 1;
            a2 = -(A[i-1][i-1] + A[i][i]);
            a3 = A[i-1][i-1]*A[i][i] - A[i-1][i]*A[i][i-1];
            e1 = (-a2 + sqrt(a2*a2 - 4*a1*a3)) / 2;
            e2 = (-a2 - sqrt(a2*a2 - 4*a1*a3)) / 2;
            /* Choose eigenvalue closest to element A[i][i]
             * as the value for the shift.
             */
            if (fabs(A[i][i]-e1) < fabs(A[i][i]-e2))
                shift[i] = e1;
            else
                shift[i] = e2;

            /* Shift the matrix.
             */
            for (j=1; j <= i; j++)
                A[j][j] -= shift[i];

            do
                {
                /* We are going to decompose A into two matrices Q, R
                 * such that:
                 *         A = Q.R
                 */

                /* Copy A into R.
                 * Note that we are working with a deflated
                 * version of A.
                 * i.e. A has dim eigenvalues
                 * We have found (dim-i) of them, so we can
                 * discard the last (dim-i) rows and columns of A.
                 * To find the remaining i eigenvalues, we just
                 * work with the deflated matrix, which is the
                 * first i rows and i columns.
                 */
                for (row=1; row <= i; row++)
                    for (col=1; col <= i; col++)
                        R[row][col] = A[row][col];


                /* Initialise Q to Identity matrix.
                 */
                for (j=1; j <= i; j++)
                    for (k=1; k <= i; k++)
                        if (j==k)
                            Q[j][k] = 1.0;
                        else
                            Q[j][k] = 0.0;


                /* Perform a plane rotation to zero the elements
                 * below the diagonal of R.
                 */
                for (p=1; p < i; p++)
                    {
                    /* Zero the element (p+1,p) immediately below
                     * the main diagonal
                     */
                    t = 1.0 / sqrt( SQR(R[p][p]) + SQR(R[p+1][p]) );
                    s = -t*R[p+1][p];
                    c = t*R[p][p];

                    /* Calculate P_t.R = R'
                     * where P is the matrix performing the plane rotation,
                     *       P_t is the inverse of P
                     *       R' is the new value for R
                     *           which has an extra zero element in it.
                     */
                    for (j=1; j <= i; j++)
                        {
                        tmp1 = c*R[p][j] - s*R[p+1][j];
                        tmp2 = s*R[p][j] + c*R[p+1][j];
                        R[p][j] = tmp1;
                        R[p+1][j] = tmp2;
                        }

                    /* Collect the plane rotations P1, P2, ... P(i-1)
                     * These form Q.
                     * Q = P1.P2...P(i-2).P(i-1)
                     */
                    for (j=1; j <= i; j++)
                        {
                        tmp1 = Q[j][p]*c - Q[j][p+1]*s;
                        tmp2 = Q[j][p]*s + Q[j][p+1]*c;
                        Q[j][p] = tmp1;
                        Q[j][p+1] = tmp2;
                        }

                    /* Build up eigenvectors
                     * We are working with P
                     * 
                     * let An be the nth matrix in the sequence generated
                     *        by the QR algorithm.
                     *     Q(n) is an orthogonal matrix.
                     *     Q(n)_t is the transpose of Q(n).
                     * 
                     * The sequence of matrices generated by QR algorithm is:
                     *     A1 = A
                     *     A2 = Q(1)_t.A1.Q(1)
                     *     A3 = Q(2)_t.A2.Q(2)
                     *       etc.
                     * we end up with
                     *     An = Q(n-1)_t.Q(n-2)_t...Q(1).A.Q(1)...Q(n-2).Q(n-1)
                     * thus
                     *     Q(1).Q(2).Q(3)...Q(n-2) are the eigenvectors of A.
                     * and each of these Q are just the plane rotation
                     * matrices multiplied together.
                     * We will record these in eig_vec[][].
                     * If A was originally a tridiagonal matrix, then
                     * eig_vec[][] should initially be the identity matrix.
                     * If A was put into tridiagonal form by the function
                     * tridiagonalize_symmetric_matrix(double **A, double **Q, int dim)
                     * then eig_vec[][] should be the matrix returned
                     * in the arguement Q of
                     * tridiagonalize_symmetric_matrix().
                     */
                    for (j=1; j <= dim; j++)
                        {
                        tmp1 = eig_vec[j][p]*c - eig_vec[j][p+1]*s;
                        tmp2 = eig_vec[j][p]*s + eig_vec[j][p+1]*c;
                        eig_vec[j][p] = tmp1;
                        eig_vec[j][p+1] = tmp2;
                        }
                    }



                /* Multiply R.Q
                 * to get next matrix in sequence.
                 */
                for (row=1; row <= i; row++)
                    for (col=1; col <= i; col++)
                        {
                        A[row][col] = (double )0;
                        for (j=1; j <= i; j++)
                            A[row][col] += R[row][j] * Q[j][col];
                        }



                if (fabs(A[i][i-1]) > 10e-4)
                    converge = FALSE;
                else
                    converge = TRUE;


                if (count++ > 1000)
                    {
                    /* Return with error.
                     * QR algorith failed to converge.
                     */
                    free(shift);
                    free_array(R, dim);
                    free_array(Q, dim);
                    free_array(P, dim);
                    return(-1);
                    }

                } while (converge == FALSE);
            }
        }       /* end of for (i=dim; i > 1; i--) */


    /* Undo the shifts.
     */
    for (i=dim; i > 1; i--)
        for (j=1; j <= i; j++)
            A[j][j] += shift[i];


    free(shift);
    free_array(R, dim);
    free_array(Q, dim);
    free_array(P, dim);


    return(0);      /* QR algorithm successfully converged. */

}   /* end of QR_algorithm() */




/* Place eigenvalues and corresponding eigenvectors
 * in order from largest to smallest.
 */
void
descending_order_eig(double *Eig_val, double **Eig_vec, int n)
{
    int k,j,i;
    double p;

    for (i=1; i < n; i++)
        {
        p = Eig_val[k=i];
        for (j=i+1; j <= n; j++)
            if (Eig_val[j] >= p) p = Eig_val[k=j];
        if (k != i)
            {
            Eig_val[k] = Eig_val[i];
            Eig_val[i] = p;
            for (j=1;j<=n;j++)
                {
                p = Eig_vec[j][i];
                Eig_vec[j][i] = Eig_vec[j][k];
                Eig_vec[j][k] = p;
                }
            }
        }
}   /* end of descending_order_eig() */



void
find_eigenvectors_2x2_positive_semi_def_real_matrix(double **covar, double *eig_val, double **eig_vec)
{
    double  a, b, c, d;
    int     i, j;
    double  tmp[3][3];
    void    find_biggest_entry_2x2(double A[][3], int *i, int *j);


    /* Find eigenvalues
     * Use characteristic equation.
     */
    a = 1;
    b = -covar[1][1] - covar[2][2];
    c = covar[1][1]*covar[2][2] - covar[1][2]*covar[2][1];

    d = SQR(b) - 4*a*c;
    if (d <= A_VERY_SMALL_POSITIVE_VALUE)
        {
        eig_val[1] = -b / 2.0;
        eig_val[2] = -b / 2.0;
        eig_vec[1][1] = 1.0;  eig_vec[1][2] = 0.0;
        eig_vec[2][1] = 0.0;  eig_vec[2][2] = 1.0;
        }
    else
        {
        eig_val[1] = (-b + sqrt(d)) / 2.0;
        eig_val[2] = (-b - sqrt(d)) / 2.0;

        /* Put eigenvalues in descending order.
         */
        if (eig_val[1] < eig_val[2])
            {
            d = eig_val[1];
            eig_val[1] = eig_val[2];
            eig_val[2] = d;
            }

        /* Find first eigenvector.
         */
        tmp[1][1] = covar[1][1] - eig_val[1];
        tmp[1][2] = covar[1][2];
        tmp[2][1] = covar[2][1];
        tmp[2][2] = covar[2][2] - eig_val[1];


        find_biggest_entry_2x2(tmp, &i, &j);
        if (j == 1)
            {
            eig_vec[2][1] = 1.0;
            eig_vec[1][1] = -tmp[i][2] / tmp[i][1];
            }
        else    /* j == 2 */
            {
            eig_vec[1][1] = 1.0;
            eig_vec[2][1] = -tmp[i][1] / tmp[i][2];
            }
        /* Normalise eigenvecotr.
         */
        d = sqrt(SQR(eig_vec[1][1]) + SQR(eig_vec[2][1]));
        eig_vec[1][1] /= d;
        eig_vec[2][1] /= d;


        /* Find secind eigenvector.
         */
        eig_vec[1][2] = -eig_vec[2][1];
        eig_vec[2][2] = eig_vec[1][1];

        }



}   /* end of find_eigenvectors_2x2_positive_semi_def_real_matrix() */



/* Find largest (absolute) entry in a 2x2 matrix.
 * Return co-ords of entry in (i,j) (row, column)
 */
void
find_biggest_entry_2x2(double m[][3], int *i, int *j)
{
    if ((ABS(m[1][1]) > ABS(m[1][2])) && (ABS(m[1][1]) > ABS(m[2][1])) && (ABS(m[1][1]) > ABS(m[2][2])))
        {
        *i = 1;
        *j = 1;
        }
    else if ((ABS(m[1][2]) > ABS(m[2][1])) && (ABS(m[1][2]) > ABS(m[2][2])))
        {
        *i = 1;
        *j = 2;
        }
    else if (ABS(m[2][1]) > ABS(m[2][2]))
        {
        *i = 2;
        *j = 1;
        }
    else
        {
        *i = 2;
        *j = 2;
        }

}   /* end of find_biggest_entry_2x2() */



/* Any eigenvalues that are less than a threshold value
 * are set to zero.
 * So are the corresponding eigenvectors.
 * If all eigenvalues are set to zero, then the
 * GAUS is removed from the linked list.
 *
 * Recall that each GAUS models a set of points which lie
 * on a manifold.
 * The aim of this function is to remove those eigenvalues
 * which correspond to noise, and only leave those
 * which model the mainifold.
 */
GAUS_LIST *
threshold_eigenvalues(GAUS_LIST *gaus_list)
{
    GAUS    *gaus, *gaus_to_be_removed;
    int     i, j;
    GAUS_LIST   *remove_from_linked_list_of_gaus(GAUS_LIST *gaus_list, GAUS *gaus);

    if (gaus_list == NULL)
        return(NULL);

    for (gaus=gaus_list->head; gaus != NULL; gaus=gaus->next)
        {
        /* Eigenvalues are listed in descending order,
         * so search for the first value less than
         * the threshold.
         */
        for (i=1; i <= gaus->dim; i++)
            if (sqrt(gaus->eig_val[i]) < gResolutionRadius/20.0)
                break;
        for ( ; i <= gaus->dim; i++)
            {
            gaus->eig_val[i] = (double )0;
            for (j=1; j <= gaus->dim; j++)
                gaus->eig_vec[j][i] = (double )0;
            }
        }

    /* Discard a gaus if all of its eigenvalues
     * were set to zero.
     */
    gaus = gaus_list->head;
    while (gaus != NULL)
        {
        if (gaus->eig_val[1] == (double )0)
            {
            gaus_to_be_removed = gaus;
            gaus=gaus->next;
            gaus_list = remove_from_linked_list_of_gaus(gaus_list, gaus_to_be_removed);
            }
        else    /* Go on to the next GAUS */
            gaus=gaus->next;
        }   /* end of 'while (gaus != NULL)' */


    return(gaus_list);

}   /* end of threshold_eigenvalues() */



/* Assign each black pixel in the image
 * to the nearest GAUS in gaus_list
 * (using euclidean metric).
 */
void
assign_pixels_to_gaus_2D(IMAGE *aImage, GAUS_LIST *gaus_list)
{
    int     x, y, x1, y1, x2, y2;
    double  dist;
    GAUS    *gaus;
    POINT   *point;
    struct gaus_dist    **array;
    int         get_pixel_value(IMAGE *aImage, int x, int y);
    POINT       *alloc_point(int dim);
    POINT_LIST  *put_in_linked_list_of_point(POINT_LIST *point_list, POINT *p);


    if (gaus_list == NULL)
        return;


    /* Alloc a 2-D array of (struct gaus_dist *).
     */
    array = (struct gaus_dist **)malloc(aImage->x * sizeof( struct gaus_dist *) );
    if (array == NULL)
        {
        fprintf(stderr, "malloc #1 failed in assign_pixels_to_gaus_2D() in upwrite_utils.c\n");
        exit(1);
        }

    for (x=0; x < (int)aImage->x; x++)
        {
        array[x] = (struct gaus_dist *)malloc(aImage->y * sizeof(struct gaus_dist) );
        if (array[x] == NULL)
            {
            fprintf(stderr, "malloc #2 failed in assign_pixels_to_gaus_2D() in upwrite_utils.c\n");
            exit(1);
            }
        }



    /* Initialise the array.
     */
    for (x=0; x < (int)aImage->x; x++)
        for (y=0; y < (int)aImage->y; y++)
            {
            array[x][y].gaus = (GAUS *)NULL;
            array[x][y].dist = DBL_MAX;
            }



    /* For each element of the array that corresponds
     * to a black pixel, record the closest GAUS.
     */
    for (gaus=gaus_list->head; gaus != NULL; gaus=gaus->next)
        {
        x1 = (int )(gaus->mean[1] - gResolutionRadius);
        x2 = (int )(gaus->mean[1] + gResolutionRadius) + 1;
        y1 = (int )(gaus->mean[2] - gResolutionRadius);
        y2 = (int )(gaus->mean[2] + gResolutionRadius) + 1;
        if (x1 < 0)
            x1 = 0;
        if (x2 >= (int)aImage->x)
            x2 = (int)aImage->x-1;
        if (y1 < 0)
            y1 = 0;
        if (y2 >= (int)aImage->y)
            y2 = (int)aImage->y-1;

        for (x=x1; x <= x2; x++)
            for (y=y1; y <= y2; y++)
                if (get_pixel_value(aImage,x,y) == BLACK)
                    {
                    dist = sqrt(SQR(x-gaus->mean[1]) + SQR(y-gaus->mean[2]));
                    if (dist < array[x][y].dist)
                        {
                        array[x][y].gaus = gaus;
                        array[x][y].dist = dist;
                        }
                    }
        }


    /* For each element of the array that corresponds
     * to a black pixel, record its coordinates
     * as part of the nearest GAUS.
     */
    for (x=0; x < (int)aImage->x; x++)
        for (y=0; y < (int)aImage->y; y++)
            if (array[x][y].gaus != NULL)
                {
                if (array[x][y].gaus == NULL)
                    {
                    fprintf(stderr, "Impossible\n");
                    exit(1);
                    }
                point = alloc_point(2);
                point->x[1] = (double )x;
                point->x[2] = (double )y;
                (array[x][y].gaus)->point_list = put_in_linked_list_of_point((array[x][y].gaus)->point_list, point);
                }



    /* Free memory.
     */
    for (x=0; x < (int)aImage->x; x++)
        free(array[x]);
    free(array);

}   /* end of assign_pixels_to_gaus_2D() */



POINT_LIST *
model_chunks(GAUS_LIST *chunk_list)
{
    GAUS_LIST   *chunk;
    GAUS    *gaus;
    POINT   *point, *upwritten_point;
    POINT_LIST  *point_list, *upwritten_point_list;
    POINT_LIST  *put_in_linked_list_of_point(POINT_LIST *point_list, POINT *p);
    void    free_linked_list_of_point(POINT_LIST *point_list);
    POINT   *copy_point(POINT *p);
    POINT   *upwrite_point_list_to_a_point(POINT_LIST *point_list);


    upwritten_point_list = NULL;

    for (chunk=chunk_list; chunk != NULL; chunk=chunk->next_list)
        {
        /* Collect all the points which generated
         * a chunk into one linked list, 'point_list'.
         */
        point_list = NULL;
        for (gaus=chunk->head; gaus != NULL; gaus=gaus->next)
            for (point=gaus->point_list->head; point != NULL; point=point->next)
                {
                point_list = put_in_linked_list_of_point(point_list, copy_point(point));
                }


        /* UpWrite 'point_list' to a single point.
         * and store it in a linked list of
         * UpWritten points.
         */
        upwritten_point = upwrite_point_list_to_a_point(point_list);
        upwritten_point_list = put_in_linked_list_of_point(upwritten_point_list, upwritten_point);
        free_linked_list_of_point(point_list);
        }


    return(upwritten_point_list);

}   /* end of model_chunks() */



GAUS_ARRAY *
//model_image_locally(IMAGE *aImage, Window theWindow, GC theGC)
model_image_locally(IMAGE *aImage)
{
    int     x, y;
    GAUS_ARRAY  *gaus_array;
    GAUS_ARRAY  *alloc_gaus_array(int x, int y);
    int get_pixel_value(IMAGE *aImage, int x, int y);



    gaus_array = alloc_gaus_array(aImage->x, aImage->y);


    if (gGraphics == TRUE)
        {
//        XSetForeground(gDisplay, theGC, gRandomColors[2].pixel);
        }

    for (x=0; x < (int)aImage->x; x++)
        for (y=0; y < (int)aImage->y; y++)
            if (get_pixel_value(aImage, x, y) == BLACK)
                {
                if (gGraphics == TRUE)
                    {
//                    XDrawPoint(gDisplay, theWindow, theGC, x, y);
  //                  XFlush(gDisplay);
                    }
                gaus_array->gaus[x][y] = compute_local_model(x, y, gResolutionRadius, aImage);
                }

    return(gaus_array);

}   /* end of model_image_locally() */



/* Models the pixels within a neighbourhood of (x,y)
 * with a GAUS.
 * This function returns a pointer to the GAUS.
 * It returns NULL if there is less than two BLACK pixels
 * in the neighbourhood.
 *
 * This function allocates memory for the GAUS.
 */
GAUS *
compute_local_model(int x, int y, double radius, IMAGE *aImage)
{
    int x1, y1, x2, y2, i, j;
    double  sum_x, sum_y, num_points;
    GAUS    *gaus;
    int     get_pixel_value(IMAGE *aImage, int x, int y);
    GAUS    *alloc_gaus(int dim);
    void    free_gaus(GAUS *g);
    int     find_eigenvectors_real_symmetric_matrix(double **A, double *Eig_val, double **Eig_vec, int dim);
    int     return_val;


    gaus = NULL;

    /* Define a square around the point (x,y).
     * We only need to check pixels which lie in this square.
     */
    x1 = x - (int )radius;
    y1 = y - (int )radius;
    if (x1 < 0)
        x1 = 0;
    if (y1 < 0)
        y1 = 0;
    x2 = x + (int )radius;
    y2 = y + (int )radius;
    if (x2 >= (int)aImage->x)
        x2 = (int)aImage->x-1;
    if (y2 >= (int)aImage->y)
        y2 = (int)aImage->y-1;


    /* Calculate mean of points in ball
     */
    sum_x = 0.0;
    sum_y = 0.0;
    num_points = 0.0;

    for (i=x1; i <= x2; i++)
        for (j=y1; j <= y2; j++)
            if (get_pixel_value(aImage,i,j) == BLACK)
                if ((SQR(i-x) + SQR(j-y)) < SQR(radius))
                    {
                    sum_x += (double )i;
                    sum_y += (double )j;
                    num_points++;
                    }

    /* Need at least a total of 1 point for a gaussian
     * or the covariance formule doesn't make sense
     * since it has a (num_points-1) in the denominator.
     */
    if (num_points >= 2.0)
        {
        gaus = alloc_gaus(2);

        gaus->num_points = num_points;
        gaus->mean[1] = sum_x / num_points;
        gaus->mean[2] = sum_y / num_points;


        /* Calculate covariance matrix
         */
        gaus->covar[1][1] = 0.0;  gaus->covar[1][2] = 0.0;
        gaus->covar[2][1] = 0.0;  gaus->covar[2][2] = 0.0;

        for (i=x1; i <=x2; i++)
            for (j=y1; j <=y2; j++)
                if (get_pixel_value(aImage,i,j) == BLACK)
                    if ((SQR(i-x) + SQR(j-y)) < SQR(radius))
                        {
                        gaus->covar[1][1] += SQR(((double )i) - gaus->mean[1]);
                        gaus->covar[1][2] += (((double )i) - gaus->mean[1])*(((double )j) - gaus->mean[2]);
                        gaus->covar[2][2] += SQR(((double )j) - gaus->mean[2]);
                        }


        gaus->covar[1][1] /= num_points-1.0;
        gaus->covar[1][2] /= num_points-1.0;
        gaus->covar[2][2] /= num_points-1.0;
        gaus->covar[2][1] = gaus->covar[1][2];




        return_val = find_eigenvectors_real_symmetric_matrix(gaus->covar, gaus->eig_val, gaus->eig_vec, 2);
        if (return_val == -1)
            {
            fprintf(stderr, "QR algorithm failed to converge in find_eigenvectors_real_symmetric_matrix()\n");
            fprintf(stderr, "Exitting from compute_local_model() in upwrite_utils.c\n");
            exit(1);
            }

        /* If it doesn't have any non-zero eigenvalues,
         * then discard it (This should never occur since
         * we have already checked that there is more than
         * one point in the neighbourhood, which means that
         * there will be a non-zero eigenvalue).
         */
        if (gaus->eig_val[1] <= 0.0)
            {
            free_gaus(gaus);
            gaus = NULL;
            }

        }   /* end of 'if (num_points > 1.0)' */


    return(gaus);

}   /* end of compute_local_model() */



/* Estimates the curvture at each non-NULL entry in gaus_array.
 * i.e. for each gaus->array->gaus[x][y].
 * Places an estimate of the curvature in gaus->array->gaus[x][y]->value.
 */
void
//estimate_curvature(GAUS_ARRAY *gaus_array, double radius, IMAGE *aImage, Window theWindow, GC theGC)
estimate_curvature(GAUS_ARRAY *gaus_array, double radius, IMAGE *aImage)
{
    int     x, y;


//    XSetForeground(gDisplay, theGC, gRandomColors[3].pixel);


    for (x=0; x < gaus_array->x; x++)
        for (y=0; y < gaus_array->y; y++)
            if (gaus_array->gaus[x][y] != NULL)
                {
//                XDrawPoint(gDisplay, theWindow, theGC, x, y);
  //              XFlush(gDisplay);
                gaus_array->gaus[x][y]->value = curvature_at_a_gaus(x, y, gaus_array, radius);
                }


}   /* end of estimate_curvature() */


/* Compute curvature of the GAUS at gaus_array->gaus[x][y].
 * Store the curvature in gaus_array->gaus[x][y]->value.
 *
 * We will refer to gaus_array->gaus[x][y] as 'central_gaus'
 */
double
curvature_at_a_gaus(int x, int y, GAUS_ARRAY *gaus_array, double radius)
{
    int     x1, y1, x2, y2, i, j;
    double  central_theta, theta;
    double  sum1, sum2, curv, dx, dtheta;
    GAUS    *central_gaus;
    double  angle_difference(double theta1, double theta2);
    double  position_difference(double x, double y, GAUS *gaus);



    /* Compute angle of central gaus.
     */
    central_gaus = gaus_array->gaus[x][y];
    central_theta = atan2(central_gaus->eig_vec[2][1], central_gaus->eig_vec[1][1]);


    /* Define a square around the central_gaus
     * We only need to check gaus which lie in this square.
     */
    x1 = x - (int )radius;
    y1 = y - (int )radius;
    if (x1 < 0)
        x1 = 0;
    if (y1 < 0)
        y1 = 0;
    x2 = x + (int )radius;
    y2 = y + (int )radius;
    if (x2 >= gaus_array->x)
        x2 = gaus_array->x-1;
    if (y2 >= gaus_array->y)
        y2 = gaus_array->y-1;


    /* Consider all GAUS in a ball around the position of central_gaus.
     * For each gaus, record the difference in angle from central_theta
     * (dtheta)
     * and the distance from central_gaus (dx) along the
     * dominant eigenvector of central_gaus.
     * Assume that dtheta is proportional to dx.
     *     dtheta = curv . dx
     * Find the best estimate for curv.
     *
     * n.b. we only record the absolute value of dx.
     * i.e. we record how far away the gaus is, but don't
     * record if it is in the positive or negative direction
     * along the dominant eigenvector of central_gaus
     * since the dominant eigenvector of central_gaus
     * could be oriented arbitrarily in the positive or
     * negative directions.
     *
     * n.b. the distance is from the maen of the gaus_array->gaus[x[y],
     * not from the point (x,y). For most gaus (i.e. those not at the
     * end of a line segment), these should be very similar.
     */

    sum1 = 0;
    sum2 = 0;
    for (i=x1; i <= x2; i++)
        for (j=y1; j <= y2; j++)
            if (gaus_array->gaus[i][j] != NULL)
                if ((SQR(i-x) + SQR(j-y)) < SQR(radius))
                    {
                    theta = atan2(gaus_array->gaus[i][j]->eig_vec[2][1], gaus_array->gaus[i][j]->eig_vec[1][1]);
                    dtheta = angle_difference(theta, central_theta);
                    dx = position_difference(gaus_array->gaus[i][j]->mean[1],
                                            gaus_array->gaus[i][j]->mean[2],
                                            central_gaus);
                    sum1 += dx * dtheta;
                    sum2 += dx * dx;
                    }
    if (sum2 != 0)
        {
        curv = sum1 / sum2;
        }
    else
        {
        /* Only one gaus within the ball, which was
         * the GAUS whose curvature we were estimating.
         * i.e. dx = 0, dtheta = 0
         */
        curv = DBL_MAX;
        }



    return(curv);

}   /* end of curvature_at_a_gaus() */



/* Returns (theta1 - theta2)
 * as an angle in the range [0, M_PI_2]
 * 0, M_PI, 2*M_PI are all considered to represent
 * the same angle.
 *
 * theta1, theta2 are expected to be in radians.
 */
double
angle_difference(double theta1, double theta2)
{
    double  dtheta, phi, dt_prdct;

    /* 0, M_PI, 2*M_PI... are all considered to be the
     * same angle.
     * First we map each angle to a point in R^2
     *    theta -> ( cos(2*theta), sin(2*theta) )
     *
     * so 0, M_PI, 2*M_PI... map to the same point.
     *
     * Let theta1 map to v1,
     *     theta2 map to v2 where v1, v2 are vectors in R^2.
     *
     * Then cos(phi) = v1 . v2
     *                 -------
     *                 |v1||v2|
     * but |v1| == |v2| == 1
     * so this simplifies to
     *      cos(phi) = v1 . v2
     *
     * The angle (theta1 - theta2) will then equal phi
     *                                             ---
     *                                              2
     */

    dt_prdct =cos(2.0*theta1)*cos(2.0*theta2) + sin(2.0*theta1)*sin(2.0*theta2);

    /* In case of rounding error.
     */
    if (dt_prdct > 1.0)
        dt_prdct = 1.0;
    else if (dt_prdct < -1.0)
        dt_prdct = -1.0;

    phi = acos(dt_prdct);
    dtheta = phi / 2.0;


    return(dtheta);

}   /* end of angle_difference() */



/* Project the point (x,y) onto the 1-D subspace spanned by
 * the dominant eigenvector of (GAUS *)gaus.
 * Return the absolute value of the euclidean distance from
 * the mean of (GAUS *)gaus to this projection.
 *
 * (GAUS *) is assumed to be a 2-D GAUS (i.e. assume gaus->dim == 2)
 */
double
position_difference(double x, double y, GAUS *gaus)
{
    double  dist, norm_sqr;

    x = x - gaus->mean[1];
    y = y - gaus->mean[2];

    norm_sqr = SQR(gaus->eig_vec[1][1]) + SQR(gaus->eig_vec[2][1]);
    dist = (x*gaus->eig_vec[1][1] + y*gaus->eig_vec[2][1]) / norm_sqr;

    dist = ABS(dist);


    return(dist);

}   /* end of position_difference() */



/* Chunk the local models
 * and color the pixels in the image,
 * one color for each chunk.
 */
void
//chunk_local_models(GAUS_ARRAY *gaus_array, XImage *aXImage)
chunk_local_models(GAUS_ARRAY *gaus_array)
{
    int x, y;
    int tag;
    BOOLEAN gaus_similar(GAUS *g1, GAUS *g2, double *d);
    TAG_ARRAY   *tag_array;
    int     *x_stack, *y_stack;

    TAG_ARRAY   *alloc_tag_array(int x, int y);
    void    tag_chunk(int x, int y, int tag, GAUS_ARRAY *gaus_array, TAG_ARRAY *tag_array, int *x_stack, int *y_stack);
    void        free_tag_array(TAG_ARRAY *tag_array);
//    void        set_XImage_pixel_value(int x, int y, int color, XImage *aXImage);
    void        set_XImage_pixel_value(int x, int y, int color);




    /* Allocate memory to store the tags.
     */
    tag_array = alloc_tag_array(gaus_array->x, gaus_array->y);

    /* Allocate memory used by the function tag_chunk()
     */
    x_stack = (int *)malloc( tag_array->x*tag_array->y*sizeof(int) );
    y_stack = (int *)malloc( tag_array->x*tag_array->y*sizeof(int) );
    if ((x_stack == NULL) || (y_stack == NULL))
        {
        fprintf(stderr, "malloc failed in chunk_local_models() in upwrite_utils.c\n");
        exit(1);
        }


    /* Initialise all tags to -1
     */
    for (x=0; x < tag_array->x; x++)
        for (y=0; y < tag_array->y; y++)
            tag_array->val[x][y] = -1;



    tag = -1;
    for (x=0; x < gaus_array->x; x++)
        for (y=0; y < gaus_array->y; y++)
            if (gaus_array->gaus[x][y] != NULL)
                {
                if (tag_array->val[x][y] == -1)
                    {
                    /* Start a new chunk
                     * 'tag' is the number assigned
                     * to represent the current chunk.
                     */
                    tag++;
                    tag_chunk(x, y, tag, gaus_array, tag_array, x_stack, y_stack);
                    }
                }



    /* We have now tagged the pixels in each chunk.
     * Assign a color to the pixels of the XImage
     * depending on the chunk.
     */
/*    for (x=0; x < aXImage->width; x++)
        for (y=0; y < aXImage->height; y++)
            {
            if (tag_array->val[x][y] != -1)
                set_XImage_pixel_value(x, y,
                            tag_array->val[x][y]%NUM_RANDOM_COLORS, aXImage);
            }
*/
    free_tag_array(tag_array);
    free(x_stack);
    free(y_stack);

}   /* end of chunk_local_models() */



BOOLEAN
gaus_similar(GAUS *g1, GAUS *g2, double *d)
{
    double  dist;

    dist = sqrt(SQR(g1->covar[1][1]-g2->covar[1][1]) +
                SQR(g1->covar[1][2]-g2->covar[1][2]) +
                SQR(g1->covar[2][2]-g2->covar[2][2]));

    *d = dist;

    if (dist < (0.30*gResolutionRadius))
        return(TRUE);
    else
        return(FALSE);

}   /* end of gaus_similar() */



TAG_ARRAY *
alloc_tag_array(int x, int y)
{
    TAG_ARRAY   *tag_array;
    int         i;

    tag_array = (TAG_ARRAY *)malloc(sizeof(TAG_ARRAY) );
    if (tag_array == NULL)
        {
        fprintf(stderr, "malloc #1 failed in alloc_tag_array() in upwrite_utils.c\n");
        exit(1);
        }

    tag_array->x = x;
    tag_array->y = y;
    tag_array->val = (int **)malloc(x * sizeof(int *) );
    if (tag_array->val == NULL)
        {
        fprintf(stderr, "malloc #2 failed in alloc_tag_array() in upwrite_utils.c\n");
        exit(1);
        }
    for (i=0; i < tag_array->x; i++)
        {
        tag_array->val[i] = (int *)malloc(y * sizeof(int ) );
        if (tag_array->val[i] == NULL)
            {
            fprintf(stderr, "malloc #3 failed in alloc_tag_array() in upwrite_utils.c\n");
            exit(1);
            }
        }

    return(tag_array);

}   /* end of alloc_tag_array() */


void
free_tag_array(TAG_ARRAY *tag_array)
{
    int     x;

    if (tag_array != NULL)
        {
        if (tag_array->val != NULL)
            {
            for(x=0; x < tag_array->x; x++)
                if (tag_array->val[x] != NULL)
                    free(tag_array->val[x]);
            free(tag_array->val);
            }
        free(tag_array);
        }
}   /* end of free_tag_array() */



void
tag_chunk(int x, int y, int tag, GAUS_ARRAY *gaus_array, TAG_ARRAY *tag_array, int *x_stack, int *y_stack)
{
    int     i, j;
    int     x1, y1, x2, y2;
    int     sp;
    double  d;


    sp = -1;

    /* Mark first pixel.
     */
    tag_array->val[x][y] = tag;

    /* Put first pixel on stack
     */
    sp++;
    x_stack[sp] = x;  y_stack[sp] = y;


    do {
        /* Take pixel off stack
         */
        x = x_stack[sp];  y = y_stack[sp];
        sp--;


        /* Check the adjacent local models.
         */
        x1 = x - (int )gResolutionRadius;
        if (x1 < 0)
            x1 = 0;
        x2 = x + (int )gResolutionRadius;
        if (x2 >= tag_array->x)
            x2 = tag_array->x-1;
        y1 = y - (int )gResolutionRadius;
        if (y1 < 0)
            y1 = 0;
        y2 = y + (int )gResolutionRadius;
        if (y2 >= tag_array->y)
            y2 = tag_array->y-1;

        for (i=x1; i <= x2; i++)
            for (j=y1; j <= y2; j++)
                {
                /* Is there an adjacent GAUS?
                 */
                if (gaus_array->gaus[i][j] == NULL)
                    continue;

                /* Is it already assigned to this chunk.
                 */
                if (tag_array->val[i][j] == tag)
                    continue;

                if (gaus_similar(gaus_array->gaus[x][y], gaus_array->gaus[i][j], &d) == TRUE)
                    {

                    tag_array->val[i][j] = tag;
                    /* Put pixel on the stack.
                     */
                    sp++;
                    x_stack[sp] = i;  y_stack[sp] = j;
                    }
                }   /* end of 'for (j=y-1; j <=...' */

    } while (sp >= 0);


}       /* end of tag_chunk() */



/* Compute mean of pixels forming a chunk.
 * Record the mean in (*x, *y).
 * The chunk is a linked list of GAUS.
 * Each gaus has a linked list of pixels which formed it.
 * We compute the mean of these pixels.
 * This only makes sense when the GASU are 2-D gaus
 */
void
compute_mean_of_chunk(GAUS_LIST *chunk, int *x, int *y)
{
    GAUS    *gaus;
    POINT   *point;
    double  sum_x, sum_y, num_points;


    if (chunk == NULL)
        {
        *x = 0;
        *y = 0;
        return;
        }

    sum_x = 0;
    sum_y = 0;
    num_points = 0;
	chunk->max_x = chunk->max_y = 0;
	chunk->min_x = chunk->min_y = 1024*1024;

	for (gaus=chunk->head; gaus != NULL; gaus=gaus->next)
	{

        for (point=gaus->point_list->head; point != NULL; point=point->next)
        {
            sum_x += point->x[1];
            sum_y += point->x[2];
            num_points++;
			if(chunk->max_x < point->x[1] )
				chunk->max_x = point->x[1];
			if(chunk->min_x > point->x[1] )
				chunk->min_x = point->x[1];
			if(chunk->max_y < point->x[2] )
				chunk->max_y = point->x[2];
			if(chunk->min_y > point->x[2] )
				chunk->min_y = point->x[2];
        }
	}
	
    *x = (int )(sum_x / num_points);
    *y = (int )(sum_y / num_points);
	chunk->mean_x = *x;
	chunk->mean_y = *y;
}   /* end of compute_mean_of_chunk() */







/* Compute 1st and 2nd order geometric (central) moments
 * of the points which comprise the GAUS in 'gaus_list'.
 *
 *  point->x[1] - 1st     (x mean)
 *  point->x[2]           (y mean)
 *  point->x[3] - 2nd     (sigma_xx)
 *  point->x[4]           (sigma_xy)
 *  point->x[5]           (sigma_yy)
 */
POINT *
compute_up_to_second_order_geometric_moments(GAUS_LIST *gaus_list)
{
    POINT   *point, *p;
    GAUS    *gaus;
    int     i;
    double  x_mean, y_mean, num_points;
    POINT   *alloc_point(int dim);
    double  **covar, **sqrt_covar;
    void    sqrt_of_square_matrix(double **A, double **Sqrt_A, int dim);
    double  **alloc_array(int row, int column);
    void    free_array(double **A, int dim);


    point = alloc_point(5);

    for (i=1; i <= point->dim; i++)
        point->x[i] = 0;

    /* Compute zeroth order moment */
    num_points = 0;
    for (gaus=gaus_list->head; gaus != NULL; gaus=gaus->next)
        {
        num_points += (double )(gaus->point_list->num_elements);
        }

    if (num_points < 1)
        {
        /* Not enough points to estimate moments.
         */
        return(point);
        }


    /* Compute 1st order moments */
    for (gaus=gaus_list->head; gaus != NULL; gaus=gaus->next)
        {
        for (p=gaus->point_list->head; p != (POINT *)NULL; p=p->next)
            {
            point->x[1] += p->x[1];
            point->x[2] += p->x[2];
            }
        }
    point->x[1] /= num_points;
    point->x[2] /= num_points;


    /* Not enough points to estimate 2nd or 3rd order moments.
     */
    if (num_points < 2)
        {
        return(point);
        }


    /* Compute 2nd order moments */
    x_mean = point->x[1];
    y_mean = point->x[2];
    covar = alloc_array(2, 2);
    sqrt_covar = alloc_array(2, 2);
    if ((covar == NULL) || (sqrt_covar == NULL))
        {
        fprintf(stderr, "Malloc failed in compute_up_to_third_order_geometric_moments() in upwrite_utils.c\n"); 
        exit(1);
        }
    covar[1][1] = 0;    covar[1][2] = 0;
    covar[2][1] = 0;    covar[2][2] = 0;
    for (gaus=gaus_list->head; gaus != NULL; gaus=gaus->next)
        {
        for (p=gaus->point_list->head; p != (POINT *)NULL; p=p->next)
            {
            covar[1][1] += SQR(p->x[1] - x_mean);
            covar[1][2] += (p->x[1] - x_mean)*(p->x[2] - y_mean);
            covar[2][2] += SQR(p->x[2] - y_mean);
            }
        }

    covar[1][1] /= (num_points-1);
    covar[1][2] /= (num_points-1);
    covar[2][2] /= (num_points-1);
    covar[2][1] = covar[1][2];

    /* Square-root the 2nd order moments,
     * so that the values are of similar orders of magnitude
     * as the 1st order moments.
     */
    sqrt_of_square_matrix(covar, sqrt_covar, 2);
    point->x[3] = sqrt_covar[1][1];
    point->x[4] = sqrt_covar[1][2];
    point->x[5] = sqrt_covar[2][2];

    free_array(covar, 2);
    free_array(sqrt_covar, 2);


    return(point);

}           /* end of compute_up_to_second_order_geometric_moments() */



/* 'chunk_list' is a linked list of chunks.
 * Some of these chunks represent the same underlying object.
 * Chunks representing the same underlying object should
 * be similar (i.e. the CHUNK.point will for a cluster).
 * This function will find these clusters.
 * For each cluster, we find the chunk closest to the
 * average of the chunks in the cluster.
 * A copy of this chunk is put into the (CHUNK_LIST *)
 * which is returned by the function.
 *
 * Note: This function modifies 'chunk_list'
 */
CHUNK_LIST *
group_chunks(CHUNK_LIST **ptr_chunk_list)
{
    CHUNK_LIST  *chunk_cluster_centre_list, *tmp_list;
    CHUNK_LIST  *chunk_list;
    CHUNK       *chunk1, *chunk2, *c1, *c2, *centre_chunk;
    BASIS       *basis;
    double      dist, min_dist;
    int         i, num_nearby_chunks;
    POINT       *point;
    BASIS       *compute_local_metric_of_nearby_chunks(CHUNK *chunk,
                                            CHUNK_LIST *chunk_list);
    double      euclidean_distance(POINT *p1, POINT *p2);
    double      distance_of_point_from_a_basis(POINT *point, BASIS *basis,
                                            double tolerance);
    CHUNK_LIST  *disconnect_from_linked_list_of_chunk(CHUNK_LIST *chunk_list,
                                            CHUNK *chunk);
    CHUNK_LIST  *put_in_linked_list_of_chunk(CHUNK_LIST *chunk_list, CHUNK *c);
    void        free_linked_list_of_chunk(CHUNK_LIST *chunk_list);
    POINT       *alloc_point(int dim);
    void        free_point(POINT *p);

#ifdef DEBUG
FILE        *tmp_dat, *tmp_glyphs, *tmp_colors, *tmp_lines;
int         line_num_centre, line_num=0;

tmp_dat = fopen("tmp.dat", "w");
tmp_glyphs = fopen("tmp.glyphs", "w");
tmp_colors = fopen("tmp.colors", "w");
tmp_lines = fopen("tmp.lines", "w");
if ((tmp_dat == NULL) || (tmp_glyphs == NULL) || (tmp_colors == NULL) || (tmp_lines == NULL))
    exit(1);
#endif



    chunk_list = *ptr_chunk_list;
    chunk_cluster_centre_list = (CHUNK_LIST *)NULL;

    if ((chunk_list == (CHUNK_LIST *)NULL) ||
                        (chunk_list->head == (CHUNK *)NULL))
        return( chunk_cluster_centre_list );


    chunk1 = chunk_list->head;
    while (chunk1 != (CHUNK *)NULL)
        {
        chunk2 = chunk1->next;

        basis = compute_local_metric_of_nearby_chunks(chunk1, chunk_list);

        if (basis == (BASIS *)NULL)
            {
            /* Something went wrong when estimating
             * the metric near this chunk.
             */
            chunk1 = chunk2;
            continue;
            }


        /* Unflag chunks
         */
        for (c1=chunk_list->head; c1 != (CHUNK *)NULL; c1=c1->next)
            c1->flag = FALSE;

        /* Find nearby chunks.
         * Flag nearby chunks.
         */
        num_nearby_chunks = 0;
        for (c1=chunk_list->head; c1 != (CHUNK *)NULL; c1=c1->next)
            {
            dist = distance_of_point_from_a_basis(c1->point, basis, 3);
            if (dist < CHUNK_TOLERANCE)
                {
                c1->flag = TRUE;
                num_nearby_chunks++;
                }
            }           /* end of 'while (c1 != (CHUNK *)NULL)' */

        /* If more than 2 chunks,
         * then we have found a cluster.
         */
        tmp_list = (CHUNK_LIST *)NULL;
        if (num_nearby_chunks > 2)
            {
            c1 = chunk_list->head;
            while (c1 != (CHUNK *)NULL)
                {
                c2 = c1->next;
                if (c1->flag == TRUE)
                    {
                    /* Remove c1 from chunk_list */
                    if (c1 == chunk2)
                        chunk2 = chunk2->next;
                    chunk_list = disconnect_from_linked_list_of_chunk(
                                                                chunk_list,
                                                                c1);

                    /* Add c1 to tmp_list
                     * which contains the chunks in this cluster.
                     */
                    tmp_list = put_in_linked_list_of_chunk(tmp_list, c1);
                    }           /* end of 'while (c1 != (CHUNK *)NULL)' */
                c1 = c2;
                }           /* end of 'while (c1 != (CHUNK *)NULL)' */
            }           /* end of 'if (num_nearby_chunks > 2)' */

        if ((tmp_list == (CHUNK_LIST *)NULL) ||
                                    (tmp_list->head == (CHUNK *)NULL))
            {
            chunk1 = chunk2;
            continue;
            }


        /* Find average of chunks in cluster.
         */
        point = alloc_point(tmp_list->head->point->dim);
        for (i=1; i <= point->dim; i++)
            point->x[i] = 0;
        for (c1=tmp_list->head; c1 != NULL; c1=c1->next)
            {
            for (i=1; i <= point->dim; i++)
                point->x[i] += c1->point->x[i];
            }
        for (i=1; i <= point->dim; i++)
            point->x[i] /= (double )tmp_list->num_elements;

        /* Find closest point in cluster to average.
         */
        min_dist = DBL_MAX;
        centre_chunk = (CHUNK *)NULL;
        for (c1=tmp_list->head; c1 != (CHUNK *)NULL; c1=c1->next)
            {
            dist = euclidean_distance(c1->point, point);
            if (dist < min_dist)
                {
                centre_chunk = c1;
                min_dist = dist;
                }
            }
        free_point(point);

#ifdef DEBUG
{
CHUNK *c;
int i;

c = centre_chunk;
for (i=1; i <= c->point->dim; i++)
    fprintf(tmp_dat, "%f ", (float )c->point->x[i]);
fprintf(tmp_dat, "\n");
fprintf(tmp_glyphs, "28\n");
fprintf(tmp_colors, "red\n");
line_num++;
line_num_centre = line_num;

for (c=tmp_list->head; c != (CHUNK *)NULL; c=c->next)
    {
    if (c != centre_chunk)
        {
        for (i=1; i <= c->point->dim; i++)
            fprintf(tmp_dat, "%f ", (float )c->point->x[i]);
        fprintf(tmp_dat, "\n");
        fprintf(tmp_glyphs, "26\n");
        fprintf(tmp_colors, "white\n");
        line_num++;
        fprintf(tmp_lines, "%d %d\n", line_num, line_num_centre);
        }
    }
}
#endif
        /* Remove 'centre_chunk' from 'tmp_list'
         */
        tmp_list = disconnect_from_linked_list_of_chunk(tmp_list, centre_chunk);

        /* Place a copy of this chunk in 'chunk_cluster_centre_list'
         */
        chunk_cluster_centre_list = put_in_linked_list_of_chunk(
                                                chunk_cluster_centre_list,
                                                centre_chunk);

        free_linked_list_of_chunk(tmp_list);

        chunk1 = chunk2;
        }           /* end of 'while (chunk1 != (CHUNK *)NULL)' */

#ifdef DEBUG
{
CHUNK *c;
int i;

if (chunk_list != (CHUNK_LIST *)NULL)
    {
    for (c=chunk_list->head; c != (CHUNK *)NULL; c=c->next)
        {
        for (i=1; i <= c->point->dim; i++)
            fprintf(tmp_dat, "%f ", (float )c->point->x[i]);
        fprintf(tmp_dat, "\n");
        fprintf(tmp_glyphs, "26\n");
        fprintf(tmp_colors, "white\n");
        line_num++;
        }
    }
fclose(tmp_dat);
fclose(tmp_glyphs);
fclose(tmp_colors);
fclose(tmp_lines);
}
#endif

    *ptr_chunk_list = chunk_list;
    return( chunk_cluster_centre_list );

}           /* end of group_chunks() */



/* Define a neighbourhood around chunk->point
 * Consider all CHUNK.point within this neighbourhood.
 * By computing the mean and covariance metric, estimate
 * the metric near chunk->point
 * Convert this into a (BASIS )
 * Return a pointer to the basis (BASIS *)
 * The return value will be (BASIS *)NULL
 * in case of error.
 */
BASIS *
compute_local_metric_of_nearby_chunks(CHUNK *chunk, CHUNK_LIST *chunk_list)
{
    double  num_points;
    double  dist;
    CHUNK   *c;
    GAUS    *gaus;
    BASIS   *basis;
    int     i, j;
    GAUS    *alloc_gaus(int dim);
    void    free_gaus(GAUS *g);
    double  euclidean_distance(POINT *p1, POINT *p2);
    BASIS   *convert_gaus_to_a_basis(GAUS *gaus);


    if ((chunk == (CHUNK *)NULL) || (chunk_list == (CHUNK_LIST *)NULL) ||
                        (chunk_list->head == (CHUNK *)NULL) )
        return( (BASIS *)NULL );


    /* Initialisation
     */
    num_points = 0;
    gaus = alloc_gaus(chunk->point->dim);
    for (i=1; i <= gaus->dim; i++)
        gaus->mean[i] = 0;
    for (i=1; i <= gaus->dim; i++)
        for (j=1; j <= gaus->dim; j++)
            gaus->covar[i][j] = 0;


    /* Compute mean of nearby chunks.
     */
    for (c=chunk_list->head; c != NULL; c=c->next)
        {
        dist = euclidean_distance(c->point, chunk->point);
        if (dist < CHUNK_NEIGHBOURHOOD)
            {
            num_points++;
            for (i=1; i <= gaus->dim; i++)
                gaus->mean[i] += c->point->x[i];
            }
        }
    for (i=1; i <= gaus->dim; i++)
        gaus->mean[i] /= num_points;


    if (num_points < 2)
        {
        /* Not enough points to define a gaussian.
         * Need atleast 2 points for the line below:
         *      gaus->covar[i][j] /= (num_points-1);
         * to make sense.
         */
        free_gaus(gaus);
        return( (BASIS *)NULL );
        }


    /* Compute covariance of nearby points.
     */
    for (c=chunk_list->head; c != NULL; c=c->next)
        {
        dist = euclidean_distance(c->point, chunk->point);
        if (dist < CHUNK_NEIGHBOURHOOD)
            {
            for (i=1; i <= gaus->dim; i++)
                for (j=1; j <= gaus->dim; j++)
                    gaus->covar[i][j] += (c->point->x[i]-gaus->mean[i]) *
                                            (c->point->x[j]-gaus->mean[j]);
            }
        }
    for (i=1; i <= gaus->dim; i++)
        for (j=1; j <= gaus->dim; j++)
            gaus->covar[i][j] /= (num_points-1);


    basis = convert_gaus_to_a_basis(gaus);
    free_gaus(gaus);


    return(basis);

}           /* end of compute_local_metric_of_nearby_chunks() */
