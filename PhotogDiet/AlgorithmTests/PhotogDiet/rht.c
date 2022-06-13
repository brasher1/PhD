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

#define NO_ANGLE    -100


/* Globals
 */
int     max_level;


/* Function Prototypes
 */
void                find_lines_with_RHT(IMAGE *aImage, XImage *aXImage,
                                        Window theWindow, GC theGC);
void                find_circles_with_RHT(IMAGE *aImage, XImage *aXImage,
                                        Window theWindow, GC theGC);
void                find_ellipses_with_RHT(IMAGE *aImage, XImage *aXImage,
                                        Window theWindow, GC theGC);
BLACK_PIXEL_LIST    *form_linked_list_of_black_pixels(IMAGE *aImage,
                                        MAP_FROM_POSITION_TO_BLACK_PIXEL **map);
POINT               *find_line_between_two_random_pixels(
                                        BLACK_PIXEL_LIST *black_pixel_list);
BLACK_PIXEL_LIST    *find_and_remove_line_from_image(double theta, double rho,
                                        int width,
                                        MAP_FROM_POSITION_TO_BLACK_PIXEL *map,
                                        BLACK_PIXEL_LIST *black_pixel_list,
                                        IMAGE *aImage,
                                        int color_index,
                                        Window theWindow, GC theGC,
                                        int *x_start, int *y_start,
                                        int *x_end, int *y_end);
BLACK_PIXEL_LIST    *remove_line_from_image_in_rht(int x_start, int y_start,
                                        int x_end, int y_end,
                                        int width,
                                        MAP_FROM_POSITION_TO_BLACK_PIXEL *map,
                                        BLACK_PIXEL_LIST *black_pixel_list,
                                        IMAGE *aImage,
                                        int color_index,
                                        Window theWindow, GC theGC);
void                add_line_to_list_of_objects_found(int x_start, int y_start,
                                        int x_end, int y_end,
                                        int width, int color_index);

int                 count_number_black_pixels_on_line(
                                        int x_start, int y_start,
                                        int x_end, int y_end,
                                        int width,
                                        IMAGE *aImage);
BOOLEAN             true_line(NODE *node, int true_line_threshold,
                                        IMAGE *aImage, int width);
POINT               *find_circle_given_three_random_pixels(
                                        BLACK_PIXEL_LIST *black_pixel_list);
BOOLEAN             compute_circle(double x1, double y1, double x2, double y2,
                                        double x3, double y3,
                                        double *x_centre, double *y_centre,
                                        double *radius);
BOOLEAN             true_circle(int x_centre, int y_centre, int radius,
                                        double true_circle_threshold,
                                        IMAGE *aImage, int width,
                                        Window theWindow, GC theGC);
void                add_circle_to_list_of_objects_found(
                                        double x_centre, double y_centre,
                                        double radius,
                                        int width, int color_index);
BLACK_PIXEL_LIST    *remove_circle_from_image(
                                        int x_centre, int y_centre,
                                        int radius,
                                        int width,
                                        MAP_FROM_POSITION_TO_BLACK_PIXEL *map,
                                        BLACK_PIXEL_LIST *black_pixel_list,
                                        IMAGE *aImage,
                                        int color_index,
                                        Window theWindow, GC theGC);
POINT               *find_ellipse_given_three_random_pixels(
                                        BLACK_PIXEL_LIST *black_pixel_list,
                                        IMAGE *aImage);
BOOLEAN             estimate_ellipse_centre(BLACK_PIXEL *p1,
                                        BLACK_PIXEL *p2, BLACK_PIXEL *p3,
                                        IMAGE *aImage,
                                        double *x_return, double *y_return);
BOOLEAN             find_line_TM(double x1, double y1, double theta1,
                                        double slope1,
                                        double x2, double y2, double theta2,
                                        double slope2,
                                        IMAGE *aImage, double *m_return,
                                        double *b_return);
BOOLEAN             estimate_other_ellipse_parameters(BLACK_PIXEL *p1,
                                        BLACK_PIXEL *p2,
                                        BLACK_PIXEL *p3,
                                        double x_centre, double y_centre,
                                        double *a_return, double *b_return,
                                        double *c_return);
int                 lu_decomposition(double **a, int n, int *indx, double *d);
void                lu_back_substitution(double **a, int n, int *indx,
                                        double *b);
BOOLEAN             transform_ellipse_parameters(double a,
                                        double b, double c,
                                        double *r1, double *r2,
                                        double *theta);
BOOLEAN             true_ellipse(double x_centre, double y_centre,
                                        double r1, double r2, double theta,
                                        double true_ellipse_threshold,
                                        IMAGE *aImage, int width,
                                        Window theWindow, GC theGC);
int                 count_number_of_pixels_near_ellipse(
                                        double x_centre, double y_centre,
                                        double r1, double r2, double theta,
                                        int width, IMAGE *aImage,
                                        Window theWindow, GC theGC);
BLACK_PIXEL_LIST    *remove_ellipse_from_image(
                                    double x_centre, double y_centre,
                                    double r1, double r2, double theta,
                                    int width,
                                    MAP_FROM_POSITION_TO_BLACK_PIXEL *map,
                                    BLACK_PIXEL_LIST *black_pixel_list,
                                    IMAGE *aImage,
                                    int color_index,
                                    Window theWindow, GC theGC );
void                add_ellipse_to_list_of_objects_found(
                                    double x_centre, double y_centre,
                                    double r1, double r2, double theta,
                                    int width, int color_index);
double              find_angle_of_tangent(IMAGE *aImage, int x, int y);






void
find_lines_with_RHT(IMAGE *aImage, XImage *aXImage, Window theWindow, GC theGC)
{
    /* Parameters */
    int                 k_max = 300;
    int                 ku_max = 5;
    int                 width = 15;
    int                 true_line_threshold = 100;
    int                 min_num_black_pixels = 10;

    /* end of parameters */

    BLACK_PIXEL_LIST    *black_pixel_list;
    MAP_FROM_POSITION_TO_BLACK_PIXEL *map;
    POINT               *point;
    double              tolerance[3], quantisation[2];
    int                 k, ku;
    NODE                *tree, *max_node;
    int                 num_lines;
    char                results_message[100];

    int                 x_start, y_start, x_end, y_end;
    int                 color_index;
    IMAGE               *copyImage;

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


    IMAGE               *alloc_IMAGE(int x, int y);
    void                untag_image(IMAGE *aImage);
    void                copy_image(IMAGE *toImage, IMAGE *fromImage);
    void                free_IMAGE(IMAGE *aImage);
    BLACK_PIXEL_LIST    *form_linked_list_of_black_pixels(IMAGE *aImage,
                                        MAP_FROM_POSITION_TO_BLACK_PIXEL **map);
    void                free_linked_list_of_black_pixel(
                                        BLACK_PIXEL_LIST *black_pixel_list);
    void                free_map_from_position_to_black_pixel(
                                        MAP_FROM_POSITION_TO_BLACK_PIXEL *map);
    POINT               *find_line_between_two_random_pixels(
                                        BLACK_PIXEL_LIST *black_pixel_list);
    NODE                *InsertNode( POINT *p, NODE *tree, double tolerance[],
                                    double quantisation[] );
    void                free_node(NODE *node);
    NODE                *find_max_node(NODE *node, NODE *max_node);
    BOOLEAN             true_line(NODE *node, int true_line_threshold,
                                        IMAGE *aImage, int width);
    BLACK_PIXEL_LIST    *find_and_remove_line_from_image(double theta,
                                        double rho, int width,
                                        MAP_FROM_POSITION_TO_BLACK_PIXEL *map,
                                        BLACK_PIXEL_LIST *black_pixel_list,
                                        IMAGE *aImage,
                                        int color_index,
                                        Window theWindow, GC theGC,
                                        int *x_start, int *y_start,
                                        int *x_end, int *y_end);
    void                add_line_to_list_of_objects_found(int x_start,
                                        int y_start,
                                        int x_end, int y_end,
                                        int width, int color_index);
    void                draw_image(XImage *aXImage, Window theWindow, GC theGC);
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
                            "Started Randomized Hough Transform",
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


    /* Initialisation
     */
    tree = (NODE *)NULL;
    color_index = 0;
    num_lines = 0;
    untag_image(aImage);
    /* When deciding whether two nodes in the tree are
     * the same, we check if they are the same to within
     * some tolerance.
     */
    tolerance[1] = 0.1;                             /* theta */
    tolerance[2] = 5.0;                             /* rho */
    /* The tree is ordered on the first co-ordinate
     * to within this quantisation.
     * Elements within this quantisation are ordered
     * based on their second co-ordinate.
     */
    quantisation[1] = 0.01;                         /* theta */


    /* Make a copy of the image.
     * Since we find multiple lines by finding the longest,
     * removing its pixels from the image, then finding the
     * longest line of those remaining etc.
     * we need a copy of the image to work with
     * (from which we can remove pixels).
     */

    copyImage = alloc_IMAGE(aImage->x, aImage->y);
    copy_image(copyImage, aImage);


    /* Form the black pixels into
     * a linked list of points
     */
    black_pixel_list = form_linked_list_of_black_pixels(copyImage, &map);




    ku = 0;
    do
        {
        for (k=0; k < k_max; k++)
            {
            point = find_line_between_two_random_pixels(black_pixel_list);
            tree = InsertNode( point, tree, tolerance, quantisation );
            }

        /* Find node with max count */
        max_node = find_max_node(tree, (NODE *)NULL);

        /* Is it a True curve, or a couple of random colinear pixels? */
        if (true_line(max_node, true_line_threshold, copyImage, width) == True)
            {
            black_pixel_list = find_and_remove_line_from_image(
                                    max_node->data->x[1], max_node->data->x[2],
                                    width, map,
                                    black_pixel_list,
                                    copyImage,
                                    color_index, theWindow, theGC,
                                    &x_start, &y_start, &x_end, &y_end);
            add_line_to_list_of_objects_found(x_start, y_start, x_end, y_end,
                                    width, color_index);
            num_lines++;
            color_index++;
            if (color_index == NUM_RANDOM_COLORS)
                color_index = 0;
            free_node(tree);
            tree = NULL;
            ku = 0;
            }
        else
            {
            ku++;
            }

        if ((black_pixel_list == (BLACK_PIXEL_LIST *)NULL) ||
                        (black_pixel_list->num_elements < min_num_black_pixels))
            {
            break;          /* out of while loop */
            }
        }
    while (ku < ku_max);

                                        

    free_IMAGE(copyImage);
    free_linked_list_of_black_pixel(black_pixel_list);
    free_map_from_position_to_black_pixel(map);

    XUnmapWindow(gDisplay, message_window->win);
    free_message_window(message_window);

    draw_image(aXImage, theWindow, theGC);
    if (num_lines == 1)
        sprintf(results_message, "Found 1 line with RHT.");
    else
        sprintf(results_message, "Found %d lines with RHT.", num_lines);
    display_message_and_wait(theWindow, 0, 0, 4,
                            "",
                            results_message,
                            "Hit Left Mouse Button to continue",
                            "");


}   /* end of find_lines_with_RHT() */



