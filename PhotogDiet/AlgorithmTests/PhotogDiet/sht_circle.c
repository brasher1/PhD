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


#define A_SMALL_NEIGHBOURHOOD   (gLineThickness*8)
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
void    find_circles_with_SHT(IMAGE *aImage, XImage *aXImage, Window theWindow, GC theGC);
CIRCLE  *alloc_circle(void);
void    free_circle(CIRCLE *c);
CIRCLE  *put_in_linked_list_of_circle(CIRCLE *head, CIRCLE *circle);
void    free_linked_list_of_circle(CIRCLE *circle_head);
void    SHT_circular_hough_transform(IMAGE *aImage, int **histogram, Window theWindow, GC theGC);
double  find_tangent_of_edge(IMAGE *aImage, int x_p, int y_p);
void    average_histogram(int **histogram, int x_max, int y_max);
void    flood_fill(int x, int y, int **histogram, int x_max, int y_max, int *x_stack, int *y_stack);
CIRCLE  *find_radius_of_circles(POINT_LIST *centre_point_list, IMAGE *aImage);
void    SHT_hough_transform_for_radius(int x_center, int y_center, IMAGE *aImage, int *hist, int radius_max);
CIRCLE  *find_circle_radius(int *hist, int radius_max, POINT *centre_point, CIRCLE *circle_list);




void
find_circles_with_SHT(IMAGE *aImage, XImage *aXImage, Window theWindow, GC theGC)
{
/* PARAMETERS */
    int     window_size = 50;           /* Window for max of histogram */
    double  sample_proportion_for_local_max = 0.1;
    int     min_local_max_value = 40;


    int     color_index=0;
    int     **histogram;
    POINT_LIST  *centre_point_list;
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
    void    free_linked_list_of_point(POINT_LIST *point_list);
    void    free_linked_list_of_circle(CIRCLE *circle_head);
    void    SHT_circular_hough_transform(IMAGE *aImage, int **histogram, Window theWindow, GC theGC);
    void    average_histogram(int **histogram, int x_max, int y_max);
    POINT_LIST      *find_local_max_of_histogram(int win_size,
                                        int min_local_max_value,
                                        double sample_proportion_for_local_max,
                                        int **histogram, int x_max, int y_max,
                                        Window theWindow, GC theGC);
    CIRCLE  *find_radius_of_circles(POINT_LIST *centre_point_list, IMAGE *aImage);
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
                            "Started Standard Hough Transform",
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
        fprintf(stderr, "Could not open file in find_circles_with_SHT() in sht_circle.c\n");
        exit(1);
        }
#endif
#ifdef HUGH_DEBUG
    dat_hugh_file = fopen("g.dat", "w");
    if (dat_hugh_file == NULL)
        {
        fprintf(stderr, "Could not open file in find_circles_with_SHT() in sht_circle.c\n");
        exit(1);
        }
#endif


    num_circles = 0;

    histogram = alloc_histogram(aImage->x, aImage->y);
    SHT_circular_hough_transform(aImage, histogram, theWindow, theGC);
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
                fprintf(dat_hugh_file, "%d %d %d\n", x_hugh, y_hugh, histogram[x_hugh][y_hugh]);
                }
#endif

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
#ifdef XGOBI_DEBUG
    fclose(dat_file);
#endif
#ifdef HUGH_DEBUG
    fclose(dat_hugh_file);
#endif


    draw_image(aXImage, theWindow, theGC);
    if (num_circles == 1)
        sprintf(results_message, "Found 1 circle with SHT.");
    else
        sprintf(results_message, "Found %d circles with SHT.", num_circles);
    display_message_and_wait(theWindow, 0, 0, 4,
                            "",
                            results_message,
                            "Hit Left Mouse Button to continue",
                            "");


}   /* end of find_circles_with_SHT() */



CIRCLE *
alloc_circle(void)
{
    CIRCLE  *circle;

    circle = (CIRCLE *)malloc(sizeof(CIRCLE));
    if (circle == NULL)
        {
        fprintf(stderr, "malloc failed in alloc_circle() in sht_circle.c\n");
        exit(1);
        }

    circle->next = NULL;

    return(circle);

}   /* end of alloc_circle() */



