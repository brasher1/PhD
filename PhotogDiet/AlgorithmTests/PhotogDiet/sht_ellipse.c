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
#include <time.h>

#define NO_ANGLE    -100

#ifdef COMPUTATION_TIME
extern int      gParity;
extern int      gOverflowAdd;
#endif


/* Structures
 */
typedef struct hough_point_type {
    int         x, y;
    double      theta, slope;
    BOOLEAN     valid_angle;        /* is the value in theta, slope valid */
    struct hough_point_type *next;
} HOUGH_POINT;



typedef struct hough_point_list_type {
    HOUGH_POINT     *head;
    HOUGH_POINT     *tail;
    int             num_elements;   /* number of elements in linked list. */

} HOUGH_POINT_LIST;



typedef struct ellipse_type {
    /* An ellipse centred on (x,y)
     * with major axis of length a at angle theta
     * and minor axis of length b.
     *
     * If the co-ordinate system were translated
     * by (x, y) and then rotated by theta,
     * so as to centre the ellipse on the origin and
     * bring the major axis in line with the x-axis,
     * then the ellipse would be defined by the points:
     *     { (x,y) : (x/a)^2 + (y/b)^2 = 1}
     *
     * note: When I refer to the major axis, I mean
     *       the largest radius of the ellipse.
     *       The minor axis refers to the smallest
     *       radius of the ellipse.
     */

    double  x, y;
    double  a, b, theta;
    struct ellipse_type *next;

} ELLIPSE;






/* Function Prototypes
 */
void        find_ellipses_with_SHT(IMAGE *aImage, XImage *aXImage, Window theWindow, GC theGC);
HOUGH_POINT *alloc_hough_point(void);
void        free_hough_point(HOUGH_POINT *hp);
HOUGH_POINT_LIST    *alloc_hough_point_list(void);
HOUGH_POINT_LIST    *put_in_linked_list_of_hough_point(
                                            HOUGH_POINT_LIST *hough_point_list,
                                            HOUGH_POINT *hp);
void        free_linked_list_of_hough_point(HOUGH_POINT_LIST *hough_point_list);
ELLIPSE     *alloc_ellipse(void);
void        free_ellipse(ELLIPSE *e);
ELLIPSE     *put_in_linked_list_of_ellipse(ELLIPSE *head, ELLIPSE *e);
void        free_linked_list_of_ellipse(ELLIPSE *ellipse_head);
void        ellipse_centre_SHT_hough_transform(IMAGE *aImage, int **histogram);
ELLIPSE     *find_other_three_ellipse_parameters(POINT_LIST *centre_point_list, IMAGE *aImage, Window theWindow, GC theGC);
void    other_three_parameters_SHT_hough_transform(int ***histogram, POINT *centre_point, double a_start, double a_end, double b_start, double b_end, double theta_start, double theta_end, IMAGE *aImage);
void        find_max_of_3D_histogram(int ***histogram, int *x_val, int *y_val, int *z_val, int x_max, int y_max, int z_max);
int ***     alloc_3D_histogram(int x_max, int y_max, int z_max);
void        free_3D_histogram(int ***histogram, int x_max, int y_max);
double      find_proportion_of_ellipse_that_exists(double x_e, double y_e, double a, double b, double theta, IMAGE *aImage);

#ifdef COMPUTATION_TIME
void        check_parity(void);
#endif





