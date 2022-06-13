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
#include <values.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "../include/upwrite.h"

/* This file contains code to implement the
 * Hierarchical Hough Transform, as described
 * in the paper:
 * "A Hierarchical Approach to Line Extraction Based on the
 *  Hough Transform",
 * John Princen, John Illingworth, and Josef Kittler,
 * Computer Vision, Graphics, and Image Understanding,
 * Vol. 52, pp 57 - 77, (1990).
 *
 * Throughout the code, when we refer to "the paper",
 * we are referring to this paper.
 */


/* Data Structures */

typedef struct sub_image_type
{
    int                     i, j;
    struct sub_image_type   *next;
} SUB_IMAGE;



typedef struct line_segment_type
{
    double                      theta;
    double                      rho;
    int                         x_origin;   /* origin for theta, rho */
    int                         y_origin;
    BOOLEAN                     used_already_flag;
    BOOLEAN                     part_of_higher_level_line;
    SUB_IMAGE                   *sub_image_list;
    struct line_segment_type    *next;
    struct line_segment_type    *prev;

} LINE_SEGMENT;



typedef struct ptr_to_line_segment_type
{
    int                 i, j;
    LINE_SEGMENT        *line_segment;

    struct ptr_to_line_segment_type *next;
    struct ptr_to_line_segment_type *prev;

} PTR_TO_LINE_SEGMENT;



typedef struct ht_entry_type
{
    int                     t;
    int                     r;
    double                  count;
    PTR_TO_LINE_SEGMENT     *list_of_ptr_to_line_segment;
    struct ht_entry_type    *next;

} HT_ENTRY;



typedef struct child_type
{
    int                     i, j;
    BOOLEAN                 tag;
} CHILD;



/* Function Prototypes
 */
void                find_lines_with_HHT(IMAGE *aImage, XImage *aXImage,
                                    Window theWindow, GC theGC);
LINE_SEGMENT        ***HHT_low_level_line_detection(int L,
                                    int i_max, int j_max,
                                    IMAGE *aImage, Window theWindow, GC theGC);
LINE_SEGMENT        ***group_line_segments(int L_parent, int L_child,
                                    int L_sub_image,
                                    int x_num_parents, int y_num_parents,
                                    int x_num_children, int y_num_children,
                                    LINE_SEGMENT ***child_line_segment_array,
                                    IMAGE *aImage,
                                    Window theWindow, GC theGC);
LINE_SEGMENT        *alloc_line_segment(void);
void                free_line_segment(LINE_SEGMENT *line_segment);
LINE_SEGMENT        *put_in_linked_list_of_line_segment(LINE_SEGMENT *head,
                                    LINE_SEGMENT *line_segment);
LINE_SEGMENT        *remove_from_linked_list_of_line_segment(
                            LINE_SEGMENT *head, LINE_SEGMENT *line_segment);
SUB_IMAGE           *alloc_sub_image(void);
void                free_sub_image(SUB_IMAGE *sub_image);
void                free_linked_list_of_sub_image(SUB_IMAGE *sub_image_list);
SUB_IMAGE           *put_in_linked_list_of_sub_image(SUB_IMAGE *head,
                                    SUB_IMAGE *sub_image);
LINE_SEGMENT        ***alloc_line_segment_array(int x, int y);
void                free_line_segment_array(LINE_SEGMENT ***p, int x);
PTR_TO_LINE_SEGMENT *alloc_ptr_to_line_segment(void);
void                free_ptr_to_line_segment( PTR_TO_LINE_SEGMENT *p);
PTR_TO_LINE_SEGMENT *put_in_linked_list_of_ptr_to_line_segment(
                                    PTR_TO_LINE_SEGMENT *list,
                                    PTR_TO_LINE_SEGMENT *p);
PTR_TO_LINE_SEGMENT *remove_from_linked_list_of_ptr_to_line_segment(
                            PTR_TO_LINE_SEGMENT *head, PTR_TO_LINE_SEGMENT *p);
void                free_linked_list_of_ptr_to_line_segment(
                                    PTR_TO_LINE_SEGMENT *p_list);
HT_ENTRY            *alloc_ht_entry(void);
void                free_ht_entry(HT_ENTRY *ht_entry);
HT_ENTRY            *put_in_linked_list_of_ht_entry(HT_ENTRY *head,
                                    HT_ENTRY *ht_entry);
void                free_linked_list_of_ht_entry( HT_ENTRY *ht_list);
void                compute_local_hough_transform(int x_origin, int y_origin,
                                    int L,
                                    int **histogram,
                                    double theta_min, double theta_max,
                                    double rho_min, double rho_max,
                                    double theta_res, double rho_res,
                                    IMAGE *aImage);
void                tag_pixels_along_line_segment(int t, int r,
                                    int x_origin, int y_origin, int L,
                                    double theta_min, double theta_max,
                                    double rho_min, double rho_max,
                                    double theta_res, double rho_res,
                                    IMAGE *aImage);
void                tag_pixels(int t, int r, int x_origin, int y_origin, int L,
                                    double theta_min, double theta_max,
                                    double rho_min, double rho_max,
                                    double theta_res, double rho_res,
                                    IMAGE *aImage);
LINE_SEGMENT        *generate_line_segment(int t, int r,
                                    int x_origin, int y_origin,
                                    int i, int j,
                                    double theta_min, double theta_max,
                                    double rho_min, double rho_max,
                                    int theta_res, int rho_res);
void                draw_line_segment_in_sub_image(LINE_SEGMENT *line_segment,
                                    int L,
                                    IMAGE *aImage,
                                    Window theWindow, GC theGC);
HT_ENTRY            *compute_hough_transform_for_parent(
                                    int i_parent, int j_parent,
                                    double x_origin, double y_origin,
                                    int x_num_children, int y_num_children,
                                    LINE_SEGMENT ***child_line_segment_array,
                                    double theta_delta, double rho_delta,
                                    double theta_min, double theta_max,
                                    double rho_min, double rho_max,
                                    double theta_res, double rho_res,
                                    IMAGE *aImage);
HT_ENTRY            *record_value_in_ht_list(HT_ENTRY *ht_list,
                                    int t, int r, double val,
                                    LINE_SEGMENT *line_segment, int i, int j);
SUB_IMAGE           *find_sub_images_forming_line(HT_ENTRY *ht);
BOOLEAN             superset_of_line_segments(SUB_IMAGE *sub_image_list,
                                    HT_ENTRY *ht_max,
                                    Window theWindow, GC theGC);
void                delete_child_line_segments_that_are_subsets(
                                    HT_ENTRY *ht_max,
                                    HT_ENTRY *ht_list,
                                    LINE_SEGMENT ***child_line_segment_array);
BOOLEAN             test_if_children_are_connected(HT_ENTRY *ht_max);
void                tag_adjacent(int index, CHILD *array, int num_entries);
void                tag_contributing_line_segments(HT_ENTRY *ht_entry);
LINE_SEGMENT        *compute_line_segment_parameters(HT_ENTRY *ht_entry,
                                    int x_origin, int y_origin,
                                    double theta_min, double theta_max,
                                    double rho_min, double rho_max,
                                    int theta_res, int rho_res,
                                    SUB_IMAGE *sub_image_list);
void                draw_line_segment_across_sub_images(
                                    LINE_SEGMENT *line_segment,
                                    int L,
                                    IMAGE *aImage,
                                    Window theWindow, GC theGC);
void                find_line_ends_HHT(LINE_SEGMENT *line_segment,
                                    int L,
                                    IMAGE *aImage,
                                    Window theWindow, GC theGC,
                                    int *x_start, int *y_start,
                                    int *x_end, int *y_end);
void                display_line_segments_at_this_level(
                                    LINE_SEGMENT ***line_segment_array,
                                    int x_num_parents, int y_num_parents,
                                    int L_sub_image,
                                    IMAGE *aImage, XImage *aXImage,
                                    Window theWindow, GC theGC);
int                 record_and_display_all_line_segments(
                                    LINE_SEGMENT ****line_segment_array,
                                    int num_levels,
                                    int *x_num_images_array,
                                    int *y_num_images_array,
                                    int L_sub_image,
                                    IMAGE *aImage, XImage *aXImage,
                                    Window theWindow, GC theGC);





