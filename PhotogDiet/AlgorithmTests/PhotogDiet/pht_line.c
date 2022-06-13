/*
 * Programmed by:   Robert Ainsley McLaughlin
 * E-mail:          ram@ee.uwa.edu.au
 * Date:            18 August, 1996
 * Last modified:   4 March, 1998
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
#include <unistd.h>     /* sleep() */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "../include/upwrite.h"



/* Function Prototypes
 */
void    find_lines_with_PHT(IMAGE *aImage, XImage *aXImage,
                            Window theWindow, GC theGC);
void    PHT_hough_transform(double sample_proportion, IMAGE *aImage,
                            int theta_res, int rho_res, int **histogram);
void    remove_pixels_along_line_PHT(double l_theta, double l_rho,
                                IMAGE *aImage, int theta_res, int rho_res,
                                int **histogram, int color_index,
                                Window theWindow, GC theGC);
BOOLEAN find_line_ends_PHT(double theta, double rho, IMAGE *aImage,
                            int *x_start, int *y_start,
                            int *x_end, int *y_end);
double  find_density_of_black_pixels_PHT(int v1, int width,
                            double m, double b, char flag, IMAGE *aImage);



void
find_lines_with_PHT(IMAGE *aImage, XImage *aXImage, Window theWindow, GC theGC)
{
/* PARAMETERS */
    double  sample_proportion = 0.1;

    int     num_lines;
    int     t, r;
    int     theta_res, rho_res;
    double  max_dimension;
    double  theta_res_d, rho_res_d; /* (double ) form of theta_res, rho_res */
    int     t_max, r_max, h_max;
    double  theta, rho, max_rho;
    int     sum, average, count, background;
    int     **histogram;
    int     color_index;
    char    results_message[100];
    BOOLEAN error;

    MESSAGE_WINDOW  *message_window;
    XEvent  report;
    int             theScreen;
    unsigned long   theBlackPixel;
    MESSAGE_WINDOW  *alloc_message_window(void);
    void    init_message_window_fields(MESSAGE_WINDOW *message_window, Window MainWindow, int x, int y, int num_strings, ...);
    void    write_message_window_strings(MESSAGE_WINDOW *message_window);
    void    free_message_window(MESSAGE_WINDOW *message_window);
    void    draw_image(XImage *aXImage, Window theWindow, GC theGC);

    void    untag_image(IMAGE *aImage);
    int     **alloc_histogram(int x_max, int y_max);
    void    free_histogram(int **histogram, int x_max);
    void    SHT_hough_transform(IMAGE *aImage, int theta_res, int rho_res, int **histogram);
    void    remove_pixels_along_line_PHT(double l_theta, double l_rho,
                                IMAGE *aImage, int theta_res, int rho_res,
                                int **histogram, int color_index,
                                Window theWindow, GC theGC);
    void    display_message_and_wait(Window theWindow, int x, int y, int num_strings, ...);
#ifdef XGOBI_DEBUG
    FILE    *dat_file, *lines_file, *colors_file, *linecolors_file;
    int     line_number;
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






#ifdef XGOBI_DEBUG
    line_number = 0;
    dat_file = fopen("h.dat", "w");
    lines_file = fopen("h.lines", "w");
    colors_file = fopen("h.colors", "w");
    linecolors_file = fopen("h.linecolors", "w");
    if ((dat_file == NULL) || (lines_file == NULL) || (colors_file == NULL) || (linecolors_file == NULL))
        {
        fprintf(stderr, "Could not open file in find_lines_with_SHT_hough() in sht_line.c\n");
        exit(1);
        }
#endif


    error = FALSE;
    /* We will tag those pixels which are
     * used in the Hough Transform.
     */
    untag_image(aImage);

    max_dimension = (aImage->x > aImage->y) ? (double )aImage->x : (double )aImage->y;
    theta_res = (int )(0.79 * max_dimension);
    rho_res = (int )(0.79 * max_dimension);

    theta_res_d = (double )theta_res;
    rho_res_d = (double )rho_res;


    histogram = alloc_histogram(theta_res, rho_res);

    PHT_hough_transform(sample_proportion, aImage,
                                        theta_res, rho_res, histogram);

#ifdef HUGH_DEBUG
    dat_hugh_file = fopen("g.dat", "w");
    if (dat_hugh_file == NULL)
        {
        fprintf(stderr, "Could not open file in find_circles_with_SHT() in sht_circle.c\n");
        exit(1);
        }
#endif
#ifdef HUGH_DEBUG
    for (x_hugh=0; x_hugh < aImage->x; x_hugh++)
        for (y_hugh=0; y_hugh < aImage->y; y_hugh++)
            if (histogram[x_hugh][y_hugh] > 1000)
                {
                fprintf(dat_hugh_file, "%d %d %d\n", x_hugh, y_hugh, histogram[x_hugh][y_hugh]);
                }
#endif

#ifdef HUGH_DEBUG
    fclose(dat_hugh_file);
#endif

    /* Find average value of histogram (only consider
     * non-zero entries in histogram).
     * Use this as a model of the background noise value
     * of the histogram.
     * When the largest histogram value is less than this
     * average, then we will have removed all maxima from
     * histogram.
     */
    sum = 0;
    count = 0;
    for (t=0; t < theta_res; t++)
        for (r=0; r < rho_res; r++)
            if (histogram[t][r] > 0)
                {
                sum += histogram[t][r];
                count++;
                }

    if (count != 0)
        average = sum / count;
    else
        average = 0;

    background = ((average > 6) ? (5*average) : 30);



    /* Find max of histogram
     */
    h_max = 0;
    t_max = 0;
    r_max = 0;
    for (t=0; t < theta_res; t++)
        for (r=0; r < rho_res; r++)
            if (histogram[t][r] > h_max)
                {
                h_max = histogram[t][r];
                t_max = t;
                r_max = r;
                }


    num_lines = 0;
    color_index = 0;

    max_rho = sqrt(SQR(aImage->x) + SQR(aImage->y));
                                                /* Maximum possible value
                                                 * of rho.
                                                 */
    while (h_max > background)
        {

#ifdef XGOBI_DEBUG
        line_number = 0;
        for (t=0; t < theta_res; t++)
            for (r=0; r < rho_res; r++)
                if (histogram[t][r] > (background/2))
                    {
                    fprintf(dat_file, "%d %d 0\n", t, r);
                    fprintf(colors_file, "green\n");
                    fprintf(dat_file, "%d %d %d\n", t, r, histogram[t][r]);
                    fprintf(colors_file, "green\n");
                    fprintf(lines_file, "%d %d\n", line_number+1, line_number+2);
                    fprintf(linecolors_file, "green\n");
                    line_number+=2;
                    }
#endif


        theta = M_PI * ( (((double )t_max) - theta_res_d/2.0) / theta_res_d );
        rho = 2.0*max_rho * ( (((double )r_max) - rho_res_d/2.0) / rho_res_d );
        remove_pixels_along_line_PHT(theta, rho, aImage, theta_res, rho_res, histogram, color_index, theWindow, theGC);


        color_index++;
        if (color_index == NUM_RANDOM_COLORS)
            color_index = 0;
        num_lines++;


        if (num_lines == MAX_NUM_SHT_AND_PHT_LINES)
            {
            /* This means that we were unable to accurately
             * remove the pixels from the last line recognised,
             * hence we keep recognising it again and again and again ...
             * To correct this, you could increase
             * 'theta_res' and  'rho_res'.
             */
            error = TRUE;
            break;
            }


        /* Recalculate average value of histogram (only consider
         * non-zero entries in histogram).
         */
        sum = 0;
        count = 0;
        for (t=0; t < theta_res; t++)
            for (r=0; r < rho_res; r++)
                if (histogram[t][r] > 0)
                    {
                    sum += histogram[t][r];
                    count++;
                    }

        if (count != 0)
            average = sum / count;
        else
            average = 0;

        background = ((average > 6) ? (5*average) : 30);


        /* Find next max of histogram
         */
        h_max = 0;
        t_max = 0;
        r_max = 0;
        for (t=0; t < theta_res; t++)
            for (r=0; r < rho_res; r++)
                if (histogram[t][r] > h_max)
                    {
                    h_max = histogram[t][r];
                    t_max = t;
                    r_max = r;
                    }

        }   /* end of 'while (h_max > background)' */



#ifdef XGOBI_DEBUG
        for (t=0; t < theta_res; t++)
            for (r=0; r < rho_res; r++)
                if (histogram[t][r] > (background/2))
                    {
                    fprintf(dat_file, "%d %d 0\n", t+2*line_num*theta_res, r);
                    fprintf(colors_file, "red\n");
                    fprintf(dat_file, "%d %d %d\n", t+2*line_num*theta_res, r, histogram[t][r]);
                    fprintf(colors_file, "red\n");
                    fprintf(lines_file, "%d %d\n", line_number+1, line_number+2);
                    fprintf(linecolors_file, "red\n");
                    line_number+=2;
                    }
#endif




    free_histogram(histogram, theta_res);

    XUnmapWindow(gDisplay, message_window->win);

    free_message_window(message_window);

#ifdef XGOBI_DEBUG
    fclose(dat_file);
    fclose(lines_file);
    fclose(colors_file);
    fclose(linecolors_file);
#endif

    draw_image(aXImage, theWindow, theGC);
    if (error == TRUE)
        {
        display_message_and_wait(theWindow, 0, 0, 10,
                            "",
                            "Probabilistic HT algorithm for line detection",
                            "iteratively detects the largest line, removes",
                            "it, then proceeds to find the next largest line.",
                            "The algorithm was unable to successfully remove",
                            "a line that it detected, so the program could",
                            "not estimate how many lines are in the image.",
                            "",
                            "Hit Left Mouse Button to continue",
                            "");
        }
    else
        {
        if (num_lines == 1)
            sprintf(results_message, "Found 1 line with PHT.");
        else
            sprintf(results_message, "Found %d lines with PHT.", num_lines);
        display_message_and_wait(theWindow, 0, 0, 4,
                                "",
                                results_message,
                                "Hit Left Mouse Button to continue",
                                "");
        }


}   /* end of find_lines_with_PHT() */



