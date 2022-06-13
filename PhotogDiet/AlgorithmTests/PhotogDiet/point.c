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
#include <malloc.h>

#include "upwrite.h"


/* Function Prototypes
 */
POINT       *alloc_point(int dim);
void        free_point(POINT *p);
POINT_LIST  *alloc_point_list(void);
POINT       *copy_point(POINT *p);
void        write_point_to_a_file(FILE *out_file, POINT *p);
POINT       *read_point_from_file(FILE *in_file);
POINT_LIST  *put_in_linked_list_of_point(POINT_LIST *point_list, POINT *p);
void        free_linked_list_of_point(POINT_LIST *point_list);
POINT_LIST  *remove_last_entries_from_point_list(POINT_LIST *point_list, int n);
POINT_LIST  *put_in_linked_list_of_linked_list_of_point(POINT_LIST *head_list, POINT_LIST *list);
void        free_linked_list_of_linked_list_of_point(POINT_LIST *point_list);



POINT *
alloc_point(int dim)
{
    POINT   *p;


    p = (POINT *)malloc(sizeof(POINT));
    if (p == NULL)
        {
        fprintf(stderr, "malloc failed in alloc_point() in point.c\n");
        exit(1);
        }
    p->dim = dim;
    p->x = alloc_vector_MACRO(dim);
    if (p->x == NULL)
        {
        fprintf(stderr, "malloc failed in alloc_point() in point.c\n");
        exit(1);
        }
    p->flag = FALSE;
    p->next = NULL;
    p->next_list = NULL;

    return(p);

}   /* end of alloc_point() */


void
free_point(POINT *p)
{
    if (p != NULL)
        {
        if (p->x != NULL)
            free(p->x);
        free(p);
        }

}   /* end of free_point() */



POINT_LIST *
alloc_point_list(void)
{
    POINT_LIST  *point_list;


    point_list = (POINT_LIST *)malloc( sizeof(POINT_LIST) );

    if (point_list == NULL)
        {
        fprintf(stderr, "malloc failed in alloc_point_list() in point.c\n");
        exit(1);
        }

    point_list->head = NULL;
    point_list->tail = NULL;
    point_list->num_elements = 0;

    point_list->next_list = NULL;

    return(point_list);

}   /* end of alloc_point_list() */


POINT *
copy_point(POINT *p)
{
    POINT   *copy_p;
    int     i;
    POINT   *alloc_point(int dim);

    copy_p = alloc_point(p->dim);
    for (i=1; i <= p->dim; i++)
        copy_p->x[i] = p->x[i];


    return(copy_p);

}   /* end of copy_point() */


void
write_point_to_a_file(FILE *out_file, POINT *p)
{
    int i;

    /* The first number printed is the dimension of the point dim.
     * The next dim points are the point co-ordinates.
     */

    if (p != NULL)
        {
        fprintf(out_file, " %d ", p->dim);
        for (i=1; i <= p->dim; i++)
            fprintf(out_file, "%.15f ", (float )p->x[i]);
        fprintf(out_file, "\n");
        }

}   /* end of write_point_to_a_file() */



POINT *
read_point_from_file(FILE *in_file)
{
    int     i, flag = FALSE;
    int     dim;
    POINT   *point;
    POINT   *alloc_point(int dim);


    if (fscanf(in_file, "%d", &dim) == EOF)
        return(NULL);

    point = alloc_point(dim);

    for (i=1; i <= dim; i++)
        if (fscanf(in_file, "%lf", &(point->x[i]) ) == EOF)
            {
            flag = TRUE;    /* Hit end of file before        */
            break;          /* we finished reading in point. */
            }

    if (flag == TRUE)
        {
        free_point(point);
        return(NULL);
        }
    else
        return(point);


}   /* end of read_point_from_file() */



POINT_LIST *
put_in_linked_list_of_point(POINT_LIST *point_list, POINT *p)
{
    POINT_LIST  *alloc_point_list(void);


    if (point_list == NULL)
        point_list = alloc_point_list();

    if (point_list->head == NULL)
        {
        point_list->head = p;
        point_list->tail = p;
        point_list->num_elements = 1;
        }
    else
        {
        /* Add p to the list.
         */
        point_list->tail->next = p;

        /* Update the tail. */
        point_list->tail = p;

        /* Update the number of elements. */
        (point_list->num_elements)++;

        }

    p->next = NULL;

    return(point_list);


}   /* end of put_in_linked_list_of_point() */



void
free_linked_list_of_point(POINT_LIST *point_list)
{
    POINT   *p1, *p2;
    void    free_point(POINT *p);

    if  (point_list != NULL)
        {
        p1 = point_list->head;
        while (p1 != NULL)
            {
            p2 = p1;
            p1 = p1->next;
            free_point(p2);
            }

        free(point_list);
        }


}   /* end of free_linked_list_of_point() */



/* Remove last n entries from the linked list.
 * If n==0, then the linked list is unchanged.
 */
POINT_LIST *
remove_last_entries_from_point_list(POINT_LIST *point_list, int n)
{
    int     i;
    POINT   *p, *p2;
    void    free_linked_list_of_point(POINT_LIST *point_list);
    void    free_point(POINT *p);


    if (n > point_list->num_elements)
        {
        fprintf(stderr, "Tried to remove %d elements from a linked list which had only %d elements\n", n, point_list->num_elements);
        fprintf(stderr, "in remove_last_entries_from_point_list() in point.c\n");
        exit(1);
        }
    else if (n == point_list->num_elements)
        {
        /* Remove the whole linked list.
         */
        free_linked_list_of_point(point_list);
        point_list = NULL;
        }
    else if (n > 0)
        {
        /* p is set to the last entry in the list which
         * will NOT to be removed.
         */
        for (i=0, p=point_list->head; i < (point_list->num_elements-n-1); i++)
            p=p->next;

        /* Update the tail. */
        point_list->tail = p;

        /* Update the number of elements. */
        (point_list->num_elements) -= n;



        /* Remove the points.
         */
        p2 = p->next;
        p->next = NULL;

        /* p is set to the first entry to be removed.
         */
        p = p2;
        while (p != NULL)
            {
            p2 = p;
            p = p->next;
            free_point(p2);
            }
        }

    return(point_list);

}   /* end of remove_last_entries_from_point_list() */



POINT_LIST *
put_in_linked_list_of_linked_list_of_point(POINT_LIST *head_list, POINT_LIST *list)
{
    POINT_LIST  *ptr;

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


}   /* end of put_in_linked_list_of_linked_list_of_point() */



void
free_linked_list_of_linked_list_of_point(POINT_LIST *point_list)
{
    POINT_LIST   *head;
    void    free_linked_list_of_point(POINT_LIST *point_list);


    while (point_list != NULL)
        {
        head = point_list;
        point_list = point_list->next_list;
        free_linked_list_of_point(head);
        }


}   /* end of free_linked_list_of_linked_list_of_point() */