void
find_lines_with_HHT(IMAGE *aImage, XImage *aXImage, Window theWindow, GC theGC)
{
    /* Parameters */
    int             L = 16;
    /* end of parameters */

    int             L_child, L_parent;
    int             n1, n2, num_levels;
    int             i;
    int             x_num_sub_images, y_num_sub_images;
    int             x_num_parents, y_num_parents;
    int             x_num_children, y_num_children;
    int             *x_num_images_array, *y_num_images_array;
    LINE_SEGMENT    ****line_segment_array;
    void            untag_image(IMAGE *aImage);
    int             num_lines;
    char            results_message[100];

    MESSAGE_WINDOW      *message_window;
    XEvent              report;
    int                 theScreen;
    unsigned long       theBlackPixel;
    MESSAGE_WINDOW      *alloc_message_window(void);
    void                init_message_window_fields(
                                        MESSAGE_WINDOW *message_window,
                                        Window MainWindow, int x, int y,
                                        int num_strings, ...);
    void                write_message_window_strings(MESSAGE_WINDOW
                                        *message_window);
    void                free_message_window(MESSAGE_WINDOW *message_window);
    void                draw_image(XImage *aXImage,
                                        Window theWindow, GC theGC);
    void                display_message_and_wait(Window theWindow,
                                        int x, int y, int num_strings, ...);



    /* Open a window which tells the
     * user that we have started.
     */
    theScreen = DefaultScreen(gDisplay);
    theBlackPixel = BlackPixel( gDisplay, theScreen);
    XSetForeground(gDisplay, theGC, theBlackPixel); /* Black text */
    message_window = alloc_message_window();
    draw_image(aXImage, theWindow, theGC);
    init_message_window_fields(message_window, theWindow, 0, 0, 4,
                            "",
                            "Started Hierarchical Hough Transform",
                            "to find line segments.",
                            "");
    XMapWindow(gDisplay, message_window->win);
    do
        {
        XNextEvent( gDisplay, &report );
        if (report.type == Expose)
            {
            /* Unless this is the last contiguous
             * expose, don't draw the window.
             */
            if (report.xany.window == message_window->win)
                {
                if (report.xexpose.count != 0)
                    break;
                else
                    {
                    write_message_window_strings(message_window);
                    }
                }
            }       /* end of 'if (report.type == Expose)' */
        } while ((report.xany.window != message_window->win) || (report.type != Expose));

    XFlush(gDisplay);



    /* Compute number of levels */
    n1 = 1 + (int )(log(aImage->x/L) / log(2) );
    n2 = 1 + (int )(log(aImage->y/L) / log(2) );
    if (n1 < n2)
        num_levels = n1;
    else
        num_levels = n2;


    line_segment_array = (LINE_SEGMENT ****)malloc(num_levels * sizeof(LINE_SEGMENT ***));
    x_num_images_array = (int *)malloc(num_levels * sizeof(int) );
    y_num_images_array = (int *)malloc(num_levels * sizeof(int) );
    if ((line_segment_array == (LINE_SEGMENT ****)NULL) ||
                (x_num_images_array == (int *)NULL) ||
                (y_num_images_array == (int *)NULL))
        {
        fprintf(stderr, "malloc failed in find_lines_with_HHT() in hht_line.c\n");
        exit(1);
        }



    /* Number of sub-images */
    x_num_sub_images = aImage->x/L - 1;
    y_num_sub_images = aImage->y/L - 1;

    untag_image(aImage);
    line_segment_array[0] = HHT_low_level_line_detection(L, x_num_sub_images,
                                        y_num_sub_images, aImage,
                                        theWindow, theGC);


    display_line_segments_at_this_level(line_segment_array[0],
                                    x_num_sub_images, y_num_sub_images, L,
                                    aImage, aXImage,
                                    theWindow, theGC);




    L_child = L;            /* Size of neighbourhood in child level */
    L_parent = 2*L_child;   /* Size of neighbourhood in parent level */

    /* Number of child sub-images */
    x_num_children = x_num_sub_images;
    y_num_children = y_num_sub_images;

    /* Number of parent sub-images */
    x_num_parents = x_num_children / 2;
    y_num_parents = y_num_children / 2;

    x_num_images_array[0] = x_num_sub_images;
    y_num_images_array[0] = y_num_sub_images;

    for (i=1; i < num_levels; i++)
        {
        /* Store the number of sub-images at each level
         * in an array.
         * This is used in record_and_display_all_line_segments()
         */
        x_num_images_array[i] = x_num_parents;
        y_num_images_array[i] = y_num_parents;

        line_segment_array[i] = group_line_segments(L_parent, L_child, L,
                                        x_num_parents, y_num_parents,
                                        x_num_children, y_num_children,
                                        line_segment_array[i-1],
                                        aImage, theWindow, theGC);

        display_line_segments_at_this_level(line_segment_array[i],
                                    x_num_parents, y_num_parents, L,
                                    aImage, aXImage,
                                    theWindow, theGC);


        L_child = L_parent;
        L_parent = L_parent*2;

        x_num_children = x_num_parents;
        y_num_children = y_num_parents;

        x_num_parents = x_num_parents/2;
        y_num_parents = y_num_parents/2;
        }


    num_lines = record_and_display_all_line_segments(line_segment_array,
                                    num_levels,
                                    x_num_images_array, y_num_images_array,
                                    L,
                                    aImage, aXImage,
                                    theWindow, theGC);


    /* Free memory
     */
    for (i=0; i < num_levels; i++)
        free_line_segment_array(line_segment_array[i], x_num_images_array[i]);
    free(line_segment_array);
    free(x_num_images_array);
    free(y_num_images_array);


    XUnmapWindow(gDisplay, message_window->win);
    free_message_window(message_window);


    draw_image(aXImage, theWindow, theGC);
    if (num_lines == 1)
        sprintf(results_message, "Found 1 line with HHT.");
    else
        sprintf(results_message, "Found %d lines with HHT.", num_lines);
    display_message_and_wait(theWindow, 0, 0, 4,
                            "",
                            results_message,
                            "Hit Left Mouse Button to continue",
                            "");


}           /* end of find_lines_with_HHT() */



/* Separate image into sub-images.
 * Find the line segments in each sub-image.
 * Put the line segments found in a particular subimage into
 * a linked list of LINE_SEGMENT.
 *
 * This functions returns a 2-D array of pointers
 * to linked lists of LINE_SEGMENT, with each entry
 * in the array corresponding to a particular sub-image.
 *
 * This function corresponds to level 0 of the HHT hierachy.
 */
LINE_SEGMENT ***
HHT_low_level_line_detection(int L, int i_max, int j_max,
                                IMAGE *aImage, Window theWindow, GC theGC)
{
    double          rho_delta, theta_delta;
    double          rho_min, rho_max;
    double          theta_min, theta_max;
    int             theta_res, rho_res;
    int             i, j;
    int             x_origin, y_origin;
    int             t, r, t_max, r_max, max_count;
    LINE_SEGMENT    *line_segment;
    int             **histogram;
    LINE_SEGMENT    ***line_segment_array;
    LINE_SEGMENT    ***alloc_line_segment_array(int x, int y);
    int             **alloc_histogram(int x_max, int y_max);
    void            free_histogram(int **histogram, int x_max);
    void            untag_subimage(IMAGE *aImage, int x_min, int x_max,
                                            int y_min, int y_max);




    /* Smallest distinguishable differences in rho and theta.
     */
    theta_delta = M_PI / (4*L);
    rho_delta = 0.5;

    /* min/max theta values */
    theta_min = 0;
    theta_max = M_PI;

    /* min/max rho values */
    rho_min = -L/2.0;
    rho_max = L/2.0;

    /* dimensions of histogram. */
    theta_res = (int )((theta_max - theta_min)/theta_delta);
    rho_res = (int )((rho_max - rho_min)/rho_delta);


    histogram = alloc_histogram(theta_res, rho_res);

    line_segment_array = alloc_line_segment_array(i_max, j_max);


    /* For each sub-image (i,j) */
    for (i=1; i <= i_max; i++)
        for (j=1; j <= j_max; j++)
            {
            /* Centre of sub-image */
            x_origin = i*L;
            y_origin = j*L;

            do
                {
                /* Compute HT histogram for this sub-image. */
                compute_local_hough_transform(x_origin, y_origin, L,
                                        histogram,
                                        theta_min, theta_max,
                                        rho_min, rho_max,
                                        theta_res, rho_res,
                                        aImage);

                /* Find max */
                max_count=0;
                t_max = 0;
                r_max = 0;
                for (t=0; t < theta_res; t++)
                    for (r=0; r < rho_res; r++)
                        if (histogram[t][r] > max_count)
                            {
                            t_max = t;
                            r_max = r;
                            max_count = histogram[t][r];
                            }

                /* If it is a sufficiently large maximum */
                if (max_count > gLineThickness*L)
                    {
                    /* Tag those pixels which contributed to the
                     * histogram maximum.
                     */
                    tag_pixels_along_line_segment(t_max, r_max,
                                        x_origin, y_origin, L,
                                        theta_min, theta_max,
                                        rho_min, rho_max,
                                        theta_res, rho_res,
                                        aImage);
                    line_segment = generate_line_segment(t_max, r_max,
                                        x_origin, y_origin,
                                        i, j,
                                        theta_min, theta_max,
                                        rho_min, rho_max,
                                        theta_res, rho_res);

                    line_segment_array[i][j] =
                            put_in_linked_list_of_line_segment(
                                        line_segment_array[i][j],
                                        line_segment);
                    }
                }
            while (max_count > gLineThickness*L);

            untag_subimage(aImage, x_origin-L, x_origin+L-1,
                                        y_origin-L, y_origin+L-1);
            }


    free_histogram(histogram, theta_res);


    return(line_segment_array);

}           /* end of HHT_low_level_line_detection() */