void
find_circles_with_RHT(IMAGE *aImage, XImage *aXImage,
                                    Window theWindow, GC theGC)
{
    /* Parameters */
    int                 k_max = 600;
    int                 ku_max = 5;
    int                 width = 10;
    int                 min_num_black_pixels = 10;
/* Proportion of circle that must exist for it to be judged a true circle. */
    double              true_circle_threshold = 0.60; 

    /* end of parameters */

    BLACK_PIXEL_LIST    *black_pixel_list;
    MAP_FROM_POSITION_TO_BLACK_PIXEL *map;
    POINT               *point;
    double              tolerance[4], quantisation[3];
    int                 k, ku;
    NODE                *tree, *max_node;
    int                 num_circles;
    char                results_message[100];

    int                 x_centre, y_centre, radius;
    int                 color_index;
    IMAGE               *copyImage;

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


    IMAGE               *alloc_IMAGE(int x, int y);
    void                copy_image(IMAGE *toImage, IMAGE *fromImage);
    void                free_IMAGE(IMAGE *aImage);
    BLACK_PIXEL_LIST    *form_linked_list_of_black_pixels(IMAGE *aImage,
                                        MAP_FROM_POSITION_TO_BLACK_PIXEL **map);
    void                free_linked_list_of_black_pixel(
                                        BLACK_PIXEL_LIST *black_pixel_list);
    void                free_map_from_position_to_black_pixel(
                                        MAP_FROM_POSITION_TO_BLACK_PIXEL *map);
    POINT               *find_circle_given_three_random_pixels(
                                        BLACK_PIXEL_LIST *black_pixel_list);
    NODE                *InsertNode( POINT *p, NODE *tree, double tolerance[],
                                    double quantisation[] );
    void                free_node(NODE *node);
    NODE                *find_max_node(NODE *node, NODE *max_node);
    BOOLEAN             true_circle(int x_centre, int y_centre, int radius,
                                        double true_circle_threshold,
                                        IMAGE *aImage, int width,
                                        Window theWindow, GC theGC);
    BLACK_PIXEL_LIST    *remove_circle_from_image(
                                        int x_centre, int y_centre,
                                        int radius,
                                        int width,
                                        MAP_FROM_POSITION_TO_BLACK_PIXEL *map,
                                        BLACK_PIXEL_LIST *black_pixel_list,
                                        IMAGE *aImage,
                                        int color_index,
                                        Window theWindow, GC theGC);
    void                add_circle_to_list_of_objects_found(
                                        double x_centre, double y_centre,
                                        double radius,
                                        int width, int color_index);
    void                draw_image(XImage *aXImage, Window theWindow, GC theGC);
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
                            "Started Randomized Hough Transform",
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


    /* Initialisation
     */
    tree = (NODE *)NULL;
    color_index = 0;
    num_circles = 0;
    /* When deciding whether two nodes in the tree are
     * the same, we check if they are the same to within
     * some tolerance.
     */
    tolerance[1] = 10.0;                                /* x_centre*/
    tolerance[2] = 10.0;                                /* y_centre */
    tolerance[3] = 10.0;                                /* radius */
    /* The tree is ordered on the first co-ordinate
     * to within quantisation[1].
     * Elements within this quantisation are ordered
     * based on their second co-ordinate, to within quantisation[2]
     * Elements within this quantisation are ordered
     * based on their third co-ordinate.
     */
    quantisation[1] = 30.00;                            /* x_centre */
    quantisation[2] = 30.00;                            /* y_centre */


    /* Make a copy of the image.
     * Since we find multiple circles by finding the largest,
     * removing its pixels from the image, then finding the
     * largest circle of those remaining etc.
     * we need a copy of the image to work with
     * (from which we can remove pixels).
     */

    copyImage = alloc_IMAGE(aImage->x, aImage->y);
    copy_image(copyImage, aImage);


    /* Form the black pixels into
     * a linked list of points
     */
    black_pixel_list = form_linked_list_of_black_pixels(copyImage, &map);




    ku = 0;
    do
        {
        for (k=0; k < k_max; k++)
            {
            point = find_circle_given_three_random_pixels(black_pixel_list);
            tree = InsertNode( point, tree, tolerance, quantisation );
            }

        /* Find node with max count */
        max_node = find_max_node(tree, (NODE *)NULL);


        if (max_node != (NODE *)NULL)
            {
            x_centre = (int )max_node->data->x[1];
            y_centre = (int )max_node->data->x[2];
            radius = (int )max_node->data->x[3];

            /* Is it a True curve, or a couple of random pixels? */
            if (true_circle(x_centre, y_centre, radius,
                                    true_circle_threshold, copyImage, width,
                                    theWindow, theGC) == True)
                {
                /* Remove circle from image */
                black_pixel_list = remove_circle_from_image(
                                    x_centre, y_centre, radius,
                                    width, map,
                                    black_pixel_list,
                                    copyImage, color_index, theWindow, theGC);
                /* Record the circle to be displayed.
                 */
                add_circle_to_list_of_objects_found(x_centre, y_centre, radius,
                                    width, color_index);
                num_circles++;
                color_index++;
                if (color_index == NUM_RANDOM_COLORS)
                    color_index = 0;
                free_node(tree);
                tree = NULL;
                ku = 0;
                }
            else
                {
                ku++;
                }
            }

        if ((black_pixel_list == (BLACK_PIXEL_LIST *)NULL) ||
                        (black_pixel_list->num_elements < min_num_black_pixels))
            {
            break;          /* out of while loop */
            }
        }
    while ((ku < ku_max) && (max_node != (NODE *)NULL));

                                        

    free_IMAGE(copyImage);
    free_linked_list_of_black_pixel(black_pixel_list);
    free_map_from_position_to_black_pixel(map);

    XUnmapWindow(gDisplay, message_window->win);
    free_message_window(message_window);


    draw_image(aXImage, theWindow, theGC);
    if (num_circles == 1)
        sprintf(results_message, "Found 1 circle with RHT.");
    else
        sprintf(results_message, "Found %d circles with RHT.", num_circles);
    display_message_and_wait(theWindow, 0, 0, 4,
                            "",
                            results_message,
                            "Hit Left Mouse Button to continue",
                            "");


}   /* end of find_circles_with_RHT() */



void
find_ellipses_with_RHT(IMAGE *aImage, XImage *aXImage,
                                    Window theWindow, GC theGC)
{
    /* Parameters */
    int                 k_max = 2000;
    int                 ku_max = 3;
    int                 width = 20;
    int                 min_num_black_pixels = 20;
/* Proportion of ellipse that must exist for it to be judged a true ellipse. */
    double              true_ellipse_threshold = 0.65; 

    /* end of parameters */

    BLACK_PIXEL_LIST    *black_pixel_list;
    MAP_FROM_POSITION_TO_BLACK_PIXEL *map;
    POINT               *point;
    double              tolerance[6], quantisation[5];
    int                 k, ku, count;
    NODE                *tree, *max_node;
    BOOLEAN             return_val;
    int                 num_ellipses;
    char                results_message[100];

    double              x_centre, y_centre, r1, r2, theta;
    int                 color_index;
    IMAGE               *copyImage;

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


    IMAGE               *alloc_IMAGE(int x, int y);
    void                copy_image(IMAGE *toImage, IMAGE *fromImage);
    void                free_IMAGE(IMAGE *aImage);
    BLACK_PIXEL_LIST    *form_linked_list_of_black_pixels(IMAGE *aImage,
                                        MAP_FROM_POSITION_TO_BLACK_PIXEL **map);
    void                free_linked_list_of_black_pixel(
                                        BLACK_PIXEL_LIST *black_pixel_list);
    void                free_map_from_position_to_black_pixel(
                                        MAP_FROM_POSITION_TO_BLACK_PIXEL *map);
    POINT               *find_ellipse_given_three_random_pixels(
                                        BLACK_PIXEL_LIST *black_pixel_list,
                                        IMAGE *aImage);
    NODE                *InsertNode( POINT *p, NODE *tree, double tolerance[],
                                    double quantisation[] );
    void                free_node(NODE *node);
    int                 output_node(NODE *node, FILE *dat_file,
                                        FILE *lines_file, int line_number);
    NODE                *find_max_node(NODE *node, NODE *max_node);
    BOOLEAN             true_ellipse(double x_centre, double y_centre,
                                        double r1, double r2, double theta,
                                        double true_ellipse_threshold,
                                        IMAGE *aImage, int width,
                                        Window theWindow, GC theGC);
    void                draw_hough_ellipse( Window theWindow, GC theGC,
                                        double x, double y,
                                        double r1, double r2, double theta);
    BLACK_PIXEL_LIST    *remove_ellipse_from_image(
                                    double x_centre, double y_centre,
                                    double r1, double r2, double theta,
                                    int width,
                                    MAP_FROM_POSITION_TO_BLACK_PIXEL *map,
                                    BLACK_PIXEL_LIST *black_pixel_list,
                                    IMAGE *aImage,
                                    int color_index,
                                    Window theWindow, GC theGC );
    void                add_ellipse_to_list_of_objects_found(
                                    double x_centre, double y_centre,
                                    double r1, double r2, double theta,
                                    int width, int color_index);
    void                draw_image(XImage *aXImage, Window theWindow, GC theGC);
    void                display_message_and_wait(Window theWindow,
                                        int x, int y, int num_strings, ...);
    int     sum_of_nodes, iteration_count, average_num_nodes;
    int     tree_size(NODE *node, int size);
#ifdef XGOBI
    BOOLEAN first_time_flag = True;
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
                            "Started Randomized Hough Transform",
                            "to find ellipses.",
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



    /* Initialisation
     */
    tree = (NODE *)NULL;
    num_ellipses = 0;
    color_index = 0;
    sum_of_nodes = 0;
    iteration_count = 0;

    /* When deciding whether two nodes in the tree are
     * the same, we check if they are the same to within
     * some tolerance.
     */
    tolerance[1] = 30.0;                                /* x_centre*/
    tolerance[2] = 30.0;                                /* y_centre */
    tolerance[3] = 50.0;                                /* r1 */
    tolerance[4] = 40.0;                                /* r1 */
    tolerance[5] = M_PI/7.0;                            /* theta */

    /* The tree is ordered on the first co-ordinate
     * to within quantisation[1].
     * Elements within this quantisation are ordered
     * based on their second co-ordinate, to within quantisation[2]
     * etc.
     * Finally, elements with the same 4 co-ordinates
     * (to within quantisation) are are ordered
     * based on their fifth co-ordinate.
     */
    quantisation[1] = 100.0;                            /* x_centre */
    quantisation[2] = 100.0;                            /* y_centre */
    quantisation[3] = 100.0;                            /* a */
    quantisation[4] = 100.0;                            /* b */


    /* Make a copy of the image.
     * Since we find multiple ellipses by finding the largest,
     * removing its pixels from the image, then finding the
     * largest circle of those remaining etc.
     * we need a copy of the image to work with
     * (from which we can remove pixels).
     */

    copyImage = alloc_IMAGE(aImage->x, aImage->y);
    copy_image(copyImage, aImage);


    /* Form the black pixels into
     * a linked list of points
     */
    black_pixel_list = form_linked_list_of_black_pixels(copyImage, &map);



    ku = 0;
    max_node = (NODE *)NULL;
    do
        {
        k = count = 0;
        while ((k < k_max) && (count < (k_max*20)))
            {
            point = find_ellipse_given_three_random_pixels(black_pixel_list,
                                                            copyImage);
            tree = InsertNode( point, tree, tolerance, quantisation );

            count++;
            if (point != (POINT *)NULL)
                k++;
            }


        if (count == (k_max*20))
            {
            ku++;
            continue;
            }


        /* Find node with max count */
        max_node = find_max_node(tree, (NODE *)NULL);


        if (max_node != (NODE *)NULL)
            {

            x_centre = max_node->data->x[1];
            y_centre = max_node->data->x[2];
            r1 = max_node->data->x[3];
            r2 = max_node->data->x[4];
            theta = max_node->data->x[5];

#ifdef XGOBI
{
if (first_time_flag == True)
    {
    FILE    *dat_file, *lines_file;
    BLACK_PIXEL *ptr;

    dat_file = fopen("tmp.dat", "w");
    lines_file = fopen("tmp.lines", "w");
    if ((dat_file == NULL) || (lines_file == NULL))
        {
        fprintf(stderr, "Failed to open file\n");
        exit(1);
        }
    (void )output_node(tree, dat_file, lines_file, 0);

    for (ptr=black_pixel_list->head; ptr != NULL; ptr=ptr->next)
        {
        fprintf(dat_file, "%d %d 0 0 0 0\n", ptr->x, ptr->y);
        }
    fprintf(dat_file, "0 0 0 0 0 0\n");
    fprintf(dat_file, "%d 0 0 0 0 0\n", aImage->x);
    fprintf(dat_file, "0 %d 0 0 0 0\n", aImage->y);
    fprintf(dat_file, "%d %d 0 0 0 0\n", aImage->x, aImage->y);

    fclose(dat_file);
    fclose(lines_file);
    first_time_flag = False;
    }
}
#endif
            return_val = true_ellipse(x_centre, y_centre, r1, r2, theta,
                                    true_ellipse_threshold,
                                    copyImage, width,
                                    theWindow, theGC);

            if (return_val == True)             /* If it is a true ellipse */
                {
                black_pixel_list = remove_ellipse_from_image(
                                    x_centre, y_centre,
                                    r1, r2, theta,
                                    width,
                                    map,
                                    black_pixel_list,
                                    copyImage,
                                    color_index,
                                    theWindow, theGC );
               /* Record the ellipse to be displayed.
                */
                add_ellipse_to_list_of_objects_found(x_centre, y_centre,
                                    r1, r2, theta,
                                    width, color_index);
                num_ellipses++;
                color_index++;
                if (color_index == NUM_RANDOM_COLORS)
                    color_index = 0;

                sum_of_nodes += tree_size(tree, 0);
                iteration_count++;

                free_node(tree);
                tree = NULL;
                ku = 0;
                }
            else
                {

                ku++;

                if (ku == ku_max)
                    {
                    sum_of_nodes += tree_size(tree, 0);
                    iteration_count++;
                    }
                }
            }               /* end of 'if (max_node != (NODE *)NULL)' */

        if ((black_pixel_list == (BLACK_PIXEL_LIST *)NULL) ||
                        (black_pixel_list->num_elements < min_num_black_pixels))
            {
            break;          /* out of while loop */
            }
        }
    while ((ku < ku_max) && (max_node != (NODE *)NULL));



    if (iteration_count != 0)
        {
        average_num_nodes = (int )(((double )sum_of_nodes)/
                                                    (double)iteration_count);
        }
                                        

    free_IMAGE(copyImage);
    free_linked_list_of_black_pixel(black_pixel_list);
    free_map_from_position_to_black_pixel(map);

    XUnmapWindow(gDisplay, message_window->win);
    free_message_window(message_window);


    draw_image(aXImage, theWindow, theGC);
    if (num_ellipses == 1)
        sprintf(results_message, "Found 1 ellipse with RHT.");
    else
        sprintf(results_message, "Found %d ellipses with RHT.", num_ellipses);
    display_message_and_wait(theWindow, 0, 0, 4,
                            "",
                            results_message,
                            "Hit Left Mouse Button to continue",
                            "");


}   /* end of find_ellipses_with_RHT() */