void
free_circle(CIRCLE *c)
{
    /* Admittedly, this function is a bit of a waste of time,
     * but I put it in since I have a free function for
     * all of my other data structures, e.g.
     * free_gaus(), free_histogram(), free_point(), free_basis(), free_image()
     */

    free(c);
}



CIRCLE *
put_in_linked_list_of_circle(CIRCLE *head, CIRCLE *circle)
{
    CIRCLE  *ptr;

    if (head == NULL)
        head = circle;
    else
        {
        for (ptr=head; ptr->next != NULL; ptr=ptr->next)
            ;
        ptr->next = circle;
        }

    circle->next = NULL;


    return(head);

}   /* end of put_in_linked_list_of_circle() */



void
free_linked_list_of_circle(CIRCLE *circle_head)
{
    CIRCLE  *c1;
    void    free_circle(CIRCLE *c);

    while (circle_head != NULL)
        {
        c1 = circle_head;
        circle_head = circle_head->next;
        free_circle(c1);
        }

}   /* end of free_linked_list_of_circle() */



void
SHT_circular_hough_transform(IMAGE *aImage, int **histogram, Window theWindow, GC theGC)
{
    int     x, y, i, j;
    int     i_min, i_max, j_min, j_max;
    int     temp;
    double  theta, slope;
    int     get_pixel_value(IMAGE *aImage, int x, int y);

    double  find_tangent_of_edge(IMAGE *aImage, int x, int y);


    /* Find Hough transform of points.
     */
    for (x=0; x < aImage->x; x++)
        for (y=0; y < aImage->y; y++)
            {
            if (get_pixel_value(aImage, x, y) == BLACK)
                {
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
                }   /* end of 'if (get_pixel_value(aImage, x, y) == BLACK)' */
            }



}   /* end of SHT_circular_hough_transform() */



/* Find the tangent of the edge at point (x_p,y_p)
 * Do this by looking at the black pixels within
 * a small neighbourhood, and find the line of
 * best fit (using the method of least-squares)
 * which passes through the point (x_p, y_p)
 * The return value is in the range [-M_PI_2, M_PI_2]
 * M_PI_2 is defined in #include <math.h>
 * If there were no other black pixels in
 * the neighbourhood, then we can not estimate
 * the angle and return the value NO_ANGLE
 * which is #defined to be a number outside
 * of the range [-M_PI_2, M_PI_2].
 */
double
find_tangent_of_edge(IMAGE *aImage, int x_p, int y_p)
{
    int     num_pixels;
    int     x, y, x_min, x_max, y_min, y_max;
    int     sum1, sum2;
    double  slope, theta;
    int     get_pixel_value(IMAGE *aImage, int x, int y);

    x_min = x_p - A_SMALL_NEIGHBOURHOOD;
    if (x_min < 0)
        x_min = 0;
    x_max = x_p + A_SMALL_NEIGHBOURHOOD;
    if (x_max >= aImage->x)
        x_max = aImage->x-1;

    y_min = y_p - A_SMALL_NEIGHBOURHOOD;
    if (y_min < 0)
        y_min = 0;
    y_max = y_p + A_SMALL_NEIGHBOURHOOD;
    if (y_max >= aImage->y)
        y_max = aImage->y-1;


    num_pixels = 0;
    sum1 = sum2 = 0;
    for (x=x_min; x <= x_max; x++)
        for (y=y_min; y <= y_max; y++)
            if (get_pixel_value(aImage, x, y) == BLACK)
                {
                num_pixels++;
                sum1 += (x-x_p)*(y-y_p);
                sum2 += (x-x_p)*(x-x_p);
                }

    /* The slope of the best line through the
     * black pixels in this neighbourhood
     * is sum1 / sum2.
     * We will convert this to an angle.
     */
    if (num_pixels == 1)
        {
        /* If there was only one black pixel
         * i.e. at co-ord (x_p, y_p)
         * In that case, we can't tell what the
         * angle is at that point, so we will
         * return the value NO_ANGLE
         */
        theta = NO_ANGLE;
        }
    else if (sum2 == 0)
        {
        /* If all points were on a vertical line.
         * In this case, set the angle to M_PI_2 radians.
         */
        theta = M_PI_2;
        }
    else
        {
        slope = ((double )sum1) / (double )sum2;
        theta = atan(slope);
        }


    return(theta);

}   /* end of find_tangent_of_edge() */