LINE_SEGMENT ***
group_line_segments(int L_parent, int L_child, int L_sub_image,
                                    int x_num_parents, int y_num_parents,
                                    int x_num_children, int y_num_children,
                                    LINE_SEGMENT ***child_line_segment_array,
                                    IMAGE *aImage,
                                    Window theWindow, GC theGC)
{
    double          rho_delta, theta_delta;
    double          rho_min, rho_max;
    double          theta_min, theta_max;
    int             theta_res, rho_res;
    int             i, j, p, q;
    int             x_origin, y_origin;
    double          max_count;
    LINE_SEGMENT    *line_segment;
    HT_ENTRY        *ht_list, *ht_entry, *ht_max;
    SUB_IMAGE       *sub_image_list;
    LINE_SEGMENT    ***parent_line_segment_array;
    LINE_SEGMENT    ***alloc_line_segment_array(int x, int y);




    /* Smallest distinguishable differences in rho and theta.
     */
    theta_delta = M_PI / (2*L_parent);
    rho_delta = 0.5;

    /* min/max theta values */
    theta_min = 0;
    theta_max = M_PI;

    /* min/max rho values */
    rho_min = -L_parent/sqrt(2.0);
    rho_max = L_parent/sqrt(2.0);

    /* dimensions of histogram
     * (except that we actually store it in a linked list).
     */
    theta_res = (int )((theta_max - theta_min)/theta_delta);
    rho_res = (int )((rho_max - rho_min)/rho_delta);


    parent_line_segment_array = alloc_line_segment_array(x_num_parents,
                                                        y_num_parents);


    /* For each parent image (i,j) */
    for (i=1; i <= x_num_parents; i++)
        for (j=1; j <= y_num_parents; j++)
            {
            /* Centre of sub-image */
            x_origin = i*L_parent - L_parent/2 + L_sub_image;
            y_origin = j*L_parent - L_parent/2 + L_sub_image;

            /* Untag sibling images */
            for (p=i*2-2; p <= i*2+1; p++)
                {
                if ((p < 1) || (p > x_num_children))
                    continue;
                for (q=j*2-2; q <= j*2+1; q++)
                    {
                    if ((q < 1) || (q > y_num_children))
                        continue;

                    for (line_segment=child_line_segment_array[p][q];
                                        line_segment != (LINE_SEGMENT *)NULL;
                                        line_segment=line_segment->next)
                        line_segment->used_already_flag = FALSE;
                    }
                }

            /* Compute HT histogram for this parent image. */
            ht_list = compute_hough_transform_for_parent(
                                        i, j,
                                        x_origin, y_origin,
                                        x_num_children, y_num_children,
                                        child_line_segment_array,
                                        theta_delta, rho_delta,
                                        theta_min, theta_max,
                                        rho_min, rho_max,
                                        theta_res, rho_res,
                                        aImage);

            do
                {
                /* Find max */
                max_count=0;
                ht_max = (HT_ENTRY *)NULL;
                for (ht_entry=ht_list; ht_entry != NULL;
                                                    ht_entry=ht_entry->next)
                    {
                    if (ht_entry->count > max_count)
                        {
                        max_count = ht_entry->count;
                        ht_max = ht_entry;
                        }
                    }


                /* If it is a sufficiently large maximum */
                if (max_count > 1)
                    {
                    /* Form the line segment
                     * i.e. work out which sub-images form it.
                     * (only use untagged child line segments).
                     */
                    sub_image_list = find_sub_images_forming_line(ht_max);


                    /* All the child line segments which contributed to
                     * this HT_ENTRY have been used already
                     * in other line segments at this level.
                     */
                    if (sub_image_list == (SUB_IMAGE *)NULL)
                        {
                        /* Clear this HT_ENTRY so that we
                         * can find the next maximum.
                         */
                        ht_max->count = 0;
                        continue;       /* Find next max */
                        }

                    /* Test that the set of sub-images is not
                     * equal to the sub-image list of any of the children.
                     */
                    if (superset_of_line_segments(sub_image_list, ht_max,
                                                    theWindow, theGC)
                                                    == FALSE)
                        {
                        /* The sub-image list of the line segment
                         * equalled the sub-image list of one
                         * of its children. Call this child L1.
                         * Thus all of the other children will be sub-sets
                         * of this child L1.
                         * Delete all child line segments except L1.
                         */
                        delete_child_line_segments_that_are_subsets(ht_max,
                                                    ht_list,
                                                    child_line_segment_array);

                        free_linked_list_of_sub_image(sub_image_list);

                        /* Clear this HT_ENTRY so that we
                         * can find the next maximum.
                         */
                        ht_max->count = 0;

                        continue;       /* Find next max */
                        }

                    /* Test that the children are connected.
                     */
                    if (test_if_children_are_connected(ht_max) == FALSE)
                        {
                        free_linked_list_of_sub_image(sub_image_list);

                        /* Clear this HT_ENTRY so that we
                         * can find the next maximum.
                         */
                        ht_max->count = 0;
                        continue;       /* Find next max */
                        }

                    /* If neither of these failed,
                     * tag the child line segments
                     * as used.
                     */
                    tag_contributing_line_segments(ht_max);

                    /* Add the line segment to
                     * parent_line_segment_array[i][j]
                     */
                    line_segment = compute_line_segment_parameters(
                                                        ht_max,
                                                        x_origin, y_origin,
                                                        theta_min, theta_max,
                                                        rho_min, rho_max,
                                                        theta_res, rho_res,
                                                        sub_image_list);

                    parent_line_segment_array[i][j] =
                            put_in_linked_list_of_line_segment(
                                        parent_line_segment_array[i][j],
                                        line_segment);

                    /* Clear this HT_ENTRY so that we
                     * can find the next maximum.
                     */
                    ht_max->count = 0;
                    }
                }
            while (max_count > 1);


            free_linked_list_of_ht_entry( ht_list );
            }


    return(parent_line_segment_array);

}           /* end of group_line_segments() */



LINE_SEGMENT *
alloc_line_segment(void)
{
    LINE_SEGMENT *line_segment;

    line_segment = (LINE_SEGMENT *)malloc( sizeof(LINE_SEGMENT) );
    if (line_segment == (LINE_SEGMENT *)NULL)
        {
        fprintf(stderr, "malloc failed in alloc_line_segment() in hht_line.c\n");
        exit(1);
        }

    line_segment->used_already_flag = FALSE;
    line_segment->part_of_higher_level_line = FALSE;
    line_segment->sub_image_list = (SUB_IMAGE *)NULL;
    line_segment->next = (LINE_SEGMENT *)NULL;
    line_segment->prev = (LINE_SEGMENT *)NULL;

    return(line_segment);

}           /* end of alloc_line_segment() */



void
free_line_segment(LINE_SEGMENT *line_segment)
{
    void    free_linked_list_of_sub_image(SUB_IMAGE *sub_image_list);


    free_linked_list_of_sub_image(line_segment->sub_image_list);
    free(line_segment);

}           /* end of free_line_segment() */



LINE_SEGMENT *
put_in_linked_list_of_line_segment(LINE_SEGMENT *head,
                                    LINE_SEGMENT *line_segment)
{
    LINE_SEGMENT  *ptr;

    if (head == NULL)
        {
        head = line_segment;
        head->next = (LINE_SEGMENT *)NULL;
        head->prev = (LINE_SEGMENT *)NULL;
        }
    else
        {
        for (ptr=head; ptr->next != NULL; ptr=ptr->next)
            ;
        ptr->next = line_segment;
        line_segment->prev = ptr;
        line_segment->next = (LINE_SEGMENT *)NULL;
        }


    return(head);

}   /* end of put_in_linked_list_of_line_segment() */



LINE_SEGMENT *
remove_from_linked_list_of_line_segment(
                            LINE_SEGMENT *head, LINE_SEGMENT *line_segment)
{
    void    free_line_segment(LINE_SEGMENT *line_segment);


    if (line_segment != (LINE_SEGMENT *)NULL)
        {
        if (line_segment == head)                               /* first */
            {
            head = head->next;
            if (head != (LINE_SEGMENT *)NULL)
                {
                /* If there is still an element left in list */
                head->prev = (LINE_SEGMENT *)NULL;
                }
            free_line_segment(line_segment);
            }
        else if (line_segment->next == (LINE_SEGMENT *)NULL)    /* last */
            {
            line_segment->prev->next = (LINE_SEGMENT *)NULL;
            free_line_segment(line_segment);
            }
        else                                                    /* other */
            {
            line_segment->prev->next = line_segment->next;
            line_segment->next->prev = line_segment->prev;
            free_line_segment(line_segment);
            }
        }

    return(head);

}           /* end of remove_from_linked_list_of_line_segment() */



SUB_IMAGE *
alloc_sub_image(void)
{
    SUB_IMAGE *sub_image;

    sub_image = (SUB_IMAGE *)malloc( sizeof(SUB_IMAGE) );
    if (sub_image == (SUB_IMAGE *)NULL)
        {
        fprintf(stderr, "malloc failed in alloc_sub_image() in hht_line.c\n");
        exit(1);
        }

    sub_image->next = (SUB_IMAGE *)NULL;

    return(sub_image);

}           /* end of alloc_sub_image() */



void
free_sub_image(SUB_IMAGE *sub_image)
{
    free(sub_image);

}           /* end of free_sub_image() */



void
free_linked_list_of_sub_image(SUB_IMAGE *sub_image_list)
{
    SUB_IMAGE   *s1, *s2;
    void        free_sub_image(SUB_IMAGE *sub_image);

    if (sub_image_list == NULL)
        return;

    s1 = sub_image_list;
    while (s1 != NULL)
        {
        s2 = s1;
        s1 = s1->next;
        free_sub_image(s2);
        }


}   /* end of free_linked_list_of_sub_image() */



