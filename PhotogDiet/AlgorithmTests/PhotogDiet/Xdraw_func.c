/*
 * Programmed by:   Robert Ainsley McLaughlin
 * E-mail:          ram@ee.uwa.edu.au
 * Date:            18 August, 1996
 * Last modified:   12 March, 1998
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
#include <string.h>
#include <stdarg.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "../include/upwrite.h"





/* Function Prototypes.
 */
void    draw_image(XImage *aXImage, Window theWindow, GC theGC);
void    display_instructions(Window MainWindow);
void    drag_out_a_line(IMAGE *aImage, XImage *aXImage, Window theWindow, GC theGC);
void    free_hand_draw(IMAGE *aImage, XImage *aXImage, Window theWindow, GC theGC);
void    write_string_in_window(Window win, GC gc, char string[], XFontStruct *font_info);
void    draw_ellipse( Window theWindow, GC theGC, double x, double y, double r1, double r2, double theta );
void    draw_linked_list_of_gaus_2D(GAUS_LIST *gaus_list, Window theWindow, GC theGC);
void    draw_linked_list_of_linked_list_of_gaus_2D(GAUS_LIST *head_linked_list, Window theWindow, GC theGC);
void    draw_points_of_linked_list_of_gaus_2D(GAUS_LIST *gaus_list, Window theWindow, GC theGC);
void    draw_points_of_linked_list_of_linked_list_of_gaus_2D(GAUS_LIST *head_linked_list, Window theWindow, GC theGC);
void    color_pixels_in_XImage(GAUS_LIST *chunk_gaus_list, XImage *aXImage);
void    uncolor_pixels_in_XImage(IMAGE *aImage, XImage *aXImage);
void    write_message_window_strings(MESSAGE_WINDOW *message_window);
void    drag_out_a_circle(IMAGE *aImage, XImage *aXImage, Window theWindow, GC theGC);
void    drag_out_an_ellipse(IMAGE *aImage, XImage *aXImage, Window theWindow, GC theGC);
void    display_message_and_wait(Window theWindow, int x, int y, int num_strings, ...);
void    color_pixels_for_this_chunk(int color_index, GAUS_LIST *chunk, XImage *aXImage);
void    display_label_of_chunk(int x, int y, char *label, XFontStruct *font_info, short width, int ascent, int descent, int color_index, Window theWindow, GC theGC);
void    display_hough_line_segment(int x_start, int y_start, int x_end, int y_end, int width, int color_index, Window theWindow, GC theGC);
void    draw_hough_ellipse( Window theWindow, GC theGC, double x, double y, double r1, double r2, double theta );
DISPLAY_OBJECT      *alloc_display_object(void);
void                free_display_object(DISPLAY_OBJECT *d);
DISPLAY_OBJECT_LIST *alloc_display_object_list(void);
DISPLAY_OBJECT_LIST *put_in_linked_list_of_display_object(DISPLAY_OBJECT_LIST *display_oject_list, DISPLAY_OBJECT *d);
void                free_linked_list_of_display_object(DISPLAY_OBJECT_LIST *display_object_list);
DISPLAY_OBJECT_LIST *remove_object_type_from_linked_list_of_display_object(DISPLAY_OBJECT_LIST *display_object_list, OBJECT_TYPE type);
DISPLAY_OBJECT_LIST *remove_specific_object_from_linked_list_of_display_object(DISPLAY_OBJECT_LIST *display_object_list, DISPLAY_OBJECT *d_to_be_removed);
void                display_number_training_examples(Window MainWindow, GC MainGC);



void
draw_image(XImage *aXImage, Window theWindow, GC theGC)
{
    XGCValues   values, old_values;
    int         x, y;
    char    str[256];
    DISPLAY_OBJECT  *d;
    void    display_label_of_chunk(int x, int y, char *label, XFontStruct *font_info, short width, int ascent, int descent, int color_index, Window theWindow, GC theGC);
    void    display_hough_line_segment(int x_start, int y_start, int x_end, int y_end, int width, int color_index, Window theWindow, GC theGC);

    /* Take a copy of the current GC settings.
     */
    XGetGCValues(gDisplay, theGC, GCLineWidth | GCFunction | GCForeground, &old_values);

    /* Change GC settings.
     */
    values.line_width = 1;
    values.function = GXcopy;
    values.foreground = gRandomColors[0].pixel;     /* red */
    XChangeGC( gDisplay, theGC, GCLineWidth | GCFunction | GCForeground, &values );


    XPutImage(gDisplay, theWindow, theGC, aXImage, 0, 0, 0, 0, aXImage->width, aXImage->height);



    /* Draw an example of the resolution radius.
     */
    x = aXImage->width-2*(int )(gResolutionRadius)-10;
    y = 10;
    XDrawArc(gDisplay, theWindow, theGC, x, y, 2*(int )gResolutionRadius, 2*(int )gResolutionRadius, 0, 23040);
    sprintf(str, "%.1f", (float )gResolutionRadius);
    XDrawString(gDisplay, theWindow, theGC, x-16, y+10+2*(int )(gResolutionRadius), str, (int )strlen(str) );


    /* Draw any objects that have
     * been found during recognition.
     */
    if (gDisplayObjectList != NULL)
        {
        for (d=gDisplayObjectList->head; d != NULL; d=d->next)
            {
            if (d->type == upwrite)
                {
                display_label_of_chunk(d->obj.upwrite.x, d->obj.upwrite.y,
                                d->obj.upwrite.label,
                                d->obj.upwrite.font_info,
                                d->obj.upwrite.width,
                                d->obj.upwrite.ascent,
                                d->obj.upwrite.descent,
                                d->obj.upwrite.color_index,
                                theWindow, theGC);
                }
            else if (d->type == hough_line)
                {
                display_hough_line_segment(d->obj.hough_line.x_start,
                                d->obj.hough_line.y_start,
                                d->obj.hough_line.x_end,
                                d->obj.hough_line.y_end,
                                d->obj.hough_line.width,
                                d->obj.hough_line.color_index,
                                theWindow, theGC);
                }
            else if (d->type == hough_circle)
                {
                values.line_width = d->obj.hough_circle.width;
                values.function = GXcopy;
                values.foreground = gRandomColors[d->obj.hough_circle.color_index].pixel;
                XChangeGC( gDisplay, theGC, GCLineWidth | GCFunction | GCForeground, &values );
                XDrawArc(gDisplay, theWindow, theGC,
                                d->obj.hough_circle.x,
                                d->obj.hough_circle.y,
                                d->obj.hough_circle.diameter,
                                d->obj.hough_circle.diameter, 0, 23040);

                }
            else if (d->type == hough_ellipse)
                {
                values.line_width = d->obj.hough_ellipse.width;
                values.function = GXcopy;
                values.foreground = gRandomColors[d->obj.hough_ellipse.color_index].pixel;
                XChangeGC( gDisplay, theGC, GCLineWidth | GCFunction | GCForeground, &values );
                draw_hough_ellipse( theWindow, theGC,
                                    d->obj.hough_ellipse.x,
                                    d->obj.hough_ellipse.y,
                                    d->obj.hough_ellipse.a,
                                    d->obj.hough_ellipse.b,
                                    d->obj.hough_ellipse.theta );
                }
            }   /* end of 'for (d=gDisplayObjectList->head;...` */
        }   /* end of 'if (gDisplayObjectList != NULL)' */



    /* Change GC back to old line width.
     */
    XChangeGC( gDisplay, theGC, GCLineWidth | GCFunction | GCForeground, &old_values );
    XFlush(gDisplay);

}   /* end of draw_image() */



void
display_instructions(Window MainWindow)
{
    MESSAGE_WINDOW  *message_window;
    XEvent  report;
    BOOLEAN finish_wait;
    MESSAGE_WINDOW  *alloc_message_window(void);
    void    init_message_window_fields(MESSAGE_WINDOW *message_window, Window MainWindow, int x, int y, int num_strings, ...);
    void    write_message_window_strings(MESSAGE_WINDOW *message_window);
    void    free_message_window(MESSAGE_WINDOW *message_window);


    /* Open a window which displays the instructions
     * then asks the user to hit the left mouse button.
     */
    message_window = alloc_message_window();
    init_message_window_fields(message_window, MainWindow, 0, 0, 24,
                            "This program allows you to draw an image",
                            "containing lines, circles and ellipses,",
                            "then test various image recognition algorithms on it.",
                            "The algorithms available are:",
                            "    * The UpWrite",
                            "    * Standard Hough Transform",
                            "    * Randomized Hough Transform",
                            "    * Probabilistic Hough Transform",
                            "    * Hierarchical Hough Transform",
                            "",
                            "",
                            "INSTRUCTIONS.",
                            "",
                            "Left mouse button:   draw lines",
                            "Middle mouse button: freehand draw",
                            "Right mouse button:  menus",
                            "",
                            "Each menu has a Help option",
                            "",
                            "Queries: Robert McLaughlin",
                            "e-mail:  ram@ee.uwa.edu.au",
                            "",
                            "Hit Left Mouse Button to continue",
                            "");
    XMapWindow(gDisplay, message_window->win);


    /* Event Loop
     * Wait for the user to press the
     * left mouse button
     */
    finish_wait = FALSE;
    do {
        XNextEvent( gDisplay, &report );
        switch (report.type)
            {
            case Expose :
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
                        XFlush(gDisplay);
                        }
                    }
                break;
            case ButtonPress :
                if (report.xbutton.button == Button1)
                    finish_wait = TRUE;
                break;
            default :
                break;

            }   /* end of 'switch (report.type)' */
        }
    while (finish_wait == FALSE);


    XUnmapWindow(gDisplay, message_window->win);
    XFlush(gDisplay);

    free_message_window(message_window);

}   /* end of display_instructions() */



