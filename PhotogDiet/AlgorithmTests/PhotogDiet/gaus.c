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
#include <math.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>     /* strlen() */

#include "upwrite.h"


/* Function Prototypes
 */
GAUS        *alloc_gaus(int dim);
void        free_gaus(GAUS *g);
GAUS_LIST   *alloc_gaus_list(void);
GAUS        *copy_gaus(GAUS *g);
GAUS_LIST   *put_in_linked_list_of_gaus(GAUS_LIST *gaus_list, GAUS *g);
GAUS_LIST   *insert_at_head_of_linked_list_of_gaus(GAUS_LIST *gaus_list,
                                            GAUS *g);
void        free_linked_list_of_gaus(GAUS_LIST *gaus_list);
GAUS_LIST   *disconnect_head_from_linked_list_of_GAUS(GAUS_LIST *gaus_list);
GAUS_LIST   *disconnect_tail_from_linked_list_of_GAUS(GAUS_LIST *gaus_list);
GAUS_LIST   *remove_from_linked_list_of_gaus(GAUS_LIST *gaus_list, GAUS *gaus);
GAUS_LIST   *put_in_linked_list_of_linked_list_of_gaus(GAUS_LIST *head_list,
                                            GAUS_LIST *list);
void        free_linked_list_of_linked_list_of_gaus(GAUS_LIST *gaus_list);
double      likelihood(POINT *point, GAUS *gaus);
double      likelihood_2D(int x, int y, GAUS *gaus);
double      likelihood_of_point_relative_to_gaus_mixture_model(POINT *point,
                                            GAUS_LIST *gaus_list);
void        write_gaus_to_a_file(FILE *out_file, GAUS *gaus);
GAUS        *read_gaus_from_file(FILE *in_file, int *return_val);
GAUS_ARRAY  *alloc_gaus_array(int x, int y);
void        free_gaus_array(GAUS_ARRAY *gaus_array);
void		fprintf_gaus(GAUS *gaus);



GAUS *
alloc_gaus(int dim)
{
    GAUS    *g;
    double  **alloc_array(int row, int column);


    g = (GAUS *)malloc(sizeof(GAUS));
    if (g == NULL)
        {
        fprintf(stderr, "malloc #1 failed in alloc_gaus() in gaus.c\n");
        exit(1);
        }

    g->dim = dim;
    g->num_points = 0.0;
    g->weight = 1.0;
    g->mean = alloc_vector_MACRO(dim);
    g->covar = alloc_array(dim, dim);
    g->inverse_covar = alloc_array(dim, dim);
    g->eig_val = alloc_vector_MACRO(dim);
    g->eig_vec = alloc_array(dim, dim);
    if ((g->mean == NULL) || (g->covar == NULL) || (g->inverse_covar == NULL) || (g->eig_val == NULL) || (g->eig_vec == NULL))
        {
        fprintf(stderr, "malloc #2 failed in alloc_gaus() in gaus.c\n");
        exit(1);
        }
    g->label = NULL;
    g->original = NULL;
    g->point_list = NULL;
    g->next = NULL;
    g->prev = NULL;


    return(g);

}       /* end of alloc_gaus() */



void
free_gaus(GAUS *g)
{
    void    free_array(double **A, int column);
    void    free_linked_list_of_point(POINT_LIST *point_list);


    if (g != NULL)
        {
        if (g->mean != NULL)
            free(g->mean);
        if (g->covar != NULL)
            free_array(g->covar, g->dim);
        if (g->inverse_covar != NULL)
            free_array(g->inverse_covar, g->dim);
        if (g->eig_val != NULL)
            free(g->eig_val);
        if (g->eig_vec != NULL)
            free_array(g->eig_vec, g->dim);
        if (g->point_list != NULL)
            free_linked_list_of_point(g->point_list);
        free(g);
        }


}   /* end of free_gaus() */



GAUS_LIST *
alloc_gaus_list(void)
{
    GAUS_LIST   *gaus_list;


    gaus_list = (GAUS_LIST *)malloc( sizeof(GAUS_LIST) );

    if (gaus_list == NULL)
        {
        fprintf(stderr, "malloc failed in alloc_gaus_list() in gaus.c\n");
        exit(1);
        }

    gaus_list->head = NULL;
    gaus_list->tail = NULL;
    gaus_list->num_elements = 0;

    gaus_list->next_list = NULL;

    return(gaus_list);

}   /* end of alloc_gaus_list() */