BLACK_PIXEL_LIST *
form_linked_list_of_black_pixels(IMAGE *aImage,
                            MAP_FROM_POSITION_TO_BLACK_PIXEL **map)
{
    int                 x, y;
    double              theta;
    BLACK_PIXEL         *p;
    BLACK_PIXEL_LIST    *black_pixel_list;
    double              find_tangent_of_edge(IMAGE *aImage, int x_p, int y_p);
    MAP_FROM_POSITION_TO_BLACK_PIXEL    *alloc_map_from_position_to_black_pixel(int x, int y);
    BLACK_PIXEL         *alloc_black_pixel(void);
    BLACK_PIXEL_LIST    *put_in_linked_list_of_black_pixel(BLACK_PIXEL_LIST *black_pixel_list, BLACK_PIXEL *p);
    int     get_pixel_value(IMAGE *aImage, int x, int y);



    *map = alloc_map_from_position_to_black_pixel(aImage->x, aImage->y);
    black_pixel_list = NULL;


    for (x=0; x < aImage->x; x++)
        for (y=0; y < aImage->y; y++)
            if (get_pixel_value(aImage, x, y) == BLACK)
                {
                theta = find_tangent_of_edge(aImage, x, y);
                if (theta != NO_ANGLE)
                    {
                    p = alloc_black_pixel();
                    p->x = x;
                    p->y = y;
                    p->angle = theta;
                    p->slope = tan(theta);
                    black_pixel_list = put_in_linked_list_of_black_pixel(
                                                    black_pixel_list, p);
                    (*map)->p[x][y] = p;
                    }
                }


    return(black_pixel_list);

}   /* end of form_linked_list_of_black_pixels() */



POINT *
find_line_between_two_random_pixels(BLACK_PIXEL_LIST *black_pixel_list)
{
    BLACK_PIXEL         *p1, *p2, *ptr;
    int                 index1, index2, i;
    double              rho, theta;
    double              t;                      /* temporary variable */
    POINT               *point;
    POINT               *alloc_point(int dim);


    if (black_pixel_list == (BLACK_PIXEL_LIST *)NULL)
        return( (POINT *)NULL );

    if (black_pixel_list->num_elements <= 2)
        return( (POINT *)NULL );



    /* Randomly choose 2 numbers.
     */
    index1 = (int )(drand48() * (double )black_pixel_list->num_elements);
    do
        index2 = (int )(drand48() * (double )black_pixel_list->num_elements);
    while (index2 == index1);

    /* p1 and p2 point to 2 randomly chosen entries
     * in the linked list of black pixels.
     */
    ptr = black_pixel_list->head;
    p1 = p2 = (BLACK_PIXEL *)NULL;
    for (i=0; ((i <= index1) || (i <= index2)); i++, ptr=ptr->next)
        {
        if (i == index1)
            p1 = ptr;
        if (i == index2)
            p2 = ptr;
        }

    /* Compute rho and theta of the line between these two points.
     */
    if (ABS(p2->y - p1->y) > ABS(p2->x - p1->x))
        {
        t = -((double )(p2->x - p1->x)) / (double )(p2->y - p1->y);
        theta = atan2( t, 1.0);
        }
    else
        {
        t = -((double )(p2->y - p1->y))/ (double )(p2->x - p1->x);
        theta = atan2( 1.0, t);
        }

    /* Normalise theta to be in the range [0, M_PI)
     * n.b. atan2 returns a value in the range [-M_PI, M_PI]
     */
    if (theta < 0)
        theta = M_PI + theta;

    rho = cos(theta)*(double )(p1->x) + sin(theta)*(double )(p1->y);

    point = alloc_point(2);
    point->x[1] = theta;
    point->x[2] = rho;

    return(point);


}   /* end of find_line_between_two_random_pixels() */



BLACK_PIXEL_LIST *
find_and_remove_line_from_image(double theta, double rho, int width,
                        MAP_FROM_POSITION_TO_BLACK_PIXEL *map,
                        BLACK_PIXEL_LIST *black_pixel_list,
                        IMAGE *aImage,
                        int color_index, Window theWindow, GC theGC,
                        int *x_start, int *y_start, int *x_end, int *y_end)
{
    BOOLEAN             found_a_line_flag;

    /* find_line_ends() is in sht_line.c */
    BOOLEAN             find_line_ends(double theta, double rho,
                                    IMAGE *aImage,
                                    int *x_start, int *y_start,
                                    int *x_end, int *y_end);
    BLACK_PIXEL_LIST    *remove_line_from_image_in_rht(
                                    int x_start, int y_start,
                                    int x_end, int y_end, int width,
                                    MAP_FROM_POSITION_TO_BLACK_PIXEL *map,
                                    BLACK_PIXEL_LIST *black_pixel_list,
                                    IMAGE *aImage,
                                    int color_index,
                                    Window theWindow, GC theGC);



    found_a_line_flag = find_line_ends(theta, rho, aImage,
                                    x_start, y_start, x_end, y_end);


    if (found_a_line_flag == True)
        {
        black_pixel_list = remove_line_from_image_in_rht(*x_start, *y_start,
                                    *x_end, *y_end,
                                    width, map, black_pixel_list,
                                    aImage, color_index, theWindow, theGC);
        }


    XFlush( gDisplay );

    return( black_pixel_list );


}   /* end of find_and_remove_line_from_image() */



BLACK_PIXEL_LIST *
remove_line_from_image_in_rht(int x_start, int y_start, int x_end, int y_end,
                                    int width,
                                    MAP_FROM_POSITION_TO_BLACK_PIXEL *map,
                                    BLACK_PIXEL_LIST *black_pixel_list,
                                    IMAGE *aImage,
                                    int color_index,
                                    Window theWindow, GC theGC)
{
    int                 x, y;
    int                 i;
    double              grad;

    BLACK_PIXEL_LIST    *remove_element_from_linked_list_of_black_pixels(
                                    BLACK_PIXEL_LIST *black_pixel_list,
                                    BLACK_PIXEL *p);
    /* set_IMAGE_pixel_value() is in image.c */
    void                set_IMAGE_pixel_value(IMAGE *aImage,
                                    int x, int y, int val);



    /* Set foreground to green. */
    XSetForeground(gDisplay, theGC, gRandomColors[color_index].pixel);


    /* Define a line, width defined in 'width' pixels,
     * between(x_start,y_start) and (x_end,y_end).
     * Set all pixels along this line to WHITE.
     */
    if (ABS(x_end-x_start) >= ABS(y_end-y_start))
        {
        /* x changes more than y
         */
        if (x_start < x_end)
            {
            grad = ((double )(y_end-y_start)) / (double )(x_end-x_start);
            for (x=x_start; x < x_end; x++)
                {
                y = y_start + (int )(grad*(double )(x - x_start));
                for (i=-width/2; i < (width+1)/2; i++)
                    {
                    if ((y+i < 0) || (y+i >= aImage->y))
                        continue;           /* outside image */

                    set_IMAGE_pixel_value(aImage, x, y+i, WHITE);
                    if (map->p[x][y+i] != (BLACK_PIXEL *)NULL)
                        {
                        black_pixel_list =
                                remove_element_from_linked_list_of_black_pixels(
                                            black_pixel_list, map->p[x][y+i]);
                        map->p[x][y+i] = (BLACK_PIXEL *)NULL;
                        }
                    XDrawPoint(gDisplay, theWindow, theGC, x, y+i);
                    }
                }
            }
        else if (x_start > x_end)
            {
            grad = ((double )(y_start-y_end)) / (double )(x_start-x_end);
            for (x=x_end; x < x_start; x++)
                {
                y = y_end + (int )(grad*(double )(x - x_end));
                for (i=-width/2; i < (width+1)/2; i++)
                    {
                    if ((y+i < 0) || (y+i >= aImage->y))
                        continue;           /* outside image */

                    set_IMAGE_pixel_value(aImage, x, y+i, WHITE);
                    if (map->p[x][y+i] != (BLACK_PIXEL *)NULL)
                        {
                        black_pixel_list =
                                remove_element_from_linked_list_of_black_pixels(
                                            black_pixel_list, map->p[x][y+i]);
                        map->p[x][y+i] = (BLACK_PIXEL *)NULL;
                        }
                    XDrawPoint(gDisplay, theWindow, theGC, x, y+i);
                    }
                }
            }
        }
    else        /* if (ABS(x_end-x_start) < ABS(y_end-y_start)) */
        {
        /* y changes more than x
         */
        if (y_start < y_end)
            {
            grad = ((double )(x_end-x_start)) / (double )(y_end-y_start);
            for (y=y_start; y < y_end; y++)
                {
                x = x_start + (int )(grad*(double )(y-y_start));
                for (i=-width/2; i < (width+1)/2; i++)
                    {
                    if ((x+i < 0) || (x+i >= aImage->x))
                        continue;           /* outside image */

                    set_IMAGE_pixel_value(aImage, x+i, y, WHITE);
                    if (map->p[x+i][y] != (BLACK_PIXEL *)NULL)
                        {
                        black_pixel_list =
                                remove_element_from_linked_list_of_black_pixels(
                                            black_pixel_list, map->p[x+i][y]);
                        map->p[x+i][y] = (BLACK_PIXEL *)NULL;
                        }
                    XDrawPoint(gDisplay, theWindow, theGC, x+i, y);
                    }
                }
            }
        else if (y_start > y_end)
            {
            grad = ((double )(x_start-x_end)) / (double )(y_start-y_end);
            for (y=y_end; y < y_start; y++)
                {
                x = x_end + (int )(grad*(double )(y - y_end));
                for (i=-width/2; i < (width+1)/2; i++)
                    {
                    if ((x+i < 0) || (x+i >= aImage->x))
                        continue;           /* outside image */

                    set_IMAGE_pixel_value(aImage, x+i, y, WHITE);
                    if (map->p[x+i][y] != (BLACK_PIXEL *)NULL)
                        {
                        black_pixel_list =
                                remove_element_from_linked_list_of_black_pixels(
                                            black_pixel_list, map->p[x+i][y]);
                        map->p[x+i][y] = (BLACK_PIXEL *)NULL;
                        }
                    XDrawPoint(gDisplay, theWindow, theGC, x+i, y);
                    }
                }
            }
        }


    return( black_pixel_list );


}   /* end of remove_line_from_image_in_rht() */