void
drag_out_a_line(IMAGE *aImage, XImage *aXImage, Window theWindow, GC theGC)
{
    Window          root, child;
    int             root_x, root_y;
    int             x1, y1, x2, y2, x2_old, y2_old, x, y;
    int             width;
    unsigned int    keys_button;
    double          grad;
    XGCValues       GC_values, GC_old_values;
    int             theScreen;
    void            set_pixel_value(IMAGE *aImage, XImage *aXImage, int x, int y, int val);




    if (gLineThickness == 0)
        return;

    /* Store the current GC values.
     */
    XGetGCValues(gDisplay, theGC, GCLineWidth | GCFunction | GCForeground, &GC_old_values);

    /* Change GC values.
     */
    GC_values.line_width = gLineThickness;
    GC_values.function = GXxor;
    GC_values.foreground = gRandomColors[0].pixel;      /* red color */
    XChangeGC( gDisplay, theGC, GCLineWidth | GCFunction | GCForeground, &GC_values );

    /* (x1, y1) is the initial position of the pointer.
     * (x2, y2) is the current position of the pointer.
     * (x2_old, y2_old) is where the pointer was a little while ago.
     */
    XQueryPointer(gDisplay, theWindow, &root, &child, &root_x, &root_y, &x1, &y1, &keys_button);

    /* If it is inside the window's parameters.
     */
    if ((x1 >= (gLineThickness-1)) && (x1 <= (aImage->x-gLineThickness)) &&
        (y1 >= (gLineThickness-1)) && (y1 <= (aImage->y-gLineThickness)) )
        {
        x2_old = x1;  y2_old = y1;

        do
            {
            /* Where is the pointer
             */
            XQueryPointer(gDisplay, theWindow, &root, &child, &root_x, &root_y, &x2, &y2, &keys_button);

            /* If we go outside of window.
             */
            if (x2 < (gLineThickness - 1))
                x2 = gLineThickness - 1;
            else if (x2 > (aImage->x - gLineThickness))
                x2 = aImage->x - gLineThickness;

            if (y2 < (gLineThickness - 1))
                y2 = gLineThickness - 1;
            else if (y2 > (aImage->y - gLineThickness))
                y2 = aImage->y - gLineThickness;


            if ((x2 != x2_old) || (y2 != y2_old))
                {
                /* If the pointer has moved.
                 */

                /* OverDraw line */
                XDrawLine(gDisplay, theWindow, theGC, x1, y1, x2_old, y2_old);

                /* Draw line */
                XDrawLine(gDisplay, theWindow, theGC, x1, y1, x2, y2);
                XFlush(gDisplay);

                x2_old = x2;  y2_old = y2;
                }
            }       /* repeat while Button1 is till pressed down. */
        while ((keys_button & Button1Mask) == Button1Mask);


        /* Overdraw line one last time.
         */
        XDrawLine(gDisplay, theWindow, theGC, x1, y1, x2_old, y2_old);


        /* Store it in (IMAGE *)aImage and (Ximage *)aXImage.
         * Set foreground color to black
         * and draw the line on the screen.
         */
        theScreen = DefaultScreen(gDisplay);
        XSetForeground(gDisplay, theGC, BlackPixel( gDisplay, theScreen) );
        XSetFunction(gDisplay, theGC, GXcopy);
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
                    for (width=-gLineThickness/2; width < (gLineThickness+1)/2; width++)
                        {
                        set_pixel_value(aImage, aXImage, x, y+width, BLACK);
                        XDrawPoint(gDisplay, theWindow, theGC, x, y+width);
                        }
                    }
                }
            else if (x1 > x2)
                {
                grad = ((double )(y1-y2)) / (double )(x1-x2);
                for (x=x2; x <= x1; x++)
                    {
                    y = y2 + (int )(grad*(double )(x - x2));
                    for (width=-gLineThickness/2; width < (gLineThickness+1)/2; width++)
                        {
                        set_pixel_value(aImage, aXImage, x, y+width, BLACK);
                        XDrawPoint(gDisplay, theWindow, theGC, x, y+width);
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
                    for (width=-gLineThickness/2; width < (gLineThickness+1)/2; width++)
                        {
                        set_pixel_value(aImage, aXImage, x+width, y, BLACK);
                        XDrawPoint(gDisplay, theWindow, theGC, x+width, y);
                        }
                    }
                }
            else if (y1 > y2)
                {
                grad = ((double )(x1-x2)) / (double )(y1-y2);
                for (y=y2; y <= y1; y++)
                    {
                    x = x2 + (int )(grad*(double )(y - y2));
                    for (width=-gLineThickness/2; width < (gLineThickness+1)/2; width++)
                        {
                        set_pixel_value(aImage, aXImage, x+width, y, BLACK);
                        XDrawPoint(gDisplay, theWindow, theGC, x+width, y);
                        }
                    }
                }
            }
        }


    /* Change GC back to old values.
     */
    XChangeGC( gDisplay, theGC, GCLineWidth | GCFunction | GCForeground, &GC_old_values );
    XFlush(gDisplay);

}       /* end of drag_out_a_line() */



void
free_hand_draw(IMAGE *aImage, XImage *aXImage, Window theWindow, GC theGC)
{
    Window          root, child;
    int             root_x, root_y;
    int             x, y, x_old, y_old, i, j;
    int             width;
    double          grad;
    unsigned int    keys_button;
    XGCValues       GC_values, GC_old_values;
    int             theScreen;
    unsigned long   theBlackPixel;
    void            set_pixel_value(IMAGE *aImage, XImage *aXImage, int x, int y, int val);


    if (gLineThickness == 0)
        return;


    theScreen = DefaultScreen(gDisplay);
    theBlackPixel = BlackPixel( gDisplay, theScreen);


    /* Store the current GC values.
     */
    XGetGCValues(gDisplay, theGC, GCLineWidth | GCFunction | GCForeground | GCCapStyle, &GC_old_values);

    /* Change GC values.
     */
    GC_values.line_width = gLineThickness;
    GC_values.function = GXcopy;
    GC_values.foreground = theBlackPixel;       /* Black Color */
    GC_values.cap_style = CapButt;
    XChangeGC( gDisplay, theGC, GCLineWidth | GCFunction | GCForeground | GCCapStyle, &GC_values );



    /* (x, y) is the current position of the pointer.
     * (x_old, y_old) is the previous position of the pointer.
     */

    /* Get initial position of pointer.
     */
    XQueryPointer(gDisplay, theWindow, &root, &child, &root_x, &root_y, &x_old, &y_old, &keys_button);


    do
        {
        XQueryPointer(gDisplay, theWindow, &root, &child, &root_x, &root_y, &x, &y, &keys_button);

        /* If it is inside the window's parameters.
         */
        if ((x >= (gLineThickness-1)) && (x <= (aImage->x-gLineThickness)) &&
            (y >= (gLineThickness-1)) && (y <= (aImage->y-gLineThickness)) )
            {

            /* Store it in (IMAGE *)aImage and (XImage *)aXImage
             */
            if (ABS(x-x_old) >= ABS(y-y_old))
                {
                /* x changes more than y
                 */
                if (x_old < x)
                    {
                    grad = ((double )(y-y_old)) / (double )(x-x_old);
                    for (i=x_old; i <= x; i++)
                        {
                        j = y_old + (int )(grad*(double )(i - x_old));
                        for (width=-gLineThickness/2; width < (gLineThickness+1)/2; width++)
                            {
                            set_pixel_value(aImage, aXImage, i, j+width, BLACK);
                            XDrawPoint(gDisplay, theWindow, theGC, i, j+width);
                            }
                        }
                    }
                else if (x_old > x)
                    {
                    grad = ((double )(y_old-y)) / (double )(x_old-x);
                    for (i=x; i <= x_old; i++)
                        {
                        j = y + (int )(grad*(double )(i - x));
                        for (width=-gLineThickness/2; width < (gLineThickness+1)/2; width++)
                            {
                            set_pixel_value(aImage, aXImage, i, j+width, BLACK);
                            XDrawPoint(gDisplay, theWindow, theGC, i, j+width);
                            }
                        }
                    }
                }
            else        /* if (ABS(x-x_old) < ABS(y-y_old)) */
                {
                /* y changes more than x
                 */
                if (y_old < y)
                    {
                    grad = ((double )(x-x_old)) / (double )(y-y_old);
                    for (j=y_old; j <= y; j++)
                        {
                        i = x_old + (int )(grad*(double )(j-y_old));
                        for (width=-gLineThickness/2; width < (gLineThickness+1)/2; width++)
                            {
                            set_pixel_value(aImage, aXImage, i+width, j, BLACK);
                            XDrawPoint(gDisplay, theWindow, theGC, i+width, j);
                            }
                        }
                    }
                else if (y_old > y)
                    {
                    grad = ((double )(x_old-x)) / (double )(y_old-y);
                    for (j=y; j <= y_old; j++)
                        {
                        i = x + (int )(grad*(double )(j - y));
                        for (width=-gLineThickness/2; width < (gLineThickness+1)/2; width++)
                            {
                            set_pixel_value(aImage, aXImage, i+width, j, BLACK);
                            XDrawPoint(gDisplay, theWindow, theGC, i+width, j);
                            }
                        }
                    }
                }

            /* Display it in theWindow.
             */
            XFlush(gDisplay);
            x_old = x;
            y_old = y;
            }
        else        /* if outside of the window */
            {
            /* Set (x_old, y_old) for when we re-enter the window.
             */
            if (x < (gLineThickness - 1))
                x_old = gLineThickness - 1;
            else if (x > (aImage->x - gLineThickness))
                x_old = aImage->x - gLineThickness;
            else
                x_old = x;

            if (y < (gLineThickness - 1))
                y_old = gLineThickness - 1;
            else if (y >= (aImage->y - gLineThickness))
                y_old = aImage->y - gLineThickness;
            else
                y_old = y;
            }

        } while ((keys_button & Button2Mask) == Button2Mask);
        /* repeat while Button2 is till pressed down. */


    /* Change GC back to old values.
     */
    XChangeGC( gDisplay, theGC, GCLineWidth | GCFunction | GCForeground | GCCapStyle, &GC_old_values );

}   /* end of free_hand_draw() */



