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
#include <float.h>     /* DBL_MAX */

#include "upwrite.h"



/* Function Prototypes
 */
BASIS   *alloc_basis(int dim, int dim_std_basis);
void    free_basis(BASIS *b);
void    write_basis_to_a_file(FILE *out_file, BASIS *basis);
BASIS   *read_basis_from_file(FILE *in_file);
BASIS   *put_in_linked_list_of_basis(BASIS *head, BASIS *b);
void    free_linked_list_of_basis(BASIS  *basis_head);
BASIS   *put_in_linked_list_of_linked_list_of_basis(BASIS *head_list, BASIS *list);
void    free_linked_list_of_linked_list_of_basis(BASIS *basis_list);
BASIS   *convert_gaus_to_a_basis(GAUS *gaus);
void    subtract_point(double *p1, double *p2, double *result, int dim);
double  dot_product(double *p1, double *p2, int dim);
double  projection(double *p1, double *p2, int dim);
double  distance_of_point_from_a_basis(POINT *point, BASIS *basis,
                                            double tolerance);
double  min_distance_of_point_from_a_list_of_basis(POINT *point, BASIS *basis_list);
void    find_co_ords_of_point_relative_to_basis(double *p, BASIS *basis, double *co_ord);
double  euclidean_distance(POINT *p1, POINT *p2);
double  mahalanobis_distance_2D(double x, double y, BASIS *basis);
void    transpose_square_matrix(double **A, int dim);
void    sqrt_of_square_matrix(double **A, double **Sqrt_A, int dim);
void    square_matrix_multiply_square_matrix(double **A, double **B, double **RESULT, int dim);
void    find_inverse(double **MAT, double **I, int dim);
void    SwapRows(double *A, double *B, int els);
void    TakeWeightRow(double *A, double *B, double num, double denom, int els);
void    ScaleRow(double *A, double fact, int els);





BASIS *
alloc_basis(int dim, int dim_std_basis)
{
    BASIS   *b;
    double  **alloc_array(int row, int column);

    b = (BASIS *)malloc(sizeof(BASIS ));
    if (b == NULL)
        {
        fprintf(stderr, "malloc failed in alloc_basis() in lin_algebra.c\n");
        exit(1);
        }
    b->dim = dim;
    b->dim_std_basis = dim_std_basis;
    b->origin = alloc_vector_MACRO(dim_std_basis);
    b->vector = alloc_array(dim_std_basis, dim);
    if ((b->origin == NULL) || (b->vector == NULL))
        {
        fprintf(stderr, "malloc failed in alloc_basis() in lin_algebra.c\n");
        exit(1);
        }
    b->next = NULL;
    b->next_list = NULL;

    return(b);

}   /* end of alloc_basis() */



void
free_basis(BASIS *b)
{
    void    free_array(double **A, int column);

    if (b != NULL)
        {
        if (b->origin != NULL)
            free(b->origin);
        if (b->vector != NULL)
            {
            free_array(b->vector, b->dim);
            }
        free(b);
        }


}   /* end of free_basis() */



/* basis is written to file in the following format:
 *
 * N
 * M
 * o1 o2 o3...oM
 * v11 v12 v13 ... v1N
 * v21 v22 v23 ... v2N
 * v31 v32 v33 ... v3N
 *  .   .   .
 *  .   .   .
 *  .   .   .
 * vM1 vM2 vM3 ... vMN
 *

 * where N = basis->dim
 *       M = basis->dim_std_basis
 *
 * where o1 is the origin, v11 is the first basis vector,
 *       o2                v21                           
 *       o3                v31                           
 *       .                 .                             
 *       .                 .                             
 *       .                 .                             
 *       oM                vM1                           
 *
 *
 *                         v12 is the second basis vector etc.
 *                         v22
 *                         v32
 *                         .
 *                         .
 *                         .
 *                         vM2
 */

void
write_basis_to_a_file(FILE *out_file, BASIS *basis)
{
    int i, j;


    if (basis != NULL)
        {
        fprintf(out_file, "%d\n", basis->dim);
        fprintf(out_file, "%d\n", basis->dim_std_basis);

        for (i=1; i <= basis->dim_std_basis; i++)
            fprintf(out_file, "%.8f ", (float )basis->origin[i]);
        fprintf(out_file, "\n");

        for (i=1; i <= basis->dim_std_basis; i++)
            {
            for (j=1; j <= basis->dim; j++)
                fprintf(out_file, "%.8f ", (float )basis->vector[i][j]);
            fprintf(out_file, "\n");
            }
        }


}   /* end of write_basis_to_a_file() */