void
add_line_to_list_of_objects_found(int x_start, int y_start,
                                    int x_end, int y_end,
                                    int width, int color_index)
{
    DISPLAY_OBJECT      *d;
    DISPLAY_OBJECT      *alloc_display_object(void);
    DISPLAY_OBJECT_LIST *put_in_linked_list_of_display_object(
                                    DISPLAY_OBJECT_LIST *display_oject_list,
                                    DISPLAY_OBJECT *d);


   /* Record the line to be displayed.
     */
    d = alloc_display_object();
    d->type = hough_line;
    d->obj.hough_line.x_start = x_start;
    d->obj.hough_line.y_start = y_start;
    d->obj.hough_line.x_end = x_end;
    d->obj.hough_line.y_end = y_end;
    d->obj.hough_line.width = width;
    d->obj.hough_line.color_index = color_index;
    gDisplayObjectList = put_in_linked_list_of_display_object(gDisplayObjectList, d);


}   /* end of add_line_to_list_of_objects_found() */




BOOLEAN
true_line(NODE *node, int true_line_threshold, IMAGE *aImage, int width)
{
    int     x_start, y_start, x_end, y_end;
    int     num_pixels;
    BOOLEAN found_a_line_flag;


    /* find_line_ends() is in sht_line.c */
    BOOLEAN             find_line_ends(double theta, double rho,
                                    IMAGE *aImage,
                                    int *x_start, int *y_start,
                                    int *x_end, int *y_end);
    int                 count_number_black_pixels_on_line(
                                    int x_start, int y_start,
                                    int x_end, int y_end,
                                    int width,
                                    IMAGE *aImage);


    if (node == (NODE *)NULL)
        {
        return(False);
        }


    found_a_line_flag = find_line_ends(node->data->x[1], node->data->x[2],
                                    aImage,
                                    &x_start, &y_start, &x_end, &y_end);

    if (found_a_line_flag == False)
        return( False );


    num_pixels = count_number_black_pixels_on_line(x_start, y_start,
                                    x_end, y_end, width, aImage );

    if (num_pixels > true_line_threshold)
        return( True );
    else
        return( False );



}   /* end of true_line() */



int
count_number_black_pixels_on_line(int x_start, int y_start,
                                    int x_end, int y_end,
                                    int width,
                                    IMAGE *aImage)
{
    int                 x, y;
    int                 i;
    double              grad;
    int                 num_pixels = 0;

    /* get_pixel_value() is in image.c */
    int                 get_pixel_value(IMAGE *aImage, int x, int y);



    /* Define a line, width defined in 'width' pixels,
     * between(x_start,y_start) and (x_end,y_end).
     * COunt all black pixels along this line.
     */
    if (ABS(x_end-x_start) >= ABS(y_end-y_start))
        {
        /* x changes more than y
         */
        if (x_start < x_end)
            {
            grad = ((double )(y_end-y_start)) / (double )(x_end-x_start);
            for (x=x_start; x < x_end; x++)
                {
                y = y_start + (int )(grad*(double )(x - x_start));
                for (i=-width/2; i < (width+1)/2; i++)
                    {
                    if ((y+i < 0) || (y+i >= aImage->y))
                        continue;           /* outside image */

                    if (get_pixel_value(aImage, x, y+i) == BLACK)
                        num_pixels++;
                    }
                }
            }
        else if (x_start > x_end)
            {
            grad = ((double )(y_start-y_end)) / (double )(x_start-x_end);
            for (x=x_end; x < x_start; x++)
                {
                y = y_end + (int )(grad*(double )(x - x_end));
                for (i=-width/2; i < (width+1)/2; i++)
                    {
                    if ((y+i < 0) || (y+i >= aImage->y))
                        continue;           /* outside image */

                    if (get_pixel_value(aImage, x, y+i) == BLACK)
                        num_pixels++;
                    }
                }
            }
        }
    else        /* if (ABS(x_end-x_start) < ABS(y_end-y_start)) */
        {
        /* y changes more than x
         */
        if (y_start < y_end)
            {
            grad = ((double )(x_end-x_start)) / (double )(y_end-y_start);
            for (y=y_start; y < y_end; y++)
                {
                x = x_start + (int )(grad*(double )(y-y_start));
                for (i=-width/2; i < (width+1)/2; i++)
                    {
                    if ((x+i < 0) || (x+i >= aImage->x))
                        continue;           /* outside image */

                    if (get_pixel_value(aImage, x+i, y) == BLACK)
                        num_pixels++;
                    }
                }
            }
        else if (y_start > y_end)
            {
            grad = ((double )(x_start-x_end)) / (double )(y_start-y_end);
            for (y=y_end; y < y_start; y++)
                {
                x = x_end + (int )(grad*(double )(y - y_end));
                for (i=-width/2; i < (width+1)/2; i++)
                    {
                    if ((x+i < 0) || (x+i >= aImage->x))
                        continue;           /* outside image */

                    if (get_pixel_value(aImage, x+i, y) == BLACK)
                        num_pixels++;
                    }
                }
            }
        }


    return( num_pixels );


}   /* end of count_number_black_pixels_on_line() */



POINT *
find_circle_given_three_random_pixels(BLACK_PIXEL_LIST *black_pixel_list)
{
    BLACK_PIXEL         *p1, *p2, *p3, *ptr;
    int                 index1, index2, index3, i;
    double              x_centre, y_centre, radius;
    double              x1, y1;
    double              x2, y2;
    double              x3, y3;
    POINT               *point;
    POINT               *alloc_point(int dim);
    BOOLEAN             compute_circle(double x1, double y1,
                                        double x2, double y2,
                                        double x3, double y3,
                                        double *x_centre, double *y_centre,
                                        double *radius);


    if (black_pixel_list == (BLACK_PIXEL_LIST *)NULL)
        return( (POINT *)NULL );

    if (black_pixel_list->num_elements <= 3)
        return( (POINT *)NULL );



    /* Randomly choose 3 numbers.
     */
    index1 = (int )(drand48() * (double )black_pixel_list->num_elements);
    do
        index2 = (int )(drand48() * (double )black_pixel_list->num_elements);
    while (index2 == index1);

    do
        index3 = (int )(drand48() * (double )black_pixel_list->num_elements);
    while ((index3 == index1) && (index3 == index2));

    /* p1, p2 and p3 point to 23randomly chosen entries
     * in the linked list of black pixels.
     */
    ptr = black_pixel_list->head;
    p1 = p2 = p3 = (BLACK_PIXEL *)NULL;
    for (i=0; ((i<=index1) || (i<=index2) || (i<=index3)); i++, ptr=ptr->next)
        {
        if (i == index1)
            p1 = ptr;
        if (i == index2)
            p2 = ptr;
        if (i == index3)
            p3 = ptr;
        }

    /* Compute centre and radius of circle.
     */
    x1 = (double )p1->x;
    y1 = (double )p1->y;
    x2 = (double )p2->x;
    y2 = (double )p2->y;
    x3 = (double )p3->x;
    y3 = (double )p3->y;
    if (compute_circle(x1, y1, x2, y2, x3, y3,
                                &x_centre, &y_centre, &radius) == True)
        {
        point = alloc_point(3);
        point->x[1] = x_centre;
        point->x[2] = y_centre;
        point->x[3] = radius;
        }
    else
        {
        /* Failed to find a circle for the given three points. */
        point = (POINT *)NULL;
        }

    return(point);


}   /* end of find_circle_given_three_random_pixels() */



/* We are given 3 points (x1,y1), (x2,y2), (x3,y3).
 * Consider the line segment passing from (x1,y1) to (x2,y2).
 * Compute the line perpendicular to this, which crosses at the midpoint.
 * Repeat this for the pairs (x2,y2), (x3,y3) and
 * (x1,y1), (x3,y3).
 * The intersection of any two of these lines is the circle centre.
 * Then use the distance from the centre to one of the points
 * to compute the radius.
 */
BOOLEAN
compute_circle(double x1, double y1, double x2, double y2,
                                    double x3, double y3,
                                    double *x_centre, double *y_centre,
                                    double *radius)
{
    double  m1, b1, m2, b2;


    if ((ABS(y2-y1) < A_VERY_SMALL_POSITIVE_VALUE) ||
                        (ABS(y3-y2) < A_VERY_SMALL_POSITIVE_VALUE))
        {
        return( False );
        }

    m1 = (x1-x2) / (y2-y1);
    b1 = ( (y1+y2)/2.0 ) - ( (x1-x2)*(x1+x2)/(2.0*(y2-y1)) );

    m2 = (x2-x3) / (y3-y2);
    b2 = ( (y2+y3)/2.0 ) - ( (x2-x3)*(x2+x3)/(2.0*(y3-y2)) );

    if (ABS(m1-m2) < A_VERY_SMALL_POSITIVE_VALUE)
        {
        /* Trying to find the intersction of almost parallel lines.
         */
        return( False );
        }

    *x_centre = (b2-b1) / (m1-m2);
    *y_centre = m1 * (*x_centre) + b1;

    *radius = sqrt( SQR(x3-(*x_centre)) + SQR(y3-(*y_centre)) );


    return( True );


}   /* end of compute_circle() */