void
write_string_in_window(Window win, GC gc, char string[], XFontStruct *font_info)
{
    int x, y;
    int direction, ascent, descent;
    XCharStruct overall;

    x = 10;
    y = font_info->max_bounds.ascent;
    XClearWindow(gDisplay, win);
    XDrawString(gDisplay, win, gc, x, y, string, (int )strlen(string) );
    XTextExtents(font_info, string, strlen(string),
                &direction, &ascent, &descent, &overall);

    /* Draw a rectangle prompt */
    x = overall.width + 12;
    XFillRectangle(gDisplay, win, gc, x, 2, font_info->max_bounds.width, font_info->ascent+font_info->descent);

    XFlush(gDisplay);

}   /* end of write_string_in_window() */



void
draw_ellipse( Window theWindow, GC theGC, double x, double y, double r1, double r2, double theta )
/*
 *      x, y;       -- centre of ellipse
 *      r1, r2;     -- major/minor axis
 *      theta;      -- angle
 */

{
    /* We have made the arrays [3][3] and the vectors [3]
     * This is so that the arrays can be indexed [1..2][1..2]
     * and the vectors indexed [1..2]
     */
    double D_neg_sqrt[3][3];
    double E[3][3], E_inv[3][3];
    double X[3], X_old[3], X_original[3];
    double T[3];
    double temp[3][3], temp2[3][3];
    double t;
    XGCValues   values, old_values;


    /* Change GC to a line width of 2
     * and GCFunction to GXxor.
     * Make a copy of the old line width. We will restore it
     * at the end of the function.
     */
    XGetGCValues(gDisplay, theGC, GCLineWidth | GCFunction, &old_values);
    values.line_width = 2;
    values.function = GXcopy;
    XChangeGC( gDisplay, theGC, GCLineWidth | GCFunction, &values );
    /* -- The negative square root of the diagonal
     *    matrix containing the eigenvalues.
     */
    D_neg_sqrt[1][1] = r1;   D_neg_sqrt[1][2] = 0;
    D_neg_sqrt[2][1] = 0;    D_neg_sqrt[2][2] = r2;

    /* -- Eigenbasis of ellipse */
    E[1][1] = cos(theta);    E[1][2] = -sin(theta);
    E[2][1] = sin(theta);    E[2][2] = cos(theta);

    /* -- Inverse of E
     * equals transpose of E since E is orthogonal.
     */
    E_inv[1][1] = E[1][1];
    E_inv[1][2] = E[2][1];
    E_inv[2][1] = E[1][2];
    E_inv[2][2] = E[2][2];

    /* -- Set up values used in calculations in loop
     * Calculate E * D^-0.5 * E^-1
     */
    temp[1][1] = E[1][1]*D_neg_sqrt[1][1] + E[1][2]*D_neg_sqrt[2][1];
    temp[1][2] = E[1][1]*D_neg_sqrt[1][2] + E[1][2]*D_neg_sqrt[2][2];
    temp[2][1] = E[2][1]*D_neg_sqrt[1][1] + E[2][2]*D_neg_sqrt[2][1];
    temp[2][2] = E[2][1]*D_neg_sqrt[1][2] + E[2][2]*D_neg_sqrt[2][2];

    temp2[1][1] = temp[1][1]*E_inv[1][1] + temp[1][2]*E_inv[2][1];
    temp2[1][2] = temp[1][1]*E_inv[1][2] + temp[1][2]*E_inv[2][2];
    temp2[2][1] = temp[2][1]*E_inv[1][1] + temp[2][2]*E_inv[2][1];
    temp2[2][2] = temp[2][1]*E_inv[1][2] + temp[2][2]*E_inv[2][2];



    T[1] = cos(0.0);
    T[2] = sin(0.0);

    X_old[1] = temp2[1][1]*T[1] + temp2[1][2]*T[2];
    X_old[2] = temp2[2][1]*T[1] + temp2[2][2]*T[2];

    X_original[1] = X_old[1] = x + X_old[1];
    X_original[2] = X_old[2] = y + X_old[2];

    for (t=0.0; t <=(2.0*M_PI); t+= 0.4)
        {
        T[1] = cos(t);
        T[2] = sin(t);
        X[1] = temp2[1][1]*T[1] + temp2[1][2]*T[2];
        X[2] = temp2[2][1]*T[1] + temp2[2][2]*T[2];

        /* -- (x, y) is the centre of the ellipse.
         */
        X[1] = x + X[1];
        X[2] = y + X[2];

        XDrawLine( gDisplay, theWindow, theGC,
                    (int )X_old[1], (int )X_old[2],
                    (int )X[1], (int )X[2] );

        X_old[1] = X[1];
        X_old[2] = X[2];
        }

    XDrawLine( gDisplay, theWindow, theGC,
                (int )X_original[1], (int )X_original[2],
                (int )X[1], (int )X[2] );

    /* Change GC back to old line width.
     */
    XChangeGC( gDisplay, theGC, GCLineWidth | GCFunction, &old_values );

    XFlush( gDisplay );

}   /* -- draw_ellipse() */



void
draw_linked_list_of_gaus_2D(GAUS_LIST *gaus_list, Window theWindow, GC theGC)
{
    GAUS    *gaus;
    int     i, num_non_zero_eig_val;
    int     x1, y1, x2, y2;
    void    draw_ellipse( Window theWindow, GC theGC, double x, double y, double r1, double r2, double theta);
    XGCValues   values, old_values;

    if (gaus_list == NULL)
        return;

    for (gaus=gaus_list->head; gaus != NULL; gaus=gaus->next)
        {
        /* Count number of non-zero eigenvalues.
         */
        for (i=1; i <= gaus->dim; i++)
            if (gaus->eig_val[i] == (double )0)
                {
                break;
                }
        num_non_zero_eig_val = i-1;

        if (num_non_zero_eig_val == 2)
            {
            draw_ellipse(theWindow, theGC,
                        gaus->mean[1], gaus->mean[2],
                        sqrt(gaus->eig_val[1]), sqrt(gaus->eig_val[2]),
                        atan2(gaus->eig_vec[2][1], gaus->eig_vec[1][1]) );
            }
        else if (num_non_zero_eig_val == 1)
            {
            XGetGCValues(gDisplay, theGC, GCLineWidth | GCFunction,&old_values);
            values.line_width = 2;
            values.function = GXcopy;
            XChangeGC( gDisplay, theGC, GCLineWidth | GCFunction, &values );
            x1 = (int )(gaus->mean[1] - sqrt(gaus->eig_val[1])*gaus->eig_vec[1][1]);
            y1 = (int )(gaus->mean[2] - sqrt(gaus->eig_val[1])*gaus->eig_vec[2][1]);
            x2 = (int )(gaus->mean[1] + sqrt(gaus->eig_val[1])*gaus->eig_vec[1][1]);
            y2 = (int )(gaus->mean[2] + sqrt(gaus->eig_val[1])*gaus->eig_vec[2][1]);
            XDrawLine(gDisplay, theWindow, theGC, x1, y1, x2, y2);
            /* Change GC back to old line width.
             */
            XChangeGC( gDisplay, theGC, GCLineWidth | GCFunction, &old_values );
            }

        }   /* end of 'for (gaus=gaus_list->head; gaus !=...' */

    XFlush(gDisplay);


}   /* end of draw_linked_list_of_gaus() */