SUB_IMAGE *
put_in_linked_list_of_sub_image(SUB_IMAGE *head, SUB_IMAGE *sub_image)
{
    SUB_IMAGE  *ptr;

    if (head == NULL)
        head = sub_image;
    else
        {
        for (ptr=head; ptr->next != NULL; ptr=ptr->next)
            ;
        ptr->next = sub_image;
        }

    sub_image->next = (SUB_IMAGE *)NULL;


    return(head);

}   /* end of put_in_linked_list_of_sub_image() */



/* Allocate a 2-D array of (LINE_SEGEMENT *)
 * indexed [1..x][1..y]
 * Initialise pointers to NULL.
 */
LINE_SEGMENT ***
alloc_line_segment_array(int x, int y)
{
    LINE_SEGMENT    ***p;
    int             a;
    int             i, j;


    if ( (p=(LINE_SEGMENT ***)malloc(((unsigned )x+1) * sizeof(LINE_SEGMENT  **)) ) == (LINE_SEGMENT ***)NULL)
        {
        fprintf(stderr, "Malloc #1 failed in alloc_line_segment_array() in hht_line.c\n");
        exit(1);
        }
    for(a=1; a <= x; a++)
        {
        if ((p[a]=(LINE_SEGMENT **)malloc(((unsigned )y+1)*sizeof(LINE_SEGMENT *))) == (LINE_SEGMENT **)NULL)
            {
            fprintf(stderr, "Malloc #2 failed in alloc_line_segment_array() in hht_line.c\n");
            exit(1);
            }  
        }

    for (i=1; i <= x; i++)
        for (j=1; j <= y; j++)
            p[i][j] = (LINE_SEGMENT *)NULL;

    return(p);

}           /* end of alloc_array_of_line_segment_lists() */



void
free_line_segment_array(LINE_SEGMENT ***p, int x)
{
    int             a;

    for(a=1; a <= x; a++)
        free(p[a]);
    free(p);
}                                   /* end of free_line_segment_array() */



PTR_TO_LINE_SEGMENT *
alloc_ptr_to_line_segment(void)
{
    PTR_TO_LINE_SEGMENT *p;

    p = malloc( sizeof(PTR_TO_LINE_SEGMENT) );
    if (p == (PTR_TO_LINE_SEGMENT *)NULL)
        {
        fprintf(stderr, "malloc failed in alloc_ptr_to_line_segment() in hht_line.c\n");
        exit(1);
        }

    p->line_segment = (LINE_SEGMENT *)NULL;
    p->next = (PTR_TO_LINE_SEGMENT *)NULL;
    p->prev = (PTR_TO_LINE_SEGMENT *)NULL;

    return(p);

}           /* end of alloc_ptr_to_line_segment() */



void
free_ptr_to_line_segment( PTR_TO_LINE_SEGMENT *p)
{
    free(p);
}           /* end of free_ptr_to_line_segment() */



PTR_TO_LINE_SEGMENT *
put_in_linked_list_of_ptr_to_line_segment(
                            PTR_TO_LINE_SEGMENT *list, PTR_TO_LINE_SEGMENT *p)
{
    PTR_TO_LINE_SEGMENT *p2;

    if (list == (PTR_TO_LINE_SEGMENT *)NULL)
        {
        list = p;
        list->next = (PTR_TO_LINE_SEGMENT *)NULL;
        list->prev = (PTR_TO_LINE_SEGMENT *)NULL;
        }
    else
        {
        /* Find last entry in list */
        for (p2=list; p2->next != (PTR_TO_LINE_SEGMENT *)NULL; p2=p2->next)
            ;
        p2->next = p;
        p->next = (PTR_TO_LINE_SEGMENT *)NULL;
        p->prev = (PTR_TO_LINE_SEGMENT *)p2;
        }



    return(list);

}           /* end of put_in_linked_list_of_ptr_to_line_segment() */



PTR_TO_LINE_SEGMENT *
remove_from_linked_list_of_ptr_to_line_segment(
                            PTR_TO_LINE_SEGMENT *head, PTR_TO_LINE_SEGMENT *p)
{
    void    free_ptr_to_line_segment( PTR_TO_LINE_SEGMENT *p);

    if (p != (PTR_TO_LINE_SEGMENT *)NULL)
        {
        if (p == head)                                          /* first */
            {
            head = head->next;
            if (head != (PTR_TO_LINE_SEGMENT *)NULL)
                {
                /* If there is still an element left in list */
                head->prev = (PTR_TO_LINE_SEGMENT *)NULL;
                }
            free_ptr_to_line_segment(p);
            }
        else if (p->next == (PTR_TO_LINE_SEGMENT *)NULL)        /* last */
            {
            p->prev->next = (PTR_TO_LINE_SEGMENT *)NULL;
            free_ptr_to_line_segment(p);
            }
        else                                                    /* other */
            {
            p->prev->next = p->next;
            p->next->prev = p->prev;
            free_ptr_to_line_segment(p);
            }
        }

    return(head);

}           /* end of remove_from_linked_list_of_ptr_to_line_segment() */



void
free_linked_list_of_ptr_to_line_segment( PTR_TO_LINE_SEGMENT *p_list)
{
    PTR_TO_LINE_SEGMENT *p1, *p2;
    void                free_ptr_to_line_segment( PTR_TO_LINE_SEGMENT *p);

    if (p_list == NULL)
        return;

    p1 = p_list;
    while (p1 != NULL)
        {
        p2 = p1;
        p1 = p1->next;
        free_ptr_to_line_segment(p2);
        }


}           /* end of free_linked_list_of_ptr_to_line_segment() */



HT_ENTRY *
alloc_ht_entry(void)
{
    HT_ENTRY    *ht_entry;

    ht_entry = (HT_ENTRY *)malloc( sizeof(HT_ENTRY) );
    if (ht_entry == (HT_ENTRY *)NULL)
        {
        fprintf(stderr, "malloc failed in alloc_ht_entry() in hht_line.c\n");
        exit(1);
        }

    ht_entry->list_of_ptr_to_line_segment = (PTR_TO_LINE_SEGMENT *)NULL;
    ht_entry->next = (HT_ENTRY *)NULL;

    return(ht_entry);

}           /* end of alloc_ht_entry() */



void
free_ht_entry(HT_ENTRY *ht_entry)
{
    void    free_linked_list_of_ptr_to_line_segment( PTR_TO_LINE_SEGMENT *p_list);


    free_linked_list_of_ptr_to_line_segment(ht_entry->list_of_ptr_to_line_segment);
    free(ht_entry);

}           /* end of free_ht_entry() */



HT_ENTRY *
put_in_linked_list_of_ht_entry(HT_ENTRY *head, HT_ENTRY *ht_entry)
{
    HT_ENTRY  *ptr;

    if (head == NULL)
        head = ht_entry;
    else
        {
        for (ptr=head; ptr->next != NULL; ptr=ptr->next)
            ;
        ptr->next = ht_entry;
        }

    ht_entry->next = (HT_ENTRY *)NULL;


    return(head);

}   /* end of put_in_linked_list_of_ht_entry() */



void
free_linked_list_of_ht_entry( HT_ENTRY *ht_list)
{
    HT_ENTRY    *ht1, *ht2;
    void        free_ht_entry( HT_ENTRY *ht);

    if (ht_list == NULL)
        return;

    ht1 = ht_list;
    while (ht1 != NULL)
        {
        ht2 = ht1;
        ht1 = ht1->next;
        free_ht_entry(ht2);
        }

}           /* end of free_linked_list_of_ptr_to_line_segment() */



/* Compute HT histogram for sub-image centred on (x_origin, y_origin)
 */
void
compute_local_hough_transform(int x_origin, int y_origin, int L,
                                        int **histogram,
                                        double theta_min, double theta_max,
                                        double rho_min, double rho_max,
                                        double theta_res, double rho_res,
                                        IMAGE *aImage)
{
    int     t, r;
    int     x, y;
    double  theta, rho;
    int     get_pixel_value(IMAGE *aImage, int x, int y);
    BOOLEAN test_pixel_for_tag(IMAGE *aImage, int x, int y, int tag);


    /* Clear the histogram */
    for (t=0; t < theta_res; t++)
        for (r=0; r < rho_res; r++)
            histogram[t][r] = 0;


    /* Compute Hough Transform for all black pixels
     * that have not been tagged.
     * A pixel is tagged if it belongs to a line segment
     * that has already been identified (hence the pixel
     * is effectively removed from the image if it lies on
     * a line segment that has already been found).
     */
    for (x=x_origin-L; x < x_origin+L; x++)
        for (y=y_origin-L; y < y_origin+L; y++)
            if ((get_pixel_value(aImage,x,y) == BLACK) &&
                                (test_pixel_for_tag(aImage,x,y,1) == FALSE))
                {
                for (t=0; t < theta_res; t++)
                    {
                    /* Given theta, compute rho
                     */
                    theta = theta_min + (theta_max-theta_min) *
                                        (((double )t) / (double )theta_res);
                    rho = cos(theta)*(double )(x-x_origin) +
                                        sin(theta)*(double )(y-y_origin);

                    r = (int )(rho_res * (rho-rho_min)/(rho_max-rho_min));

                    /* Check array bounds */
                    if ((r >= 0) && (r < rho_res))
                        histogram[t][r]++;

                    /* Check array bounds */
                    if (((r-1) >= 0) && ((r-1) < rho_res))
                        histogram[t][r-1]++;

                    /* Check array bounds */
                    if (((r+1) >= 0) && ((r+1) < rho_res))
                        histogram[t][r+1]++;
                    }
                }

}           /* end of compute_local_hough_transform() */