BOOLEAN
true_circle(int x_centre, int y_centre, int radius,
                        double true_circle_threshold, IMAGE *aImage, int width,
                        Window theWindow, GC theGC)
{
    int     x, y;
    int     x1, x2, x3, x4;
    int     y1, y2, y3, y4;
    int     num_pixels = 0;
    double  dist;
    double  proportion;

    /* get_pixel_value() is in image.c */
    int                 get_pixel_value(IMAGE *aImage, int x, int y);


    if (radius < width)
        return( False );


/* Count number of pixels lying near the circle.
 * We check every pixel lying between two squares,
 * one a little bigger than the circle, the other a little
 * smaller than the circle.
 * For each black pixel within these squares, we check
 * if it lies near the circle.
 *              ____________________
 *              |                   |
 *              |        ***        |
 *              |     ***   ***     |
 *              |   ** _______ **   |
 *              |  **  |      | **  |
 *              | **   |      |  ** |
 *              | **   |      |  ** |
 *              |  **  |      | **  |
 *              |   ** ------- **   |
 *              |     ***   ***     |
 *              |        ***        |
 *              |
 *              ---------------------
 */

/* Compute the co-ordinates of the corners of the squares.
 *          y4  ____________________
 *              |                   |
 *              |        ***        |
 *              |     ***   ***     |
 *          y3  |   ** _______ **   |
 *              |  **  |      | **  |
 *              | **   |      |  ** |
 *              | **   |      |  ** |
 *              |  **  |      | **  |
 *          y2  |   ** ------- **   |
 *              |     ***   ***     |
 *              |        ***        |
 *              |
 *          y1  ---------------------
 *             x1      x2    x3     x4
 */

    x1 = x_centre - radius - width/2;
    if (x1 < 0)
        x1 = 0;
    else if (x1 > aImage->x)
        x1 = aImage->x;

    x2 = x_centre - (int )(((double )(radius - width/2))/sqrt(2.0));
    if (x2 < 0)
        x2 = 0;
    else if (x2 > aImage->x)
        x2 = aImage->x;

    x3 = x_centre + (int )(((double )(radius - width/2))/sqrt(2.0));
    if (x3 < 0)
        x3 = 0;
    else if (x3 > aImage->x)
        x3 = aImage->x;

    x4 = x_centre + radius + width/2;
    if (x4 < 0)
        x4 = 0;
    else if (x4 > aImage->x)
        x4 = aImage->x;

    y1 = y_centre - radius - width/2;
    if (y1 < 0)
        y1 = 0;
    else if (y1 > aImage->y)
        y1 = aImage->y;

    y2 = y_centre - (int )(((double )(radius - width/2))/sqrt(2.0));
    if (y2 < 0)
        y2 = 0;
    else if (y2 > aImage->y)
        y2 = aImage->y;

    y3 = y_centre + (int )(((double )(radius - width/2))/sqrt(2.0));
    if (y3 < 0)
        y3 = 0;
    else if (y3 > aImage->y)
        y3 = aImage->y;

    y4 = y_centre + radius + width/2;
    if (y4 < 0)
        y4 = 0;
    else if (y4 > aImage->y)
        y4 = aImage->y;


/* First, check the bottom region
 *              ____________________
 *              |                   |
 *              |        ***        |
 *              |     ***   ***     |
 *              |   ** _______ **   |
 *              |  **  |      | **  |
 *              | **   |      |  ** |
 *              | **   |      |  ** |
 *              |  **  |      | **  |
 *              |   ** ------- **   |
 *              |$$$$$$$$$$$$$$$$$$$|
 *              |$$$$$$$$$$$$$$$$$$$|
 *              |$$$$$$$$$$$$$$$$$$$|
 *              ---------------------
 *
 */

    for (x=x1; x < x4; x++)
        for (y=y1; y < y2; y++)
            {
            dist = sqrt(SQR(x-x_centre) + SQR(y-y_centre));
            if (ABS(dist-radius) < width/2)
                {
                if (get_pixel_value(aImage, x, y) == BLACK)
                    num_pixels++;
                }
            }



/* Next, check the top region
 *              ____________________ 
 *              |$$$$$$$$$$$$$$$$$$$|
 *              |$$$$$$$$$$$$$$$$$$$|
 *              |$$$$$$$$$$$$$$$$$$$|
 *              |   ** _______ **   |
 *              |  **  |      | **  |
 *              | **   |      |  ** |
 *              | **   |      |  ** |
 *              |  **  |      | **  |
 *              |   ** ------- **   |
 *              |     ***   ***     |
 *              |        ***        |
 *              |                   |
 *              ---------------------
 */

    for (x=x1; x < x4; x++)
        for (y=y3; y < y4; y++)
            {
            dist = sqrt(SQR(x-x_centre) + SQR(y-y_centre));
            if (ABS(dist-radius) < width/2)
                {
                if (get_pixel_value(aImage, x, y) == BLACK)
                    num_pixels++;
                }
            }


/* Next, check the left region
 *              ____________________
 *              |                   |
 *              |        ***        |
 *              |     ***   ***     |
 *              |$$$$$ _______ **   |
 *              |$$$$$ |      | **  |
 *              |$$$$$ |      |  ** |
 *              |$$$$$ |      |  ** |
 *              |$$$$$ |      | **  |
 *              |$$$$$ ------- **   |
 *              |     ***   ***     |
 *              |        ***        |
 *              |
 *              ---------------------
 */

    for (x=x1; x < x2; x++)
        for (y=y2; y < y3; y++)
            {
            dist = sqrt(SQR(x-x_centre) + SQR(y-y_centre));
            if (ABS(dist-radius) < width/2)
                {
                if (get_pixel_value(aImage, x, y) == BLACK)
                    num_pixels++;
                }
            }


/* Finally, check the right region
 *              ____________________
 *              |                   |
 *              |        ***        |
 *              |     ***   ***     |
 *              |   ** _______ $$$$$|
 *              |  **  |      |$$$$$|
 *              | **   |      |$$$$$|
 *              | **   |      |$$$$$|
 *              |  **  |      |$$$$$|
 *              |   ** ------- $$$$$|
 *              |     ***   ***     |
 *              |        ***        |
 *              |
 *              ---------------------
 */

    for (x=x3; x < x4; x++)
        for (y=y2; y < y3; y++)
            {
            dist = sqrt(SQR(x-x_centre) + SQR(y-y_centre));
            if (ABS(dist-radius) < width/2)
                {
                if (get_pixel_value(aImage, x, y) == BLACK)
                    num_pixels++;
                }
            }





    /* 'proportion' is how much of the circle
     * exists. Notice the
     * (2*M_PI*(double )(radius)
     * is the perimeter of the circle multiplied
     * by the line thickness.
     */
    proportion = ((double )num_pixels) /
                            (2*M_PI*(double )(radius)*gLineThickness);


    if (proportion > true_circle_threshold)
        return( True );
    else
        return( False );


}   /* end of true_circle() */



BLACK_PIXEL_LIST *
remove_circle_from_image(int x_centre, int y_centre, int radius,
                            int width,
                            MAP_FROM_POSITION_TO_BLACK_PIXEL *map,
                            BLACK_PIXEL_LIST *black_pixel_list,
                            IMAGE *aImage,
                            int color_index,
                            Window theWindow, GC theGC)
{
    int     x, y;
    int     x1, x2, x3, x4;
    int     y1, y2, y3, y4;
    double  dist;

    BLACK_PIXEL_LIST    *remove_element_from_linked_list_of_black_pixels(
                                    BLACK_PIXEL_LIST *black_pixel_list,
                                    BLACK_PIXEL *p);
    /* get_pixel_value() is in image.c */
    int                 get_pixel_value(IMAGE *aImage, int x, int y);
    /* set_IMAGE_pixel_value() is in image.c */
    void                set_IMAGE_pixel_value(IMAGE *aImage,
                                    int x, int y, int val);



    if (radius < width)
        return( black_pixel_list );


    /* Set foreground to green. */
    XSetForeground(gDisplay, theGC, gRandomColors[color_index].pixel);



/* Count number of pixels lying near the circle.
 * We check every pixel lying between two squares,
 * one a little bigger than the circle, the other a little
 * smaller than the circle.
 * For each black pixel within these squares, we check
 * if it lies near the circle.
 *              ____________________
 *              |                   |
 *              |        ***        |
 *              |     ***   ***     |
 *              |   ** _______ **   |
 *              |  **  |      | **  |
 *              | **   |      |  ** |
 *              | **   |      |  ** |
 *              |  **  |      | **  |
 *              |   ** ------- **   |
 *              |     ***   ***     |
 *              |        ***        |
 *              |
 *              ---------------------
 */

/* Compute the co-ordinates of the corners of the squares.
 *          y4  ____________________
 *              |                   |
 *              |        ***        |
 *              |     ***   ***     |
 *          y3  |   ** _______ **   |
 *              |  **  |      | **  |
 *              | **   |      |  ** |
 *              | **   |      |  ** |
 *              |  **  |      | **  |
 *          y2  |   ** ------- **   |
 *              |     ***   ***     |
 *              |        ***        |
 *              |
 *          y1  ---------------------
 *             x1      x2    x3     x4
 */

    x1 = x_centre - radius - width/2;
    if (x1 < 0)
        x1 = 0;
    else if (x1 > aImage->x)
        x1 = aImage->x;

    x2 = x_centre - (int )(((double )(radius - width/2))/sqrt(2.0));
    if (x2 < 0)
        x2 = 0;
    else if (x2 > aImage->x)
        x2 = aImage->x;

    x3 = x_centre + (int )(((double )(radius - width/2))/sqrt(2.0));
    if (x3 < 0)
        x3 = 0;
    else if (x3 > aImage->x)
        x3 = aImage->x;

    x4 = x_centre + radius + width/2;
    if (x4 < 0)
        x4 = 0;
    else if (x4 > aImage->x)
        x4 = aImage->x;

    y1 = y_centre - radius - width/2;
    if (y1 < 0)
        y1 = 0;
    else if (y1 > aImage->y)
        y1 = aImage->y;

    y2 = y_centre - (int )(((double )(radius - width/2))/sqrt(2.0));
    if (y2 < 0)
        y2 = 0;
    else if (y2 > aImage->y)
        y2 = aImage->y;

    y3 = y_centre + (int )(((double )(radius - width/2))/sqrt(2.0));
    if (y3 < 0)
        y3 = 0;
    else if (y3 > aImage->y)
        y3 = aImage->y;

    y4 = y_centre + radius + width/2;
    if (y4 < 0)
        y4 = 0;
    else if (y4 > aImage->y)
        y4 = aImage->y;


/* First, check the bottom region
 *              ____________________
 *              |                   |
 *              |        ***        |
 *              |     ***   ***     |
 *              |   ** _______ **   |
 *              |  **  |      | **  |
 *              | **   |      |  ** |
 *              | **   |      |  ** |
 *              |  **  |      | **  |
 *              |   ** ------- **   |
 *              |$$$$$$$$$$$$$$$$$$$|
 *              |$$$$$$$$$$$$$$$$$$$|
 *              |$$$$$$$$$$$$$$$$$$$|
 *              ---------------------
 *
 */

    for (x=x1; x < x4; x++)
        for (y=y1; y < y2; y++)
            {
            dist = sqrt(SQR(x-x_centre) + SQR(y-y_centre));
            if (ABS(dist-radius) < width/2)
                {
                XDrawPoint(gDisplay, theWindow, theGC, x, y);
                if (get_pixel_value(aImage, x, y) == BLACK)
                    {
                    set_IMAGE_pixel_value(aImage, x, y, WHITE);
                    if (map->p[x][y] != (BLACK_PIXEL *)NULL)
                        {
                        black_pixel_list =
                                remove_element_from_linked_list_of_black_pixels(
                                            black_pixel_list, map->p[x][y]);
                        map->p[x][y] = (BLACK_PIXEL *)NULL;
                        }
                    }
                }
            }



/* Next, check the top region
 *              ____________________
 *              |$$$$$$$$$$$$$$$$$$$|
 *              |$$$$$$$$$$$$$$$$$$$|
 *              |$$$$$$$$$$$$$$$$$$$|
 *              |   ** _______ **   |
 *              |  **  |      | **  |
 *              | **   |      |  ** |
 *              | **   |      |  ** |
 *              |  **  |      | **  |
 *              |   ** ------- **   |
 *              |     ***   ***     |
 *              |        ***        |
 *              |                   |
 *              ---------------------
 */

    for (x=x1; x < x4; x++)
        for (y=y3; y < y4; y++)
            {
            dist = sqrt(SQR(x-x_centre) + SQR(y-y_centre));
            if (ABS(dist-radius) < width/2)
                {
                XDrawPoint(gDisplay, theWindow, theGC, x, y);
                if (get_pixel_value(aImage, x, y) == BLACK)
                    {
                    set_IMAGE_pixel_value(aImage, x, y, WHITE);
                    if (map->p[x][y] != (BLACK_PIXEL *)NULL)
                        {
                        black_pixel_list =
                                remove_element_from_linked_list_of_black_pixels(
                                            black_pixel_list, map->p[x][y]);
                        map->p[x][y] = (BLACK_PIXEL *)NULL;
                        }
                    }
                }
            }


/* Next, check the left region
 *              ____________________
 *              |                   |
 *              |        ***        |
 *              |     ***   ***     |
 *              |$$$$$ _______ **   |
 *              |$$$$$ |      | **  |
 *              |$$$$$ |      |  ** |
 *              |$$$$$ |      |  ** |
 *              |$$$$$ |      | **  |
 *              |$$$$$ ------- **   |
 *              |     ***   ***     |
 *              |        ***        |
 *              |
 *              ---------------------
 */

    for (x=x1; x < x2; x++)
        for (y=y2; y < y3; y++)
            {
            dist = sqrt(SQR(x-x_centre) + SQR(y-y_centre));
            if (ABS(dist-radius) < width/2)
                {
                XDrawPoint(gDisplay, theWindow, theGC, x, y);
                if (get_pixel_value(aImage, x, y) == BLACK)
                    {
                    set_IMAGE_pixel_value(aImage, x, y, WHITE);
                    if (map->p[x][y] != (BLACK_PIXEL *)NULL)
                        {
                        black_pixel_list =
                                remove_element_from_linked_list_of_black_pixels(
                                            black_pixel_list, map->p[x][y]);
                        map->p[x][y] = (BLACK_PIXEL *)NULL;
                        }
                    }
                }
            }


/* Finally, check the right region
 *              ____________________
 *              |                   |
 *              |        ***        |
 *              |     ***   ***     |
 *              |   ** _______ $$$$$|
 *              |  **  |      |$$$$$|
 *              | **   |      |$$$$$|
 *              | **   |      |$$$$$|
 *              |  **  |      |$$$$$|
 *              |   ** ------- $$$$$|
 *              |     ***   ***     |
 *              |        ***        |
 *              |
 *              ---------------------
 */

    for (x=x3; x < x4; x++)
        for (y=y2; y < y3; y++)
            {
            dist = sqrt(SQR(x-x_centre) + SQR(y-y_centre));
            if (ABS(dist-radius) < width/2)
                {
                XDrawPoint(gDisplay, theWindow, theGC, x, y);
                if (get_pixel_value(aImage, x, y) == BLACK)
                    {
                    set_IMAGE_pixel_value(aImage, x, y, WHITE);
                    if (map->p[x][y] != (BLACK_PIXEL *)NULL)
                        {
                        black_pixel_list =
                                remove_element_from_linked_list_of_black_pixels(
                                            black_pixel_list, map->p[x][y]);
                        map->p[x][y] = (BLACK_PIXEL *)NULL;
                        }
                    }
                }
            }


    XFlush( gDisplay );

    return( black_pixel_list );


}   /* end of remove_circle_from_image() */



