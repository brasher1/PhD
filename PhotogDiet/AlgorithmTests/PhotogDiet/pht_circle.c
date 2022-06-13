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


#define A_SMALL_NEIGHBOURHOOD   (gLineThickness*5)
#define NO_ANGLE    -100


/* Data Structures
 */


typedef struct circle_type {

    double  x, y;               /* co-ords of center */
    double  radius;             /* circle radius */
    struct circle_type  *next;

} CIRCLE;




/* Function Prototypes
 */
void                find_circles_with_PHT(IMAGE *aImage, XImage *aXImage,
                                            Window theWindow, GC theGC);
void                PHT_circular_hough_transform(double sample_proportion,
                                            IMAGE *aImage,
                                            BLACK_PIXEL_LIST *black_pixel_list,
                                            int **histogram,
                                            Window theWindow, GC theGC);
BLACK_PIXEL_LIST    *form_linked_list_of_black_pixels_for_PHT(IMAGE *aImage);




void
find_circles_with_PHT(IMAGE *aImage, XImage *aXImage, Window theWindow, GC theGC)
{
/* PARAMETERS */
    double  sample_proportion = 0.1;
    int     window_size = 50;           /* Window for max of histogram */
    double  sample_proportion_for_local_max = 0.1;
    int     min_local_max_value = 40;


    int     color_index=0;
    int     **histogram;
    BLACK_PIXEL_LIST    *black_pixel_list;
    POINT_LIST   *centre_point_list;
    CIRCLE  *circle, *circle_list;
    DISPLAY_OBJECT  *d;
    int     num_circles;
    char    results_message[100];

    MESSAGE_WINDOW  *message_window;
    XEvent  report;
    int             theScreen;
    unsigned long   theBlackPixel;
    MESSAGE_WINDOW  *alloc_message_window(void);
    void    init_message_window_fields(MESSAGE_WINDOW *message_window, Window MainWindow, int x, int y, int num_strings, ...);
    void    write_message_window_strings(MESSAGE_WINDOW *message_window);
    void    free_message_window(MESSAGE_WINDOW *message_window);
    void    draw_image(XImage *aXImage, Window theWindow, GC theGC);

    int     **alloc_histogram(int x_max, int y_max);
    void    free_histogram(int **histogram, int x_max);
    BLACK_PIXEL_LIST    *form_linked_list_of_black_pixels_for_PHT(IMAGE *aImage);
    void    free_linked_list_of_black_pixel(BLACK_PIXEL_LIST *black_pixel_list);
    void    free_linked_list_of_point(POINT_LIST *point_list);
    void    free_linked_list_of_circle(CIRCLE *circle_head);
    void    PHT_circular_hough_transform(double sample_proportion, IMAGE *aImage, BLACK_PIXEL_LIST *black_pixel_list, int **histogram, Window theWindow, GC theGC);
    void    average_histogram(int **histogram, int x_max, int y_max);
    POINT_LIST      *find_local_max_of_histogram(int win_size,
                                        int min_local_max_value,
                                        double sample_proportion_for_local_max,
                                        int **histogram, int x_max, int y_max,
                                        Window theWindow, GC theGC);
    CIRCLE  *find_radius_of_circles(POINT_LIST *centre_point_list,
                                        IMAGE *aImage);
    DISPLAY_OBJECT      *alloc_display_object(void);
    DISPLAY_OBJECT_LIST *put_in_linked_list_of_display_object(DISPLAY_OBJECT_LIST *display_oject_list, DISPLAY_OBJECT *d);
    void    display_message_and_wait(Window theWindow, int x, int y, int num_strings, ...);
#ifdef XGOBI_DEBUG
    int     x_xgobi, y_xgobi;
    FILE    *dat_file;
#endif
#ifdef HUGH_DEBUG
    int     x_hugh, y_hugh;
    FILE    *dat_hugh_file;
#endif





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
                            "Started Probabilistic Hough Transform",
                            "to find circles.",
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






#ifdef XGOBI_DEBUG
    dat_file = fopen("h.dat", "w");
    if (dat_file == NULL)
        {
        fprintf(stderr, "Could not open file in find_circles_with_PHT() in pht_circle.c\n");
        exit(1);
        }
#endif
#ifdef HUGH_DEBUG
    dat_hugh_file = fopen("g.dat", "w");
    if (dat_hugh_file == NULL)
        {
        fprintf(stderr, "Could not open file in find_circles_with_PHT() in pht_circle.c\n");
        exit(1);
        }
#endif


    num_circles = 0;

/* Form a linked list of the black pixels in the image */
    black_pixel_list = form_linked_list_of_black_pixels_for_PHT( aImage );

/* No black pixels in image */
    if ((black_pixel_list == (BLACK_PIXEL_LIST *)NULL) ||
                                        (black_pixel_list->num_elements == 0))
        {
        XUnmapWindow(gDisplay, message_window->win);
        free_message_window(message_window);

        draw_image(aXImage, theWindow, theGC);
        sprintf(results_message, "Found 0 circles.");
        display_message_and_wait(theWindow, 0, 0, 4,
                            "",
                            results_message,
                            "Hit Left Mouse Button to continue",
                            "");
        return;
        }



    histogram = alloc_histogram(aImage->x, aImage->y);
    PHT_circular_hough_transform(sample_proportion, aImage, black_pixel_list,
                                                histogram,
                                                theWindow, theGC);
    average_histogram(histogram, aImage->x, aImage->y);


#ifdef XGOBI_DEBUG
    for (x_xgobi=0; x_xgobi < aImage->x; x_xgobi++)
        for (y_xgobi=0; y_xgobi < aImage->y; y_xgobi++)
            if (histogram[x_xgobi][y_xgobi] > (5*gLineThickness))
                {
                fprintf(dat_file, "%d %d %d\n", x_xgobi, y_xgobi, histogram[x_xgobi][y_xgobi]);
                }
#endif



#ifdef HUGH_DEBUG
    for (x_hugh=0; x_hugh < aImage->x; x_hugh++)
        for (y_hugh=0; y_hugh < aImage->y; y_hugh++)
            if (histogram[x_hugh][y_hugh] > 3)
                {
                fprintf(dat_hugh_file, "%d %d %d\n", x_hugh, y_hugh, thresholded_histogram[x_hugh][y_hugh]);
                }
#endif

    /* Scale 'min_local_max_value' depending on what
     * proportion of the image we are sampling.
     */
    min_local_max_value = (int )( ((double )min_local_max_value) *
                                                        2*sample_proportion);
    centre_point_list = find_local_max_of_histogram(window_size,
                                        min_local_max_value,
                                        sample_proportion_for_local_max,
                                        histogram, aImage->x, aImage->y,
                                        theWindow, theGC);

    circle_list = find_radius_of_circles(centre_point_list, aImage);
    free_linked_list_of_point(centre_point_list);

    /* Record each circle to be displayed.
     * It will be drawn when the screen re-draws.
     */

    color_index = 0;
    for (circle=circle_list; circle != NULL; circle=circle->next)
        {
        num_circles++;
        /* Record the circle to be displayed.
         */
        d = alloc_display_object();
        d->type = hough_circle;
        d->obj.hough_circle.x = (int )(circle->x-circle->radius);
        d->obj.hough_circle.y = (int )(circle->y-circle->radius);
        d->obj.hough_circle.diameter = (unsigned int)(2*circle->radius);
        d->obj.hough_circle.width = 15;
        d->obj.hough_circle.color_index = color_index;
        gDisplayObjectList = put_in_linked_list_of_display_object(gDisplayObjectList, d);
        color_index++;
        if (color_index == NUM_RANDOM_COLORS)
            color_index = 0;
        }


    XUnmapWindow(gDisplay, message_window->win);

    free_message_window(message_window);


    free_histogram(histogram, aImage->x);
    free_linked_list_of_circle(circle_list);
    free_linked_list_of_black_pixel(black_pixel_list);
#ifdef XGOBI_DEBUG
    fclose(dat_file);
#endif
#ifdef HUGH_DEBUG
    fclose(dat_hugh_file);
#endif

    draw_image(aXImage, theWindow, theGC);
    if (num_circles == 1)
        sprintf(results_message, "Found 1 circle with PHT.");
    else
        sprintf(results_message, "Found %d circles with PHT.", num_circles);
    display_message_and_wait(theWindow, 0, 0, 4,
                            "",
                            results_message,
                            "Hit Left Mouse Button to continue",
                            "");


}   /* end of find_circles_with_PHT() */