/* m points to the image to be used.
 * rho_res and theta_res is the resolution used over
 * the rho and theta parameters (respectively).
 * e.g. rho_res = 100
 *      theta_res = 100
 * then we produce a 100 x 100 histogram which covers the
 * relevent area of rho-theta space.
 * histogram is stored as an array of pointers to arrays.
 */
void
PHT_hough_transform(double sample_proportion, IMAGE *aImage,
                            int theta_res, int rho_res, int **histogram)
{
    int x, y;
    double  theta, rho, max_rho;
    int t, r;
    double  theta_res_d, rho_res_d; /* (double ) form of theta_res, rho_res */
    int     get_pixel_value(IMAGE *aImage, int x, int y);
    void    tag_pixel(IMAGE *aImage, int x, int y, int tag);


    theta_res_d = (double )theta_res;
    rho_res_d = (double )rho_res;
    max_rho = sqrt(SQR(aImage->x) + SQR(aImage->y));
                                                /* Maximum possible value
                                                 * of rho.
                                                 */



    /* Find Hough transform of points.
     */
    for (x=0; x < aImage->x; x++)
        for (y=0; y < aImage->y; y++)
            {
            if (get_pixel_value(aImage, x, y) == BLACK)
                {
                /* Don't compute hough transform of all black pixels.
                 * Only do so with a sample of them.
                 */
                if (drand48() > sample_proportion)
                    {
                    /* Don't compute HT of this pixel */
                    continue;
                    }

                /* Tag pixel as having been used in histogram.
                 */
                tag_pixel(aImage, x, y, 1);
                for (t=0; t < theta_res; t++)
                    {
                    theta = M_PI * ( (((double )t) - theta_res_d/2.0) / theta_res_d );
                    rho = cos(theta)*(double )x + sin(theta)*(double )y;

                    r = (int )(rho_res_d * rho/(2.0*max_rho) + rho_res_d/2.0);
                    histogram[t][r]++;
                    }
                }
            }



}   /* end of PHT_hough_transform() */



