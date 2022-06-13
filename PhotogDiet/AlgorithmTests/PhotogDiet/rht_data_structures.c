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
#include <math.h>
#include <malloc.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "../include/upwrite.h"
#include "../include/rht.h"





/* Function Prototypes
 */
BLACK_PIXEL         *alloc_black_pixel(void);
void                free_black_pixel(BLACK_PIXEL *p);
BLACK_PIXEL_LIST    *alloc_black_pixel_list(void);
BLACK_PIXEL_LIST    *put_in_linked_list_of_black_pixel(BLACK_PIXEL_LIST *black_pixel_list, BLACK_PIXEL *p);
void                free_linked_list_of_black_pixel(BLACK_PIXEL_LIST *black_pixel_list);
BLACK_PIXEL_LIST    *remove_element_from_linked_list_of_black_pixels(BLACK_PIXEL_LIST *black_pixel_list, BLACK_PIXEL *p);
MAP_FROM_POSITION_TO_BLACK_PIXEL    *alloc_map_from_position_to_black_pixel(int x, int y);
void                free_map_from_position_to_black_pixel(
                                        MAP_FROM_POSITION_TO_BLACK_PIXEL *map);


BLACK_PIXEL *
alloc_black_pixel(void)
{
    BLACK_PIXEL *p;


    p = (BLACK_PIXEL *)malloc(sizeof(BLACK_PIXEL));
    if (p == NULL)
        {
        fprintf(stderr, "malloc failed in alloc_black_pixel() in rht_data_structures.c\n");
        exit(1);
        }
    p->next = NULL;
    p->prev = NULL;

    return(p);

}   /* end of alloc_black_pixel() */


void
free_black_pixel(BLACK_PIXEL *p)
{
    if (p != NULL)
        {
        free(p);
        }

}   /* end of free_black_pixel() */



BLACK_PIXEL_LIST *
alloc_black_pixel_list(void)
{
    BLACK_PIXEL_LIST    *black_pixel_list;


    black_pixel_list = (BLACK_PIXEL_LIST *)malloc( sizeof(BLACK_PIXEL_LIST) );

    if (black_pixel_list == NULL)
        {
        fprintf(stderr, "malloc failed in alloc_black_pixel_list() in rht_data_structures.c\n");
        exit(1);
        }

    black_pixel_list->head = NULL;
    black_pixel_list->tail = NULL;
    black_pixel_list->num_elements = 0;


    return(black_pixel_list);

}   /* end of alloc_black_pixel_list() */



/* Remember that this is a doubly-linked list.
 */
BLACK_PIXEL_LIST *
put_in_linked_list_of_black_pixel(BLACK_PIXEL_LIST *black_pixel_list, BLACK_PIXEL *p)
{
    BLACK_PIXEL_LIST    *alloc_black_pixel_list(void);


    if (black_pixel_list == NULL)
        black_pixel_list = alloc_black_pixel_list();

    if (black_pixel_list->head == NULL)
        {
        black_pixel_list->head = p;
        black_pixel_list->tail = p;
        black_pixel_list->num_elements = 1;
        }
    else
        {
        /* Add p to the list.
         */
        black_pixel_list->tail->next = p;
        p->prev = black_pixel_list->tail;

        /* Update the tail. */
        black_pixel_list->tail = p;

        /* Update the number of elements. */
        (black_pixel_list->num_elements)++;

        }

    p->next = NULL;

    return(black_pixel_list);


}   /* end of put_in_linked_list_of_black_pixel() */



void
free_linked_list_of_black_pixel(BLACK_PIXEL_LIST *black_pixel_list)
{
    BLACK_PIXEL     *p1, *p2;
    void    free_black_pixel(BLACK_PIXEL *p);


    if (black_pixel_list != NULL)
        {
        p1 = black_pixel_list->head;
        while (p1 != NULL)
            {
            p2 = p1;
            p1 = p1->next;
            free_black_pixel(p2);
            }

        free(black_pixel_list);
        }



}   /* end of free_linked_list_of_black_pixel() */