GAUS *
copy_gaus(GAUS *g)
{
    GAUS    *copy;
    int     i, j, length;
    POINT   *p1, *p2;
    GAUS    *alloc_gaus(int dim);
    POINT   *copy_point(POINT *p);
    POINT_LIST  *put_in_linked_list_of_point(POINT_LIST *point_list, POINT *p);

    copy = alloc_gaus(g->dim);

    copy->num_points = g->num_points;
    for (i=1; i <= copy->dim; i++)
        {
        copy->mean[i] = g->mean[i];
        copy->eig_val[i] = g->eig_val[i];
        for (j=1; j <= copy->dim; j++)
            {
            copy->covar[i][j] = g->covar[i][j];
            copy->inverse_covar[i][j] = g->inverse_covar[i][j];
            copy->eig_vec[i][j] = g->eig_vec[i][j];
            }
        }
    copy->flag = g->flag;
    copy->value = g->value;

    if (g->label == NULL)
        copy->label = NULL;
    else
        {
        length = strlen(g->label);
        /* malloc memory for the label.
         * +1 to include the NULL termintion.
         */
        copy->label = (char *)(malloc((length+1) * sizeof(char) ));
        if (copy->label == NULL)
            {
            fprintf(stderr, "malloc failed in copy_gaus() in gaus.c\n");
            exit(1);
            }
        for (i=0; i < length; i++)
            copy->label[i] = g->label[i];
        copy->label[length] = (char )NULL;
        }

    copy->original = g;     /* a pointer to the original GAUS */

    if (g->point_list != NULL)
        {
        for (p1=g->point_list->head; p1 != NULL; p1=p1->next)
            {
            p2 = copy_point(p1);
            copy->point_list = put_in_linked_list_of_point(copy->point_list, p2);
            }
        }

    copy->next = NULL;
    copy->prev = NULL;


    return(copy);

}   /* end of copy_gaus() */



GAUS_LIST *
put_in_linked_list_of_gaus(GAUS_LIST *gaus_list, GAUS *g)
{
    GAUS_LIST   *alloc_gaus_list(void);

    if (gaus_list == NULL)
        gaus_list = alloc_gaus_list();

    if (gaus_list->head == NULL)
        {
        gaus_list->head = g;
        gaus_list->tail = g;
        gaus_list->num_elements = 1;
        g->prev = NULL;
        g->next = NULL;
        }
    else
        {
        /* Add g to the list. */
        gaus_list->tail->next = g;
        g->prev = gaus_list->tail;
        g->next = NULL;

        /* Update the tail. */
        gaus_list->tail = g;

        /* Update the number of elements. */
        (gaus_list->num_elements)++;
        }


    return(gaus_list);


}   /* end of put_in_linked_list_of_gaus() */



GAUS_LIST *
insert_at_head_of_linked_list_of_gaus(GAUS_LIST *gaus_list, GAUS *g)
{
    GAUS_LIST   *alloc_gaus_list(void);

    if (gaus_list == NULL)
        gaus_list = alloc_gaus_list();

    if (gaus_list->head == NULL)
        {
        gaus_list->head = g;
        gaus_list->tail = g;
        gaus_list->num_elements = 1;
        g->prev = NULL;
        g->next = NULL;
        }
    else
        {
        /* Add g to head of list. */
        (gaus_list->head)->prev = g;
        g->next = gaus_list->head;
        g->prev = NULL;
        
        /* Update the head. */
        gaus_list->head = g;

        /* Update the number of elements. */
        (gaus_list->num_elements)++;
        }


    return(gaus_list);


}   /* end of insert_at_head_of_linked_list_of_gaus() */



void
free_linked_list_of_gaus(GAUS_LIST *gaus_list)
{
    GAUS    *g1, *g2;
    void    free_gaus(GAUS *g);

    if (gaus_list == NULL)
        return;

    g1 = gaus_list->head;
    while (g1 != NULL)
        {
        g2 = g1;
        g1 = g1->next;
        free_gaus(g2);
        }

    free(gaus_list);


}   /* end of free_linked_list_of_gaus() */



/* Disconnect the head GAUS in the linked list,
 * so that gaus_list->head->next becomes the new head GAUS.
 * Return a pointer to the disconnected GAUS.
 */
