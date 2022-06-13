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
#include <crtdbg.h>

#include "upwrite.h"


/* Function Prototypes
 */
CHUNK       *alloc_chunk(void);
void        free_chunk(CHUNK *c);
CHUNK_LIST  *alloc_chunk_list(void);
CHUNK_LIST  *put_in_linked_list_of_chunk(CHUNK_LIST *chunk_list, CHUNK *c);
void        free_linked_list_of_chunk(CHUNK_LIST *chunk_list);
CHUNK_LIST  *put_in_linked_list_of_linked_list_of_chunk(CHUNK_LIST *head_list,
                                                    CHUNK_LIST *list);
void        free_linked_list_of_linked_list_of_chunk(CHUNK_LIST *chunk_list);
CHUNK_LIST  *disconnect_from_linked_list_of_chunk(CHUNK_LIST *chunk_list,
                                                    CHUNK *chunk);



CHUNK *
alloc_chunk(void)
{
    CHUNK   *c;


    c = (CHUNK *)malloc(sizeof(CHUNK), _CLIENT_BLOCK, __FILE__, __LINE__);
    if (c == NULL)
        {
        fprintf(stderr, "malloc failed in alloc_chunk() in chunk.c\n");
        exit(1);
        }
    c->gaus_list = (GAUS_LIST *)NULL;
    c->point = (POINT *)NULL;
    c->next = (CHUNK *)NULL;
    c->prev = (CHUNK *)NULL;

    return(c);

}   /* end of alloc_chunk() */


void
free_chunk(CHUNK *c)
{
    void    free_linked_list_of_gaus(GAUS_LIST *gaus_list);
    void    free_point(POINT *p);


    if (c != NULL)
        {
        free_linked_list_of_gaus(c->gaus_list);
        free_point(c->point);
        free(c);
        }

}   /* end of free_chunk() */



CHUNK_LIST *
alloc_chunk_list(void)
{
    CHUNK_LIST  *chunk_list;


    chunk_list = (CHUNK_LIST *)malloc( sizeof(CHUNK_LIST));

    if (chunk_list == NULL)
        {
        fprintf(stderr, "malloc failed in alloc_chunk_list() in chunk.c\n");
        exit(1);
        }

    chunk_list->head = NULL;
    chunk_list->tail = NULL;
    chunk_list->num_elements = 0;

    chunk_list->next_list = NULL;

    return(chunk_list);

}   /* end of alloc_chunk_list() */



CHUNK_LIST *
put_in_linked_list_of_chunk(CHUNK_LIST *chunk_list, CHUNK *c)
{
    CHUNK_LIST  *alloc_chunk_list(void);


    if (chunk_list == NULL)
        chunk_list = alloc_chunk_list();

    if (chunk_list->head == NULL)
        {
        chunk_list->head = c;
        chunk_list->tail = c;
        chunk_list->num_elements = 1;

        c->next = (CHUNK *)NULL;
        c->prev = (CHUNK *)NULL;
        }
    else
        {
        /* Add c to the list.
         */
        chunk_list->tail->next = c;

        /* Update c->prev */
        c->prev = chunk_list->tail;

        /* Update the tail. */
        chunk_list->tail = c;

        /* Update the number of elements. */
        (chunk_list->num_elements)++;

        }

    c->next = NULL;

    return(chunk_list);


}   /* end of put_in_linked_list_of_chunk() */



void
free_linked_list_of_chunk(CHUNK_LIST *chunk_list)
{
    CHUNK   *c1, *c2;
    void    free_chunk(CHUNK *c);

    if  (chunk_list != NULL)
        {
        c1 = chunk_list->head;
        while (c1 != NULL)
            {
            c2 = c1;
            c1 = c1->next;
            free_chunk(c2);
            }

        free(chunk_list);
        }


}   /* end of free_linked_list_of_chunk() */



CHUNK_LIST *
put_in_linked_list_of_linked_list_of_chunk(CHUNK_LIST *head_list, CHUNK_LIST *list)
{
    CHUNK_LIST  *ptr;

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


}   /* end of put_in_linked_list_of_linked_list_of_chunk() */



void
free_linked_list_of_linked_list_of_chunk(CHUNK_LIST *chunk_list)
{
    CHUNK_LIST  *head;
    void        free_linked_list_of_chunk(CHUNK_LIST *chunk_list);


    while (chunk_list != NULL)
        {
        head = chunk_list;
        chunk_list = chunk_list->next_list;
        free_linked_list_of_chunk(head);
        }


}   /* end of free_linked_list_of_linked_list_of_chunk() */



CHUNK_LIST *
disconnect_from_linked_list_of_chunk(CHUNK_LIST *chunk_list, CHUNK *chunk)
{
    if (chunk != NULL)
        {

        /* Is the linked list already empty?
         */
        if ((chunk_list == NULL) || (chunk_list->head == NULL))
            {
            fprintf(stderr, "Tried to remove a CHUNK from an empty linked list of CHUNK\n");
            fprintf(stderr, "in disconnect_from_linked_list_of_chunk() in chunk.c.\n");
            exit(1);
            }

        if (chunk_list->num_elements == 1)
            {
            /* If there is only one element in the linked list.
             */

            if (chunk == chunk_list->head)
                {
                /* Free the list. */
                chunk->prev = (CHUNK *)NULL;
                chunk->next = (CHUNK *)NULL;
                free(chunk_list);
                chunk_list = NULL;
                }
            else
                {
                fprintf(stderr, "Couldn't find CHUNK to be removed from linked list\n");
                fprintf(stderr, "in disconnect_from_linked_list_of_chunk() in chunk.c\n");
                exit(1);
                }
            }
        else if (chunk == chunk_list->head)
            {
            /* If we are removing the first element
             * and there is more than one element in the list.
             */
            chunk_list->head = chunk_list->head->next;
            (chunk_list->head)->prev = (CHUNK *)NULL;
            (chunk_list->num_elements)--;
            chunk->next = (CHUNK *)NULL;
            chunk->prev = (CHUNK *)NULL;
            }
        else    /* Remove an element that is not the head. */
            {
            /* Remove it from the linked list.
             */
            (chunk->prev)->next = chunk->next;
            if (chunk->next != (CHUNK *)NULL)
                (chunk->next)->prev = chunk->prev;

            /* If we removed the last element,
             * then update the tail.
             */
            if (chunk_list->tail == chunk)
                {
                chunk_list->tail = chunk->prev;
                }
            (chunk_list->num_elements)--;

            chunk->next = (CHUNK *)NULL;
            chunk->prev = (CHUNK *)NULL;
            }   /* end of 'else  Remove an element that is not...' */

        }   /* end of 'if (chunk != NULL)' */


    return(chunk_list);

}   /* end of disconnect_from_linked_list_of_chunk() */