BLACK_PIXEL_LIST *
remove_element_from_linked_list_of_black_pixels(BLACK_PIXEL_LIST *black_pixel_list, BLACK_PIXEL *p)
{
    void        free_black_pixel(BLACK_PIXEL *p);


    if (p == NULL)
        return(black_pixel_list);


    /* Is the linked list already empty?
     */
    if ((black_pixel_list == NULL) || (black_pixel_list->head == NULL))
        {
        fprintf(stderr, "Tried to remove a BLACK_PIXEL from an empty linked list of BLACK_PIXEL\n");
        fprintf(stderr, "in remove_element_from_linked_list_of_black_pixels() in rht_data_structures.c.\n");
        exit(1);
        }


    if (black_pixel_list->num_elements == 1)
        {
        /* If there is only one element in the linked list.
         */

        if (p == black_pixel_list->head)
            {
            /* Free the list.
             */
            free_black_pixel(p);
            free(black_pixel_list);
            black_pixel_list = NULL;
            }
        else
            {
            fprintf(stderr, "Couldn't find BLACK_PIXEL to be removed from linked list\n");
            fprintf(stderr, "in remove_element_from_linked_list_of_black_pixels() in rht_data_structures.c\n");
            exit(1);
            }
        }
    else if (p == black_pixel_list->head)
        {
        /* If we are removing the first element
         * and there is more than one element in the list.
         */

        /* Update head of list.
         */
        black_pixel_list->head = black_pixel_list->head->next;
        (black_pixel_list->num_elements)--;

        /* Unlink element from list.
         */
        black_pixel_list->head->prev = NULL;

        /* Free element
         */
        free_black_pixel(p);
        }
    else    /* Remove an element that is not the head. */
        {
        /* Remove it from the linked list.
         */
        p->prev->next = p->next;
        (black_pixel_list->num_elements)--;


        /* Are we removing the last element
         * in the list?
         */
        if (p == black_pixel_list->tail)
            {
            /* If we removed the last element,
             * then update the tail.
             */
            black_pixel_list->tail = p->prev;
            }
        else
            {
            /* If not the last element,
             * then update the backward looking link
             * of the next element in the list.
             */
            p->next->prev = p->prev;
            }


        free_black_pixel(p);
        }   /* end of 'else  Remove an element that is not the head' */



    return(black_pixel_list);

}   /* end of remove_element_from_linked_list_of_black_pixels() */



/* Allocate memory for a 2-D array of pointers.
 * Initialise all pointers to NULL.
 */
MAP_FROM_POSITION_TO_BLACK_PIXEL *
alloc_map_from_position_to_black_pixel(int x, int y)
{
    int     i, j;
    MAP_FROM_POSITION_TO_BLACK_PIXEL    *map;


    map = (MAP_FROM_POSITION_TO_BLACK_PIXEL *)malloc(sizeof(MAP_FROM_POSITION_TO_BLACK_PIXEL) );
    if (map == NULL)
        {
        fprintf(stderr, "malloc #1 failed in alloc_map_from_position_to_black_pixel() in rht_data_structures.c\n");
        exit(1);
        }


    map->p = (BLACK_PIXEL ***)malloc(x * sizeof(BLACK_PIXEL **) );
    if (map->p == NULL)
        {
        fprintf(stderr, "malloc #2 failed in alloc_map_from_position_to_black_pixel() in rht_data_structures.c\n");
        exit(1);
        }


    for (i=0; i < x; i++)
        {
        map->p[i] = (BLACK_PIXEL **)malloc(y * sizeof(BLACK_PIXEL *) );
        if (map->p[i] == NULL)
            {
            fprintf(stderr, "malloc #3 failed in alloc_map_from_position_to_black_pixel() in rht_data_structures.c\n");
            exit(1);
            }
        }



    map->x = x;
    map->y = y;
    for (i=0; i < x; i++)
        for (j=0; j < y; j++)
            map->p[i][j] = NULL;


    return(map);

}   /* end of alloc_map_from_position_to_black_pixel() */




void
free_map_from_position_to_black_pixel( MAP_FROM_POSITION_TO_BLACK_PIXEL *map )
{
    int     i;


    if (map != (MAP_FROM_POSITION_TO_BLACK_PIXEL *)NULL)
        {
        if (map->p != (BLACK_PIXEL ***)NULL);
            {
            for (i=0; i < map->x; i++)
                if (map->p[i] != (BLACK_PIXEL **)NULL)
                    free(map->p[i]);

            free(map->p);
            }

        free(map);
        }


}   /* end of free_map_from_position_to_black_pixel() */