BASIS *
read_basis_from_file(FILE *in_file)
{
    int     dim, dim_std_basis;
    int     i, j, flag = FALSE;
    BASIS   *basis;
    BASIS   *alloc_basis(int dim, int dim_std_basis);
    void    free_basis(BASIS *basis);


    if (fscanf(in_file, "%d\n%d", &dim, &dim_std_basis) == EOF)
        return(NULL);


    basis = alloc_basis(dim, dim_std_basis);

    for (i=1; i <= basis->dim_std_basis; i++)
        if (fscanf(in_file, "%lf", &(basis->origin[i])) == EOF)
            {
            flag = TRUE;    /* Hit end of file before       */
            break;          /* we finished reading in basis */
            }

    if (flag == FALSE)
        for (i=1; i <= basis->dim_std_basis; i++)
            for (j=1; j <= basis->dim; j++)
                if (fscanf(in_file, "%lf", &(basis->vector[i][j])) == EOF)
                    {
                    flag = TRUE;    /* Hit end of file before       */
                    break;          /* we finished reading in basis */
                    }

    if (flag == TRUE)
        {
        free_basis(basis);
        return(NULL);
        }
    else
        return(basis);


}   /* end of read_basis_from_file() */



BASIS
*put_in_linked_list_of_basis(BASIS *head, BASIS *b)
{
    BASIS   *ptr;

    if (head == NULL)
        head = b;
    else
        {
        for (ptr=head; ptr->next != NULL; ptr=ptr->next)
            ;
        ptr->next = b;
        }

    b->next = NULL;

    return(head);


}   /* end of put_in_linked_list_of_basis() */



void
free_linked_list_of_basis(BASIS  *basis_head)
{
    BASIS   *b1;
    void    free_basis(BASIS *g);

    while (basis_head != NULL)
        {
        b1 = basis_head;
        basis_head = basis_head->next;
        free_basis(b1);
        }

}   /* end of free_linked_list_of_basis() */



BASIS *
put_in_linked_list_of_linked_list_of_basis(BASIS *head_list, BASIS *list)
{
    BASIS    *ptr;

    if (head_list == NULL)
        head_list = list;
    else
        {
        for (ptr=head_list; ptr->next_list != NULL; ptr=ptr->next_list)
            ;
        ptr->next_list = list;
        }

    list->next_list = NULL;

    return(head_list);


}   /* end of put_in_linked_list_of_linked_list_of_basis() */



void
free_linked_list_of_linked_list_of_basis(BASIS *basis_list)
{
    BASIS   *head;
    void    free_linked_list_of_basis(BASIS *basis_head);


    while (basis_list != NULL)
        {
        head = basis_list;
        basis_list = basis_list->next_list;
        free_linked_list_of_basis(head);
        }


}   /* end of free_linked_list_of_linked_list_of_basis() */



BASIS *
convert_gaus_to_a_basis(GAUS *gaus)
{
    BASIS   *basis;
    int     i, j, num_vec;
    int     find_eigenvectors_real_symmetric_matrix(double **A, double *Eig_val, double **Eig_vec, int dim);
    double  **alloc_array(int row, int column);
    void    free_array(double **A, int dim);
    BASIS   *alloc_basis(int dim, int dim_std_basis);
    int     return_val;


    if (gaus == NULL)
        return(NULL);


    return_val = find_eigenvectors_real_symmetric_matrix(gaus->covar, gaus->eig_val, gaus->eig_vec, gaus->dim);
    if (return_val == -1)
        {
        return((BASIS *)NULL);
        }

    /* num_vec is the number of vectors in the
     * basis. This equals the number of non-zero
     * eigenvalues of the gaus.
     * Any eigenvalues 10000 times smaller than the biggest
     * eigenvalue are considered to equal zero.
     */
    for (num_vec=0; num_vec < gaus->dim; num_vec++)
            {
            if (num_vec == 0)
                {
                /* Note: the array gaus->eig_val[] is indexed [1..gaus->dim]
                 */
                if (gaus->eig_val[num_vec+1] < A_VERY_SMALL_POSITIVE_VALUE)
                    break;
                }
            else
                if (gaus->eig_val[num_vec+1] < (gaus->eig_val[1]/100000.0))
                    break;
            }
    /* num_vec equals the number of large eigenvalues of the gaussian
     * i.e. those that are of the same order as the dominant eigenvalue.
     * Each eigenvalue and its corresponding eigenvector will be
     * mapped to a basis vector.
     */


    basis = alloc_basis(num_vec, gaus->dim);

    for (i=1; i <= gaus->dim; i++)
        basis->origin[i] = gaus->mean[i];
    for (i=1; i <= num_vec; i++)
        for (j=1; j <= gaus->dim; j++)
            basis->vector[j][i] = sqrt(gaus->eig_val[i]) * gaus->eig_vec[j][i];



    return(basis);

}   /* end of convert_gaus_to_a_basis() */