void
PHT_circular_hough_transform(double sample_proportion, IMAGE *aImage, BLACK_PIXEL_LIST *black_pixel_list, int **histogram, Window theWindow, GC theGC)
{
    int                 x, y, i, j, index, count;
    int                 i_min, i_max, j_min, j_max;
    int                 num_points_sampled;
    int                 temp;
    double              theta, slope;
    BLACK_PIXEL         *ptr, *p1;
    int                 get_pixel_value(IMAGE *aImage, int x, int y);
    double              find_tangent_of_edge(IMAGE *aImage, int x, int y);




/* If no points in image */
    if (black_pixel_list == (BLACK_PIXEL_LIST *)NULL)
        return;


    
    num_points_sampled = (int )(sample_proportion *
                                    (double )black_pixel_list->num_elements);

    /* Find Hough transform of points.
     */
    for (count=0; count < num_points_sampled; count++)
        {
        /* Randomly choose a black pixel */
        index = (int )(drand48() * (double )black_pixel_list->num_elements);

        /* p1 is set to point to a randomly chosen entry
         * in the linked list of black pixels.
         */
        ptr = black_pixel_list->head;
        for (i=0; i < index; i++, ptr=ptr->next)
            ;
        p1 = ptr;

        x = p1->x;
        y = p1->y;
        
        /* Estimate norm to this pixel.
         * using least squares.
         */
        theta = find_tangent_of_edge(aImage, x, y);
        /* NO_ANGLE means that we were unable
         * to estimate the angle of the edge
         * at position (x,y).
         */

        if (theta != NO_ANGLE)
            {
            theta += M_PI_2;    /* The angle of the norm */
            /* Keep theta in the range [M_PI_2, -M_PI_2]
             */
            if (theta > M_PI_2)
                theta -= M_PI;

            /* Draw a straight line in the transform space
             * along the norm.
             */
            if (ABS(theta) < M_PI_4)
                {
                /* x co-ordinate varies more than y co-ordinate.
                 */
                slope = tan(theta);
                /* First, calculate the i values which
                 * give j = 0 and j = aImage->y
                 */

                /* zero intercept
                 */
                if (ABS(slope) < 10e-5) /* near zero */
                    i_min = 0;
                else
                    i_min = (int )((slope*(double )x - (double )y) / slope);

                /* when j = aImage->y-1
                 */
                if (ABS(slope) < 10e-5) /* near zero */
                    i_max = aImage->x-1;
                else
                    i_max = (int )((slope*(double )x + (double )(aImage->y-1 - y)) / slope);
                /* If the slope is negative, i_min will be
                 * greater than i_max.
                 */
                if (i_min > i_max)
                    {
                    temp = i_max;
                    i_max = i_min;
                    i_min = temp;
                    }

                /* Check that these lie within the image.
                 */
                if (i_min < 0)
                    i_min = 0;
                if (i_max >= aImage->x)
                    i_max = aImage->x-1;

                /* Just to make sure that we don't go
                 * outside the range for j of [0, aImage->y)
                 */
                i_min++; i_max--;
                for (i=i_min; i <= i_max; i++)
                    {
                    j = y + (int )(slope*(double )(i-x));
                    histogram[i][j]++;
                    }
                }
            else        /* if (ABS(theta) >= M_PI_4) */
                {
                /* y co-ordinate varies more than x co-ordinate.
                 */
                slope = cos(theta) / sin(theta);

                /* First, calculate the j values which
                 * give i = 0 and i = aImage->x
                 */

                /* zero intercept
                 */
                if (ABS(slope) < 10e-5) /* near zero */
                    j_min = 0;
                else
                    j_min = (int )((slope*(double )y - (double )x) / slope);

                /* when i = aImage->x-1
                 */
                if (ABS(slope) < 10e-5) /* near zero */
                    j_max = aImage->y-1;
                else
                    j_max = (int )((slope*(double )y + (double )(aImage->x-1 - x)) / slope);
                /* If the slope is negative, j_min will be
                 * greater than j_max.
                 */
                if (j_min > j_max)
                    {
                    temp = j_max;
                    j_max = j_min;
                    j_min = temp;
                    }

                /* Check that these lies within the image.
                 */
                if (j_min < 0)
                    j_min = 0;
                if (j_max >= aImage->y)
                    j_max = aImage->y-1;

                /* Just to make sure that we don't go
                 * outside the range for i of [0, aImage->x)
                 */
                j_min++; j_max--;
                for (j=j_min; j <= j_max; j++)
                    {
                    i = x + (int )(slope*(double )(j-y));
                    histogram[i][j]++;
                    }
                }
            }   /* end of 'if (theta != NO_ANGLE)' */
        }       /* end of 'for (i=0; i <...' */




}   /* end of PHT_circular_hough_transform() */



BLACK_PIXEL_LIST *
form_linked_list_of_black_pixels_for_PHT(IMAGE *aImage)
{
    int                 x, y;
    BLACK_PIXEL         *p;
    BLACK_PIXEL_LIST    *black_pixel_list;
    BLACK_PIXEL         *alloc_black_pixel(void);
    BLACK_PIXEL_LIST    *put_in_linked_list_of_black_pixel(BLACK_PIXEL_LIST *black_pixel_list, BLACK_PIXEL *p);
    int                 get_pixel_value(IMAGE *aImage, int x, int y);



    black_pixel_list = NULL;


    for (x=0; x < aImage->x; x++)
        for (y=0; y < aImage->y; y++)
            if (get_pixel_value(aImage, x, y) == BLACK)
                {
                p = alloc_black_pixel();
                p->x = x;
                p->y = y;
                black_pixel_list = put_in_linked_list_of_black_pixel(
                                                    black_pixel_list, p);
                }


    return(black_pixel_list);

}   /* end of form_linked_list_of_black_pixels_for_PHT() */