/* Tag those pixels which contributed to the histogram
 * value histogram[t][r], or to one of its neighbours.
 * The motivation is to tag those pixels which contributed
 * to the histogram's maximum.
 */
void
tag_pixels_along_line_segment(int t, int r,
                            int x_origin, int y_origin, int L,
                            double theta_min, double theta_max,
                            double rho_min, double rho_max,
                            double theta_res, double rho_res,
                            IMAGE *aImage)
{

    if ((t != 0) && (t != theta_res-1))
        {
        tag_pixels(t-1, r, x_origin, y_origin, L, theta_min, theta_max,
                            rho_min, rho_max, theta_res, rho_res,
                            aImage);
        tag_pixels(t, r, x_origin, y_origin, L, theta_min, theta_max,
                            rho_min, rho_max, theta_res, rho_res,
                            aImage);
        tag_pixels(t+1, r, x_origin, y_origin, L, theta_min, theta_max,
                            rho_min, rho_max, theta_res, rho_res,
                            aImage);
        }
    else if (t == 0)
        {
        tag_pixels(theta_res-1, r, x_origin, y_origin, L, theta_min, theta_max,
                            rho_min, rho_max, theta_res, rho_res,
                            aImage);
        tag_pixels(0, r, x_origin, y_origin, L, theta_min, theta_max,
                            rho_min, rho_max, theta_res, rho_res,
                            aImage);
        tag_pixels(1, r, x_origin, y_origin, L, theta_min, theta_max,
                            rho_min, rho_max, theta_res, rho_res,
                            aImage);
        }
    else if (t == theta_res-1)
        {
        tag_pixels(theta_res-2, r, x_origin, y_origin, L, theta_min, theta_max,
                            rho_min, rho_max, theta_res, rho_res,
                            aImage);
        tag_pixels(theta_res-1, r, x_origin, y_origin, L, theta_min, theta_max,
                            rho_min, rho_max, theta_res, rho_res,
                            aImage);
        tag_pixels(0, r, x_origin, y_origin, L, theta_min, theta_max,
                            rho_min, rho_max, theta_res, rho_res,
                            aImage);
        }

}           /* end of tag_pixels_along_line() */



/* Tag pixels which contibuted to the histogram entries:
 * histogram[t][r-1], histogram[t][r], histogram[t][r+1]
 */
void
tag_pixels(int t, int r, int x_origin, int y_origin, int L,
                            double theta_min, double theta_max,
                            double rho_min, double rho_max,
                            double theta_res, double rho_res,
                            IMAGE *aImage)
{
    int     x, y;
    int     r2;
    double  theta, rho;
    int     get_pixel_value(IMAGE *aImage, int x, int y);
    BOOLEAN test_pixel_for_tag(IMAGE *aImage, int x, int y, int tag);
    void    tag_pixel(IMAGE *aImage, int x, int y, int tag);


    for (x=x_origin-L; x < x_origin+L; x++)
        for (y=y_origin-L; y < y_origin+L; y++)
            if ((get_pixel_value(aImage,x,y) == BLACK) &&
                                (test_pixel_for_tag(aImage,x,y,1) == FALSE))
                {
                /* Given theta, compute rho
                 */
                theta = theta_min + (theta_max-theta_min) *
                                        (((double )t) / (double )theta_res);
                rho = cos(theta)*(double )(x-x_origin) +
                                        sin(theta)*(double )(y-y_origin);

                r2 = (int )(rho_res * (rho-rho_min)/(rho_max-rho_min));

                /* If pixel lies along line,
                 * tag it.
                 */
                if ( ((r2 == r-1) || (r2 == r) || (r2 == r+1)) &&
                                (r2 >= 0) && (r2 < theta_res))
                    tag_pixel(aImage, x, y, 1);
                }


}           /* end of tag_pixels() */


LINE_SEGMENT *
generate_line_segment(int t, int r,
                    int x_origin, int y_origin,
                    int i, int j,
                    double theta_min, double theta_max,
                    double rho_min, double rho_max,
                    int theta_res, int rho_res)
{
    LINE_SEGMENT    *line_segment;
    SUB_IMAGE       *sub_image;


    line_segment = alloc_line_segment();
    line_segment->theta = theta_min +
                            (theta_max-theta_min) *
                            (((double )t) /
                                        (double )theta_res );
    line_segment->rho = rho_min +
                            (rho_max-rho_min) *
                            (((double )r) /
                                        (double )rho_res );

    /* Origin for rho, theta */
    line_segment->x_origin = x_origin;
    line_segment->y_origin = y_origin;

    line_segment->used_already_flag = FALSE;
    line_segment->part_of_higher_level_line = FALSE;

    /* Record which sub-image that the
     * line segment passes through
     */
    line_segment->sub_image_list = NULL;
    sub_image = alloc_sub_image();
    sub_image->i = i;
    sub_image->j = j;
    line_segment->sub_image_list =
        put_in_linked_list_of_sub_image(
                        line_segment->sub_image_list,
                        sub_image);
    line_segment->next = (LINE_SEGMENT *)NULL;


    return(line_segment);

}           /* end of generate_line_segment() */



/* Draw line segment found in low level sub-image.
 * This function is called from 'HHT_low_level_line_detection()'
 */
void
draw_line_segment_in_sub_image(LINE_SEGMENT *line_segment,
                            int L,
                            IMAGE *aImage,
                            Window theWindow, GC theGC)
{
    int     x_origin, y_origin;
    int     x0, y0, x1, y1, x2, y2;


    /* centre of sub-image
     * Note that sub_image_list only contains one entry,
     * ie. we are assuming that this function is called
     * from 'HHT_low_level_line_detection()'
     */
    x_origin = L * line_segment->sub_image_list->i;
    y_origin = L * line_segment->sub_image_list->j;

    /* Centre of line segment */
    x0 = x_origin + line_segment->rho * cos(line_segment->theta);
    y0 = y_origin + line_segment->rho * sin(line_segment->theta);

    /* Compute start and end points of
     * line segment that is to be drawn.
     */
    x1 = x0 - (L/2)*cos(line_segment->theta + M_PI_2);
    y1 = y0 - (L/2)*sin(line_segment->theta + M_PI_2);
    x2 = x0 + (L/2)*cos(line_segment->theta + M_PI_2);
    y2 = y0 + (L/2)*sin(line_segment->theta + M_PI_2);

    XDrawLine(gDisplay, theWindow, theGC, x1, y1, x2, y2);
    XFlush(gDisplay);

}           /* end of draw_line_segment_in_sub_image() */



HT_ENTRY *
compute_hough_transform_for_parent( int i_parent, int j_parent,
                                    double x_origin, double y_origin,
                                    int x_num_children, int y_num_children,
                                    LINE_SEGMENT ***child_line_segment_array,
                                    double theta_delta, double rho_delta,
                                    double theta_min, double theta_max,
                                    double rho_min, double rho_max,
                                    double theta_res, double rho_res,
                                    IMAGE *aImage)
{
    HT_ENTRY        *ht_list;
    LINE_SEGMENT    *line_segment_list, *line_segment;
    int             i, j;
    double          theta, rho;
    int             theta_index;
    double          delta_x, delta_y;
    int             t, r;


    ht_list = (HT_ENTRY *)NULL;


    for (i=i_parent*2-2; i <= i_parent*2+1; i++)
        {
        if ((i < 1) || (i > x_num_children))
            continue;
        for (j=j_parent*2-2; j <= j_parent*2+1; j++)
            {
            if ((j < 1) || (j > y_num_children))
                continue;

            /* For each line segment in this child */
            line_segment_list = child_line_segment_array[i][j];
            for (line_segment=line_segment_list; line_segment != NULL;
                                            line_segment=line_segment->next)
                {
                delta_x = x_origin - line_segment->x_origin;
                delta_y = y_origin - line_segment->y_origin;
                for (theta_index=-3; theta_index <= 3; theta_index++)
                    {
                    theta = line_segment->theta + theta_index*theta_delta;
                    rho = line_segment->rho -
                                        delta_x * cos(theta) -
                                        delta_y * sin(theta);


                    /* t in [t-3, t-2, t-1, t, t+1, t+2, t+3]
                     * r in [r-2, r-1, r, r+1, r+2]
                     */
                    t = theta_index + (int )(theta_res *
                                        (line_segment->theta-theta_min)/
                                        (theta_max-theta_min));

                    r = (int )(rho_res * (rho-rho_min)/(rho_max-rho_min));
                    ht_list = record_value_in_ht_list(ht_list,
                                                        t, r-2, 0.95,
                                                        line_segment, i, j);
                    ht_list = record_value_in_ht_list(ht_list,
                                                        t, r-1, 0.98,
                                                        line_segment, i, j);
                    ht_list = record_value_in_ht_list(ht_list,
                                                        t, r, 1.0,
                                                        line_segment, i, j);
                    ht_list = record_value_in_ht_list(ht_list,
                                                        t, r+1, 0.98,
                                                        line_segment, i, j);
                    ht_list = record_value_in_ht_list(ht_list,
                                                        t, r+2, 0.95,
                                                        line_segment, i, j);

                    }           /* end of 'for (theta_index=-2;...' */
                }           /* end of 'for (line_segment=...' */
            }           /* end of 'for (j=j_parent/2-1;...' */
        }           /* end of 'for (i=i_parent/2-1;...' */


    return(ht_list);

}           /* end of compute_hough_transform_for_parent() */