GAUS_LIST *
disconnect_head_from_linked_list_of_GAUS(GAUS_LIST *gaus_list)
{
    GAUS    *gaus;
    void    free_linked_list_of_gaus(GAUS_LIST *gaus_list);

    /* Empty list */
    if ((gaus_list == (GAUS_LIST *)NULL) || (gaus_list->head == (GAUS *)NULL))
        return((GAUS_LIST *)NULL);


    gaus = gaus_list->head;

    /* Disconnect gaus from linked list */
    gaus_list->head = (gaus_list->head)->next;

    if (gaus_list->head != NULL)
        {
        (gaus_list->head)->prev = (GAUS *)NULL;
        (gaus_list->num_elements)--;
        }
    else
        {
        /* Now an empty list - free any memory that was associated with it */
        free(gaus_list);
        gaus_list = (GAUS_LIST *)NULL;
        }


    /* Make sure that gaus doesn't point back
     * to the linked list.
     */
    gaus->next = (GAUS *)NULL;
    gaus->prev = (GAUS *)NULL;


    if ((gaus_list == (GAUS_LIST *)NULL) || (gaus_list->head == (GAUS *)NULL))
        return((GAUS_LIST *)NULL);          /* Now an empty linked list */
    else
        return(gaus_list);

}       /* end of disconnect_head_from_linked_list_of_GAUS() */



/* Disconnect the tail GAUS in the linked list,
 * so that gaus_list->tail->prev becomes the new tail GAUS.
 * Return a pointer to the disconnected GAUS.
 */
GAUS_LIST *
disconnect_tail_from_linked_list_of_GAUS(GAUS_LIST *gaus_list)
{
    GAUS    *gaus;
    void    free_linked_list_of_gaus(GAUS_LIST *gaus_list);

    /* Empty list */
    if ((gaus_list == (GAUS_LIST *)NULL) || (gaus_list->head == (GAUS *)NULL))
        return((GAUS_LIST *)NULL);


    gaus = gaus_list->tail;

    /* Disconnect gaus from linked list */
    gaus_list->tail = (gaus_list->tail)->prev;

    if (gaus_list->tail != NULL)
        {
        (gaus_list->tail)->next = (GAUS *)NULL;
        (gaus_list->num_elements)--;
        }
    else
        {
        /* Now an empty list - free any memory that was associated with it */
        free(gaus_list);
        gaus_list = (GAUS_LIST *)NULL;
        }


    /* Make sure that gaus doesn't point back
     * to the linked list.
     */
    gaus->next = (GAUS *)NULL;
    gaus->prev = (GAUS *)NULL;


    if ((gaus_list == (GAUS_LIST *)NULL) || (gaus_list->head == (GAUS *)NULL))
        return((GAUS_LIST *)NULL);          /* Now an empty linked list */
    else
        return(gaus_list);


}       /* end of disconnect_tail_from_linked_list_of_GAUS() */



GAUS_LIST *
remove_from_linked_list_of_gaus(GAUS_LIST *gaus_list, GAUS *gaus)
{
    void    free_gaus(GAUS *g);

    if (gaus != NULL)
        {

        /* Is the linked list already empty?
         */
        if ((gaus_list == NULL) || (gaus_list->head == NULL))
            {
            fprintf(stderr, "Tried to remove a GAUS from an empty linked list of GAUS\n");
            fprintf(stderr, "in remove_from_linked_list_of_gaus() in gaus.c.\n");
            exit(1);
            }

        if (gaus_list->num_elements == 1)
            {
            /* If there is only one element in the linked list.
             */

            if (gaus == gaus_list->head)
                {
                /* Free the list. */
                free_gaus(gaus);
                free(gaus_list);
                gaus_list = (GAUS_LIST *)NULL;
                }
            else
                {
                fprintf(stderr, "Couldn't find GAUS to be removed from linked list\n");
                fprintf(stderr, "in remove_from_linked_list_of_gaus() in gaus.c\n");
                exit(1);
                }
            }
        else if (gaus == gaus_list->head)
            {
            /* If we are removing the first element
             * and there is more than one element in the list.
             */
            gaus_list->head = gaus_list->head->next;
            gaus_list->head->prev = (GAUS *)NULL;
            free_gaus(gaus);
            (gaus_list->num_elements)--;
            }
        else    /* Remove an element that is not the head. */
            {
            /* Remove it from the linked list.
             */
            (gaus->prev)->next = gaus->next;
            if (gaus->next != (GAUS *)NULL)
                (gaus->next)->prev = gaus->prev;

            /* If we removed the last element,
             * then update the tail.
             */
            if (gaus_list->tail == gaus)
                gaus_list->tail = gaus->prev;

            (gaus_list->num_elements)--;

            free_gaus(gaus);
            }   /* end of 'else  Remove an element that is...' */

        }   /* end of 'if (gaus != NULL)' */


    return(gaus_list);

}   /* end of remove_from_linked_list_of_gaus() */