/* Identify the pixels along a given line.
 * Remove them from the image.
 * Remove the contribution of these pixels
 * to the histogram.
 */
void
remove_pixels_along_line_PHT( double l_theta, double l_rho,
                                IMAGE *aImage, int theta_res, int rho_res,
                                int **histogram, int color_index,
                                Window theWindow, GC theGC)
{
    int     x, y;
    double  m, b;
    double  theta_res_d, rho_res_d; /* (double ) form of theta_res, rho_res */
    int     width = 15;
    double  theta, rho, max_rho;
    int     t, r, i, j;
    int     x_start, y_start, x_end, y_end;
    DISPLAY_OBJECT  *d;
    BOOLEAN find_line_ends_PHT(double theta, double rho, IMAGE *aImage, int *x_start, int *y_start, int *x_end, int *y_end);
    int     get_pixel_value(IMAGE *aImage, int x, int y);
    BOOLEAN test_pixel_for_tag(IMAGE *aImage, int x, int y, int tag);
    void    tag_pixel(IMAGE *aImage, int x, int y, int tag);
    DISPLAY_OBJECT      *alloc_display_object(void);
    DISPLAY_OBJECT_LIST *put_in_linked_list_of_display_object(DISPLAY_OBJECT_LIST *display_oject_list, DISPLAY_OBJECT *d);
    void    display_hough_line_segment(int x_start, int y_start, int x_end, int y_end, int width, int line_num, Window theWindow, GC theGC);


    theta_res_d = (double )theta_res;
    rho_res_d = (double )rho_res;
    max_rho = sqrt(SQR(aImage->x) + SQR(aImage->y));
                                                /* Maximum possible value
                                                 * of rho.
                                                 */

    (void )find_line_ends_PHT(l_theta, l_rho, aImage,
                                &x_start, &y_start, &x_end, &y_end);


    m = -cos(l_theta)/sin(l_theta);

    if (ABS(m) <= 1.0)
        {
        b = l_rho/sin(l_theta);
        /* The line is in the form:
         *     y = mx + b
         * and |m| <= 1.0
         */
        for (x=x_start; x < x_end; x++)
            {
            y = (int )( m*(double )x + b );
            for (j=y-width/2; j <= y+width/2; j++)
                {
                if ((j >= 0) && (j < aImage->y))
                    {
                    /* If BLACK.
                     */
                    if (get_pixel_value(aImage, x, j) == BLACK)
                        {
                        /* tagged 1 means it was used in histogram.
                         */
                        if (test_pixel_for_tag(aImage, x, j, 1) == TRUE)
                            {
                            /* Remove its contribution from the histogram.
                             */
                            for (t=0; t < theta_res; t++)
                                {
                                theta = M_PI * ( (((double )t) - theta_res_d/2.0) / theta_res_d );
                                rho = cos(theta)*(double )x + sin(theta)*(double )j;
                                r = (int )(rho_res_d * rho/(2.0*max_rho) + rho_res_d/2.0);
                                histogram[t][r]--;
                                }
                            }
                        /* tagged 2 means removed from image.
                         */
                        tag_pixel(aImage, x, j, 2);
                        }
                    }
                }
            }
        }
    else
        {
        m = 1.0/m;
        b = l_rho/cos(l_theta);
        /* The line is in the form:
         *     x = my + b
         * and |m| < 1.0;
         */
        for (y=y_start; y < y_end; y++)
            {
            x = (int )( m*(double )y + b );
            for (i=x-width/2; i <= x+width/2; i++)
                {
                if ((i >= 0) && (i < aImage->x))
                    {
                    /* If BLACK
                     */
                    if (get_pixel_value(aImage, i, y) == BLACK)
                        {
                        /* tagged 1 means it was used in histogram.
                         */
                        if (test_pixel_for_tag(aImage, i, y, 1) == TRUE)
                            {
                            /* Remove its contribution from the histogram.
                             */
                            for (t=0; t < theta_res; t++)
                                {
                                theta = M_PI * ( (((double )t) - theta_res_d/2.0) / theta_res_d );
                                rho = cos(theta)*(double )i + sin(theta)*(double )y;
                                r = (int )(rho_res_d * rho/(2.0*max_rho) + rho_res_d/2.0);
                                histogram[t][r]--;
                                }
                            }
                        /* tagged 2 means removed from image.
                         */
                        tag_pixel(aImage, i, y, 2);
                        }               /* end of 'if (get_pixel_value...' */
                    }
                }
            }
        }

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

    display_hough_line_segment(x_start, y_start, x_end, y_end, width, color_index, theWindow, theGC);

}       /* end of remove_pixels_along_line_PHT() */