HT_ENTRY *
record_value_in_ht_list(HT_ENTRY *ht_list, int t, int r, double val,
                                LINE_SEGMENT *line_segment, int i, int j)
{
    HT_ENTRY    *ht;
    HT_ENTRY    *alloc_ht_entry(void);
    PTR_TO_LINE_SEGMENT *p;
    PTR_TO_LINE_SEGMENT *alloc_ptr_to_line_segment(void);
    PTR_TO_LINE_SEGMENT *put_in_linked_list_of_ptr_to_line_segment(
                            PTR_TO_LINE_SEGMENT *list, PTR_TO_LINE_SEGMENT *p);


    if (ht_list == (HT_ENTRY *)NULL)
        {
        ht_list = alloc_ht_entry();
        ht_list->t = t;
        ht_list->r = r;
        ht_list->count = val;

        ht_list->list_of_ptr_to_line_segment = (PTR_TO_LINE_SEGMENT *)NULL;
        p = alloc_ptr_to_line_segment();
        p->i = i;
        p->j = j;
        p->line_segment = line_segment;
        ht_list->list_of_ptr_to_line_segment =
                            put_in_linked_list_of_ptr_to_line_segment(
                                        ht_list->list_of_ptr_to_line_segment,
                                        p);

        ht_list->next = (HT_ENTRY *)NULL;
        return (ht_list);
        }


    /* Search for entry with same (t,r) values */
    for (ht=ht_list; ht->next != NULL; ht=ht->next)
        {
        if ((ht->t == t) && (ht->r == r))
            break;
        }

    /* If found matching entry */
    if ((ht->t == t) && (ht->r == r))
        {
        ht->count += val;
        p = alloc_ptr_to_line_segment();
        p->i = i;
        p->j = j;
        p->line_segment = line_segment;
        ht->list_of_ptr_to_line_segment =
                            put_in_linked_list_of_ptr_to_line_segment(
                                        ht->list_of_ptr_to_line_segment,
                                        p);
        }
    else
        {
        /* Create new entry */
        ht->next = alloc_ht_entry();
        ht = ht->next;
        ht->t = t;
        ht->r = r;
        ht->count = val;

        ht->list_of_ptr_to_line_segment = (PTR_TO_LINE_SEGMENT *)NULL;
        p = alloc_ptr_to_line_segment();
        p->i = i;
        p->j = j;
        p->line_segment = line_segment;
        ht->list_of_ptr_to_line_segment =
                            put_in_linked_list_of_ptr_to_line_segment(
                                        ht->list_of_ptr_to_line_segment,
                                        p);

        ht->next = (HT_ENTRY *)NULL;
        }


    return (ht_list);

}           /* end of record_value_in_ht_list() */



/* Create a linked list containing which sub-images
 * make up the line segment described by the (HT_ENTRY )ht.
 *
 * To get this, we use the line segments which added to the
 * (HT_ENTRY )ht, and make up a linked list of the sub-images
 * which formed these line segments
 * (checking that we don't include any sub-images twice).
 */
SUB_IMAGE *
find_sub_images_forming_line(HT_ENTRY *ht)
{
    SUB_IMAGE   *sub_image_list;
    SUB_IMAGE   *s, *s_copy, *sub_image;
    PTR_TO_LINE_SEGMENT *p;
    SUB_IMAGE   *alloc_sub_image(void);
    SUB_IMAGE   *put_in_linked_list_of_sub_image(SUB_IMAGE *head,
                                                    SUB_IMAGE *sub_image);

    sub_image_list = (SUB_IMAGE *)NULL;


    for (p=ht->list_of_ptr_to_line_segment; p != (PTR_TO_LINE_SEGMENT *)NULL;
                                                p=p->next)
        {
        if (p->line_segment->used_already_flag == TRUE)
            continue;

        for (s=p->line_segment->sub_image_list; s != (SUB_IMAGE *)NULL;
                                                    s=s->next)
            {
            /* Is sub-image already in linked list? */
            for (sub_image=sub_image_list; sub_image != (SUB_IMAGE *)NULL;
                                                    sub_image=sub_image->next)
                {
                if ((s->i == sub_image->i) && (s->j == sub_image->j))
                    break;
                }
            /* If not in linked list, add it to linked list */
            if (sub_image == (SUB_IMAGE *)NULL)
                {
                s_copy = alloc_sub_image();
                s_copy->i = s->i;
                s_copy->j = s->j;
                sub_image_list = put_in_linked_list_of_sub_image(
                                                sub_image_list, s_copy);
                }
            }
        }


    return(sub_image_list);

}           /* end of find_sub_images_forming_line() */



/* Test that the set of sub-images is not
 * equal to the sub-image list of any of the children.
 * sub_image_list is a linked list of the sub-images
 * making up the line segment.
 * ht_max is the HT_ENTRY which generated this line segment.
 * ht_max contains a link to all of the sub-line-segments which
 * made up the line segment, so we can check that the line segment
 * is not equal to any of the contributing sub-line-segments.
 */
BOOLEAN
superset_of_line_segments(SUB_IMAGE *sub_image_list, HT_ENTRY *ht_entry,
                                        Window theWindow, GC theGC)
{
    SUB_IMAGE       *s1, *s2;
    PTR_TO_LINE_SEGMENT *p;
    LINE_SEGMENT    *line_segment;


    /* For each line segment that contributed to this
     * HT entry.
     */
    for (p=ht_entry->list_of_ptr_to_line_segment;
                                    p != (PTR_TO_LINE_SEGMENT *)NULL;
                                    p=p->next)
        {
        line_segment = p->line_segment;

        if (line_segment->used_already_flag == TRUE)
            continue;


        /* Check if the line segment's sub-images are a sub-set
         * of those in 'sub_image_list'.
         * i.e. search for an element in 'sub_image_list'
         * that is not in 'line_segment->sub_image_list'.
         */
        for (s1=sub_image_list; s1 != (SUB_IMAGE *)NULL; s1=s1->next)
            {
            for (s2=line_segment->sub_image_list; s2 != (SUB_IMAGE *)NULL;
                                                                s2=s2->next)
                {
                if ((s1->i == s2->i) && (s1->j == s2->j))
                    break;          /* found it */
                }
            if (s2 == (SUB_IMAGE *)NULL)
                {
                /* Found an element which is in 'sub_image'
                 * but not in 'line_segment->sub_image',
                 * so 'sub_image' is superset of
                 * 'line_segment->sub_image'.
                 */
                break;          /* check next line segment */
                }
            }

        if (s1 == (SUB_IMAGE *)NULL)
            {
            /* If we made it this far, then
             * then sub-image is a subset of
             * line_segment->sub_image_list
             */
            return(FALSE);
            }
        }           /* end of 'for (p=ht_entry->...' */


    /* We checked all of the line-segments in
     * ht_entry->list_of_ptr_to_line_segment
     * and the line segment was a subset of all of them.
     */
    return(TRUE);

}           /* end of superset_of_line_segments() */



/* The sub-image list of the line segment
 * equalled the sub-image list of one
 * of its children. Call this child L1.
 * Thus all of the other children will be sub-sets
 * of this child L1.
 * Delete all child line segments except L1.
 */