void
add_circle_to_list_of_objects_found(double x_centre, double y_centre,
                                    double radius,
                                    int width, int color_index)
{
    DISPLAY_OBJECT      *d;
    DISPLAY_OBJECT      *alloc_display_object(void);
    DISPLAY_OBJECT_LIST *put_in_linked_list_of_display_object(
                                    DISPLAY_OBJECT_LIST *display_oject_list,
                                    DISPLAY_OBJECT *d);


   /* Record the line to be displayed.
    */
    d = alloc_display_object();
    d->type = hough_circle;
    d->obj.hough_circle.x = (int )(x_centre - radius);
    d->obj.hough_circle.y = (int )(y_centre - radius);
    d->obj.hough_circle.diameter = (unsigned int)(2*radius);
    d->obj.hough_circle.width = width;
    d->obj.hough_circle.color_index = color_index;
    gDisplayObjectList = put_in_linked_list_of_display_object(gDisplayObjectList, d);


}   /* end of add_circle_to_list_of_objects_found() */




POINT *
find_ellipse_given_three_random_pixels(BLACK_PIXEL_LIST *black_pixel_list,
                                            IMAGE *aImage)
{
    BLACK_PIXEL         *p1, *p2, *p3, *ptr;
    int                 index1, index2, index3, i;
    double              x_centre, y_centre;
    double              a, b, c;
    double              r1, r2, theta;
    POINT               *point;
    BOOLEAN             return_val;
    POINT               *alloc_point(int dim);
    BOOLEAN             estimate_ellipse_centre(BLACK_PIXEL *p1,
                                        BLACK_PIXEL *p2, BLACK_PIXEL *p3,
                                        IMAGE *aImage,
                                        double *x_return, double *y_return);
    BOOLEAN             transform_ellipse_parameters(double a,
                                        double b, double c,
                                        double *r1, double *r2,
                                        double *theta);


    if (black_pixel_list == (BLACK_PIXEL_LIST *)NULL)
        return( (POINT *)NULL );

    if (black_pixel_list->num_elements <= 3)
        return( (POINT *)NULL );



    /* Randomly choose 3 numbers.
     */
    index1 = (int )(drand48() * (double )black_pixel_list->num_elements);
    do
        index2 = (int )(drand48() * (double )black_pixel_list->num_elements);
    while (index2 == index1);

    do
        index3 = (int )(drand48() * (double )black_pixel_list->num_elements);
    while ((index3 == index1) && (index3 == index2));

    /* p1, p2 and p3 point to 23randomly chosen entries
     * in the linked list of black pixels.
     */
    ptr = black_pixel_list->head;
    p1 = p2 = p3 = (BLACK_PIXEL *)NULL;
    for (i=0; ((i<=index1) || (i<=index2) || (i<=index3)); i++, ptr=ptr->next)
        {
        if (i == index1)
            p1 = ptr;
        if (i == index2)
            p2 = ptr;
        if (i == index3)
            p3 = ptr;
        }

    /* Compute parameters of ellipse */
    if (estimate_ellipse_centre(p1, p2, p3, aImage,
                                &x_centre, &y_centre) == False)
        {
        /* Failed to find ellipse centre for the given three points. */
        return( (POINT *)NULL );
        }

/* Now that we know the ellipse centre, we can translate
 * the ellipse so that its centre is at the origin.
 * This leaves 3 parameters to be estimated, a, b, c.
 * where the equation of an ellipse centred on the origin is:
 *
 *                 -   -   -
 *         -   -  |a   b| |x|
 *        |x,  y| |     | | |  =  1
 *         -   -  |b   c| |y|
 *                 -   -   -
 */
    if (estimate_other_ellipse_parameters(p1, p2, p3,
                                        x_centre, y_centre,
                                        &a, &b, &c)      == False)
        {
        /* Failed to find other circle parameters
         * for the given three points.
         */
        return( (POINT *)NULL );
        }


    return_val = transform_ellipse_parameters(a, b, c,
                                    &r1, &r2, &theta);
    if (return_val == False)
        {
        return( (POINT *)NULL );
        }
    else
        {
        point = alloc_point(5);
        point->x[1] = x_centre;
        point->x[2] = y_centre;
        point->x[3] = r1;
        point->x[4] = r2;
        point->x[5] = theta;
        }

    return(point);


}   /* end of find_ellipse_given_three_random_pixels() */




BOOLEAN
estimate_ellipse_centre(BLACK_PIXEL *p1, BLACK_PIXEL *p2, BLACK_PIXEL *p3,
                                IMAGE *aImage,
                                double *x_return, double *y_return)
{
/* Estimate centre of ellipse
 *
 *
 *                   ************ p1
 *                ***            **
 *              **                  **
 *             *                      *
 *            *                        *
 *            *                        *
 *             *                      *p2
 *              **                  **
 *                ***            ***
 *                   ************
 *
 * Given 2 points p1, p2 on an ellipse,
 * we estimate the tangent at these 2 points and find the
 * intersection of the 2 tangents. Call this point T.
 * We also find the midpoint of the chord passing
 * from p1 to p2. Call this point M.
 * We then define the line passing through T and M.
 * The centre of the ellipse will lie on this line.
 *
 * Next, repeating this with p2 and another point on the ellipse p3,
 * we get another line containing the ellipse centre.
 * The intersection of these 2 lines will gives us an estimate
 * of the ellipse centre.
 */
    double  m1, b1;
    double  m2, b2;
    double  theta1, theta2;
    BOOLEAN return_val;
    BOOLEAN find_line_TM(double x1, double y1, double theta1, double slope1,
                    double x2, double y2, double theta2, double slope2,
                    IMAGE *aImage, double *m_return, double *b_return);



    return_val = find_line_TM((double) p1->x, (double )p1->y,
                            p1->angle, p1->slope,
                            (double )p2->x, (double )p2->y,
                            p2->angle, p2->slope,
                            aImage, &m1, &b1);
    if (return_val == False)
        return( False );


    return_val = find_line_TM((double) p2->x, (double )p2->y,
                            p2->angle, p2->slope,
                            (double )p3->x, (double )p3->y,
                            p3->angle, p3->slope,
                            aImage, &m2, &b2);
    if (return_val == False)
        return( False );

/* We now have 2 lines of the form:
 *              y = m1 * x + b1
 *              y = m2 * x + b2
 * The intersection of these lines will be the ellipse centre.
 * If the 2 lines are almost paralell, then don't try to find
 * the intersection.
 */


    /* Don't bother finding intersection point
     * of lines if they are almost parallel.
     */
    theta1 = atan2(1.0, m1);
    theta2 = atan2(1.0, m2);

    /* Make sure that theta2 < theta1
     * n.b. atan2 returns a value in the range [-M_PI, M_PI]
     */
    if (theta2 > theta1)
        theta2 -= M_PI;

    if ( (((theta1-theta2) > (M_PI/20.0)) &&
                 ((theta1-theta2) < (19.0*M_PI/20.0))) )
        {
        *x_return = (b1 - b2) / (m2 - m1);
        *y_return = m1 * (*x_return) + b1;
        return( True );
        }
    else
        return( False );


}   /* end of estimate_ellipse_centre() */


BOOLEAN
find_line_TM(double x1, double y1, double theta1, double slope1,
                    double x2, double y2, double theta2, double slope2,
                    IMAGE *aImage, double *m_return, double *b_return)
{
    double      t1, t2, m1, m2;



    if ((theta1 == NO_ANGLE) || (theta2 == NO_ANGLE))
        {
        /* Could not estimate angle of edge at one of the points
         */
        return( False );
        }
    if ((ABS(theta1) > 0.99*M_PI_2) || (ABS(theta2) > 0.99*M_PI_2))
        {
        /* If tangent is almost vertical, then fail.
         * Soon, we will need to do equations with
         * the slope (dy/dx) which will be near infinite
         * for near vertical tangents.
         */
        return( False );
        }


    /* Make sure that theta2 < theta1
     */
    if (theta2 > theta1)
        theta2 -= M_PI;

    /* Don't bother finding meeting point
     * of tangents if the normals are almost
     * parallel.
     */
    if ( (((theta1-theta2) > (M_PI/20.0)) &&
                 ((theta1-theta2) < (19.0*M_PI/20.0))) )
        {
        /* (t1, t2) is the intersection point
         * of the two tangents.
         * (m1, m2) is the midpoint
         * of (x,y) and (i,j)
         */
        t1 = (y1 - y2 - x1*slope1 + x2*slope2) / (slope2-slope1);
        t2 = (slope1*slope2*(x2-x1) - y2*slope1 + y1*slope2) / (slope2-slope1);
        m1 = (x1 + x2) / 2;
        m2 = (y1 + y2) / 2;

        *m_return = (t2-m2) / (t1-m1);
        *b_return = (m2*t1 - m1*t2) / (t1-m1);
        return( True );
        }
    else
        return( False );

}       /* end of find_line_TM() */



BOOLEAN
estimate_other_ellipse_parameters(BLACK_PIXEL *p1, BLACK_PIXEL *p2,
                                BLACK_PIXEL *p3,
                                double x_centre, double y_centre,
                                double *a_return, double *b_return,
                                double *c_return)
{
    double      **M, *b, d;
    int         *indx;
    double      x1, y1, x2, y2, x3, y3;
    double      **alloc_array(int row, int column);
    void        free_array(double **A, int row);
    double      *alloc_vector(int dim);


    M = alloc_array(3, 3);
    b = alloc_vector(3);
    indx = (int *)malloc(4 * sizeof(int) );
    if ((M == NULL) || (b == NULL) || (indx == NULL))
        {
        fprintf(stderr, "malloc failed in estimate_other_ellipse_parameters() in rht.c\n");
        exit(1);
        }

    x1 = ((double )p1->x) - x_centre;
    y1 = ((double )p1->y) - y_centre;
    x2 = ((double )p2->x) - x_centre;
    y2 = ((double )p2->y) - y_centre;
    x3 = ((double )p3->x) - x_centre;
    y3 = ((double )p3->y) - y_centre;
    /* Set up a system of equations to be solved simultaneously
     * to find c[].
     *
     *        _   _     _  _     _  _
     *       |     |   | a  |   |    |
     *       |  M  | . | b  | = | b  |
     *       |     |   | c  |   |    |
     *        -   -     -  -     -  -
     */
    M[1][1] = SQR(x1);    M[1][2] = 2.0*x1*y1;    M[1][3] = SQR(y1);
    M[2][1] = SQR(x2);    M[2][2] = 2.0*x2*y2;    M[2][3] = SQR(y2);
    M[3][1] = SQR(x3);    M[3][2] = 2.0*x3*y3;    M[3][3] = SQR(y3);
    b[1] = 1.0;
    b[2] = 1.0;
    b[3] = 1.0;


    /* Solve the systems of equations using LU decomposition.
     */
    if (lu_decomposition(M, 3, indx, &d) != 0)
        {
        return( False );
        }
    lu_back_substitution(M, 3, indx, b);

    *a_return = b[1];
    *b_return = b[2];
    *c_return = b[3];

    free_array(M, 3);
    free(b);
    free(indx);


/* Check that these are the parameters of an ellipse
 * by checking if the determinant of the matrix
 *
 *         -   - 
 *        |a   b|
 *        |b   c|
 *         -   - 
 * is positive.
 */

    if ((*a_return) * (*c_return) - SQR(*b_return) > 0)
        {
        return( True );
        }
    else
        {
        return( False );
        }

}       /* end of estimate_other_ellipse_parameters() */