void
average_histogram(int **histogram, int x_max, int y_max)
{
    int     **temp_histogram;
    int     x, y, i, j;
    int     sum;
    int     **alloc_histogram(int x_max, int y_max);
    void    free_histogram(int **histogram, int x_max);

    temp_histogram = alloc_histogram(x_max, y_max);

    for (x=0; x < x_max; x++)
        for (y=0; y < y_max; y++)
            temp_histogram[x][y] = histogram[x][y];

    for (x=1; x < x_max-1; x++)
        for (y=1; y < y_max-1; y++)
            {
            sum = 0;
            for (i=x-1; i <= x+1; i++)
                for (j=y-1; j <= y+1; j++)
                    sum += temp_histogram[i][j];
            histogram[x][y] = sum / 9;
            }

    free_histogram(temp_histogram, x_max);

}   /* end of average_histogram() */


void
flood_fill(int x, int y, int **histogram, int x_max, int y_max, int *x_stack, int *y_stack)
{
    int     i, j;
    int     sp;


    sp = -1;

    /* Mark first pixel.
     */
    histogram[x][y] = 1;
    /* Put first pixel on stack
     */
    sp++;
    x_stack[sp] = x;  y_stack[sp] = y;


    do {
        /* Take pixel off stack
         */
        x = x_stack[sp];  y = y_stack[sp];
        sp--;

        /* Check the surrounding pixels.
         */
        for (i=x-1; i <= x+1; i++)
            for (j=y-1; j <= y+1; j++)
                if ((i >= 0) && (i < x_max) && (j >= 0) && (j < y_max))
                    if (histogram[i][j] == 255)
                        {
                        /* Mark pixel.
                         */
                        histogram[i][j] = 1;
                        /* Put pixel on the stack.
                         */
                        sp++;
                        x_stack[sp] = i;  y_stack[sp] = j;
                        }

    } while (sp >= 0);


}       /* end of flood_fill() */



/* We know the circle centers. They are the points
 * stored in centre_point_list.
 * We will now do a 1-D hough transform, to find the radius
 * of the circles.
 */
CIRCLE *
find_radius_of_circles(POINT_LIST *centre_point_list, IMAGE *aImage)
{
    POINT   *centre_point;
    int     *hist, radius_max;
    CIRCLE  *circle_list;
    void    SHT_hough_transform_for_radius(int x_center, int y_center, IMAGE *aImage, int *hist, int radius_max);
    CIRCLE  *find_circle_radius(int *hist, int radius_max, POINT *centre_point, CIRCLE *circle_list);


    if (centre_point_list == NULL)              /* Error check */
        return((CIRCLE *)NULL);


    circle_list = NULL;


    /* Allocate a 1-D histogram
     */
    radius_max = (int )sqrt(SQR(aImage->x) + SQR(aImage->y));
    hist = (int *)malloc( radius_max * sizeof(int) );
    if (hist == NULL)
        {
        fprintf(stderr, "malloc failed in find_radius_of_circles() in sht_circle.c\n");
        exit(1);
        }


    for (centre_point=centre_point_list->head; centre_point != NULL; centre_point=centre_point->next)
        {
        SHT_hough_transform_for_radius((int )centre_point->x[1], (int )centre_point->x[2], aImage, hist, radius_max);
        /* Given the hough transform for the radius,
         * find_circle_radius() finds the radius of the circle
         * at the given center (there may be more than one).
         * and puts them in the linked list of circles.
         */
        circle_list = find_circle_radius(hist, radius_max, centre_point, circle_list);
        }

    free(hist);

    return(circle_list);


}   /* end of find_radius_of_circles() */