void    draw_linked_list_of_linked_list_of_gaus_2D(GAUS_LIST *head_linked_list, Window theWindow, GC theGC)
{
    GAUS_LIST   *gaus_list;
    int     color_index = 0;
    void    draw_linked_list_of_gaus_2D(GAUS_LIST *gaus_list, Window theWindow, GC theGC);


    if (head_linked_list == (GAUS_LIST *)NULL)
        return;

    for (gaus_list=head_linked_list; gaus_list != NULL; gaus_list=gaus_list->next_list)
        {
        /* Draw each linked list in a random color.
         */
        XSetForeground(gDisplay, theGC, gRandomColors[color_index++].pixel);
        if (color_index == NUM_RANDOM_COLORS)
            color_index = 0;
        draw_linked_list_of_gaus_2D(gaus_list, theWindow, theGC);
        }


}   /* end of draw_linked_list_of_linked_list_of_gaus_2D() */



/* Draw the points which generated a linked list of GAUS.
 */
void
draw_points_of_linked_list_of_gaus_2D(GAUS_LIST *gaus_list, Window theWindow, GC theGC)
{
    GAUS    *gaus;
    POINT   *point;

    if (gaus_list == NULL)
        return;


    for (gaus=gaus_list->head; gaus != NULL; gaus=gaus->next)
        for (point=gaus->point_list->head; point != NULL; point=point->next)
            XDrawPoint(gDisplay, theWindow, theGC, (int )point->x[1], (int )point->x[2]);
    XFlush(gDisplay);


}   /* end of draw_points_of_linked_list_of_gaus_2D() */



void
draw_points_of_linked_list_of_linked_list_of_gaus_2D(GAUS_LIST *head_linked_list, Window theWindow, GC theGC)
{
    GAUS_LIST    *gaus_list;
    int     color_index;
    void    draw_points_of_linked_list_of_gaus_2D(GAUS_LIST *gaus_list, Window theWindow, GC theGC);


    color_index = (int )(drand48() * NUM_RANDOM_COLORS);

    for (gaus_list=head_linked_list; gaus_list != NULL; gaus_list=gaus_list->next_list)
        {
        /* Draw each linked list in a random color.
         */
        XSetForeground(gDisplay, theGC, gRandomColors[color_index++].pixel);
        if (color_index == NUM_RANDOM_COLORS)
            color_index = 0;
        draw_points_of_linked_list_of_gaus_2D(gaus_list, theWindow, theGC);
        }


}   /* end of draw_points_of_linked_list_of_linked_list_of_gaus_2D() */



void
color_pixels_in_XImage(GAUS_LIST *chunk_gaus_list, XImage *aXImage)
{
    GAUS_LIST    *chunk;
    GAUS    *gaus;
    POINT   *point;
    int     x, y;
    int     color_index = 0;
    void    set_XImage_pixel_value(int x, int y, int color, XImage *aXImage);



    for (chunk=chunk_gaus_list; chunk != NULL; chunk=chunk->next_list)
        {
        /* Color each chunk in a random color.
         */
        if (color_index == NUM_RANDOM_COLORS)
            color_index = 0;


        for (gaus=chunk->head; gaus != NULL; gaus=gaus->next)
            for (point=gaus->point_list->head; point != NULL; point=point->next)
                {
                x  = (int )point->x[1];
                y  = (int )point->x[2];
                set_XImage_pixel_value(x, y, color_index, aXImage);
                }
        color_index++;
        }

}   /* end of draw_points_of_linked_list_of_linked_list_of_gaus_2D() */



void
uncolor_pixels_in_XImage(IMAGE *aImage, XImage *aXImage)
{
    int     x, y;
    int     get_pixel_value(IMAGE *aImage, int x, int y);
    void    set_pixel_value(IMAGE *aImage, XImage *aXImage, int x, int y, int val);


    for (x=0; x < aImage->x; x++)
        for (y=0; y < aImage->y; y++)
            set_pixel_value(aImage, aXImage, x, y, get_pixel_value(aImage, x, y));

}   /* end of uncolor_pixels_in_XImage() */



void
write_message_window_strings(MESSAGE_WINDOW *message_window)
{
    int i;
    int x, y;

    x = 10;

    XClearWindow(gDisplay, message_window->win);

    /* Set the appropriate font.
     */
    XSetFont(gDisplay, message_window->gc, message_window->font_info->fid);
 

    for (i=0; i < message_window->num_strings; i++)
        {
        y = i*message_window->line_height + message_window->font_info->max_bounds.ascent;
        XDrawString(gDisplay, message_window->win, message_window->gc, x, y, message_window->message[i], (int )strlen(message_window->message[i]) );
        }

    XFlush(gDisplay);

}   /* end of write_message_window_messages() */



void
drag_out_a_circle(IMAGE *aImage, XImage *aXImage, Window theWindow, GC theGC)
{
    XEvent          report;
    Window          root, child;
    int             root_x, root_y;
    int             x1, y1, x2, y2;
    int             x2_old, y2_old;
    int             x_bl_corner, y_bl_corner, width;
    int             x_bl_corner_old, y_bl_corner_old, width_old;
    int             first_flag;
    unsigned int    keys_button;
    XGCValues       GC_values, GC_old_values;
    double          radius, perimeter, theta;
    int             x_centre, y_centre, x, y;
    int             theScreen;
    void            set_pixel_value(IMAGE *aImage, XImage *aXImage, int x, int y, int val);


    /* Wait until the left button is pressed.
     */
    do
        {
        XNextEvent( gDisplay, &report );
        }
    while ((report.type != ButtonPress) || (report.xbutton.button != Button1));


    /* Find out where the pointer is
     * We will draw a bounding square for the circle.
     * This point marks a corner of the bounding square.
     */
    XQueryPointer(gDisplay, theWindow, &root, &child, &root_x, &root_y, &x1, &y1, &keys_button);




    /* Take a copy of the GC.
     * We will make a few changes, but restore the GC before
     * we leave this function.
     *
     * Set foreground color to red.
     */
    XGetGCValues(gDisplay, theGC, GCLineWidth | GCFunction | GCForeground, &GC_old_values);
    GC_values.line_width = 1;
    GC_values.function = GXxor;
    GC_values.foreground = gRandomColors[0].pixel;
    XChangeGC( gDisplay, theGC, GCLineWidth | GCFunction | GCForeground, &GC_values );


    x_bl_corner = -1;      y_bl_corner = -1;
    width = -1;
    x2_old = -1;           y2_old = -1;
    x_bl_corner_old = -1;  y_bl_corner_old = -1;
    width_old = -1;
    first_flag = TRUE;
    do
        {
        XQueryPointer(gDisplay, theWindow, &root, &child, &root_x, &root_y, &x2, &y2, &keys_button);

        /* Has the pointer moved?
         */
        if ((x2 != x2_old) || (y2 != y2_old))
            {
            /* We will draw a bounding square for the circle.
             * Decide on the squares width.
             */
            if (ABS(x2-x1) > ABS(y2-y1))
                width = ABS(x2-x1);
            else
                width = ABS(y2-y1);

            /* bl_corner stands for bottom left-hand corner
             * Find the bottom left-hand corner of the square.
             */
            if (x2 < x1)
                x_bl_corner = x1-width;
            else
                x_bl_corner = x1;

            if (y2 < y1)
                y_bl_corner = y1-width;
            else
                y_bl_corner = y1;

            /* Over-draw the previous bounding square.
             * We don't need to do this the first time
             * through this loop.
             */
            if (first_flag == FALSE)
                {
                XDrawRectangle(gDisplay, theWindow, theGC, x_bl_corner_old, y_bl_corner_old, width_old, width_old);
                }
            XDrawRectangle(gDisplay, theWindow, theGC, x_bl_corner, y_bl_corner, width, width);
            XFlush(gDisplay);


            x_bl_corner_old = x_bl_corner;
            y_bl_corner_old = y_bl_corner;
            width_old = width;
            x2_old = x2;        y2_old = y2;
            first_flag = FALSE;
            }
        }
    while ((keys_button & Button1Mask) == Button1Mask);

    /* Over-draw the bounding square one last time.
     */
    XDrawRectangle(gDisplay, theWindow, theGC, x_bl_corner_old, y_bl_corner_old, width_old, width_old);


    /* Draw the circle in black.
     * Change a few values in the GC
     * (line thickness to gLineThickness, color to black,
     * GC_function to GXcopy).
     */
    GC_values.line_width = gLineThickness;
    GC_values.function = GXcopy;
    theScreen = DefaultScreen(gDisplay);
    GC_values.foreground = BlackPixel( gDisplay, theScreen);
    XChangeGC( gDisplay, theGC, GCLineWidth | GCFunction | GCForeground, &GC_values );

    if (width > gLineThickness)
        {
        radius = width/2;
        x_centre = x_bl_corner + radius;
        y_centre = y_bl_corner + radius;
        for (width=-gLineThickness/2; width < (gLineThickness+1)/2; width++)
            {
            perimeter = 2 * M_PI*(radius+width);
            for (theta=0; theta < 2*M_PI; theta+=1/perimeter)
                {
                x = x_centre + (radius+width)*cos(theta);
                y = y_centre + (radius+width)*sin(theta);
                if ((x >= 0) && (y >= 0) && (x < aImage->x) && (y < aImage->y))
                    {
                    set_pixel_value(aImage, aXImage, x, y, BLACK);
                    XDrawPoint(gDisplay, theWindow, theGC, x, y);
                    }
                }
            }   /* end of 'for (width=-gLineThickness/2;...' */
        }   /* end of if (width > gLineThickness)' */

    /* Change GC back to old values.
     */
    XChangeGC( gDisplay, theGC, GCLineWidth | GCFunction | GCForeground, &GC_old_values );

}   /* end of drag_out_a_circle() */