BOOLEAN
find_line_ends_PHT(double theta, double rho, IMAGE *aImage, int *x_start, int *y_start, int *x_end, int *y_end)
{
    int     x, y, sz;
    double  m, b;
    int     width = 15;
    int     min_line_break = 15;
    double  density, threshold = 0.04;
    double  length, min_length = 30;
    double  find_density_of_black_pixels_PHT(int v1, int width,
                                    double m, double b, char flag,
                                    IMAGE *aImage);


    m = -cos(theta)/sin(theta);

    if (ABS(m) <= 1.0)
        {
        b = rho/sin(theta);
        /* The line is in the form:
         *     y = mx + b
         * and |m| <= 1.0
         */
        length = 0;
        x = 0;
        do {

            /* Find line beginning
             */
            for (; x < aImage->x; x++)
                {
                density = find_density_of_black_pixels_PHT(x, width, m, b, 'x', aImage);
                if (density > threshold)
                    break;
                }
            *x_start = x;

            /* Find line end
             */
            for (; x < aImage->x; x++)
                {
                density = find_density_of_black_pixels_PHT(x, width, m, b, 'x', aImage);

                /* Look for a break in the line.
                 */
                if (density < threshold)
                    {
                    /* How big is the break in the line?
                     */
                    for (sz=0;(((x+sz) < aImage->x) && (density < threshold)); sz++)
                        density = find_density_of_black_pixels_PHT((x+sz), width, m, b, 'x', aImage);
                    if ((sz < min_line_break) && ((x+sz) < aImage->x))
                        {
                        /* It was just a small break in the line.
                         * It does not mark the line end.
                         */
                        x += (sz-1);    /* sz was incremented by one too
                                         * many in the 'for loop'.
                                         */
                        }
                    else
                        {
                        /* There was a notable break in the line.
                         * The beginning of the break (i.e. the value x)
                         * marks the line end.
                         */
                        break;  /* out of 'for (; x < aImage->x; x++)' */
                        }
                    }
                }
            *x_end = x;

            *y_start = (int )( m*(double )(*x_start) + b );
            *y_end = (int )( m*(double )(*x_end) + b );

            length = sqrt( SQR(*x_end-*x_start) + SQR(*y_end-*y_start) );

            } while ((x < aImage->x) && (length < min_length));
        }
    else    /* ABS(m) > 1.0 */
        {
        m = 1.0/m;
        b = rho/cos(theta);
        /* The line is in the form:
         *     x = my + b
         * and |m| < 1.0;
         */

        length = 0;
        y = 0;
        do {

            /* Find line beginning
             */
            for (; y < aImage->y; y++)
                {
                density = find_density_of_black_pixels_PHT(y, width, m, b, 'y', aImage);
                if (density > threshold)
                    break;
                }
            *y_start = y;

            /* Find line end
             */
            for (; y < aImage->y; y++)
                {
                density = find_density_of_black_pixels_PHT(y, width, m, b, 'y', aImage);

                /* Look for a break in the line.
                 */
                if (density < threshold)
                    {
                    /* How big is the break in the line?
                     */
                    for (sz=0;(((y+sz) < aImage->y) && (density < threshold)); sz++)
                        density = find_density_of_black_pixels_PHT((y+sz), width, m, b, 'y', aImage);
                    if ((sz < min_line_break) && ((y+sz) < aImage->y))
                        {
                        /* It was just a small break in the line.
                         * It does not mark the line end.
                         */
                        y += (sz-1);    /* sz was incremented by one too
                                         * many in the 'for loop'.
                                         */
                        }
                    else
                        {
                        /* There was a notable break in the line.
                         * The beginning of the break (i.e. the value y)
                         * marks the line end.
                         */
                        break;  /* out of 'for (; y < aImage->y; y++)' */
                        }
                    }
                }
            *y_end = y;

            *x_start = (int )( m*(double )(*y_start) + b );
            *x_end = (int )( m*(double )(*y_end) + b );

            length = sqrt( SQR(*x_end-*x_start) + SQR(*y_end-*y_start) );

            } while ((y < aImage->y) && (length < min_length));
        }

    if (length < min_length)
        return(False);
    else
        return(True);


}       /* end of find_line_ends_PHT() */