void
SHT_hough_transform_for_radius(int x_center, int y_center, IMAGE *aImage, int *hist, int radius_max)
{
    int     x, y;
    int     radius;
    int     get_pixel_value(IMAGE *aImage, int x, int y);

    /* Initialise 1-D histogram to 0
     */
    for (radius=0; radius < radius_max; radius++)
        hist[radius] = 0;


    /* hough transform for finding radius
     */
    for (x=0; x < aImage->x; x++)
        for (y=0; y < aImage->y; y++)
            if (get_pixel_value(aImage, x, y) == BLACK)
                {
                radius = (int )sqrt(SQR(x-x_center) + SQR(y-y_center));
                hist[radius]++;
                }


}   /* end of SHT_hough_transform_for_radius() */



CIRCLE *
find_circle_radius(int *hist, int radius_max, POINT *centre_point, CIRCLE *circle_list)
{
    CIRCLE  *circle;
    int     i, j;
    int     radius, best_radius, r_lower, r_upper, count;
    int     start, end;
    int     *temp_hist;
    double  max_value, proportion;  /* proportion of the circle that exists */
    CIRCLE  *alloc_circle(void);
    CIRCLE  *put_in_linked_list_of_circle(CIRCLE *head, CIRCLE *circle);


    /* Blur the 1-D histogram
     * This allows the radius to vary by a few pixels
     * as we go around the circle.
     */
    temp_hist = (int *)malloc( radius_max * sizeof(int) );
    if (temp_hist == NULL)
        {
        fprintf(stderr, "malloc failed in find_circle_radius() in sht_circle.c\n");
        exit(1);
        }
    for (i=0; i < radius_max; i++)
        {
        temp_hist[i] = hist[i];
        hist[i] = 0;
        }

    for (i=7; i < (radius_max-7); i++)
        for (j=i-7; j <= (i+7); j++)
            hist[i] += temp_hist[j];

    free(temp_hist);



    do
        {
        /* Find the best radius for the circle.
         */
        max_value = 0.65;   /* must be larger than this value,
                             * i.e. at least .65 of the circle must exist.
                             */
        best_radius = 0;
        /* Don't bother with circles with radius less than 15
         * They are probably just a blob of black pixels.
         */
/* PARAM */
        for (radius=15; radius < radius_max; radius++)
            {
            /* 'proportion' is how much of the circle
             * exists. Notice the
             * (2*M_PI*(double )(radius)
             * is the perimeter of the circle multiplied
             * by the line thickness.
             * Find the radius at which most of the circle exists.
             */
            proportion = ((double )hist[radius]) /
                                    (2*M_PI*(double )(radius)*gLineThickness);
            if (proportion > max_value)
                {
                best_radius = radius;
                max_value = proportion;
                }
            }

            /* It is posible that the histogram
             * doesn't have a peak, but a plateau.
             * i.e. a couple of adjacent entries all share
             * the maximum value.
             * This was found to happen when we tested the
             * program with perfect circles.
             * If this is so, then find the central value.
             */
            count = hist[best_radius];
            for (r_lower=best_radius; ((r_lower>=15) && (hist[r_lower]==count)); r_lower--)
                ;
            r_lower++;  /* Went one too far */
            for (r_upper=best_radius; ((r_upper<radius_max) && (hist[r_upper]==count)); r_upper++)
                ;
            r_upper--;  /* Went one too far */
            best_radius = (r_upper+r_lower) / 2;


            /* If over 0.65 of the circle was found to exist
             * at that radius (give or take 4 pixels)
             * then add it to the linked list of circles
             */
            if (max_value > 0.65)
                {
                circle = alloc_circle();
                circle->x = centre_point->x[1];
                circle->y = centre_point->x[2];
                circle->radius = (double )best_radius;
                circle_list = put_in_linked_list_of_circle(circle_list, circle);
                }

            /* Remove the hist[] values at about that radius
             * and then search to see if there is a second circle
             * with the same mean.
             */
            if ((r_upper-r_lower) > 30)
                {
                start = r_lower;
                end = r_upper;
                }
            else
                {
                start = best_radius-15;
                end = best_radius+15;
                }
            if (start < 0)
                start = 0;
            if (end >= radius_max)
                end = radius_max-1;
            for (radius = start; radius <= end; radius++)
                hist[radius] = 0;

        }
    while (max_value > 0.65);




    return(circle_list);

}   /* end of find_circle_radius() */