GAUS_LIST *
put_in_linked_list_of_linked_list_of_gaus(GAUS_LIST *head_list, GAUS_LIST *list)
{
    GAUS_LIST    *ptr;

    if (list != NULL)
        {
        if (head_list == NULL)
            head_list = list;
        else
            {
            for (ptr=head_list; ptr->next_list != NULL; ptr=ptr->next_list)
                ;
            ptr->next_list = list;
            }

        list->next_list = NULL;
        }

    return(head_list);


}   /* end of put_in_linked_list_of_linked_list_of_gaus() */



void
free_linked_list_of_linked_list_of_gaus(GAUS_LIST *gaus_list)
{
    GAUS_LIST   *head;
    void        free_linked_list_of_gaus(GAUS_LIST *gaus_list);


    while (gaus_list != NULL)
        {
        head = gaus_list;
        gaus_list = gaus_list->next_list;
        free_linked_list_of_gaus(head);
        }


}   /* end of free_linked_list_of_linked_list_of_gaus() */



/* Assumes that gaus->inverse_covar and gaus->sqrt_det
 * have been set.
 */
double
likelihood(POINT *point, GAUS *gaus)
{
    double  *x, *inverse_covar_x;
    int     i, j;
    double  val, likelihood;


    if (gaus->sqrt_det <= 0.0)
        {
        return(0.0);
        }

    if (gaus->dim != point->dim)
        {
        fprintf(stderr, "Tried to find the likelihood of a %d dimension point\n", point->dim);
        fprintf(stderr, "relative to a %d dimension gaussian\n", gaus->dim);
        fprintf(stderr, "in likelihood() in gaus.c\n");
        exit(1);
        }


    x = alloc_vector_MACRO(gaus->dim);
    inverse_covar_x = alloc_vector_MACRO(gaus->dim);
    if ((x == NULL) || (inverse_covar_x == NULL))
        {
        fprintf(stderr, "malloc failed in likelihood() in gaus.c\n");
        exit(1);
        }
    /* Find difference from origin.
     */
    for (i=1; i <= gaus->dim; i++)
        x[i] = point->x[i] - gaus->mean[i];

    /* Calculate inverse_covar.x
     */
    for (i=1; i <= gaus->dim; i++)
        {
        inverse_covar_x[i] = 0;
        for (j=1; j <= gaus->dim; j++)
            inverse_covar_x[i] += gaus->inverse_covar[i][j]*x[j];
        }

    /* val = (x^T).inverse_covar.x
     */
    val = 0.0;
    for (i=1; i <= gaus->dim; i++)
        val+= x[i]*inverse_covar_x[i];

    /* SQRT_2_PI is #defined as sqrt(2*M_PI)
     */
    likelihood = gaus->weight * exp(-0.5*val) / (gaus->sqrt_det * pow(SQRT_2_PI, gaus->dim));


    /* Free memory.
     */
    free(x);
    free(inverse_covar_x);


    return(likelihood);

}   /* end of likelihood() */



double
likelihood_2D(int x, int y, GAUS *gaus)
{
    double  p[3], inverse_covar_p[3];
    double  val, likelihood;


    if (gaus->sqrt_det <= 0.0)
        {
        return(0.0);
        }

    if (gaus->dim != 2)
        {
        fprintf(stderr, "Passed a %d dimension GAUS to likelihood_2D() in gaus.c\n", gaus->dim);
        fprintf(stderr, "Expected a 2-D GAUS.\n");
        exit(1);
        }


    /* Find difference from origin.
     */
    p[1] = x - gaus->mean[1];
    p[2] = y - gaus->mean[2];

    /* Calculate inverse_covar.p
     */
    inverse_covar_p[1] = gaus->inverse_covar[1][1]*p[1] + gaus->inverse_covar[1][2]*p[2];
    inverse_covar_p[2] = gaus->inverse_covar[2][1]*p[1] + gaus->inverse_covar[2][2]*p[2];


    /* val = (x^T).inverse_covar.x
     */
    val = p[1]*inverse_covar_p[1] + p[2]*inverse_covar_p[2];


    likelihood = gaus->weight * exp(-0.5*val) / (gaus->sqrt_det * 2*M_PI);


    return(likelihood);

}   /* end of likelihood_2D() */