/* Subtract point
 * result = p1 - p2
 */
void
subtract_point(double *p1, double *p2, double *result, int dim)
{
    int i;

    for (i=1; i <= dim; i++)
        {
        result[i] = p1[i] - p2[i];
        }

}   /* end of subtract_point() */



double
dot_product(double *p1, double *p2, int dim)
{
    int i;
    double  d = 0.0;


    for (i=1; i <= dim; i++)
        d += p1[i] * p2[i];


    return(d);

}   /* end of dot_product() */



/* The projection of vector p1
 * onto vector p2.
 */
double
projection(double *p1, double *p2, int dim)
{
    int i;
    double  sum1, sum2, proj;

    sum1 = 0.0;
    sum2 = 0.0;

    for (i=1; i <= dim; i++)
        {
        sum1 += p1[i] * p2[i];
        sum2 += p2[i] * p2[i];
        }

    proj = sum1 / sum2;

    return(proj);

}   /* end of projection() */



double
distance_of_point_from_a_basis(POINT *point, BASIS *basis, double tolerance)
{
    /* Note: point->dim should equal basis->dim_std_basis
     *       i.e. *basis is a subspace of some space S.
     *       *point is a point in the space S.
     */
    int     i, j;
    double  *x, *co_ord, *v;
    double  a, dist, sum;
    POINT   *p1;
    void    subtract_point(double *p1, double *p2, double *result, int dim);
    double  *copy_column_vector(double **A, int row, int column, int i);
    double  projection(double *p1, double *p2, int dim);
    POINT   *alloc_point(int dim);
    void    free_point(POINT *p);
    double  euclidean_distance(POINT *p1, POINT *p2);


    if (basis == NULL)
        {
        dist = DBL_MAX;   /* We were not passed a basis. */
        fprintf(stderr, "Warning - distance_of_point_from_a_basis(POINT *point, BASIS *basis) was passed a NULL basis\n");
        return(DBL_MAX);
        }
    if (point == NULL)
        {
        dist = DBL_MAX;   /* We were not passed a point. */
        fprintf(stderr, "Warning - distance_of_point_from_a_basis(POINT *point, BASIS *basis) was passed a NULL point\n");
        return(DBL_MAX);
        }



    p1 = alloc_point(point->dim);

    co_ord = alloc_vector_MACRO(basis->dim);
    x = alloc_vector_MACRO(point->dim);
    if ((co_ord == NULL) || (x == NULL))
        {
        fprintf(stderr, "Malloc failed in distance_of_point_from_a_basis() in lin_algebra.c\n");
        exit(1);
        }
    subtract_point(point->x, basis->origin, x, point->dim);

    sum = 0.0;
    for (i=1; i <= basis->dim; i++)
        {
        v = copy_column_vector(basis->vector, basis->dim_std_basis, basis->dim, i);
        a = projection(x, v, point->dim);
        co_ord[i] = a;  /* co_ord will contain the co-ords of point
                         * relative to basis.
                         */
        sum += SQR(a);
        free(v);
        }

    /* Reconstruct the point from its co-ords relative
     * to the basis (stored in co-ords[]).
     * If the reconstructed point is different from the
     * original point, then point does not lie in the
     * subspace spanned by basis.
     * In which case, the distance of the point from
     * the basis should be infinite (i.e. DBL_MAX)
     */
    for (i=1; i <= p1->dim; i++)
        p1->x[i] = basis->origin[i];
    for (i=1; i <= basis->dim; i++)
        for (j=1; j <= p1->dim; j++)
            p1->x[j] += co_ord[i]*basis->vector[j][i];

    if (euclidean_distance(point, p1) > tolerance)
        {           /* Not in the space spanned by the basis. */
        dist = DBL_MAX;
        }
    else
        dist = sqrt(sum);

    free_point(p1);
    free(co_ord);
    free(x);



    return(dist);


}   /* end of distance_of_point_from_a_basis() */