void
drag_out_an_ellipse(IMAGE *aImage, XImage *aXImage, Window theWindow, GC theGC)
{
    XEvent          report;
    Window          root, child;
    int             root_x, root_y;
    int             x1, y1, x2, y2;
    int             x2_old, y2_old;
    int             x_bl_corner, y_bl_corner, width, height;
    int             x_bl_corner_old, y_bl_corner_old, width_old, height_old;
    int             first_flag;
    unsigned int    keys_button;
    XGCValues       GC_values, GC_old_values;
    int             length1, length2;
    double          angle, base_angle, angle_old;
    int             return_flag;
    int             theScreen;
    int             x, y;
    double          x_centre, y_centre, r1, r2, theta, grad;
    double          perimeter, incr;
    double          M[3][3];
    void            set_pixel_value(IMAGE *aImage, XImage *aXImage, int x, int y, int val);


    /* Wait until the left button is pressed.
     */
    do
        {
        XNextEvent( gDisplay, &report );
        }
    while ((report.type != ButtonPress) || (report.xbutton.button != Button1));


    /* Find out where the pointer is
     * We will draw a bounding square for the circle.
     * This point marks a corner of the bounding square.
     */
    XQueryPointer(gDisplay, theWindow, &root, &child, &root_x, &root_y, &x1, &y1, &keys_button);




    /* Take a copy of the GC.
     * We will make a few changes, but restore the GC before
     * we leave this function.
     * Set foreground color to red.
     */
    XGetGCValues(gDisplay, theGC, GCLineWidth | GCFunction | GCForeground, &GC_old_values);
    GC_values.line_width = 1;
    GC_values.function = GXxor;
    GC_values.foreground = gRandomColors[0].pixel;
    XChangeGC( gDisplay, theGC, GCLineWidth | GCFunction | GCForeground, &GC_values );


    x_bl_corner = -1;      y_bl_corner = -1;
    width = -1;            height = -1;
    x2_old = -1;           y2_old = -1;
    x_bl_corner_old = -1;  y_bl_corner_old = -1;
    width_old = -1;        height_old = -1;
    first_flag = TRUE;
    do
        {
        XQueryPointer(gDisplay, theWindow, &root, &child, &root_x, &root_y, &x2, &y2, &keys_button);

        /* Is pointer in the window?
         */
        if ((x2 >= 0) && (x2 < aImage->x) && (y2 >= 0) && (y2 < aImage->y))
            {
            /* Has the pointer moved?
             */
            if ((x2 != x2_old) || (y2 != y2_old))
                {
                /* We will draw a bounding rectangle for the ellipse.
                 */
                width = ABS(x2-x1);
                height = ABS(y2-y1);

                /* bl_corner stands for bottom left-hand corner
                 * Find the bottom left-hand corner of the rectangle. 
                 */
                if (x1 < x2)
                    x_bl_corner = x1;
                else
                    x_bl_corner = x2;

                if (y1 < y2)
                    y_bl_corner = y1;
                else
                    y_bl_corner = y2;

                /* Over-draw the previous bounding rectangle.
                 * We don't need to do this the first time
                 * through this loop.
                 */
                if (first_flag == FALSE)
                    {
                    XDrawRectangle(gDisplay, theWindow, theGC, x_bl_corner_old, y_bl_corner_old, width_old, height_old);
                    }
                XDrawRectangle(gDisplay, theWindow, theGC, x_bl_corner, y_bl_corner, width, height);
                XFlush(gDisplay);


                x_bl_corner_old = x_bl_corner;
                y_bl_corner_old = y_bl_corner;
                width_old = width;  height_old = height;
                x2_old = x2;        y2_old = y2;
                first_flag = FALSE;
                }
            }
        }
    while ((keys_button & Button1Mask) == Button1Mask);

    /* Over-draw the bounding rectangle one last time.
     */
    XDrawRectangle(gDisplay, theWindow, theGC, x_bl_corner_old, y_bl_corner_old, width_old, height_old);

    if (((y2-y1) == 0) && ((x2-x1) == 0))
        base_angle = 0.0;
    else
        base_angle = atan2((double )(y2-y1), (double )(x2-x1));
    length1 = x2 - x1;
    length2 = y2 - y1;


    /* Error check.
     */
    if ((ABS(length1) <= (double )gLineThickness) || (ABS(length2) <= (double )gLineThickness))
        return;         /* zero width or length ellipse.
                         * Don't draw it.
                         */


    /* Next we need to decide the angle of the ellipse.
     * Rotate the rectangle to be in line with the
     * pointer, until the user presses the left mouse button
     * (Button1) again.
     */
    angle = -1;
    angle_old = -1;
    x2_old = -1;  y2_old = -1;
    first_flag = TRUE;
    do
        {
        XQueryPointer(gDisplay, theWindow, &root, &child, &root_x, &root_y, &x2, &y2, &keys_button);

        /* Has the pointer moved?
         */
        if ((x2 != x2_old) || (y2 != y2_old))
            {
            if (((y2-y1) == 0) && ((x2-x1) == 0))
                angle = 0.0;
            else
                angle = atan2((double )(y2-y1), (double )(x2-x1)) - base_angle;
            angle = -angle;     /* Since y is indexed from the
                                 * top of the window to the bottom,
                                 */


            /* Over-draw the previous bounding rectangle.
             * We don't need to do this the first time
             * through this loop.
             */
            if (first_flag == FALSE)
                {
                XDrawLine(gDisplay, theWindow, theGC, x1, y1,
                    x1 + (int )(length1*cos(angle_old)),
                    y1 - (int )(length1*sin(angle_old)) );
                XDrawLine(gDisplay, theWindow, theGC, x1, y1,
                    x1 + (int )(length2*sin(angle_old)),
                    y1 + (int )(length2*cos(angle_old)) );
                XDrawLine(gDisplay, theWindow, theGC,
                    x1 + (int )(length1*cos(angle_old)),
                    y1 - (int )(length1*sin(angle_old)),
                    x1 + (int )(length1*cos(angle_old)+length2*sin(angle_old)),
                    y1 + (int )(-length1*sin(angle_old)+length2*cos(angle_old)));
                XDrawLine(gDisplay, theWindow, theGC,
                    x1 + (int )(length2*sin(angle_old)),
                    y1 + (int )(length2*cos(angle_old)),
                    x1 + (int )(length1*cos(angle_old)+length2*sin(angle_old)),
                    y1 + (int )(-length1*sin(angle_old)+length2*cos(angle_old)));
                }
            XDrawLine(gDisplay, theWindow, theGC, x1, y1,
                x1 + (int )(length1*cos(angle)),
                y1 - (int )(length1*sin(angle)) );
            XDrawLine(gDisplay, theWindow, theGC, x1, y1,
                x1 + (int )(length2*sin(angle)),
                y1 + (int )(length2*cos(angle)) );
            XDrawLine(gDisplay, theWindow, theGC,
                x1 + (int )(length1*cos(angle)),
                y1 - (int )(length1*sin(angle)),
                x1 + (int )(length1*cos(angle)+length2*sin(angle)),
                y1 + (int )(-length1*sin(angle)+length2*cos(angle)));
            XDrawLine(gDisplay, theWindow, theGC,
                x1 + (int )(length2*sin(angle)),
                y1 + (int )(length2*cos(angle)),
                x1 + (int )(length1*cos(angle)+length2*sin(angle)),
                y1 + (int )(-length1*sin(angle)+length2*cos(angle)));
            XFlush(gDisplay);


            angle_old = angle;
            x2_old = x2;  y2_old = y2;
            first_flag = FALSE;
            }
        }
    while ((keys_button & Button1Mask) != Button1Mask);



    /* Over-draw the bounding rectangle one last time
     * to remove it from the screen.
     */
    XDrawLine(gDisplay, theWindow, theGC, x1, y1,
        x1 + (int )(length1*cos(angle_old)),
        y1 - (int )(length1*sin(angle_old)) );
    XDrawLine(gDisplay, theWindow, theGC, x1, y1,
        x1 + (int )(length2*sin(angle_old)),
        y1 + (int )(length2*cos(angle_old)) );
    XDrawLine(gDisplay, theWindow, theGC,
        x1 + (int )(length1*cos(angle_old)),
        y1 - (int )(length1*sin(angle_old)),
        x1 + (int )(length1*cos(angle_old)+length2*sin(angle_old)),
        y1 + (int )(-length1*sin(angle_old)+length2*cos(angle_old)));
    XDrawLine(gDisplay, theWindow, theGC,
        x1 + (int )(length2*sin(angle_old)),
        y1 + (int )(length2*cos(angle_old)),
        x1 + (int )(length1*cos(angle_old)+length2*sin(angle_old)),
        y1 + (int )(-length1*sin(angle_old)+length2*cos(angle_old)));



    /* Pressing the left-hand mouse button (Button1)
     * will have generated a ButtonPress event.
     * We need to find and remove the event
     * from the event queue.
     * We do this be removing all ButtonPress events from the queue.
     */
    do
        {
        return_flag = XCheckTypedWindowEvent(gDisplay, theWindow, ButtonPress, &report);
        } while (return_flag == TRUE);



    /* Now draw the ellipse.
     * The ellipse is defined as all points [x,y] given by:
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
    r1 = length1/2;
    r2 = length2/2;
    x_centre = x1 + r1*cos(angle) + r2*sin(angle);
    y_centre = y1 - r1*sin(angle) + r2*cos(angle);
    /* The ellipse will be drawn as a sequence of
     * line segments.
     * incr indicates how much of the ellipse
     * each line segments will cover.
     */
    perimeter = M_PI* ( 3*(ABS(r1)+ABS(r2)) - sqrt( (ABS(r1)+3*ABS(r2))*(3*ABS(r1)+ABS(r2)) ) );


    /* Error check.
     */
    if (perimeter <= gLineThickness)
        return;         /* zero width or length ellipse.
                         * Don't draw it.
                         */


    incr = 40.0/perimeter;

    M[1][1] = r1*SQR(cos(angle)) + r2*SQR(sin(angle));
    M[1][2] = M[2][1] = (r2-r1)*sin(angle)*cos(angle);
    M[2][2] = r1*SQR(sin(angle)) + r2*SQR(cos(angle));

    /* Change the GXxor to GXcopy
     * since otherwise the XDrawPoint() below
     * sometimes draws over itself, cancelling itself out.
     */
    x1 = (int )(x_centre + M[1][1]*cos(0.0) + M[1][2]*sin(0.0));
    y1 = (int )(y_centre + M[2][1]*cos(0.0) + M[2][2]*sin(0.0));

    /* Draw the ellipse in black
     * (set color to black, line thickness to gLineThickness
     * and GC_function to GXcopy).
     */
    GC_values.function = GXcopy;
    theScreen = DefaultScreen(gDisplay);
    GC_values.foreground = BlackPixel( gDisplay, theScreen);
    XChangeGC( gDisplay, theGC, GCFunction | GCForeground, &GC_values );

    for (theta=incr; theta < (2*M_PI + incr); theta += incr)
        {
        x2 = (int )(x_centre + M[1][1]*cos(theta) + M[1][2]*sin(theta));
        y2 = (int )(y_centre + M[2][1]*cos(theta) + M[2][2]*sin(theta));
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
                        for (width=-gLineThickness/2; width < (gLineThickness+1)/2; width++)
                            {
                            set_pixel_value(aImage, aXImage, x, y+width, BLACK);
                            XDrawPoint(gDisplay, theWindow, theGC, x, y+width);
                            }
                        }
                    }
                else if (x1 > x2)
                    {
                    grad = ((double )(y1-y2)) / (double )(x1-x2);
                    for (x=x2; x <= x1; x++)
                        {
                        y = y2 + (int )(grad*(double )(x - x2));
                        for (width=-gLineThickness/2; width < (gLineThickness+1)/2; width++)
                            {
                            set_pixel_value(aImage, aXImage, x, y+width, BLACK);
                            XDrawPoint(gDisplay, theWindow, theGC, x, y+width);
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
                        for (width=-gLineThickness/2; width < (gLineThickness+1)/2; width++)
                            {
                            set_pixel_value(aImage, aXImage, x+width, y, BLACK);
                            XDrawPoint(gDisplay, theWindow, theGC, x+width, y);
                            }
                        }
                    }
                else if (y1 > y2)
                    {
                    grad = ((double )(x1-x2)) / (double )(y1-y2);
                    for (y=y2; y <= y1; y++)
                        {
                        x = x2 + (int )(grad*(double )(y - y2));
                        for (width=-gLineThickness/2; width < (gLineThickness+1)/2; width++)
                            {
                            set_pixel_value(aImage, aXImage, x+width, y, BLACK);
                            XDrawPoint(gDisplay, theWindow, theGC, x+width, y);
                            }
                        }
                    }
                }
            }   /* end of 'if ((x1 >= 0) && (x1 < aImage->x) &&...' */
        x1 = x2;
        y1 = y2;
        }
    XFlush(gDisplay);

    /* Change GC back to old values.
     */
    XChangeGC( gDisplay, theGC, GCLineWidth | GCFunction | GCForeground, &GC_old_values );

}   /* end of drag_out_an_ellipse() */