double
likelihood_of_point_relative_to_gaus_mixture_model(POINT *point, GAUS_LIST *gaus_list)
{
    GAUS    *gaus;
    double  total_likelihood;
    double  likelihood(POINT *point, GAUS *gaus);

    if (gaus_list == NULL)
        return(0.0);

    total_likelihood = 0.0;
    for (gaus=gaus_list->head; gaus != NULL; gaus=gaus->next)
        total_likelihood += likelihood(point, gaus);


    return(total_likelihood);

}   /* end of likelihood_of_point_relative_to_gaus_mixture_model() */



/* gaus is written to file in the following format:
 *
 * D
 * N
 * m1 m2 m3...mM
 * c11 c12 c13 ... c1D
 * c21 c22 c23 ... c2D
 * c31 c32 c33 ... c3D
 *  .   .   .
 *  .   .   .
 *  .   .   .
 * cD1 cD2 cD3 ... cDD
 * W
 *

 * where D = gaus->dim
 *       N = gaus->num_points
 *       W = gaus->weight
 *
 * where m1 is the mean,    c11 c12 c13 ... c1D is the covariance matrix.
 *       m2                 c21 c22 c23 ... c2D
 *       m3                 c31 c32 c33 ... c3D
 *       .                   .   .   .
 *       .                   .   .   .
 *       .                   .   .   .
 *       mM                 cD1 cD2 cD3 ... cDD
 *
 */

void
write_gaus_to_a_file(FILE *out_file, GAUS *gaus)
{
    int i, j;


    if (gaus != NULL)
        {
        fprintf(out_file, "%d\n", gaus->dim);
        fprintf(out_file, "%.15f\n", (float )gaus->num_points);

        for (i=1; i <= gaus->dim; i++)
            fprintf(out_file, "%.15f ", (float )gaus->mean[i]);
        fprintf(out_file, "\n");

        for (i=1; i <= gaus->dim; i++)
            {
            for (j=1; j <= gaus->dim; j++)
                fprintf(out_file, "%.15f ", (float )gaus->covar[i][j]);
            fprintf(out_file, "\n");
            }
        fprintf(out_file, "%.15f\n", (float )gaus->weight);
        }


}   /* end of write_gaus_to_a_file() */



/* On success, returns a pointer to a gaus read
 * in from file and *return_val is set to 0.
 *
 * If error reading in file, returns NULL
 * and *return_val is set to -1.
 *
 * If it successfully reads in the GAUS,
 * but find_eigenvectors_real_symmetric_matrix() fails,
 * then returns NULL and *return_val is set to 0.
 */
GAUS *
read_gaus_from_file(FILE *in_file, int *return_val)
{
    int     dim;
    int     i, j, flag = FALSE;
    GAUS    *gaus;
    double  det;
    GAUS    *alloc_gaus(int dim);
    void    free_gaus(GAUS *g);
    int     find_eigenvectors_real_symmetric_matrix(double **A, double *Eig_val, double **Eig_vec, int dim);
    void    find_inverse(double **MAT, double **I, int dim);


    if (fscanf(in_file, "%d", &dim) == EOF)
        {
        *return_val = -1;
        return(NULL);
        }


    gaus = alloc_gaus(dim);

    if (fscanf(in_file, "%lf", &(gaus->num_points)) == EOF)
        {
        flag = TRUE;    /* Hit end of file before
                         * we finished reading in gaus
                         */
        }

    if (flag == FALSE)
        for (i=1; i <= gaus->dim; i++)
            if (fscanf(in_file, "%lf", &(gaus->mean[i])) == EOF)
                {
                flag = TRUE;    /* Hit end of file before
                                 * we finished reading in gaus
                                 */
                }

    if (flag == FALSE)
        for (i=1; i <= gaus->dim; i++)
            for (j=1; j <= gaus->dim; j++)
                if (fscanf(in_file, "%lf", &(gaus->covar[i][j])) == EOF)
                    {
                    flag = TRUE;    /* Hit end of file before       */
                    break;          /* we finished reading in basis */
                    }

    if (flag == FALSE)
        if (fscanf(in_file, "%lf", &(gaus->weight)) == EOF)
            {
            flag = TRUE;    /* Hit end of file before
                             * we finished reading in gaus
                             */
            }


    if (flag == TRUE)
        {
        free_gaus(gaus);
        *return_val = -1;
        return(NULL);
        }
    else
        {
        /* Fill out the other values in the GAUS structure.
         */
        *return_val = find_eigenvectors_real_symmetric_matrix(gaus->covar, gaus->eig_val, gaus->eig_vec, gaus->dim);
        if (*return_val == -1)
            {
            fprintf(stderr, "QR algorithm failed to converge in find_eigenvectors_real_symmetric_matrix()\n");
            fprintf(stderr, "Called from read_gaus_from_file() in gaus.c\n");
            fprintf(stderr, "Deleteing this GAUS from model.\n");
            free_gaus(gaus);
            *return_val = -1;
            return(NULL);
            }
        find_inverse(gaus->covar, gaus->inverse_covar, gaus->dim);
        /* The determinant of gaus->covar equals
         * the product of the eigenvalues.
         */
        det = 1.0;
        for (i=1; i <= gaus->dim; i++)
            det *= gaus->eig_val[i];
        gaus->sqrt_det = sqrt(det);

        *return_val = 0;
		fprintf_gaus(gaus);
        return(gaus);
        }


}   /* end of read_gaus_from_file() */