double
find_density_of_black_pixels_PHT(int v1, int width, double m, double b, char flag, IMAGE *aImage)
{
    int count = 0;
    int x, y, i, j;
    double  density;
    int     get_pixel_value(IMAGE *aImage, int x, int y);
    BOOLEAN test_pixel_for_tag(IMAGE *aImage, int x, int y, int tag);



    if (flag == 'x')
        {
        if (v1 < width/2)
            v1 = width/2;
        if (v1 > aImage->x - width/2)
            v1 = aImage->x - width/2;

        for (x=v1-width/2; x < v1+width/2; x++)
                {
                y = (int )( m*(double )x + b );
                for (j=y-width/2; j <= y+width/2; j++)
                    {
                    if ((j >= 0) && (j < aImage->y))
                        {
                        /* If BLACK and not tagged 2
                         * tagged 2 means it has been removed from image.
                         */
                        if ((get_pixel_value(aImage, x, j) == BLACK) &&
                            (test_pixel_for_tag(aImage, x, j, 2) == FALSE))
                            count++;
                        }
                    }
                }
        }
    else if (flag == 'y')
        {
        if (v1 < width/2)
            v1 = width/2;
        if (v1 > aImage->y - width/2)
            v1 = aImage->y - width/2;

        for (y=v1-width/2; y < v1+width/2; y++)
                {
                x = (int )( m*(double )y + b );
                for (i=x-width/2; i <= x+width/2; i++)
                    {
                    if ((i >= 0) && (i < aImage->x))
                        {
                        /* If BLACK and not tagged 2
                         * tagged 2 means it has been removed from image.
                         */
                        if ((get_pixel_value(aImage, i, y) == BLACK) &&
                            (test_pixel_for_tag(aImage, i, y, 2) == FALSE))
                            count++;
                        }
                    }
                }
        }


    density = (double )count / (double )SQR(width);


    return(density);

}   /* end of find_density_of_black_pixels_PHT() */