/* Open a window and display a message,
 * wait for user to hit left mouse button.
 */
void
display_message_and_wait(Window theWindow, int x, int y, int num_strings, ...)
{
    MESSAGE_WINDOW  *message_window;
    va_list     ap;
    int         char_count, max_char_count, longest, i;
    char        *string;
    char        *font_name = "fixed";
    int         direction, ascent, descent;
    XCharStruct overall;
    int         border_width = 4;
    int         theScreen;
    unsigned long   theBlackPixel, theWhitePixel;
    XGCValues       theGCValues;
    unsigned long   valuemask;
    XEvent  report;
    BOOLEAN finish_wait;
    MESSAGE_WINDOW  *alloc_message_window(void);
    void    write_message_window_strings(MESSAGE_WINDOW *message_window);
    void    free_message_window(MESSAGE_WINDOW *message_window);



    message_window = alloc_message_window();

    message_window->x = x;
    message_window->y = y;
    message_window->num_strings = num_strings;

    /* Allocate an array of pointers to the strings.
     */
    message_window->message = (char **)malloc(num_strings * sizeof(char *) );
    if (message_window->message == NULL)
        {
        fprintf(stderr, "malloc #1 failed in init_message_window_fields() in init_X_stuff.c\n");
        exit(1);
        }

    /* Read in the strings
     * and allocate memory for each.
     */
    longest = 0;
    max_char_count = 0;
    va_start(ap, num_strings);
    for (i=0; i < num_strings; i++)
        {
        (string = va_arg(ap, char *));
        char_count = (int )strlen(string);
        message_window->message[i] = (char *)malloc((char_count+1) * sizeof(char) );
        if (message_window->message[i] == NULL)
            {
            fprintf(stderr, "malloc #2 failed in init_message_window_fields() in init_X_stuff.c\n");
            exit(1);
            }
        strcpy(message_window->message[i], string);
        if (char_count > max_char_count)    /* Find the longest string */
            {
            longest = i;
            max_char_count = char_count;
            }
        }
    va_end(ap);


    /* Access font */
    message_window->font_info = XLoadQueryFont(gDisplay,font_name);

    if (message_window->font_info == NULL)
        {
        fprintf(stderr, "Can not open font %s in init_message_window_fields() in init_X_stuff.c\n", font_name);
        exit(1);
        }


    /* Determine the width of the window based
     * on the font size.
     */
    XTextExtents(message_window->font_info, message_window->message[longest],
                    max_char_count, &direction, &ascent, &descent, &overall);

    message_window->width = overall.width + 20;
    message_window->line_height = overall.ascent + overall.descent + 4;
    message_window->height = num_strings * message_window->line_height;
    message_window->border_width = border_width;



    theScreen = DefaultScreen(gDisplay);
    theBlackPixel = BlackPixel( gDisplay, theScreen);
    theWhitePixel = WhitePixel( gDisplay, theScreen);


    /* Create a window at position (x, y) in theWindow
     * theWindow is the Window in which the message_window->win appears.
     */
    message_window->win = XCreateSimpleWindow(gDisplay, theWindow, x, y, message_window->width, message_window->height, border_width, theBlackPixel, theWhitePixel);
    XSelectInput(gDisplay, message_window->win, ButtonPressMask |
                        ButtonReleaseMask | KeyPressMask |
                        ExposureMask | EnterWindowMask | LeaveWindowMask);


    /* Create a graphics context for (MESSAGE_WINDOW *)message_window.
     */
    valuemask = GCForeground | GCBackground;
    theGCValues.foreground = theBlackPixel;
    theGCValues.background = theWhitePixel;
    message_window->gc = XCreateGC( gDisplay, message_window->win, valuemask, &theGCValues );



    XMapWindow(gDisplay, message_window->win);


    /* Event Loop
     * Wait for the user to press the
     * left mouse button
     */
    finish_wait = FALSE;
    do {
        XNextEvent( gDisplay, &report );
        switch (report.type)
            {
            case Expose :
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
                        XFlush(gDisplay);
                        }
                    }
                break;
            case ButtonPress :
                if (report.xbutton.button == Button1)
                    finish_wait = TRUE;
                break;
            default :
                break;

            }   /* end of 'switch (report.type)' */
        }
    while (finish_wait == FALSE);


    XUnmapWindow(gDisplay, message_window->win);
    XFlush(gDisplay);

    free_message_window(message_window);

}   /* end of display_message_and_wait() */