void
find_ellipses_with_SHT(IMAGE *aImage, XImage *aXImage, Window theWindow, GC theGC)
{
/* PARAMETERS */
    int     window_size = 50;           /* Window for max of histogram */
    double  sample_proportion_for_local_max = 0.1;
    int     min_local_max_value = 4500;


    int     color_index = 0;
    int     **histogram;
    POINT_LIST   *centre_point_list;
    ELLIPSE *ellipse, *ellipse_list;
    DISPLAY_OBJECT  *d;
    int     num_ellipses;
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
    void    ellipse_centre_SHT_hough_transform(IMAGE *aImage, int **histogram);
    void    average_histogram(int **histogram, int x_max, int y_max);
    POINT_LIST      *find_local_max_of_histogram(int win_size,
                                        int min_local_max_value,
                                        double sample_proportion_for_local_max,
                                        int **histogram, int x_max, int y_max,
                                        Window theWindow, GC theGC);
    ELLIPSE *find_other_three_ellipse_parameters(POINT_LIST *centre_point_list, IMAGE *aImage, Window theWindow, GC theGC);
    void    free_linked_list_of_ellipse(ELLIPSE *ellipse_head);
    void    free_linked_list_of_point(POINT_LIST *point_list);
    DISPLAY_OBJECT      *alloc_display_object(void);
    DISPLAY_OBJECT_LIST *put_in_linked_list_of_display_object(DISPLAY_OBJECT_LIST *display_oject_list, DISPLAY_OBJECT *d);
    void    display_message_and_wait(Window theWindow, int x, int y, int num_strings, ...);
#ifdef XGOBI
    int     x_xgobi, y_xgobi;
    FILE    *dat_file, *lines_file;
#endif
#ifdef HUGH
    int     x_hugh, y_hugh;
    FILE    *dat_hugh_file;
#endif




    /* Open a window which tells the
     * user that we have started
     * and warns them that the Hough Transform with ellipses
     * tends to take a long time.
     */
    theScreen = DefaultScreen(gDisplay);
    theBlackPixel = BlackPixel( gDisplay, theScreen);
    XSetForeground(gDisplay, theGC, theBlackPixel); /* Black text */
    message_window = alloc_message_window();
    draw_image(aXImage, theWindow, theGC);
    init_message_window_fields(message_window, theWindow, 0, 0, 9,
                            "",
                            "Started Standard Hough Transform",
                            "to find ellipses.",
                            "",
                            "This may take a long time",
                            "(i.e. from minutes to hours).",
                            "The window will not redraw until",
                            "Hough Transform has finished.",
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





    num_ellipses = 0;
    histogram = alloc_histogram(aImage->x, aImage->y);
    ellipse_centre_SHT_hough_transform(aImage, histogram);

    average_histogram(histogram, aImage->x, aImage->y);

    centre_point_list = find_local_max_of_histogram(window_size,
                                        min_local_max_value,
                                        sample_proportion_for_local_max,
                                        histogram, aImage->x, aImage->y,
                                        theWindow, theGC);



#ifdef XGOBI
    dat_file = fopen("h.dat", "w");
    lines_file = fopen("h.lines", "w");
    if ((dat_file == NULL) || (lines_file == NULL))
        {
        fprintf(stderr, "Could not open file in find_ellipses_with_SHT() in sht_ellipsee.c\n");
        exit(1);
        }

    fprintf(dat_file, "0 0 1000\n");
    fprintf(dat_file, "%d 0 1000\n", aImage->x);
    fprintf(dat_file, "0 %d 1000\n", aImage->y);
    fprintf(dat_file, "%d %d 1000\n", aImage->x, aImage->y);
    fprintf(lines_file, "1 2\n1 3\n2 4\n3 4\n");
    for (x_xgobi=0; x_xgobi < aImage->x; x_xgobi++)
        for (y_xgobi=0; y_xgobi < aImage->y; y_xgobi++)
            if (histogram[x_xgobi][y_xgobi] > (gLineThickness*gLineThickness*1000))
                {
                fprintf(dat_file, "%d %d %d\n", x_xgobi, y_xgobi, histogram[x_xgobi][y_xgobi]);
                }
    fclose(dat_file);
    fclose(lines_file);
    exit(1);
#endif
#ifdef HUGH
    dat_hugh_file = fopen("g.dat", "w");
    if (dat_hugh_file == NULL)
        {
        fprintf(stderr, "Could not open file in find_ellipses_with_SHT() in sht_ellipse.c\n");
        exit(1);
        }
    for (x_hugh=0; x_hugh < aImage->x; x_hugh++)
        for (y_hugh=0; y_hugh < aImage->y; y_hugh++)
            if (histogram[x_hugh][y_hugh] > 700)
                {
                fprintf(dat_hugh_file, "%d %d %d\n", x_hugh, y_hugh, histogram[x_hugh][y_hugh]);
                }
    fclose(dat_hugh_file);
#endif

    /* Don't need this anymore.
     */
    free_histogram(histogram, aImage->x);




    ellipse_list = find_other_three_ellipse_parameters(centre_point_list, aImage, theWindow, theGC);
    /* Don't need this anymore.
     */
    free_linked_list_of_point(centre_point_list);



    /* Record each ellipse to be displayed.
     * It will be drawn when the screen re-draws.
     */

    color_index = 0;
    for (ellipse=ellipse_list; ellipse != NULL; ellipse=ellipse->next)
        {
        num_ellipses++;
        /* Record the circle to be displayed.
         */
        d = alloc_display_object();
        d->type = hough_ellipse;
        d->obj.hough_ellipse.x = ellipse->x;
        d->obj.hough_ellipse.y = ellipse->y;
        d->obj.hough_ellipse.a = ellipse->a;
        d->obj.hough_ellipse.b = ellipse->b;
        d->obj.hough_ellipse.theta = ellipse->theta;
        d->obj.hough_ellipse.width = 15;
        d->obj.hough_ellipse.color_index = color_index;
        gDisplayObjectList = put_in_linked_list_of_display_object(gDisplayObjectList, d);
        color_index++;
        if (color_index == NUM_RANDOM_COLORS)
            color_index = 0;
        }


    XUnmapWindow(gDisplay, message_window->win);

    free_message_window(message_window);


    free_linked_list_of_ellipse(ellipse_list);

#ifdef COMPUTATION_TIME
    check_parity();
#endif


    draw_image(aXImage, theWindow, theGC);
    if (num_ellipses == 1)
        sprintf(results_message, "Found 1 ellipse with SHT.");
    else
        sprintf(results_message, "Found %d ellipses with SHT.", num_ellipses);
    display_message_and_wait(theWindow, 0, 0, 4,
                            "",
                            results_message,
                            "Hit Left Mouse Button to continue",
                            "");


}   /* end of find_ellipses_with_SHT() */



HOUGH_POINT *
alloc_hough_point(void)
{
    HOUGH_POINT *hp;


    hp = (HOUGH_POINT *)malloc(sizeof(HOUGH_POINT));
    if (hp == NULL)
        {
        fprintf(stderr, "malloc failed in alloc_hough_point() in sht_ellipse.c\n");
        exit(1);
        }

    hp->next = NULL;

    return(hp);

}       /* end of alloc_hough_point() */



void
free_hough_point(HOUGH_POINT *hp)
{
    /* Admittedly, this function is a bit of a waste of time,
     * but I put it in since I have a free function for
     * my other data structures, e.g.
     * free_gaus(), free_histogram(), free_point(), free_basis(), free_image()
     */

    free(hp);

}   /* end of free_hough_point() */



HOUGH_POINT_LIST *
alloc_hough_point_list(void)
{
    HOUGH_POINT_LIST  *hough_point_list;


    hough_point_list = (HOUGH_POINT_LIST *)malloc( sizeof(HOUGH_POINT_LIST) );

    if (hough_point_list == NULL)
        {
        fprintf(stderr, "malloc failed in alloc_hough_point_list() in sht_ellipse.c\n");
        exit(1);
        }

    hough_point_list->head = NULL;
    hough_point_list->tail = NULL;
    hough_point_list->num_elements = 0;

    return(hough_point_list);

}   /* end of alloc_hough_point_list() */



HOUGH_POINT_LIST *
put_in_linked_list_of_hough_point(HOUGH_POINT_LIST *hough_point_list,
                                                            HOUGH_POINT *hp)
{
    HOUGH_POINT_LIST  *alloc_hough_point_list(void);


    if (hough_point_list == NULL)
        hough_point_list = alloc_hough_point_list();

    if (hough_point_list->head == NULL)
        {
        hough_point_list->head = hp;
        hough_point_list->tail = hp;
        hough_point_list->num_elements = 1;
        }
    else
        {
        /* Add hp to the list.
         */
        hough_point_list->tail->next = hp;

        /* Update the tail. */
        hough_point_list->tail = hp;

        /* Update the number of elements. */
        (hough_point_list->num_elements)++;

        }

    hp->next = NULL;

    return(hough_point_list);


}   /* end of put_in_linked_list_of_hough_point() */



void
free_linked_list_of_hough_point(HOUGH_POINT_LIST *hough_point_list)
{
    HOUGH_POINT   *hp1, *hp2;
    void    free_hough_point(HOUGH_POINT *p);

    if  (hough_point_list != NULL)
        {
        hp1 = hough_point_list->head;
        while (hp1 != NULL)
            {
            hp2 = hp1;
            hp1 = hp1->next;
            free_hough_point(hp2);
            }

        free(hough_point_list);
        }


}   /* end of free_linked_list_of_hough_point() */



ELLIPSE *
alloc_ellipse(void)
{
    ELLIPSE *e;


    e = (ELLIPSE *)malloc(sizeof(ELLIPSE));
    if (e == NULL)
        {
        fprintf(stderr, "malloc failed in alloc_ellipse() in sht_ellipse.c\n");
        exit(1);
        }

    e->next = NULL;

    return(e);

}       /* end of alloc_ellipse() */



void
free_ellipse(ELLIPSE *e)
{
    /* Admittedly, this function is a bit of a waste of time,
     * but I put it in since I have a free function for
     * my other data structures, e.g.
     * free_gaus(), free_histogram(), free_point(), free_basis(), free_image()
     */

    free(e);

}   /* end of free_ellipse() */



ELLIPSE *
put_in_linked_list_of_ellipse(ELLIPSE *head, ELLIPSE *e)
{
    ELLIPSE *ptr;

    if (head == NULL)
        head = e;
    else
        {
        for (ptr=head; ptr->next != NULL; ptr=ptr->next)
            ;
        ptr->next = e;
        }

    e->next = NULL;


    return(head);

}   /* end of put_in_linked_list_of_ellipse() */



void
free_linked_list_of_ellipse(ELLIPSE *ellipse_head)
{
    ELLIPSE     *e1;
    void        free_ellipse(ELLIPSE *e);

    while (ellipse_head != NULL)
        {
        e1 = ellipse_head;
        ellipse_head = ellipse_head->next;
        free_ellipse(e1);
        }

}   /* end of free_linked_list_of_ellipse() */



void
ellipse_centre_SHT_hough_transform(IMAGE *aImage, int **histogram)
{
    HOUGH_POINT *hp, *hp1, *hp2;
    HOUGH_POINT_LIST    *hough_point_list;
    int         x, y, x_h, y_h;
    double      theta1, theta2;
    double      slope1, slope2;
    double      x1, y1, x2, y2, t1, t2, m1, m2;
    int         x_end, y_end;
    double      slope, b;
    int         get_pixel_value(IMAGE *aImage, int x, int y);
    HOUGH_POINT *alloc_hough_point(void);
    HOUGH_POINT_LIST    *put_in_linked_list_of_hough_point(
                                            HOUGH_POINT_LIST *hough_point_list,
                                            HOUGH_POINT *hp);
    void        free_linked_list_of_hough_point(
                                            HOUGH_POINT_LIST *hough_point_list);
    double  find_tangent_of_edge(IMAGE *aImage, int x, int y);




    hough_point_list = (HOUGH_POINT_LIST *)NULL;

    for (x=0; x < aImage->x; x++)
        for (y=0; y < aImage->y; y++)
            if (get_pixel_value(aImage,x,y) == BLACK)
                {
                /* Calculate angle of edge at this point.
                 * theta1 is in the range [-M_PI_2, M_PI_2].
                 */
                theta1 = find_tangent_of_edge(aImage, x, y);
                if (theta1 == NO_ANGLE)
                    {
                    /* Could not estimate angle of edge at the
                     * point (x,y)
                     */
                    continue;
                    }
                if (ABS(theta1) > 0.99*M_PI_2)
                    {
                    /* If tangent is almost vertical, then contiue.
                     * Soon, we will need to do equations with
                     * the slope (dy/dx) which will be near infinite
                     * for near vertical tangents.
                     */
                    continue;
                    }
                hp = alloc_hough_point();
                hp->x = x;
                hp->y = y;
                hp->theta = theta1;
                hp->slope = tan(theta1);

                hough_point_list = put_in_linked_list_of_hough_point(hough_point_list, hp);
                }   /* end of 'if (get_pixel_value(aImage,x,y) == BLACK)' */



    /* No BLACK pixels in image. */
    if (hough_point_list == (HOUGH_POINT_LIST *)NULL)
        return;


    /* For every black pixel in the image
     */
    for (hp1=hough_point_list->head; hp1 != NULL; hp1=hp1->next)
        {
        x1 = (double )hp1->x;  y1 = (double )hp1->y;
        theta1 = hp1->theta;
        slope1 = hp1->slope;
        /* For every other black pixel in the image
         */
        for (hp2=hough_point_list->head; hp2 != NULL; hp2=hp2->next)
            {
            theta2 = hp2->theta;
            slope2 = hp2->slope;
            /* Make sure that theta2 < theta1
             */
            if (theta2 > theta1)
                theta2 -= M_PI;

            /* Don't bother finding meeting point
             * of tangents if the normals are almost
             * parallel or almost perpendicular.
             */
            if ( (((theta1-theta2) > (M_PI/10.0)) &&
                  ((theta1-theta2) < (4.0*M_PI/10.0)) ) ||
                        (((theta1-theta2) > (6.0*M_PI/10.0)) &&
                         ((theta1-theta2) < (9.0*M_PI/10.0))) )
                {
                /* (t1, t2) is the intersection point
                 * of the two tangents.
                 * (m1, m2) is the midpoint
                 * of (x,y) and (i,j)
                 */
                x2 = (double )hp2->x;  y2 = (double )hp2->y;

                t1 = (y1 - y2 - x1*slope1 + x2*slope2) / (slope2-slope1);
                t2 = (slope1*slope2*(x2-x1) - y2*slope1 + y1*slope2) / (slope2-slope1);
                m1 = (x1 + x2) / 2;
                m2 = (y1 + y2) / 2;
                /* We can assume that (m1, m2) is in the
                 * image as it is the midpoint of 2 points
                 * in the image: (x1,y1) and (x2,y2)
                 */

                /* Draw a line on the hough histogram.
                 * This line will pass through the
                 * ellipse centre.
                 * 
                 * If x co-ord varies more quickly
                 * than y co-ord (i.e. slope < 1),
                 * then index the line by the x co-ord.
                 */
                if (ABS(t2-m2) < ABS(t1-m1))
                    {
                    slope = (t2-m2) / (t1-m1);
                    b = (m2*t1 - m1*t2) / (t1-m1);

                    /* If (m1,m2) is to the left of (t1,t2)
                     */
                    if (m1 < t1)
                        {
                        if (slope == 0)
                            {
                            x_end = 0;
                            }
                        else if (slope > 0)
                            {
                            /* zero intercept
                             * i.e. y = 0
                             */
                            x_end = (int )(-b/slope);
                            }
                        else /* if slope < 0 */
                            {
                            /* y = edge of image
                             * i.e. y = aImage->y
                             */
                            x_end = (int )( (((double )aImage->y)-b) / slope );
                            }


                        /* Increase x_end by one.
                         * Otherwise, because of rounding errors,
                         * we may get a value of y_h=aImage->y
                         * which would give a go outside the bounds
                         * of the array histogram[][].
                         */
                        x_end++;


                        if (x_end < 0)
                            x_end = 0;


                        for (x_h=(int )m1; x_h > x_end; x_h--)
                            {
                            y_h = (int )(slope*(double )x_h + b);
                            histogram[x_h][y_h]++;
                            }
                        }
                    else    /* if m1 > t1 */
                        {
                    /* If (m1,m2) is to the right of (t1,t2)
                     */
                        if (slope == 0)
                            {
                            x_end = aImage->x;
                            }
                        else if (slope > 0)
                            {
                            /* y = edge of image
                             * i.e. y = aImage->y
                             */
                            x_end = (int )( (((double )aImage->y)-b) / slope );
                            }
                        else /* if slope < 0 */
                            {
                            /* zero intercept
                             * i.e. y = 0
                             */
                            x_end = (int )(-b/slope);
                            }


                        /* Decrease x_end by one.
                         * Otherwise, because of rounding errors,
                         * we may get a value of y_h=aImage->y
                         * which would give a go outside the bounds
                         * of the array histogram[][].
                         */
                        x_end--;


                        if (x_end > aImage->x)
                            x_end = aImage->x;

                        for (x_h=(int )m1; x_h < x_end; x_h++)
                            {
                            y_h = (int )(slope*(double )x_h + b);
                            histogram[x_h][y_h]++;
                            }
                        }
                    }
                else    /* ABS(t1-m1) <= ABS(t2-m2) */
                    {
                    /*
                     * y co-ord varies more quickly
                     * than x co-ord (i.e. slope > 1),
                     * so index the line by the y co-ord.
                     */
                    slope = (t1-m1) / (t2-m2);
                    b = (m1*t2 - m2*t1) / (t2-m2);

                    /* If (m1,m2) is below (t1,t2)
                     */
                    if (m2 < t2)
                        {
                        if (slope == 0)
                            {
                            y_end = 0;
                            }
                        else if (slope > 0)
                            {
                            /* zero intercept
                             * i.e. x = 0
                             */
                            y_end = (int )(-b/slope);
                            }
                        else /* if slope < 0 */
                            {
                            /* x = edge of image
                             * i.e. x = aImage->x
                             */
                            y_end = (int )( (((double )aImage->x)-b) / slope );
                            }


                        /* Increase y_end by one.
                         * Otherwise, because of rounding errors,
                         * we may get a value of x_h=aImage->x
                         * which would give a go outside the bounds
                         * of the array histogram[][].
                         */
                        y_end++;


                        if (y_end < 0)
                            y_end = 0;

                        for (y_h=(int )m2; y_h > y_end; y_h--)
                            {
                            x_h = (int )(slope*(double )y_h + b);
                            histogram[x_h][y_h]++;
                            }
                        }
                    else    /* if m2 > t2 */
                        {
                    /* If (m1,m2) is above (t1,t2)
                     */
                        if (slope == 0)
                            {
                            y_end = aImage->y;
                            }
                        else if (slope > 0)
                            {
                            /* x = edge of image
                             * i.e. x = aImage->x
                             */
                            y_end = (int )( (((double )aImage->x)-b) / slope );
                            }
                        else /* if slope < 0 */
                            {
                            /* zero intercept
                             * i.e. x = 0
                             */
                            y_end = (int )(-b/slope);
                            }


                        /* Decrease y_end by one.
                         * Otherwise, because of rounding errors,
                         * we may get a value of x_h=aImage->x
                         * which would give a go outside the bounds
                         * of the array histogram[][].
                         */
                        y_end--;


                        if (y_end > aImage->y)
                            y_end = aImage->y;

                        for (y_h=(int )m2; y_h < y_end; y_h++)
                            {
                            x_h = (int )(slope*(double )y_h + b);
                            histogram[x_h][y_h]++;
                            }
                        }
                    }

                }   /* end of if ((theta1-theta2 > M_PI/10.0)... */
            }   /* end of for loop based on hp2 */
#ifdef COMPUTATION_TIME
        check_parity();
#endif
        }   /* end of for loop based on hp1 */


    free_linked_list_of_hough_point(hough_point_list);



}   /* end of ellipse_centre_SHT_hough_transform() */



ELLIPSE *
find_other_three_ellipse_parameters(POINT_LIST *centre_point_list, IMAGE *aImage, Window theWindow, GC theGC)
{
    double      a_start, a_end, b_start, b_end, theta_start, theta_end;
    double      da, db, dtheta;
    int         a_index, b_index, theta_index;
    double      a_max, b_max, theta_max;
    int         i;
    ELLIPSE     *ellipse, *ellipse_list;
    int         ***histogram;
    POINT       *centre_point;
    double      proportion;
    int ***     alloc_3D_histogram(int x_max, int y_max, int z_max);
    void        other_three_parameters_SHT_hough_transform(int ***histogram, POINT *centre_point, double a_start, double a_end, double b_start, double b_end, double theta_start, double theta_end, IMAGE *aImage);
    void        find_max_of_3D_histogram(int ***histogram, int *x_val, int *y_val, int *z_val, int x_max, int y_max, int z_max);
    void        free_3D_histogram(int ***histogram, int x_max, int y_max);
    double      find_proportion_of_ellipse_that_exists(double x_e, double y_e, double a, double b, double theta, IMAGE *aImage);
    ELLIPSE     *alloc_ellipse(void);
    ELLIPSE     *put_in_linked_list_of_ellipse(ELLIPSE *head, ELLIPSE *e);
    void        draw_hough_ellipse( Window theWindow, GC theGC, double x, double y, double r1, double r2, double theta );





    if ((centre_point_list == (POINT_LIST *)NULL) ||
                                    (centre_point_list->head == (POINT *)NULL))
        return( (ELLIPSE *)NULL );


    ellipse_list = NULL;

    histogram = alloc_3D_histogram(20, 20, 20);

    for (centre_point=centre_point_list->head; centre_point != NULL;
                                                centre_point=centre_point->next)
        {
        a_start = b_start = 0.0;
        /* a_end, b_end define the size of the
         * largest ellipse that we can find.
         *
         * We have set this largest ellipse to have a
         * major radius 1/2 as large as the diagonal of the image.
         */
        a_end = b_end = sqrt(SQR(aImage->x) + SQR(aImage->y)) / 2.0;
        theta_start = 0.0;  theta_end = M_PI;

        for (i=0; i < 5; i++)
            {
            other_three_parameters_SHT_hough_transform(histogram, centre_point, a_start, a_end, b_start, b_end, theta_start, theta_end, aImage);
            find_max_of_3D_histogram(histogram, &a_index, &b_index, &theta_index, 20, 20, 20);


            da = ((double )(a_end-a_start)) / 20;
            db = ((double )(b_end-b_start)) / 20;
            dtheta = ((double )(theta_end-theta_start)) / 20;

            a_max = a_start + da*(double )a_index;
            b_max = b_start + db*(double )b_index;
            theta_max = theta_start + dtheta*(double )theta_index;

            a_start = a_max - da;
            if (a_start < 0)
                a_start = 0;
            a_end = a_max + da;
            b_start = b_max - db;
            if (b_start < 0)
                b_start = 0;
            b_end = b_max + db;
            theta_start = theta_max - dtheta;
            theta_end = theta_max + dtheta;
#ifdef COMPUTATION_TIME
            check_parity();
#endif
            }

        /* Having found the parameters for an ellipse,
         * we need to check how much of the ellipse exists.
         * If only a small bit of it exists, then the ellipse
         * probably does not exist.
         * We are not talking about occluded ellipses here.
         * We are trying to detect when the hough transform
         * has taken a few black pixels from around the image
         * and interpretted them as bits of an ellipse.
         */
        proportion = find_proportion_of_ellipse_that_exists(centre_point->x[1], centre_point->x[2], a_max, b_max, theta_max, aImage);
        /* If over 0.65 of the ellipse is found to exist
         * then record it.
         */
        if (proportion > 0.65)
            {

            ellipse = alloc_ellipse();
            ellipse->x = centre_point->x[1];
            ellipse->y = centre_point->x[2];
            ellipse->a = a_max;
            ellipse->b = b_max;
            ellipse->theta = theta_max;
            ellipse_list = put_in_linked_list_of_ellipse(ellipse_list, ellipse);
            /* Set foreground color to blue.
             */
            XSetForeground(gDisplay, theGC, gRandomColors[2].pixel);
            draw_hough_ellipse( theWindow, theGC, ellipse->x, ellipse->y, ellipse->a, ellipse->b, ellipse->theta );
            }

#ifdef COMPUTATION_TIME
        check_parity();
#endif
        }


    free_3D_histogram(histogram, 20, 20);



    return(ellipse_list);

}   /* end of find_other_three_ellipse_parameters */



void
other_three_parameters_SHT_hough_transform(int ***histogram, POINT *centre_point, double a_start, double a_end, double b_start, double b_end, double theta_start, double theta_end, IMAGE *aImage)
{
    int     x, y;
    double  x1, y1, x2, y2;
    double  a, b, theta, da, db, dtheta;
    int     a_index, b_index, theta_index;
    int     get_pixel_value(IMAGE *aImage, int x, int y);



    /* Initialise all histogram values
     * to zero.
     */
    for (a_index=0; a_index < 20; a_index++)
        for (b_index=0; b_index < 20; b_index++)
            for (theta_index=0; theta_index < 20; theta_index++)
                histogram[a_index][b_index][theta_index] = 0;


    da = ((double )(a_end-a_start)) / 20;
    db = ((double )(b_end-b_start)) / 20;
    dtheta = ((double )(theta_end-theta_start)) / 20;

    for (x=0; x < aImage->x; x++)
        for (y=0; y < aImage->y; y++)
            if (get_pixel_value(aImage,x,y) == BLACK)
                {
                /* Translate (x,y) relative to centre of ellipse.
                 */
                x1= ((double )x)-centre_point->x[1];
                y1 = ((double )y)-centre_point->x[2];

                for (a_index=0, a=a_start; a_index < 20; a_index++, a+=da)
                    for (theta_index=0, theta=theta_start; theta_index < 20; theta_index++, theta+=dtheta)
                        {
                        /* Given a and theta, calculate b
                         */

                        /* Rotate (x1,y1) relative to theta.
                         */
                        x2 = x1*cos(theta) + y1*sin(theta);
                        y2 = -x1*sin(theta) + y1*cos(theta);

                        if (SQR(x2/a) < 1.0)    /* avoid divide by zero */
                            {
                            b = sqrt( SQR(y2) / (1.0 - SQR(x2/a)) );
                            b_index = (int )((b-b_start) / db);
                            if ((b_index >= 0) && (b_index < 20))
                                histogram[a_index][b_index][theta_index]++;
                            }
                        }
                }


}   /* end of other_three_parameters_SHT_hough_transform() */



void
find_max_of_3D_histogram(int ***histogram, int *x_val, int *y_val, int *z_val, int x_max, int y_max, int z_max)
{
    int     x, y, z;
    int     max_val;


    *x_val = 0;  *y_val = 0;  *z_val = 0;
    max_val = 0;

    for (x=0; x < x_max; x++)
        for (y=0; y < y_max; y++)
            for (z=0; z < z_max; z++)
                if (histogram[x][y][z] > max_val)
                    {
                    max_val = histogram[x][y][z];
                    *x_val = x;
                    *y_val = y;
                    *z_val = z;
                    }


}   /* end of find_max_of_3D_histogram() */



int ***
alloc_3D_histogram(int x_max, int y_max, int z_max)
{
    int x, y;
    int ***histogram;

    /* Allocate space for a 3D histogram
     * histogram is an array of pointers,
     * each of which points to an array of pointers,
     * each of which points to an array of data.
     * So, histogram is a (int ***)
     *     histogram[] is a (int **)
     *     histogram[][] is a (int *)
     *     histogram[][][] is a (int )
     */

    histogram = (int ***)malloc((int )x_max * sizeof(int **));
    if (histogram == NULL)
        {
        fprintf(stderr, "first malloc failed in alloc_3D_histogram() in sht_ellipse.c\n");
        exit(1);
        }

    for (x=0; x < x_max; x++)
        {
        histogram[x] = (int **)malloc((int )y_max * sizeof(int *));
        if (histogram[x] == NULL)
            {
            fprintf(stderr, "second malloc failed in alloc_3D_histogram() in sht_ellipse.c\n");
            exit(1);
            }
        for (y=0; y < y_max; y++)
            {
            histogram[x][y] = (int *)malloc((int )z_max * sizeof(int ));
            if (histogram[x][y] == NULL)
                {
                fprintf(stderr, "third malloc failed in alloc_3D_histogram() in sht_ellipse.c\n");
                exit(1);
                }
            }
        }


    return(histogram);

}   /* end of alloc_3D_histogram() */



void
free_3D_histogram(int ***histogram, int x_max, int y_max)
{
    int x, y;

    for (x=0; x < x_max; x++)
        {
        for (y=0; y < y_max; y++)
            free(histogram[x][y]);
        free(histogram[x]);
        }

    free(histogram);

}   /* end of free_3D_histogram() */



double
find_proportion_of_ellipse_that_exists(double x_e, double y_e, double a, double b, double theta, IMAGE *aImage)
{
    int     x, y;
    double  x1, y1, x2, y2;
    double  count = 0.0;
    double  perimeter;
    int     get_pixel_value(IMAGE *aImage, int x, int y);


    for (x=0; x < aImage->x; x++)
        for (y=0; y < aImage->y; y++)
            if (get_pixel_value(aImage,x,y) == BLACK)
                {
                x1 = (double )x - x_e;  /* translate pixel relative */
                y1 = (double )y - y_e;  /* to ellipse centre.       */

                /* Rotate (x1,y1) relative to theta.
                 */
                x2 = x1*cos(theta) + y1*sin(theta);
                y2 = -x1*sin(theta) + y1*cos(theta);

                /* Scale it relative to a, b
                 */
                x2 /= a;
                y2 /= b;

                /* If (x,y) lies on or near the ellipse,
                 * then (x2,y2) will lie on or near
                 * the unit circle.
                 */
                if (ABS(1.0 - (SQR(x2) + SQR(y2))) < 0.45)
                    {
                    count++;
                    }
                }


    /* Note: we multiply the ellipse perimeter by the current
     * line thickness.
     */

    perimeter = M_PI * ( 3*(a+b) - sqrt((a+3*b)*(3*a+b)) ) * gLineThickness;


    return(count/perimeter);

}   /* end of find_proportion_of ellipse_that_exists() */



#ifdef COMPUTATION_TIME
void
check_parity(void)
{
    clock_t     current_time;

    current_time = clock();

    if ((current_time > 0) && (gParity == -1))
        {
        gParity = 1;
        gOverflowAdd++;
        }
    else if ((current_time < 0) && (gParity == 1))
        {
        gParity = -1;
        }
}
#endif