void
delete_child_line_segments_that_are_subsets(HT_ENTRY *ht_max,
                                    HT_ENTRY *ht_list,
                                    LINE_SEGMENT ***child_line_segment_array)
{
    PTR_TO_LINE_SEGMENT *p, *p2, *p3;
    LINE_SEGMENT        *line_segment, *largest_line_segment;
    SUB_IMAGE           *sub_image;
    HT_ENTRY            *ht;
    int                 max_num_sub_images, num_sub_images;


    /* Find the child which comprises
     * the most sub-images - largest_line_segment
     * All other children will be a subset of this one.
     */
    max_num_sub_images = 0;
    largest_line_segment = (LINE_SEGMENT *)NULL;
    for (p=ht_max->list_of_ptr_to_line_segment;
                                p != (PTR_TO_LINE_SEGMENT *)NULL; p=p->next)
        {
        line_segment = p->line_segment;

        if (line_segment->used_already_flag == TRUE)
            continue;

        num_sub_images = 0;
        for (sub_image=line_segment->sub_image_list;
                                sub_image != (SUB_IMAGE *)NULL;
                                sub_image=sub_image->next)
            {
            num_sub_images++;
            }
        if (num_sub_images > max_num_sub_images)
            {
            max_num_sub_images = num_sub_images;
            largest_line_segment = line_segment;
            }
        }

    if (max_num_sub_images == 0)
        {
        /* This should never happen.
         * I'm just being paranoid checking for it.
         */
        return;
        }


    /* Remove references in ht_list to all of the smaller line segments
     * but not the references in ht_max.
     * These will be removed last.
     */
    for (p=ht_max->list_of_ptr_to_line_segment;
                                p != (PTR_TO_LINE_SEGMENT *)NULL; p=p->next)
        {
        line_segment = p->line_segment;

        if (line_segment->used_already_flag == TRUE)
            continue;

        if (line_segment == largest_line_segment)
            continue;

        for (ht=ht_list; ht != (HT_ENTRY *)NULL; ht=ht->next)
            {
            if (ht == ht_max)
                continue;
            p2=ht->list_of_ptr_to_line_segment;
            while (p2 != (PTR_TO_LINE_SEGMENT *)NULL)
                {
                p3=p2;
                p2=p2->next;
                if (p3->line_segment == line_segment)
                    {
                    ht->list_of_ptr_to_line_segment =
                                remove_from_linked_list_of_ptr_to_line_segment(
                                        ht->list_of_ptr_to_line_segment, p3);
                    }
                }           /* end of 'while (p2 !=...' */

            }           /* end of 'for (ht=ht_list;...' */
        }           /* end of 'for (p=ht_max->...' */


    /* Remove reference to the smaller line segments
     * in child_line_segment_array[][]
     */
    for (p=ht_max->list_of_ptr_to_line_segment;
                                p != (PTR_TO_LINE_SEGMENT *)NULL; p=p->next)
        {
        line_segment = p->line_segment;

        if (line_segment->used_already_flag == TRUE)
            continue;

        if (line_segment == largest_line_segment)
            continue;
        child_line_segment_array[p->i][p->j] =
                    remove_from_linked_list_of_line_segment(
                                child_line_segment_array[p->i][p->j],
                                line_segment);
        }


    /* Finally, remove reference to the
     * smaller line_segments in ht_max.
     */
    p=ht_max->list_of_ptr_to_line_segment;
    while (p != (PTR_TO_LINE_SEGMENT *)NULL)
        {
        line_segment = p->line_segment;
        p2=p;
        p=p->next;

        if (line_segment->used_already_flag == TRUE)
            continue;

        if (line_segment == largest_line_segment)
            continue;

        ht_max->list_of_ptr_to_line_segment =
                    remove_from_linked_list_of_ptr_to_line_segment(
                                ht_max->list_of_ptr_to_line_segment, p2);
        }           /* end of 'while (p !=...' */


}           /* end of delete_child_line_segments_that_are_subsets() */



BOOLEAN
test_if_children_are_connected(HT_ENTRY *ht_entry)
{
    int                 num_children, index;
    PTR_TO_LINE_SEGMENT *p;
    CHILD               *array;
    BOOLEAN             adjacent_flag;


    /* Count number of child images */
    num_children = 0;
    for (p = ht_entry->list_of_ptr_to_line_segment;
                            p != (PTR_TO_LINE_SEGMENT *)NULL; p=p->next)
        {
        /* sub-line segment already used to form another line segment */
        if (p->line_segment->used_already_flag == TRUE)
            continue;

        num_children++;
        }


    /* All siblings have already been used in other line segments */
    if (num_children == 0)
        return( FALSE );


    array = (CHILD *)malloc(num_children * sizeof(CHILD));

    /* Fill array with co-ords of children images */
    index = 0;
    for (p = ht_entry->list_of_ptr_to_line_segment;
                            p != (PTR_TO_LINE_SEGMENT *)NULL; p=p->next)
        {
        /* sub-line segment already used to form another line segment */
        if (p->line_segment->used_already_flag == TRUE)
            continue;

        array[index].i = p->i;
        array[index].j = p->j;
        array[index].tag = FALSE;
        index++;
        }

    /* Set tag to TRUE on any CHILD ajacent to the first CHILD */
    tag_adjacent(0, array, num_children);


    /* If any untagged CHILD remain, then the CHILD
     * are not adjacent */
    adjacent_flag = TRUE;
    for (index=0; index < num_children; index++)
        {
        if (array[index].tag == FALSE)
            {
            adjacent_flag = FALSE;
            break;                      /* out of for-loop */
            }
        }

    free(array);

    return(adjacent_flag);

}           /* end of test_if_children_are_connected() */



void
tag_adjacent(int index, CHILD *array, int num_entries)
{
    int     c;
    int     di, dj;


    array[index].tag = TRUE;

    for (c=0; c < num_entries; c++)
        {
        /* If untagged and adjacent */
        di = array[index].i - array[c].i;
        dj = array[index].j - array[c].j;
        if ((array[c].tag == FALSE) &&
                        ((di == -1) || (di == 0) || (di == 1)) &&
                        ((dj == -1) || (dj == 0) || (dj == 1)))
            {
            tag_adjacent(c, array, num_entries);
            }
        }
}           /* end of tag_adjacent() */




/* tag the child line segments
 * as used.
 */
void
tag_contributing_line_segments(HT_ENTRY *ht_entry)
{
    PTR_TO_LINE_SEGMENT     *p;


    for (p=ht_entry->list_of_ptr_to_line_segment;
                        p != (PTR_TO_LINE_SEGMENT *)NULL; p=p->next)
        {
        if (p->line_segment->used_already_flag == TRUE)
            continue;
        p->line_segment->used_already_flag = TRUE;
        p->line_segment->part_of_higher_level_line = TRUE;
        }

}           /* end of tag_contributing_line_segments() */



LINE_SEGMENT *
compute_line_segment_parameters(HT_ENTRY *ht_entry,
                    int x_origin, int y_origin,
                    double theta_min, double theta_max,
                    double rho_min, double rho_max,
                    int theta_res, int rho_res,
                    SUB_IMAGE *sub_image_list)
{
    LINE_SEGMENT    *line_segment;


    line_segment = alloc_line_segment();
    line_segment->theta = theta_min +
                            (theta_max-theta_min) *
                            (((double )ht_entry->t) /
                                        (double )theta_res );
    line_segment->rho = rho_min +
                            (rho_max-rho_min) *
                            (((double )ht_entry->r) /
                                        (double )rho_res );

    /* Origin for rho, theta */
    line_segment->x_origin = x_origin;
    line_segment->y_origin = y_origin;

    line_segment->used_already_flag = FALSE;
    line_segment->part_of_higher_level_line = FALSE;

    /* Record which sub-image that the
     * line segment passes through
     */
    line_segment->sub_image_list = sub_image_list;

    line_segment->next = (LINE_SEGMENT *)NULL;
    line_segment->prev = (LINE_SEGMENT *)NULL;


    return(line_segment);

}           /* end of compute_line_segment_parameters() */



void
draw_line_segment_across_sub_images(LINE_SEGMENT *line_segment,
                            int L,                  /* size of sub-image */
                            IMAGE *aImage,
                            Window theWindow, GC theGC)
{
    double      x0, y0;
    double      dx, dy;
    double      t_min, t_max, t;
    SUB_IMAGE   *s;
    double      x1, y1, x2, y2, x, y;
    int         x_start, y_start, x_end, y_end;


    /* Define the line by the equation:
     *    (x,y) = (x0,y0) + t*(dx,dy)
     */

    /* A point on the line */
    x0 = line_segment->x_origin + line_segment->rho *cos(line_segment->theta);
    y0 = line_segment->y_origin + line_segment->rho *sin(line_segment->theta);

    /* Gradient vector of line */
    dx = cos(line_segment->theta+M_PI_2);
    dy = sin(line_segment->theta+M_PI_2);


    t_min = MAXDOUBLE;          /* a really big number */
    t_max = -(MAXDOUBLE-1);     /* a really small number */
    /* Decide the starting and end points of the line that we
     * are going to draw
     * i.e. t_min and t_max
     */
    for (s=line_segment->sub_image_list; s != (SUB_IMAGE *)NULL; s=s->next)
        {
        /* Compute the begining and end of this sub-image */
        x1 = (s->i)*L - L/2;
        x2 = (s->i)*L + L/2;
        y1 = (s->j)*L - L/2;
        y2 = (s->j)*L + L/2;

        /* Compute where the line starts and stops
         * within this sub-image.
         */
        if (dx != 0)
            {
            /* Where do we intercept x0 */
            t = (x1 - x0) / dx;
            /* Is this point in the sub-image */
            y = y0 + t*dy;
            if (((y-1 >= y1) || (y+1 >= y1)) &&
                ((y-1 <= y2) || (y+1 <= y2)) )
                {
                /* It is in the sub-image */
                if (t < t_min)
                    t_min = t;
                if (t > t_max)
                    t_max = t;
                }

            /* Where do we intercept y0 */
            t = (x2 - x0) / dx;
            /* Is this point in the sub-image */
            y = y0 + t*dy;
            if (((y-1 >= y1) || (y+1 >= y1)) &&
                ((y-1 <= y2) || (y+1 <= y2)) )
                {
                /* It is in the sub-image */
                if (t < t_min)
                    t_min = t;
                if (t > t_max)
                    t_max = t;
                }
            }                   /* end of 'if (dx != 0)' */

        if (dy != 0)
            {
            /* Where do we intercept y0 */
            t = (y1 - y0) / dy;
            /* Is this point in the sub-image */
            x = x0 + t*dx;
            if (((x-1 >= x1) || (x+1 >= x1)) &&
                ((x-1 <= x2) || (x+1 <= x2)) )
                {
                /* It is in the sub-image */
                if (t < t_min)
                    t_min = t;
                if (t > t_max)
                    t_max = t;
                }

            /* Where do we intercept y0 */
            t = (y2 - y0) / dy;
            /* Is this point in the sub-image */
            x = x0 + t*dx;
            if (((x-1 >= x1) || (x+1 >= x1)) &&
                ((x-1 <= x2) || (x+1 <= x2)) )
                {
                /* It is in the sub-image */
                if (t < t_min)
                    t_min = t;
                if (t > t_max)
                    t_max = t;
                }
            }                   /* end of 'if (dy != 0)' */
        }           /* end of 'for (s=line_segment->sub_image_list;...' */

    x_start = (int )(x0 + t_min*dx);
    y_start = (int )(y0 + t_min*dy);

    x_end = (int )(x0 + t_max*dx);
    y_end = (int )(y0 + t_max*dy);

    XDrawLine(gDisplay, theWindow, theGC, x_start, y_start, x_end, y_end);
    XFlush(gDisplay);

}           /* end of draw_line_segment_across_sub_images() */