void
color_pixels_for_this_chunk(int color_index, GAUS_LIST *chunk, XImage *aXImage)
{
    GAUS    *gaus;
    POINT   *point;
    int     x, y;
    void    set_XImage_pixel_value(int x, int y, int color, XImage *aXImage);

    if (chunk == NULL)
        return;

    for (gaus=chunk->head; gaus != NULL; gaus=gaus->next)
        for (point=gaus->point_list->head; point != NULL; point=point->next)
            {
            x = (int )point->x[1];
            y = (int )point->x[2];
            set_XImage_pixel_value(x, y, color_index, aXImage);
            }


}   /* end of color_pixels_for_this_chunk() */


void
display_label_of_chunk(int x, int y, char *label, XFontStruct *font_info, short width, int ascent, int descent, int color_index, Window theWindow, GC theGC)
{
    int         theScreen;
    unsigned long   theWhitePixel;
    int         char_count;


    theScreen = DefaultScreen(gDisplay);
    theWhitePixel = WhitePixel( gDisplay, theScreen);
    char_count = strlen(label);


    /* Set the appropriate font.
     */
    XSetFont(gDisplay, theGC, font_info->fid);


    /* Draw a label at the centre of the object.
     */
    XSetForeground(gDisplay, theGC, theWhitePixel);
    XFillRectangle(gDisplay, theWindow, theGC, x-(width/2)-1, y-ascent-1, width+2, ascent+descent+2);
    XSetForeground(gDisplay, theGC, gRandomColors[color_index].pixel);
    XDrawString(gDisplay, theWindow, theGC, x-(width/2), y, label, char_count);
    XFlush(gDisplay);

}   /* end of display_label_of_chunk() */



void
display_hough_line_segment(int x_start, int y_start, int x_end, int y_end, int width, int color_index, Window theWindow, GC theGC)
{
    XGCValues   values, old_values;


    /* Change the line width.
     * Make a copy of the old line width. We will restore it
     * at the end of the function.
     */
    XGetGCValues(gDisplay, theGC, GCLineWidth | GCForeground | GCFunction, &old_values);
    values.line_width = width;
    values.foreground = gRandomColors[color_index].pixel;
    values.function = GXcopy;
    XChangeGC( gDisplay, theGC, GCLineWidth | GCForeground | GCFunction, &values );


    XDrawLine(gDisplay, theWindow, theGC, x_start, y_start, x_end, y_end);


    /* Change GC back to old line width and old foreground color
     */
    XChangeGC( gDisplay, theGC, GCLineWidth | GCForeground | GCFunction, &old_values );
    XFlush(gDisplay);

}   /* end of display_hough_line_segment() */



/* This function is almost exactly the same as
 * draw_ellipse() in Xgraphics.c
 * but the ellipse is drawn more smoothly
 * i.e. it is drawn as a collection of
 * connected line segments, but the line segments
 * used here are smaller than in draw_ellipse().
 * I only bothered doing this so that it would
 * look a bit better at the final output
 * of the hough transform for ellipse finding.
 */
void
draw_hough_ellipse( Window theWindow, GC theGC, double x, double y, double r1, double r2, double theta )
/*
 *      x, y;       -- centre of ellipse
 *      r1, r2;     -- major/minor axis
 *      theta;      -- angle
 */

{
    /* We have made the arrays [3][3] and the vectors [3]
     * This is so that the arrays can be indexed [1..2][1..2]
     * and the vectors indexed [1..2]
     */
    double D_neg_sqrt[3][3];
    double E[3][3], E_inv[3][3];
    double X[3], X_old[3], X_original[3];
    double T[3];
    double temp[3][3], temp2[3][3];
    double t;
    XGCValues   values, old_values;


    /* Change GCCapStyle to CapRound.
     * Copy the old value. We will restore it
     * at the end of the function.
     */
    XGetGCValues(gDisplay, theGC, GCCapStyle, &old_values);
    values.cap_style = CapRound;
    XChangeGC( gDisplay, theGC, GCCapStyle, &values );
    /* -- The negative square root of the diagonal
     *    matrix containing the eigenvalues.
     */
    D_neg_sqrt[1][1] = r1;   D_neg_sqrt[1][2] = 0;
    D_neg_sqrt[2][1] = 0;    D_neg_sqrt[2][2] = r2;

    /* -- Eigenbasis of ellipse */
    E[1][1] = cos(theta);    E[1][2] = -sin(theta);
    E[2][1] = sin(theta);    E[2][2] = cos(theta);

    /* -- Inverse of E
     * equals transpose of E since E is orthogonal.
     */
    E_inv[1][1] = E[1][1];
    E_inv[1][2] = E[2][1];
    E_inv[2][1] = E[1][2];
    E_inv[2][2] = E[2][2];

    /* -- Set up values used in calculations in loop
     * Calculate E * D^-0.5 * E^-1
     */
    temp[1][1] = E[1][1]*D_neg_sqrt[1][1] + E[1][2]*D_neg_sqrt[2][1];
    temp[1][2] = E[1][1]*D_neg_sqrt[1][2] + E[1][2]*D_neg_sqrt[2][2];
    temp[2][1] = E[2][1]*D_neg_sqrt[1][1] + E[2][2]*D_neg_sqrt[2][1];
    temp[2][2] = E[2][1]*D_neg_sqrt[1][2] + E[2][2]*D_neg_sqrt[2][2];

    temp2[1][1] = temp[1][1]*E_inv[1][1] + temp[1][2]*E_inv[2][1];
    temp2[1][2] = temp[1][1]*E_inv[1][2] + temp[1][2]*E_inv[2][2];
    temp2[2][1] = temp[2][1]*E_inv[1][1] + temp[2][2]*E_inv[2][1];
    temp2[2][2] = temp[2][1]*E_inv[1][2] + temp[2][2]*E_inv[2][2];



    T[1] = cos(0.0);
    T[2] = sin(0.0);

    X_old[1] = temp2[1][1]*T[1] + temp2[1][2]*T[2];
    X_old[2] = temp2[2][1]*T[1] + temp2[2][2]*T[2];

    X_original[1] = X_old[1] = x + X_old[1];
    X_original[2] = X_old[2] = y + X_old[2];

    for (t=0.0; t <=(2.0*M_PI); t+= 0.1)
        {
        T[1] = cos(t);
        T[2] = sin(t);
        X[1] = temp2[1][1]*T[1] + temp2[1][2]*T[2];
        X[2] = temp2[2][1]*T[1] + temp2[2][2]*T[2];

        /* -- (x, y) is the centre of the ellipse.
         */
        X[1] = x + X[1];
        X[2] = y + X[2];

        XDrawLine( gDisplay, theWindow, theGC,
                    (int )X_old[1], (int )X_old[2],
                    (int )X[1], (int )X[2] );

        X_old[1] = X[1];
        X_old[2] = X[2];
        }

    XDrawLine( gDisplay, theWindow, theGC,
                (int )X_original[1], (int )X_original[2],
                (int )X[1], (int )X[2] );

    /* Change GC back to old cap style.
     */
    XChangeGC( gDisplay, theGC, GCCapStyle, &old_values );

    XFlush( gDisplay );

}   /* -- draw_hough_ellipse() */