double
min_distance_of_point_from_a_list_of_basis(POINT *point, BASIS *basis_list)
{
    BASIS   *basis;
    double  dist, min_dist;
    double  distance_of_point_from_a_basis(POINT *point, BASIS *basis,
                                                double tolerance);


    min_dist = DBL_MAX;
    for (basis=basis_list; basis != NULL; basis=basis->next)
        {
        dist = distance_of_point_from_a_basis(point, basis,
                                                A_VERY_SMALL_POSITIVE_VALUE);
        if (dist < min_dist)
            min_dist = dist;
        }


    return(min_dist);

}   /* end of min_distance_of_point_from_a_list_of_basis() */


void
find_co_ords_of_point_relative_to_basis(double *p, BASIS *basis, double *co_ord)
{
    int     i;
    double  *x, *v;
    void    subtract_point(double *p1, double *p2, double *result, int dim);
    double  *copy_column_vector(double **A, int row, int column, int i);
    double  projection(double *p1, double *p2, int dim);


    x = alloc_vector_MACRO(basis->dim_std_basis);
    if (x == NULL)
        {
        fprintf(stderr, "Malloc failed in find_coords_of_point_relative_to_basis() in lin_algebra.c\n");
        exit(1);
        }
    subtract_point(p, basis->origin, x, basis->dim_std_basis);


    for (i=1; i <= basis->dim; i++)
        {
        v = copy_column_vector(basis->vector, basis->dim_std_basis, basis->dim, i);
        co_ord[i] = projection(x, v, basis->dim_std_basis);
        free(v);
        }

    free(x);

}   /* end of find_co_ords_of_point_relative_to_basis() */



double
euclidean_distance(POINT *p1, POINT *p2)
{
    int i;
    double  dist;

    if (p1->dim != p2->dim)
        {
        fprintf(stderr, "Warning: Tried to find euclidean distance between two points\n");
        fprintf(stderr, "of different dimension in euclidean_distance() in lin_algebra.c\n");
        fprintf(stderr, "Dimension of point 1: %d       Dimension of point 2: %d\n", p1->dim, p2->dim);
        exit(1);
        }
    else
        {
        dist = 0.0;
        for (i=1; i <= p1->dim; i++)
            {
            dist += SQR(p1->x[i] - p2->x[i]);
            }
        dist = sqrt(dist);
        }


    return(dist);

}   /* end of euclidean_distance() */



double
mahalanobis_distance_2D(double x, double y, BASIS *basis)
{
    double  p[3];
    double  a, b, sum1, sum2, dist;

    sum1 = 0.0;
    sum2 = 0.0;
    p[1] = x - basis->origin[1];
    p[2] = y - basis->origin[2];

    sum1 = SQR(basis->vector[1][1]) + SQR(basis->vector[2][1]);
    sum2 = SQR(basis->vector[1][2]) + SQR(basis->vector[2][2]);

    a = (p[1]*basis->vector[1][1] + p[2]*basis->vector[2][1]) / sum1;
    b = (p[1]*basis->vector[1][2] + p[2]*basis->vector[2][2]) / sum2;


    dist = sqrt(SQR(a) + SQR(b));

    return(dist);
}



/* Assumes that A is a square matrix.
 */
void
transpose_square_matrix(double **A, int dim)
{
    int i, j;
    double  temp;

    for (i=1; i <= dim; i++)
        for (j=i+1; j <= dim; j++)
            {
            temp = A[i][j];
            A[i][j] = A[j][i];
            A[j][i] = temp;
            }

}   /* end of transpose_square_matrix() */



/* Assumes A is a square matrix.
 */