void
find_line_ends_HHT(LINE_SEGMENT *line_segment,
                            int L,                  /* size of sub-image */
                            IMAGE *aImage,
                            Window theWindow, GC theGC,
                            int *x_start, int *y_start,
                            int *x_end, int *y_end)
{
    double      x0, y0;
    double      dx, dy;
    double      t_min, t_max, t;
    SUB_IMAGE   *s;
    double      x1, y1, x2, y2, x, y;


    /* Define the line by the equation:
     *    (x,y) = (x0,y0) + t*(dx,dy)
     */

    /* A point on the line */
    x0 = line_segment->x_origin + line_segment->rho *cos(line_segment->theta);
    y0 = line_segment->y_origin + line_segment->rho *sin(line_segment->theta);

    /* Gradient vector of line */
    dx = cos(line_segment->theta+M_PI_2);
    dy = sin(line_segment->theta+M_PI_2);


    t_min = MAXDOUBLE;          /* a really big number */
    t_max = -(MAXDOUBLE-1);     /* a really small number */
    /* Decide the starting and end points of the line that we
     * are going to draw
     * i.e. t_min and t_max
     */
    for (s=line_segment->sub_image_list; s != (SUB_IMAGE *)NULL; s=s->next)
        {
        /* Compute the begining and end of this sub-image */
        x1 = (s->i)*L - L/2;
        x2 = (s->i)*L + L/2;
        y1 = (s->j)*L - L/2;
        y2 = (s->j)*L + L/2;

        /* Compute where the line starts and stops
         * within this sub-image.
         */
        if (dx != 0)
            {
            /* Where do we intercept x0 */
            t = (x1 - x0) / dx;
            /* Is this point in the sub-image */
            y = y0 + t*dy;
            if (((y-1 >= y1) || (y+1 >= y1)) &&
                ((y-1 <= y2) || (y+1 <= y2)) )
                {
                /* It is in the sub-image */
                if (t < t_min)
                    t_min = t;
                if (t > t_max)
                    t_max = t;
                }

            /* Where do we intercept y0 */
            t = (x2 - x0) / dx;
            /* Is this point in the sub-image */
            y = y0 + t*dy;
            if (((y-1 >= y1) || (y+1 >= y1)) &&
                ((y-1 <= y2) || (y+1 <= y2)) )
                {
                /* It is in the sub-image */
                if (t < t_min)
                    t_min = t;
                if (t > t_max)
                    t_max = t;
                }
            }                   /* end of 'if (dx != 0)' */

        if (dy != 0)
            {
            /* Where do we intercept y0 */
            t = (y1 - y0) / dy;
            /* Is this point in the sub-image */
            x = x0 + t*dx;
            if (((x-1 >= x1) || (x+1 >= x1)) &&
                ((x-1 <= x2) || (x+1 <= x2)) )
                {
                /* It is in the sub-image */
                if (t < t_min)
                    t_min = t;
                if (t > t_max)
                    t_max = t;
                }

            /* Where do we intercept y0 */
            t = (y2 - y0) / dy;
            /* Is this point in the sub-image */
            x = x0 + t*dx;
            if (((x-1 >= x1) || (x+1 >= x1)) &&
                ((x-1 <= x2) || (x+1 <= x2)) )
                {
                /* It is in the sub-image */
                if (t < t_min)
                    t_min = t;
                if (t > t_max)
                    t_max = t;
                }
            }                   /* end of 'if (dy != 0)' */
        }           /* end of 'for (s=line_segment->sub_image_list;...' */

    *x_start = (int )(x0 + t_min*dx);
    *y_start = (int )(y0 + t_min*dy);

    *x_end = (int )(x0 + t_max*dx);
    *y_end = (int )(y0 + t_max*dy);

}           /* end of find_line_ends_HHT() */



void
display_line_segments_at_this_level(LINE_SEGMENT ***line_segment_array,
                                    int x_num_parents, int y_num_parents,
                                    int L_sub_image,
                                    IMAGE *aImage, XImage *aXImage,
                                    Window theWindow, GC theGC)
{
    int             i, j;
    int             color_index;
    LINE_SEGMENT    *line_segment;
    XGCValues       values, old_values;
    void            draw_image(XImage *aXImage, Window theWindow, GC theGC);


    /* Display what we found in this level of line detection */


    /* Change the line width.
     * Make a copy of the old line width. We will restore it
     * at the end of the function.
     */
    XGetGCValues(gDisplay, theGC, GCLineWidth | GCForeground | GCFunction, &old_values);
    values.line_width = 15;
    values.function = GXcopy;
    XChangeGC( gDisplay, theGC, GCLineWidth | GCFunction, &values );


    color_index = 0;
    draw_image(aXImage, theWindow, theGC);
    XSetFunction(gDisplay, theGC, GXcopy);
    for (i=1; i <= x_num_parents; i++)
        for (j=1; j <= y_num_parents; j++)
            {
            for (line_segment=line_segment_array[i][j];
                                            line_segment != NULL;
                                            line_segment = line_segment->next)
                {
                /* Change the line width.
                 */
                XSetForeground(gDisplay, theGC,
                                            gRandomColors[color_index++].pixel);
                if (color_index == NUM_RANDOM_COLORS)
                    color_index=0;
                /* Draw line segment */
                draw_line_segment_across_sub_images(line_segment, L_sub_image,
                                        aImage, theWindow, theGC);

                }
            }


    /* Change GC back to old line width.
     */
    XChangeGC( gDisplay, theGC, GCLineWidth | GCFunction, &old_values );

}           /* end of display_line_segments_at_this_level() */



int
record_and_display_all_line_segments(LINE_SEGMENT ****line_segment_array,
                                    int num_levels,
                                    int *x_num_images_array,
                                    int *y_num_images_array,
                                    int L_sub_image,
                                    IMAGE *aImage, XImage *aXImage,
                                    Window theWindow, GC theGC)
{
    int             line_width = 15;
    int             num_lines;
    int             level, i, j;
    int             color_index;
    int             x_start, y_start, x_end, y_end;
    LINE_SEGMENT    *line_segment;
    XGCValues       values, old_values;
    void            draw_image(XImage *aXImage, Window theWindow, GC theGC);
    void            add_line_to_list_of_objects_found(int x_start, int y_start,
                                    int x_end, int y_end,
                                    int width, int color_index);




    /* Change the line width.
     * Make a copy of the old line width. We will restore it
     * at the end of the function.
     */
    XGetGCValues(gDisplay, theGC, GCLineWidth | GCForeground | GCFunction, &old_values);
    values.line_width = line_width;
    values.function = GXcopy;
    XChangeGC( gDisplay, theGC, GCLineWidth | GCFunction, &values );


    color_index = 0;
    draw_image(aXImage, theWindow, theGC);
    XSetFunction(gDisplay, theGC, GXcopy);



    num_lines = 0;

    /* Display longest lines first
     */
    for (level=num_levels-1; level > 0 ;level--)
        {
        for (i=1; i <= x_num_images_array[level]; i++)
            for (j=1; j <= y_num_images_array[level]; j++)
                {
                for (line_segment=line_segment_array[level][i][j];
                                            line_segment != NULL;
                                            line_segment = line_segment->next)
                    {
                    if (line_segment->part_of_higher_level_line == TRUE)
                        continue;


                    /* find lines ends */
                    find_line_ends_HHT(line_segment,
                                        L_sub_image,
                                        aImage, theWindow, theGC,
                                        &x_start, &y_start,
                                        &x_end, &y_end);

                    /* Record the line segment */
                    num_lines++;
                    add_line_to_list_of_objects_found(x_start, y_start,
                                        x_end, y_end,
                                        line_width, color_index);

                    /* Draw the line segment */
                    XSetForeground(gDisplay, theGC,
                                            gRandomColors[color_index++].pixel);
                    if (color_index == NUM_RANDOM_COLORS)
                        color_index=0;
                    XDrawLine(gDisplay, theWindow, theGC,
                                        x_start, y_start, x_end, y_end);
                    XFlush(gDisplay);
                    }
                }
        }

    /* Change GC back to old line width.
     */
    XChangeGC( gDisplay, theGC, GCLineWidth | GCFunction, &old_values );


    return(num_lines);

}           /* end of record_and_display_all_line_segments() */