DISPLAY_OBJECT *
alloc_display_object(void)
{
    DISPLAY_OBJECT  *d;


    d = (DISPLAY_OBJECT *)malloc(sizeof(DISPLAY_OBJECT));
    if (d == NULL)
        {
        fprintf(stderr, "malloc failed in alloc_display_object() in Xdraw_func.c\n");
        exit(1);
        }

    d->next = NULL;


    return(d);

}       /* end of alloc_display_object() */



void
free_display_object(DISPLAY_OBJECT *d)
{
    if (d != NULL)
        {
        if (d->type == upwrite)
            {
            if (d->obj.upwrite.label != NULL)
                free(d->obj.upwrite.label);
            if (d->obj.upwrite.font_info != NULL)
                XUnloadFont(gDisplay, d->obj.upwrite.font_info->fid);
            }

        free(d);
        }

}   /* end of free_display_object() */



DISPLAY_OBJECT_LIST *
alloc_display_object_list(void)
{
    DISPLAY_OBJECT_LIST *display_object_list;


    display_object_list = (DISPLAY_OBJECT_LIST *)malloc( sizeof(DISPLAY_OBJECT_LIST) );

    if (display_object_list == NULL)
        {
        fprintf(stderr, "malloc failed in alloc_display_object_list() in Xdraw_func.c\n");
        exit(1);
        }

    display_object_list->head = NULL;
    display_object_list->tail = NULL;
    display_object_list->num_elements = 0;


    return(display_object_list);

}   /* end of alloc_display_object_list() */



DISPLAY_OBJECT_LIST *
put_in_linked_list_of_display_object(DISPLAY_OBJECT_LIST *display_object_list, DISPLAY_OBJECT *d)
{
    DISPLAY_OBJECT_LIST *alloc_display_object_list(void);


    if (display_object_list == NULL)
        display_object_list = alloc_display_object_list();

    if (display_object_list->head == NULL)
        {
        display_object_list->head = d;
        display_object_list->tail = d;
        display_object_list->num_elements = 1;
        }
    else
        {
        /* Add d to the list. */
        display_object_list->tail->next = d;

        /* Update the tail. */
        display_object_list->tail = d;

        /* Update the number of elements. */
        (display_object_list->num_elements)++;
        }

    d->next = NULL;

    return(display_object_list);


}   /* end of put_in_linked_list_of_display_object() */



void
free_linked_list_of_display_object(DISPLAY_OBJECT_LIST *display_object_list)
{
    DISPLAY_OBJECT    *d1, *d2;
    void                free_display_object(DISPLAY_OBJECT *d);

    if (display_object_list == NULL)
        return;

    d1 = display_object_list->head;
    while (d1 != NULL)
        {
        d2 = d1;
        d1 = d1->next;
        free_display_object(d2);
        }

    free(display_object_list);


}   /* end of free_linked_list_of_display_object() */




DISPLAY_OBJECT_LIST *
remove_object_type_from_linked_list_of_display_object(DISPLAY_OBJECT_LIST *display_object_list, OBJECT_TYPE type)
{
    DISPLAY_OBJECT  *d, *d_to_be_removed;
    DISPLAY_OBJECT_LIST *remove_specific_object_from_linked_list_of_display_object(DISPLAY_OBJECT_LIST *display_object_list, DISPLAY_OBJECT *d_to_be_removed);


    /* Error check.
     */
    if ((display_object_list == NULL) || (display_object_list->head == NULL))
        return(display_object_list);


    d = display_object_list->head;
    while (d != NULL)
        {
        if (d->type == type)        /* remove it */
            {
            d_to_be_removed = d;
            d=d->next;
            display_object_list = remove_specific_object_from_linked_list_of_display_object(display_object_list, d_to_be_removed);
            }
        else
            d=d->next;              /* go on to next object */
        }   /* end of 'while (d != NULL)' */


    return(display_object_list);

}   /* end of remove_from_linked_list_of_display_object() */



DISPLAY_OBJECT_LIST *
remove_specific_object_from_linked_list_of_display_object(DISPLAY_OBJECT_LIST *display_object_list, DISPLAY_OBJECT *d_to_be_removed)
{
    DISPLAY_OBJECT  *d;
    void    free_display_object(DISPLAY_OBJECT *d);
    void    free_linked_list_of_display_object(DISPLAY_OBJECT_LIST *display_object_list);

    if (d_to_be_removed != NULL)
        {

        /* Is the linked list already empty?
         */
        if ((display_object_list==NULL) || (display_object_list->head==NULL))
            {
            fprintf(stderr, "Tried to remove a DISPLAY_OBJECT from an empty linked list of DISPLAY_OBJECT\n");
            fprintf(stderr, "in remove_specific_object_from_linked_list_of_display_object() in Xdraw_func.c.\n");
            exit(1);
            }

        if (display_object_list->num_elements == 1)
            {
            /* If there is only one element in the linked list.
             */

            if (d_to_be_removed == display_object_list->head)
                {
                /* Free the list. */
                free_linked_list_of_display_object(display_object_list);
                display_object_list = NULL;
                }
            else
                {
                fprintf(stderr, "Couldn't find DISPLAY_OBJECT to be removed from linked list\n");
                fprintf(stderr, "in remove_specific_object_from_linked_list_of_display_object() in Xdraw_func.c\n");
                exit(1);
                }
            }
        else if (d_to_be_removed == display_object_list->head)
            {
            /* If we are removing the first element
             * and there is more than one element in the list.
             */
            display_object_list->head = display_object_list->head->next;
            free_display_object(d_to_be_removed);
            (display_object_list->num_elements)--;
            }
        else    /* Remove an element that is not the head. */
            {
            /* Find the previous element in the linked list.
             * (i.e. d->next == d_to_be_removed)
             */
            for (d=display_object_list->head; d->next != NULL; d=d->next)
                if (d->next == d_to_be_removed)
                    break;
            if (d->next != NULL)
                {
                /* Remove it from the linked list.
                 */
                d->next = d_to_be_removed->next;
                (display_object_list->num_elements)--;

                /* If we removed the last element,
                 * then update the tail.
                 */
                if (display_object_list->tail == d_to_be_removed)
                    display_object_list->tail = d;

                free_display_object(d_to_be_removed);
                }
            else    /* couldn't find it in list */
                {
                fprintf(stderr, "Couldn't find DISPLAY_OBJECT to be removed from linked list\n");
                fprintf(stderr, "in remove_specific_object_from_linked_list_of_display_object() in Xdraw_func.c.c\n");
                exit(1);
                }
            }   /* end of 'else  Remove an element that is...' */

        }   /* end of 'if (d_to_be_removed != NULL)' */


    return(display_object_list);

}   /* end of remove_from_linked_list_of_display_object() */



void
display_number_training_examples(Window MainWindow, GC MainGC)
{
    int         theScreen;
    unsigned long   theWhitePixel;
    char        str[500];
    XFontStruct *font_info; /* info on the font used. */
    int         x, y, direction, ascent, descent;
    XCharStruct overall;


    /* Display number of training objects.
     */
    theScreen = DefaultScreen(gDisplay);
    theWhitePixel = WhitePixel( gDisplay, theScreen);
    if (gTrainingPointList == NULL)
        sprintf(str, "0 training examples.");
    else if (gTrainingPointList->num_elements == 1)
        sprintf(str, "1 training example.");
    else
        sprintf(str, "%d training examples.", gTrainingPointList->num_elements);

    /* Set the appropriate font.
     */
    font_info = XLoadQueryFont(gDisplay, DISPLAY_FONT_NAME);
    if (font_info == NULL)
        {
        fprintf(stderr, "Can not open font %s in callback_Undo() in menu_callback_functions.c\n", DISPLAY_FONT_NAME);
        exit(1);
        }
    XSetFont(gDisplay, MainGC, font_info->fid);

    /* Determine width, ascent and descent
     * of string (in pixels).
     */
    XTextExtents(font_info, str, strlen(str), &direction, &ascent, &descent, &overall);

    x = 10;
    y = 20 + ascent;
    XSetForeground(gDisplay, MainGC, theWhitePixel);
    XFillRectangle(gDisplay, MainWindow, MainGC, x-1, y-ascent-1, overall.width+2, ascent+descent+2);
    /* Text in red
     */
    XSetForeground(gDisplay, MainGC, gRandomColors[0].pixel);
    XDrawString(gDisplay, MainWindow, MainGC, x, y, str, strlen(str));
    if (font_info != NULL)
        XFreeFont(gDisplay, font_info);
    XFlush(gDisplay);


}   /* end of display_number_training_examples() */