void
sqrt_of_square_matrix(double **A, double **Sqrt_A, int dim)
{
    /*
     * Suppose
     * A = B B
     *
     * Let O be the eigenbasis of A (orthogonal).
     * n.b. transpose of O = inverse of O
     * Let D be the diagonal matrix containing the eigenvalues of A.
     * Let E be the diagonal matrix containing the square root
     * of the eigenvalues of A.
     *
     *        A = O D O^T
     * i.e.   A = O E E O^T
     * i.e.   A = O E (O^T O) E O^T
     * i.e.   A = (O E O^T)(O E O^T)
     * i.e.   B = O E O^T
     */

    int     i, j;
    double  **O, **E, **Temp;
    double  *Eig_val;
    double  **alloc_array(int row, int column);
    void    free_array(double **A, int dim);
    int     find_eigenvectors_real_symmetric_matrix(double **A, double *Eig_val, double **Eig_vec, int dim);
    void    transpose_square_matrix(double **A, int dim);
    void    square_matrix_multiply_square_matrix(double **A, double **B, double **RESULT, int dim);
    int     return_val;


    Eig_val = alloc_vector_MACRO(dim);
    O = alloc_array(dim, dim);
    E = alloc_array(dim, dim);
    Temp = alloc_array(dim, dim);
    if ((Eig_val == NULL) || (O == NULL) || (E == NULL) || (Temp == NULL))
        { 
        fprintf(stderr, "Malloc failed in sqrt_of_square_matrix() in lin_algebra.c\n");
        exit(1); 
        }

    return_val = find_eigenvectors_real_symmetric_matrix(A, Eig_val, O, dim);
    if (return_val == -1)
        {
        fprintf(stderr, "QR algorithm failed to converge in find_eigenvectors_real_symmetric_matrix()\n");
        fprintf(stderr, "Exitting from sqrt_of_square_matrix() in lin_algebra.c\n");
        exit(1);
        }

    for (i=1; i <= dim; i++)
        for (j=1; j <= dim; j++)
            if (i==j)
                {
                if (Eig_val[i]  < A_VERY_SMALL_POSITIVE_VALUE)
                    E[i][j] = 0.0;
                else
                    E[i][j] = sqrt(Eig_val[i]);
                }
            else
                E[i][j] = 0.0;

    square_matrix_multiply_square_matrix(O, E, Temp, dim);
    transpose_square_matrix(O, dim);
    square_matrix_multiply_square_matrix(Temp, O, Sqrt_A, dim);


    free(Eig_val);
    free_array(O, dim);
    free_array(E, dim);
    free_array(Temp, dim);


}   /* end of sqrt_of_square_matrix() */


void
square_matrix_multiply_square_matrix(double **A, double **B, double **RESULT, int dim)
{
    int i, j, k;

    for (i=1; i <= dim; i++)
        for (j=1; j <= dim; j++)
            {
            RESULT[i][j] = (double )0;
            for (k=1; k <= dim; k++)
                RESULT[i][j] += A[i][k] * B[k][j];
            }

}       /* end of square_matrix_multiply_square_matrix() */



void
find_inverse(double **MAT, double **I, int dim)
{

    int     a,b,R;
    double  num,denom;
    double  **M;
    double  **alloc_array(int row, int column);
    void    free_array(double **A, int row);
    void    SwapRows(double *A, double *B, int els);
    void    TakeWeightRow(double *A, double *B, double num, double denom, int els);
    void    ScaleRow(double *A, double fact, int els);


    M = alloc_array(dim, dim);
    for (a=1; a<=dim; a++)
        for (b=1; b<=dim; b++)
           M[a][b] = MAT[a][b];

    for(R=1;R<=dim;R++)
        for(a=1;a<=dim;a++)
            I[R][a]=(R==a)?1:0;
    for(R=1;R<=dim;R++)
        {  
        if(M[R][R]==0)
            {
            for(a=R+1;a<=dim;a++)
                if (M[a][R]!=0) break;
            if (a==(dim+1))
                {
                fprintf(stderr, "Matrix non-invertable, fail on row %d left.\n",R);
                fprintf(stderr, "in find_inverse() in lin_algebra.c\n");
                exit(1);
                }
            SwapRows(M[R],M[a],dim);
            SwapRows(I[R],I[a],dim);
            }
        denom=M[R][R];
        for(a=R+1;a<=dim;a++)
            {
            num=M[a][R];
            TakeWeightRow(M[a],M[R],num,denom,dim);
            TakeWeightRow(I[a],I[R],num,denom,dim);
            }
        }
    for(R=dim;R>=1;R--)
        {  
        for(a=R+1;a<=dim;a++)
            {
            num=M[R][a];
            denom=M[a][a];
            TakeWeightRow(M[R],M[a],num,denom,dim);
            TakeWeightRow(I[R],I[a],num,denom,dim);
            }
        }
    for(R=1;R<=dim;R++)
        ScaleRow(I[R],M[R][R],dim);

    free_array(M, dim);


}   /* end of find_inverse() */


void
SwapRows(double *A, double *B, int els)
{
    double temp;
    int a;
 
    for(a=1;a<=els;a++)
        {
        temp=A[a];
        A[a]=B[a];
        B[a]=temp;
        }
}   /* end of SwapRows() */
 

void
TakeWeightRow(double *A, double *B, double num, double denom, int els)
{
    int a;
 
    for(a=1;a<=els;a++)
        A[a]=(A[a]*denom-B[a]*num)/denom;

}   /* end of TakeWeightRow() */

 
void
ScaleRow(double *A, double fact, int els)
{
    int a;
 
    for(a=1;a<=els;a++)
        A[a]/=fact;

}   /* end of ScaleRow() */