/* Malloc space for a GAUS_ARRAY of the appropriate size
 * including space for the array of pointers to GAUS,
 * and initialise all pointers in the array to point to NULL.
 */
GAUS_ARRAY *
alloc_gaus_array(int x, int y)
{
    GAUS_ARRAY  *gaus_array;
    int         i, j;


    gaus_array = (GAUS_ARRAY *)malloc(sizeof(GAUS_ARRAY) );
    if (gaus_array == NULL)
        {
        fprintf(stderr, "malloc #1 failed in alloc_gaus_array() in gaus.c\n");
        exit(1);
        }

    gaus_array->x = x;
    gaus_array->y = y;


    /* Alloc a 2-D array of (GAUS *).
     */
    gaus_array->gaus = (GAUS ***)malloc(x * sizeof( GAUS **) );
    if (gaus_array->gaus == NULL)
        {
        fprintf(stderr, "malloc #2 failed in alloc_gaus_array() in gaus.c\n");
        exit(1);
        }

    for (i=0; i < x; i++)
        {
        gaus_array->gaus[i] = (GAUS **)malloc(y * sizeof(GAUS *) );
        if (gaus_array->gaus[i] == NULL)
            {
            fprintf(stderr, "malloc #3 failed in alloc_gaus_array() in gaus.c\n");
            exit(1);
            }
        }

    /* Initialise the array entries to the NULL pointer.
     */
    for (i=0; i < x; i++)
        for (j=0; j < y; j++)
            gaus_array->gaus[i][j] = NULL;


    return(gaus_array);

}   /* end of alloc_gaus_array() */



void
free_gaus_array(GAUS_ARRAY *gaus_array)
{
    int     x, y;
    void    free_gaus(GAUS *g);


    if (gaus_array != NULL)     /* Were we passed a GAUS_ARRAY? */
        {
        if (gaus_array->gaus != NULL)   /* Does it have an array attatched? */
            {
            for (x=0; x < gaus_array->x; x++)
                if (gaus_array->gaus[x] != NULL)    /* If column exists */
                    {
                    for (y=0; y < gaus_array->y; y++)
                        {
                        /* Free each GAUS
                         */
                        if (gaus_array->gaus[x][y] != NULL)
                            free_gaus(gaus_array->gaus[x][y]);
                        }
                    /* Free the column of pointers to GAUS.
                     */
                    free(gaus_array->gaus[x]);
                    }
            /* Free the row of pointers to columns.
             */
            free(gaus_array->gaus);
            }
        /* Free the GAUS_ARRAY
         */
        free(gaus_array);
        }


}   /* end of free_gaus_array() */

void fprintf_gaus(GAUS *gaus)
{
#ifdef VERBOSE_INSTRUMENTATION
	FILE *output;
	int i,j;

	output = fopen("Ellipse_UpwriteData.txt", "a+");

fprintf(output, "%d\n", gaus->dim);

fprintf(output, "%lf\n", gaus->num_points);


    for (i=1; i <= gaus->dim; i++)
            fprintf(output, "%lf, ", gaus->mean[i]);
	fprintf(output, "\n");
    for (i=1; i <= gaus->dim; i++)
	{
        for (j=1; j <= gaus->dim; j++)
		{
                fprintf(output, "%lf, ", gaus->covar[i][j]);
		}
		fprintf(output, "\n");
	}
    fprintf(output, "%lf\n", gaus->weight);
	fclose(output);
#endif //VERBOSE_INSTRUMENTATION
}