#define TINY_VALUE 1.0e-20;
int
lu_decomposition(double **M, int n, int *indx, double *d)
{
    int     i, imax, j, k;
    double  big,dum,sum,temp;
    double  *vec;
    double  *alloc_vector(int dim);

    vec = alloc_vector(n);
    *d = 1.0;
    for (i=1; i <= n; i++)
        {
        big=0.0;
        for (j=1; j <= n; j++)
            if ((temp=ABS(M[i][j])) > big) big=temp;
        if (big == 0.0)
            {
            /* Singular matrix in routine lu_decomposition
             */
            return(-1);
            }
        vec[i]=1.0/big;
        }
    for (j=1; j <= n; j++)
        {
        for (i=1; i < j; i++)
            {
            sum=M[i][j];
            for (k=1;k<i;k++)
                sum -= M[i][k]*M[k][j];
            M[i][j]=sum;
            }
        big=0.0;
        imax = 0;
        for (i=j; i <= n; i++)
            {
            sum = M[i][j];
            for (k=1; k < j; k++)
                sum -= M[i][k]*M[k][j];
            M[i][j]=sum;
            if ( (dum=vec[i]*ABS(sum)) >= big)
                {
                big = dum;
                imax = i;
                }
            }
        if (j != imax)
            {
            for (k=1; k <= n; k++)
                {
                dum=M[imax][k];
                M[imax][k]=M[j][k];
                M[j][k]=dum;
                }
            *d = -(*d);
            vec[imax]=vec[j];
            }
        indx[j]=imax;
        if (M[j][j] == 0.0)
            M[j][j] = TINY_VALUE;
        if (j != n)
            {
            dum=1.0/(M[j][j]);
            for (i=j+1; i<=n; i++)
                M[i][j] *= dum;
            }
        }
    free(vec);


    return(0);

}           /* end of lu_decomposition() */
#undef TINY_VALUE



void
lu_back_substitution(double **M, int n, int *indx, double *b)
{
    int     i, ii=0, ip, j;
    double  sum;

    for (i=1;i<=n;i++)
        {
        ip=indx[i];
        sum=b[ip];
        b[ip]=b[i];
        if (ii)
            for (j=ii;j<=i-1;j++)
                sum -= M[i][j]*b[j];
        else if (sum) ii=i;
        b[i] = sum;
        }
    for (i=n; i >=1 ;i--)
        {
        sum = b[i];
        for (j=i+1;j<=n;j++)
            sum -= M[i][j]*b[j];
        b[i] = sum/M[i][i];
        }
}           /* end of lu_decomposition() */




BOOLEAN
transform_ellipse_parameters(double a, double b, double c,
                                    double *r1, double *r2, double *theta)
{
    double  **M, **C, **eig_vec;
    double  *eig_val;
    double  **alloc_array(int row, int column);
    double  *alloc_vector(int dim);
    void    find_inverse(double **MAT, double **I, int dim);
    void    find_eigenvectors_2x2_positive_semi_def_real_matrix(double **covar, double *eig_val, double **eig_vec);
    void    free_array(double **A, int row);



    M = alloc_array(2, 2);
    C = alloc_array(2, 2);
    eig_vec = alloc_array(2, 2);
    eig_val = alloc_vector(2);

    if ((M == NULL) || (C == NULL) || (eig_val == NULL) || (eig_vec == NULL))
        {
        fprintf(stderr, "malloc failed in transform_ellipse_parameters() in rht.c\n");
        exit(1);
        }


    M[1][1] = a;
    M[1][2] = M[2][1] = b;
    M[2][2] = c;
    find_inverse(M, C, 2);
    find_eigenvectors_2x2_positive_semi_def_real_matrix(C, eig_val, eig_vec);

    if ((eig_val[1] <= 0) || (eig_val[2] <= 0))
        {
        return( False );
        }


    *r1 = sqrt(eig_val[1]);
    *r2 = sqrt(eig_val[2]);
    *theta = atan2(eig_vec[2][1], eig_vec[1][1]);


    free_array(M, 2);
    free_array(C, 2);
    free_array(eig_vec, 2);
    free(eig_val);


    return( True );

}       /* end of transform_ellipse_parameters() */




BOOLEAN
true_ellipse(double x_centre, double y_centre,
                        double r1, double r2, double theta,
                        double true_ellipse_threshold, IMAGE *aImage,
                        int width, Window theWindow, GC theGC)
{
    int     num_pixels = 0;
    double  perimeter, proportion;

    /* get_pixel_value() is in image.c */
    int     get_pixel_value(IMAGE *aImage, int x, int y);
    int     count_number_of_pixels_near_ellipse(
                                        double x_centre, double y_centre,
                                        double r1, double r2, double theta,
                                        int width, IMAGE *aImage,
                                        Window theWindow, GC theGC);




    if ((r1 <= width) || (r2 < width))
        return( False );


    /* Count number of black pixels
     * that lie near the ellipse.
     */
    num_pixels = count_number_of_pixels_near_ellipse(x_centre, y_centre,
                                        r1, r2, theta, width, aImage,
                                        theWindow, theGC);


    /* 'proportion' is how much of the ellipse
     * exists. Notice that the
     * perimeter of the ellipse is multiplied
     * by the line thickness.
     */
    perimeter = M_PI * ( 3*(r1+r2) - sqrt((r1+3*r2)*(3*r1+r2)) );

    proportion = ((double )num_pixels) / (perimeter*gLineThickness);


    if (proportion > true_ellipse_threshold)
        return( True );
    else
        return( False );


}   /* end of true_ellipse() */




int
count_number_of_pixels_near_ellipse( double x_centre, double y_centre,
                                        double r1, double r2, double theta,
                                        int width, IMAGE *aImage,
                                        Window theWindow, GC theGC)
{
/*
 *      x, y;       -- centre of ellipse
 *      r1, r2;     -- major/minor axis
 *      theta;      -- angle
 */
    int             x, y;
    int             x1, y1, x2, y2;
    double          perimeter, incr, grad, rho;
    double          M[3][3];
    int             offset;
    int             num_pixels = 0;
    BOOLEAN         test_pixel_for_tag(IMAGE *aImage, int x, int y, int tag);
    void            tag_pixel(IMAGE *aImage, int x, int y, int tag);
    void            untag_image(IMAGE *aImage);
    int             get_pixel_value(IMAGE *aImage, int x, int y);



    if ((r1 <= width) || (r2 <= width))
        return(0);


    untag_image(aImage);


    /* The ellipse will be contructed as a sequence of
     * line segments.
     * incr indicates how much of the ellipse
     * each line segments will cover.
     */
    perimeter = M_PI * ( 3*(ABS(r1)+ABS(r2)) - sqrt( (ABS(r1)+3*ABS(r2))*(3*ABS(r1)+ABS(r2)) ) );


    /* Error check.
     */
    if (perimeter <= width)
        return( 0 );        /* zero width or length ellipse.
                             * Don't bother counting pixels.
                             */



    incr = 60.0/perimeter;


    /* The ellipse is defined as all points [x,y] given by:
     *
     * { [x,y] : for all theta in [0, 2pi),
     *           [x,y] = E . D . E^-1 . [cos(theta), sin (theta)}
     *
     * where E is the matrix containing the direction of
     * the pricipal axes
     * of the ellipse
     * and D is:   -    -
     *            |r1  0 |
     *            | 0  r2|
     *             -    -
     * First calculate E . D . E^-1
     */
    M[1][1] = r1*SQR(cos(theta)) + r2*SQR(sin(theta));
    M[1][2] = M[2][1] = (r1-r2)*sin(theta)*cos(theta);
    M[2][2] = r1*SQR(sin(theta)) + r2*SQR(cos(theta));

    x1 = (int )(x_centre + M[1][1]*cos(0.0) + M[1][2]*sin(0.0));
    y1 = (int )(y_centre + M[2][1]*cos(0.0) + M[2][2]*sin(0.0));


    for (rho=incr; rho < (2*M_PI + incr); rho += incr)
        {
        x2 = (int )(x_centre + M[1][1]*cos(rho) + M[1][2]*sin(rho));
        y2 = (int )(y_centre + M[2][1]*cos(rho) + M[2][2]*sin(rho));
        if ((x1 >= 0) && (x1 < aImage->x) && (y1 >= 0) && (y1 < aImage->y) &&
            (x2 >= 0) && (x2 < aImage->x) && (y2 >= 0) && (y2 < aImage->y))
            {
            if (ABS(x2-x1) >= ABS(y2-y1))
                {
                /* x changes more than y
                 */
                if (x1 < x2)
                    {
                    grad = ((double )(y2-y1)) / (double )(x2-x1);
                    for (x=x1; x <= x2; x++)
                        {
                        y = y1 + (int )(grad*(double )(x - x1));
                        for (offset=-width/2; offset < (width+1)/2; offset++)
                            {
                            if (((y+offset) >= 0) && ((y+offset) < aImage->y))
                                {
                                if ((get_pixel_value(aImage,x,y+offset)
                                                                == BLACK) &&
                                    (test_pixel_for_tag(aImage,x,y+offset,1)
                                                                == False))
                                    {
                                    num_pixels++;
                                    tag_pixel( aImage, x, y+offset, 1);
                                    }
                                }
                            }
                        }
                    }
                else if (x1 > x2)
                    {
                    grad = ((double )(y1-y2)) / (double )(x1-x2);
                    for (x=x2; x <= x1; x++)
                        {
                        y = y2 + (int )(grad*(double )(x - x2));
                        for (offset=-width/2; offset < (width+1)/2; offset++)
                            {
                            if (((y+offset) >= 0) && ((y+offset) < aImage->y))
                                {
                                if ((get_pixel_value(aImage,x,y+offset)
                                                                == BLACK) &&
                                    (test_pixel_for_tag(aImage,x,y+offset,1)
                                                                == False))
                                    {
                                    num_pixels++;
                                    tag_pixel( aImage, x, y+offset, 1);
                                    }
                                }
                            }
                        }
                   }
                }
            else        /* if (ABS(x2-x1) < ABS(y2-y1)) */
                {
                /* y changes more than x
                 */
                if (y1 < y2)
                    {
                    grad = ((double )(x2-x1)) / (double )(y2-y1);
                    for (y=y1; y <= y2; y++)
                        {
                        x = x1 + (int )(grad*(double )(y-y1));
                        for (offset=-width/2; offset < (width+1)/2; offset++)
                            {
                            if (((x+offset) >= 0) && ((x+offset) < aImage->x))
                                {
                                if ((get_pixel_value(aImage,x+offset,y)
                                                                == BLACK) &&
                                    (test_pixel_for_tag(aImage,x+offset,y,1)
                                                                == False))
                                    {
                                    num_pixels++;
                                    tag_pixel( aImage, x+offset, y, 1);
                                    }
                                }
                            }
                        }
                    }
                else if (y1 > y2)
                    {
                    grad = ((double )(x1-x2)) / (double )(y1-y2);
                    for (y=y2; y <= y1; y++)
                        {
                        x = x2 + (int )(grad*(double )(y - y2));
                        for (offset=-width/2; offset < (width+1)/2; offset++)
                            {
                            if (((x+offset) >= 0) && ((x+offset) < aImage->x))
                                {
                                if ((get_pixel_value(aImage,x+offset,y)
                                                                == BLACK) &&
                                    (test_pixel_for_tag(aImage,x+offset,y,1)
                                                                == False))
                                    {
                                    num_pixels++;
                                    tag_pixel( aImage, x+offset, y, 1);
                                    }
                                }
                            }
                        }
                    }
                }
            }   /* end of 'if ((x1 >= 0) && (x1 < aImage->x) &&...' */
        x1 = x2;
        y1 = y2;
        }


    return( num_pixels );


}   /* count_number_of_pixels_near_ellipse() */




BLACK_PIXEL_LIST *
remove_ellipse_from_image( double x_centre, double y_centre,
                                    double r1, double r2, double theta,
                                    int width,
                                    MAP_FROM_POSITION_TO_BLACK_PIXEL *map,
                                    BLACK_PIXEL_LIST *black_pixel_list,
                                    IMAGE *aImage,
                                    int color_index,
                                    Window theWindow, GC theGC )
{
/*
 *      x, y;       -- centre of ellipse
 *      r1, r2;     -- major/minor axis
 *      theta;      -- angle
 */
    int                 x, y;
    int                 x1, y1, x2, y2;
    double              perimeter, incr, grad, rho;
    double              M[3][3];
    int                 offset;
    int                 get_pixel_value(IMAGE *aImage, int x, int y);
    void                set_IMAGE_pixel_value(IMAGE *aImage,
                                        int x, int y, int val);
    BLACK_PIXEL_LIST    *remove_element_from_linked_list_of_black_pixels(
                                        BLACK_PIXEL_LIST *black_pixel_list,
                                        BLACK_PIXEL *p);



    if ((r1 <= width) || (r2 <= width))
        return( black_pixel_list );



    /* Set foreground to green. */
    XSetForeground(gDisplay, theGC, gRandomColors[color_index].pixel);


    /* The ellipse will be contructed as a sequence of
     * line segments.
     * incr indicates how much of the ellipse
     * each line segments will cover.
     */
    perimeter = M_PI * ( 3*(ABS(r1)+ABS(r2)) - sqrt( (ABS(r1)+3*ABS(r2))*(3*ABS(r1)+ABS(r2)) ) );


    /* Error check.
     */
    if (perimeter <= width)
        return( black_pixel_list );     /* zero width or length ellipse.
                                         * Don't bother counting pixels.
                                         */



    incr = 60.0/perimeter;


    /* The ellipse is defined as all points [x,y] given by:
     *
     * { [x,y] : for all theta in [0, 2pi),
     *           [x,y] = E . D . E^-1 . [cos(theta), sin (theta)}
     *
     * where E is the matrix containing the direction of
     * the pricipal axes
     * of the ellipse
     * and D is:   -    -
     *            |r1  0 |
     *            | 0  r2|
     *             -    -
     * First calculate E . D . E^-1
     */
    M[1][1] = r1*SQR(cos(theta)) + r2*SQR(sin(theta));
    M[1][2] = M[2][1] = (r1-r2)*sin(theta)*cos(theta);
    M[2][2] = r1*SQR(sin(theta)) + r2*SQR(cos(theta));

    x1 = (int )(x_centre + M[1][1]*cos(0.0) + M[1][2]*sin(0.0));
    y1 = (int )(y_centre + M[2][1]*cos(0.0) + M[2][2]*sin(0.0));


    for (rho=incr; rho < (2*M_PI + incr); rho += incr)
        {
        x2 = (int )(x_centre + M[1][1]*cos(rho) + M[1][2]*sin(rho));
        y2 = (int )(y_centre + M[2][1]*cos(rho) + M[2][2]*sin(rho));
        if ((x1 >= 0) && (x1 < aImage->x) && (y1 >= 0) && (y1 < aImage->y) &&
            (x2 >= 0) && (x2 < aImage->x) && (y2 >= 0) && (y2 < aImage->y))
            {
            if (ABS(x2-x1) >= ABS(y2-y1))
                {
                /* x changes more than y
                 */
                if (x1 < x2)
                    {
                    grad = ((double )(y2-y1)) / (double )(x2-x1);
                    for (x=x1; x <= x2; x++)
                        {
                        y = y1 + (int )(grad*(double )(x - x1));
                        for (offset=-width/2; offset < (width+1)/2; offset++)
                            {
                            if (((y+offset) >= 0) && ((y+offset) < aImage->y))
                                {
                                XDrawPoint(gDisplay, theWindow, theGC, x, y+offset);
                                if (get_pixel_value(aImage, x, y+offset) == BLACK)
                                    {
                                    set_IMAGE_pixel_value(aImage, x, y+offset,
                                                                    WHITE);
                                    if (map->p[x][y+offset] != (BLACK_PIXEL *)NULL)
                                        {
                                        black_pixel_list = remove_element_from_linked_list_of_black_pixels(black_pixel_list, map->p[x][y+offset]);
                                        map->p[x][y+offset] = (BLACK_PIXEL *)NULL;
                                        }
                                    }
                                }
                            }
                        }
                    }
                else if (x1 > x2)
                    {
                    grad = ((double )(y1-y2)) / (double )(x1-x2);
                    for (x=x2; x <= x1; x++)
                        {
                        y = y2 + (int )(grad*(double )(x - x2));
                        for (offset=-width/2; offset < (width+1)/2; offset++)
                            {
                            if (((y+offset) >= 0) && ((y+offset) < aImage->y))
                                {
                                XDrawPoint(gDisplay, theWindow, theGC, x, y+offset);
                                if (get_pixel_value(aImage, x, y+offset) == BLACK)
                                    {
                                    set_IMAGE_pixel_value(aImage, x, y+offset,
                                                                    WHITE);
                                    if (map->p[x][y+offset] != (BLACK_PIXEL *)NULL)
                                        {
                                        black_pixel_list = remove_element_from_linked_list_of_black_pixels(black_pixel_list, map->p[x][y+offset]);
                                        map->p[x][y+offset] = (BLACK_PIXEL *)NULL;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            else        /* if (ABS(x2-x1) < ABS(y2-y1)) */
                {
                /* y changes more than x
                 */
                if (y1 < y2)
                    {
                    grad = ((double )(x2-x1)) / (double )(y2-y1);
                    for (y=y1; y <= y2; y++)
                        {
                        x = x1 + (int )(grad*(double )(y-y1));
                        for (offset=-width/2; offset < (width+1)/2; offset++)
                            {
                            if (((x+offset) >= 0) && ((x+offset) < aImage->x))
                                {
                                XDrawPoint(gDisplay, theWindow, theGC, x+offset, y);
                                if (get_pixel_value(aImage, x+offset, y) == BLACK)
                                    {
                                    set_IMAGE_pixel_value(aImage, x+offset, y,
                                                                    WHITE);
                                    if (map->p[x+offset][y] != (BLACK_PIXEL *)NULL)
                                        {
                                        black_pixel_list = remove_element_from_linked_list_of_black_pixels(black_pixel_list, map->p[x+offset][y]);
                                        map->p[x+offset][y] = (BLACK_PIXEL *)NULL;
                                        }
                                    }
                                }
                            }
                        }
                    }
                else if (y1 > y2)
                    {
                    grad = ((double )(x1-x2)) / (double )(y1-y2);
                    for (y=y2; y <= y1; y++)
                        {
                        x = x2 + (int )(grad*(double )(y - y2));
                        for (offset=-width/2; offset < (width+1)/2; offset++)
                            {
                            if (((x+offset) >= 0) && ((x+offset) < aImage->x))
                                {
                                XDrawPoint(gDisplay, theWindow, theGC, x+offset, y);
                                if (get_pixel_value(aImage, x+offset, y) == BLACK)
                                    {
                                    set_IMAGE_pixel_value(aImage, x+offset, y,
                                                                    WHITE);
                                    if (map->p[x+offset][y] != (BLACK_PIXEL *)NULL)
                                        {
                                        black_pixel_list = remove_element_from_linked_list_of_black_pixels(black_pixel_list, map->p[x+offset][y]);
                                        map->p[x+offset][y] = (BLACK_PIXEL *)NULL;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }   /* end of 'if ((x1 >= 0) && (x1 < aImage->x) &&...' */
        x1 = x2;
        y1 = y2;
        }
    XFlush(gDisplay);


    return( black_pixel_list );


}   /* remove_ellipse_from_image() */



void
add_ellipse_to_list_of_objects_found(double x_centre, double y_centre,
                                    double r1, double r2, double theta,
                                    int width, int color_index)
{
    DISPLAY_OBJECT      *d;
    DISPLAY_OBJECT      *alloc_display_object(void);
    DISPLAY_OBJECT_LIST *put_in_linked_list_of_display_object(
                                    DISPLAY_OBJECT_LIST *display_oject_list,
                                    DISPLAY_OBJECT *d);


   /* Record the line to be displayed.
    */
    d = alloc_display_object();
    d->type = hough_ellipse;
    d->obj.hough_ellipse.x = x_centre;
    d->obj.hough_ellipse.y = y_centre;
    d->obj.hough_ellipse.a = r1;
    d->obj.hough_ellipse.b = r2;
    d->obj.hough_ellipse.theta = theta;
    d->obj.hough_ellipse.width = width;
    d->obj.hough_ellipse.color_index = color_index;
    gDisplayObjectList = put_in_linked_list_of_display_object(gDisplayObjectList, d);


}   /* end of add_ellipse_to_list_of_objects_found() */



/* Find the tangent of the edge at point (x,y)
 */
double
find_angle_of_tangent(IMAGE *aImage, int x, int y)
{
    int     x1, y1, x2, y2, i, j;
    double  mean_x, mean_y, num_points;
    double  theta;
    double  radius;
    double  **covar, **eig_vec, *eig_val;
    int     get_pixel_value(IMAGE *aImage, int x, int y);
    double  **alloc_array(int row, int column);
    void    free_array(double **A, int row);
    double  *alloc_vector(int dim);
    void    find_eigenvectors_2x2_positive_semi_def_real_matrix(double **covar, double *eig_val, double **eig_vec);



    radius = (double )(gLineThickness*5);

    x1 = x - (int )radius;
    y1 = y - (int )radius;
    if (x1 < 0)
        x1 = 0;
    if (y1 < 0)
        y1 = 0;
    x2 = x + (int )radius;
    y2 = y + (int )radius;
    if (x2 >= aImage->x)
        x2 = aImage->x-1;
    if (y2 >= aImage->y)
        y2 = aImage->y-1;



    /* Calculate mean of points in ball
     */
    mean_x = 0.0;
    mean_y = 0.0;
    num_points = 0.0;

    for (i=x1; i <= x2; i++)
        for (j=y1; j <= y2; j++)
            if (get_pixel_value(aImage,i,j) == BLACK)
                if ((SQR(i-x) + SQR(j-y)) < SQR(radius))
                    {
                    mean_x += (double )i;
                    mean_y += (double )j;
                    num_points++;
                    }

    /* Need at least a total of 1 point for a gaussian
     * or the covariance formule doesn't make sense
     * since it has a (num_points-1) in the denominator.
     */
    if (num_points <= 1.0)
        return( NO_ANGLE );

    mean_x = mean_x / num_points;
    mean_y = mean_y / num_points;




    /* Calculate covariance matrix
     */
    covar = alloc_array(2, 2);
    eig_vec = alloc_array(2, 2);
    eig_val = alloc_vector(2);
    covar[1][1] = 0.0;  covar[1][2] = 0.0;
    covar[2][1] = 0.0;  covar[2][2] = 0.0;

    for (i=x1; i <=x2; i++)
        for (j=y1; j <=y2; j++)
            if (get_pixel_value(aImage,i,j) == BLACK)
                if ((SQR(i-x) + SQR(j-y)) < SQR(radius))
                    {
                    covar[1][1] += SQR(((double )i) - mean_x);
                    covar[1][2] += (((double )i) - mean_x)*(((double )j) - mean_y);
                    covar[2][2] += SQR(((double )j) - mean_y);
                    }


    covar[1][1] /= (double )(num_points-1);
    covar[1][2] /= (double )(num_points-1);
    covar[2][2] /= (double )(num_points-1);
    covar[2][1] = covar[1][2];




    find_eigenvectors_2x2_positive_semi_def_real_matrix(covar,eig_val,eig_vec);

    theta = atan2(eig_vec[2][1], eig_vec[1][1]);

    if (eig_val[1] > 0)
        {
        free_array(covar, 2);
        free_array(eig_vec, 2);
        free(eig_val);
        return( theta );
        }
    else
        {
        free_array(covar, 2);
        free_array(eig_vec, 2);
        free(eig_val);
        return( NO_ANGLE );
        }


}   /* end of find_angle_of_tangent() */
