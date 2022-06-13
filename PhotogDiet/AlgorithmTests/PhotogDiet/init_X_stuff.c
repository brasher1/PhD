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
#include <string.h>
#include <strings.h>
#include <stdarg.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/Xmd.h>
#include "../include/upwrite.h"



void    setup_window_gc(IMAGE *aImage, char win_name[], Window *win, GC *gc);
Window  open_Window(int x, int y, int width, int height, GC *gc, char win_name[]);
void    setup_colormap(Window theWindow);
XColor  *set_up_colormap_with_random_colors(int num_colors, Window theWindow);
XColor  *set_up_colormap_with_grey_levels(int desired_num_greyscale_levels);
XImage  *setup_XImage(IMAGE *aImage, Window theWindow);
MENU    *alloc_menu(int num_options);
void    free_menu(MENU *menu);
void    init_menu_window_gc_and_font(Window MainWindow, MENU *menu);
int     menu_option(Window theWindow, MENU *menu);
void    paint_menu_pane(int option, MENU *menu, int mode);
void    open_input_prompt(int x, int y, char *message, Window MainWindow, INPUT_PROMPT **prompt);
INPUT_PROMPT    *alloc_input_prompt(void);
void    free_input_prompt(INPUT_PROMPT *prompt);
void    init_input_prompt_fields(INPUT_PROMPT *prompt, Window MainWindow, int x, int y, char *message);
double  prompt_user_for_input(INPUT_PROMPT *prompt);
char    *prompt_user_for_alphanumeric_input(INPUT_PROMPT *prompt);
MESSAGE_WINDOW  *alloc_message_window(void);
void    free_message_window(MESSAGE_WINDOW *message_window);
void    init_message_window_fields(MESSAGE_WINDOW *message_window, Window MainWindow, int x, int y, int num_strings, ...);
void    quitX(XImage *aXImage, Window MainWindow, GC MainGC, PARENT_MENU *ParentMenu[], int num_parent_menus);
PARENT_MENU *alloc_parent_menu(int num_menus);
void    free_parent_menu(PARENT_MENU *parent_menu);
void    init_parent_menu_window_gc_and_font(Window MainWindow, PARENT_MENU *parent_menu);
int     parent_menu_option(Window theWindow, PARENT_MENU *parent_menu);
void    paint_parent_menu_pane(int option, PARENT_MENU *parent_menu, int mode);
void    setup_menu_off_parent_menu(PARENT_MENU *parent_menu, Window aWindow, int parent_menu_num, int num_options, ...);
int     setup_menus(Window MainWindow, PARENT_MENU *(*parent_menu[]));


void
setup_window_gc(IMAGE *aImage, char win_name[], Window *win, GC *gc)
{
    Window      open_Window(int x, int y, int width, int height, GC *gc, char win_name[]);
    XWindowAttributes   theWindowAttributes;
    VisualID    theVisualID;
    XVisualInfo vinfo_template, *visual_info;
    int         num_visuals_matched;


    if (aImage == NULL)
        {
        fprintf(stderr, "arguement `IMAGE *aImage' was NULL in setup_window_and_gc() in init_X_stuff.c\n");
        exit(1);
        }


    gDisplay = XOpenDisplay(NULL);
    if (gDisplay == NULL)
        {
        fprintf(stderr, "Could not open connection to X on display %s in setup_window_and_gc() in init_X_stuff.c\n", XDisplayName(NULL) );
        exit(1);
        }


    /* Open Window
     */
    *win = open_Window(0, 0, aImage->x, aImage->y, gc, win_name);

    /* Set gVisualDepth.
     * Find the depth of the visual associated with the window we just
     * opened.
     * i.e. how many bits are used to specify the color
     * of each pixel.
     */
    XGetWindowAttributes( gDisplay, *win, &theWindowAttributes );
    theVisualID = XVisualIDFromVisual(theWindowAttributes.visual);
    vinfo_template.visualid = theVisualID;
    visual_info = XGetVisualInfo(gDisplay, VisualIDMask, &vinfo_template, &num_visuals_matched);
    if (visual_info == NULL)
        {
        fprintf(stderr, "XGetVisualInfo() failed in setup_window_gc() in init_X_stuff.c\n");
        exit(1);
        }
    gVisualDepth = visual_info->depth;
    XFree((caddr_t )visual_info);       /* don't need this anymore. */

    if ((gVisualDepth != 8) &&
        (gVisualDepth != 12) && (gVisualDepth != 15) && (gVisualDepth != 16) &&
        (gVisualDepth != 24) && (gVisualDepth != 32))
        {
        fprintf(stderr, "Unsupported visual depth: %d\n", gVisualDepth);
        fprintf(stderr, "recog suports the following depths: 8, 12, 15, 16, 24, 32\n");
        exit(1);
        }


}   /* end of setup_window_gc() */




Window
open_Window(int x, int y, int width, int height, GC *gc, char win_name[])
{
    int     theScreen;
    int     theDepth;
    Visual  *theVisual;
    XSetWindowAttributes    theWindowAttributes;
    XSizeHints      theSizeHints;
    unsigned long   theWindowMask;
    Window          theWindow;
    XWMHints        theWMHints;
    XTextProperty   title;
    unsigned long   valuemask;  /* Used when creating the gc. */
    XGCValues       theGCValues;


    theScreen = DefaultScreen(gDisplay);
    theDepth = DefaultDepth(gDisplay, theScreen);
    theVisual = DefaultVisual(gDisplay, theScreen);
    theWindowAttributes.border_pixel = BlackPixel( gDisplay, theScreen);
    theWindowAttributes.background_pixel = WhitePixel( gDisplay, theScreen);
    theWindowAttributes.override_redirect = False;

    theWindowMask = CWBackPixel | CWBorderPixel | CWOverrideRedirect;

    theWindow = XCreateWindow( gDisplay,
                    RootWindow( gDisplay, theScreen),
                    x, y,           /* upper-left hand corner of window */
                    width, height,
                    2,              /* border width */
                    theDepth,
                    InputOutput,
                    theVisual,
                    theWindowMask,
                    &theWindowAttributes );

    theWMHints.initial_state = NormalState;
    theWMHints.flags    = StateHint;

    XSetWMHints( gDisplay, theWindow, &theWMHints );

    theSizeHints.flags  = PPosition | PSize;
    theSizeHints.x      = x;
    theSizeHints.y      = y;
    theSizeHints.width  = width;
    theSizeHints.height = height;

    XSetNormalHints( gDisplay, theWindow, &theSizeHints );

    XStringListToTextProperty(&win_name, 1, &title);
    XSetWMName( gDisplay, theWindow, &title);


    valuemask = GCForeground | GCBackground;
    theGCValues.foreground = BlackPixel( gDisplay, theScreen);
    theGCValues.background = WhitePixel( gDisplay, theScreen);
    *gc = XCreateGC( gDisplay, theWindow, valuemask, &theGCValues );
    if (*gc == 0)
        {
        XDestroyWindow( gDisplay, theWindow);
        fprintf(stderr, "Could not create a Graphics Context in open_Window() in init_X_stuff.c\n");
        exit(1);
        }

    XSelectInput( gDisplay, theWindow,
            ExposureMask | KeyPressMask | ButtonPressMask | ButtonReleaseMask);
    XMapWindow( gDisplay, theWindow );

    return(theWindow );

}   /* end of open_Window() */



void
setup_colormap(Window theWindow)
{
    XColor  *set_up_colormap_with_random_colors(int num_colors, Window theWindow);
    XColor  *set_up_colormap_with_grey_levels(int desired_num_greyscale_levels);


    gColormap = DefaultColormap( gDisplay, DefaultScreen(gDisplay) );

    gRandomColors = set_up_colormap_with_random_colors(NUM_RANDOM_COLORS, theWindow);
    gGreyValues = set_up_colormap_with_grey_levels(32);  /* Black and white image. */


}   /* end of setup_colormap() */



/* Returns an array `XColor x_color[]' which
 * gives the colormap entries for `num_colors' random colors,
 * all of which are guaranteed to not be white.
 * It first tries to allocate some random colors in the
 * colormap. If this fails, then x_Color[] entries are set
 * to already existing entries in the colormap that aren't
 * white.
 */
XColor *
set_up_colormap_with_random_colors(int num_colors, Window theWindow)
{
    /* Set up colormap with some random colors
     */

    XColor  *x_color;
    int     i;
    unsigned long   j;
    unsigned short  red, green, blue;
    double  largest_value;
    XWindowAttributes   theWindowAttributes;
    VisualID    theVisualID;
    XVisualInfo vinfo_template, *visual_info;
    int         num_visuals_matched, colormap_size;
    BOOLEAN     is_it_white_pixel;




    /* Calculate largest value of an unsigned short
     */
    largest_value = pow(2.0, 8*(double )sizeof(unsigned short)) - 1;


    x_color = (XColor *)malloc( num_colors * sizeof(XColor) );
    if (x_color == (XColor *)NULL)
        {
        fprintf(stderr, "malloc failed in set_up_colormap_with_random_colors() in init_X_stuff.c\n");
        exit(1);
        }


    /* Set the first 3 colors as red, green and blue.
     */
    if (num_colors > 0)
        {
        x_color[0].red = (unsigned short)largest_value;
        x_color[0].green = 0;
        x_color[0].blue = 0;
        x_color[0].flags = DoRed | DoGreen | DoBlue;
        }
    if (num_colors > 1)
        {
        x_color[1].red = 0;
        x_color[1].green = (unsigned short)largest_value;
        x_color[1].blue = 0;
        x_color[1].flags = DoRed | DoGreen | DoBlue;
        }
    if (num_colors > 2)
        {
        x_color[2].red = 0;
        x_color[2].green = 0;
        x_color[2].blue = (unsigned short)largest_value;
        x_color[2].flags = DoRed | DoGreen | DoBlue;
        }

    /* Set up the rest as random colors
     */
    for (i=3; i < num_colors; i++)
        {
        /* largest_value is equal to maximum
         * value for an unsigned short.
         */
        do
            {
            /* R, G, B values for random color.
             * Make sure that it isn't white.
             */
            red = (unsigned short )(drand48() * largest_value);
            green = (unsigned short )(drand48() * largest_value);
            blue = (unsigned short )(drand48() * largest_value);
            }
        while ((red==largest_value) && (green==largest_value) && (blue==largest_value));

        x_color[i].red = red;
        x_color[i].green = green;
        x_color[i].blue = blue;
        x_color[i].flags = DoRed | DoGreen | DoBlue;
        }

    /* Note: in the explanation below, 'colormap entry'
     * and 'pixel value' are used interchangeably.
     *
     * For read-only colormaps:
     * x_color[].pixel is set to the pixel value with
     * RGB values closest to those specified in the
     * red, green, blue fields.
     *
     * For read-write colormaps:
     * If a colormap entry already exist which matches
     * the RGB values, then x_color[].pixel
     * is set to that value. If no match is found but an
     * unallocated colormap cell exists, then it is
     * allocated to the specified RGB values and
     * x_color[].pixel is set to this value.
     * If no match is found and no unallocated colormap cells
     * are available then XAllocColor returns 0.
     * i.e. For read-write colormaps, the closest match
     * is not returned. In this case, we set x_color.pixel
     * to an arbitrary colorcell entry - the value j.
     * We check that the pixel is not white and that
     * j is not greater than the number of entries in
     * the colormap.
     */

    /* We need to find the number of colormap entries
     * for the visual associated with theWindow.
     * We are not supposed to access the fields of the
     * Visual (they are supposd to be opaque to programmers),
     * but we can access the fields of XVisualInfo structures.
     * So, first we find the Visual that we are using.
     * Then we find its I.D. number.
     * Then we say "Find all visuals with this I.D. number."
     * by using XGetVisualInfo().
     * This, of course, returns an XVisualInfo structure
     * corresponding to our visual.
     * Then we are allowed to access the colormap_size field
     * of the XVisualInfo structure.
     */

    XGetWindowAttributes( gDisplay, theWindow, &theWindowAttributes );
    theVisualID = XVisualIDFromVisual(theWindowAttributes.visual);
    vinfo_template.visualid = theVisualID;
    visual_info = XGetVisualInfo(gDisplay, VisualIDMask, &vinfo_template, &num_visuals_matched);
    if (visual_info == NULL)
        {
        fprintf(stderr, "XGetVisualInfo() failed in set_up_colormap_with_random_colors() in init_X_stuff.c\n");
        exit(1);
        }
    colormap_size = visual_info->colormap_size;
    XFree((caddr_t )visual_info);       /* don't need this anymore. */
    for (i=0; i < num_colors; i++)
        {
        if (XAllocColor(gDisplay, gColormap, &(x_color[i]) ) == 0)
            {
            /* If XAllocColor() fails, then set x_color[].pixel
             * to the arbitrary colormap entry j.
             */
            j = (unsigned long )i%colormap_size;    /* j is not larger
                                                     * than the number
                                                     * of colormap entries.
                                                     */
            /* Check that the color isn't white.
             */
            do
                {
                x_color[i].pixel = j;
                XQueryColor(gDisplay, gColormap, &(x_color[i]));
                if ((x_color[i].red == largest_value) && (x_color[i].green == largest_value) && (x_color[i].blue == largest_value))
                    {
                    is_it_white_pixel = TRUE;
                    j++;
                    if (j == colormap_size)
                        j = 0;
                    }
                else
                    is_it_white_pixel = FALSE;

                } while (is_it_white_pixel == TRUE);

            }   /* end of 'if (XAllocColor...' */
        }   /* end of 'for (i=0; i < num_colors; i++)' */

    return(x_color);

}   /* end of set_up_colormap_with_random_colors() */



XColor *
set_up_colormap_with_grey_levels(int desired_num_greyscale_levels)
{
    XColor  *x_color;
    int     i;
    unsigned long   j;
    unsigned short  red, green, blue;
    double  largest_value;
    int     SUCCESS_flag;
    int     theScreen;
    unsigned long   theBlackPixel, theWhitePixel;

    /* Calculate largest value of an unsigned short
     */
    largest_value = pow(2.0, 8*(double )sizeof(unsigned short)) - 1;

    gNumGreyLevels = desired_num_greyscale_levels;

    do
        {
        SUCCESS_flag = TRUE;
        x_color = (XColor *)malloc( gNumGreyLevels * sizeof(XColor) );
        if (x_color == (XColor *)NULL)
            {
            fprintf(stderr, "malloc failed in set_up_colormap_with_greyscale() in init_X_stuff.c\n");
            exit(1);
            }


        /* Set up the rgb values for different shades of grey.
         */
        for (i=0; i < gNumGreyLevels; i++)
            {
            /* largest_value is equal to maximum
             * value for an unsigned short.
             */
            red = (unsigned short )(((double )i)*(double )largest_value / (double )(gNumGreyLevels-1));
            green = (unsigned short )(((double )i)*(double )largest_value / (double )(gNumGreyLevels-1));
            blue= (unsigned short )(((double )i)*(double )largest_value / (double )(gNumGreyLevels-1));

            x_color[i].red = red;
            x_color[i].green = green;
            x_color[i].blue = blue;
            x_color[i].flags = DoRed | DoGreen | DoBlue;
            }

        /* Note: in the explanation below, 'colormap entry'
         * and 'pixel value' are used interchangeably.
         *
         * For read-only colormaps:
         * x_color[].pixel is set to the pixel value with
         * RGB values closest to those specified in the
         * red, green, blue fields.
         *
         * For read-write colormaps:
         * If a colormap entry already exist which matches
         * the RGB values, then x_color[].pixel
         * is set to that value. If no match is found but an
         * unallocated colormap cell exists, then it is
         * allocated to the specified RGB values and
         * x_color[].pixel is set to this value.
         * If no match is found and no unallocated colormap cells
         * are available then XAllocColor returns 0.
         * In this case, we try again, but with fewer levels
         * of grey.
         */

        for (i=0; i < gNumGreyLevels; i++)
            {
            if (XAllocColor(gDisplay, gColormap, &(x_color[i]) ) == 0)
                {
                /* If XAllocColor() fails, then we will try
                 * again with less grey levels.
                 * First we must free the colors that we did allocate.
                 */
                SUCCESS_flag = FALSE;
                for (j=0; j < i; j++)
                    XFreeColors(gDisplay, gColormap, &(x_color[j].pixel), 1, 0);
                free(x_color);
                gNumGreyLevels--;
                break;

                }   /* end of 'if (XAllocColor...' */
            }   /* end of 'for (i=0; i < num_colors; i++)' */

    } while ((SUCCESS_flag == FALSE) && (gNumGreyLevels > 2));


    /* Black and White */
    if (gNumGreyLevels == 2)
        {
        x_color = (XColor *)malloc( gNumGreyLevels * sizeof(XColor) );
        if (x_color == (XColor *)NULL)
            {
            fprintf(stderr, "malloc failed in set_up_colormap_with_greyscale() in init_X_stuff.c\n");
            exit(1);
            }


        theScreen = DefaultScreen(gDisplay);

        theBlackPixel = BlackPixel( gDisplay, theScreen);
        x_color[0].red = (unsigned short )0;
        x_color[0].green = (unsigned short )0;
        x_color[0].blue = (unsigned short )0;
        x_color[0].flags = DoRed | DoGreen | DoBlue;
        x_color[0].pixel = theBlackPixel;


        theWhitePixel = WhitePixel( gDisplay, theScreen);
        x_color[1].red = (unsigned short )largest_value;
        x_color[1].green = (unsigned short )largest_value;
        x_color[1].blue = (unsigned short )largest_value;
        x_color[1].flags = DoRed | DoGreen | DoBlue;
        x_color[1].pixel = theWhitePixel;
        }


    return(x_color);

}   /* end of set_up_colormap_with_greyscale() */



XImage *
setup_XImage(IMAGE *aImage, Window theWindow)
{
    XImage  *aXImage;
    XWindowAttributes   theWindowAttributes;
    union {
        INT8    *d8;            /* Use X Windows machine-dependent */
        INT16   *d16;           /* data types.                     */
        INT32   *d32;           /* You can find them in Xmd.h      */
    } data;

    /* Allocate memory for the aXImage->data.
     */
    switch (gVisualDepth)
        {
        case 8:
            data.d8 = (INT8 *)malloc(aImage->x * aImage->y * sizeof(INT8));
            if (data.d8 == (INT8 *)NULL)
                {
                fprintf(stderr, "malloc failed in setup_XImage() in init_X_stuff.c for visual depth of 8\n");
                exit(1);
                }
            XGetWindowAttributes( gDisplay, theWindow, &theWindowAttributes );
            aXImage = XCreateImage( gDisplay, theWindowAttributes.visual,
                            gVisualDepth, ZPixmap, 0, (char *)data.d8,
                            aImage->x, aImage->y, 8, 0 );
            break;

        case 12:
        case 15:
        case 16:
            data.d16 = (INT16 *)malloc(aImage->x * aImage->y * sizeof(INT16));
            if (data.d16 == (INT16 *)NULL)
                {
                fprintf(stderr, "malloc failed in setup_XImage() in init_X_stuff.c for visual depth of 16\n");
                exit(1);
                }
            XGetWindowAttributes( gDisplay, theWindow, &theWindowAttributes );
            aXImage = XCreateImage( gDisplay, theWindowAttributes.visual,
                            gVisualDepth, ZPixmap, 0, (char *)data.d16,
                            aImage->x, aImage->y, 16, 0 );
            break;

        case 24:
        case 32:
            data.d32 = (INT32 *)malloc(aImage->x * aImage->y * sizeof(INT32));
            if (data.d32 == (INT32 *)NULL)
                {
                fprintf(stderr, "malloc failed in setup_XImage() in init_X_stuff.c for visual depth of 32\n");
                exit(1);
                }
            XGetWindowAttributes( gDisplay, theWindow, &theWindowAttributes );
            aXImage = XCreateImage( gDisplay, theWindowAttributes.visual,
                            gVisualDepth, ZPixmap, 0, (char *)data.d32,
                            aImage->x, aImage->y, 32, 0 );
            break;
        default:
            fprintf(stderr, "Invalid visual depth of %d in setup_XImage() in init_X_stuff.c\n", gVisualDepth);
           exit(1);
        }       /* end of 'switch (gVisualDepth)' */


    return(aXImage);

}   /* end of setup_XImage() */




/* Allocate all memory required for a MENU
 * with 'num_options' options.
 * set the num_options field to 'num_options'.
 * Return a pointer to the MENU.
 */
MENU *
alloc_menu(int num_options)
{
    MENU    *menu;
    int     i;

    menu = (MENU *)malloc( sizeof(MENU) );
    if (menu == NULL)
        {
        fprintf(stderr, "malloc #1 failed in alloc_menu() in init_X_stuff.c\n");
        exit(1);
        }

    menu->num_options = num_options;

    /* Allocate space for the labels.
     */
    menu->label = (char **)malloc(menu->num_options * sizeof(char *) );
    if (menu->label == NULL)
        {
        fprintf(stderr, "malloc #2 failed in alloc_menu() in init_X_stuff.c\n");
        exit(1);
        }

    for (i=0; i < menu->num_options; i++)
        {
        menu->label[i] = (char *)malloc( 256*sizeof(char) );
        if (menu->label[i] == NULL)
            {
            fprintf(stderr, "malloc #3 failed in alloc_menu() in init_X_stuff.c\n");
            exit(1);
            }

        /* Initialise the label as an empty string.
         */
        menu->label[i][0] = (char )NULL;
        }

    /* Allocate space for the array of Window,
     * one for each menu option.
     */
    menu->pane = (Window *)malloc( num_options * sizeof(Window) );

    menu->font_info = NULL;

    /* Allocate space for the array of callback functions
     * i.e. a function to be executed when a menu option
     * is selected.
     * There is one function for each menu option.
     */
    menu->callback = (func_ptr *)malloc(num_options * sizeof(func_ptr) );

    return(menu);

}   /* end of alloc_menu() */



void
free_menu(MENU *menu)
{
    int     i;

    if (menu != NULL)
        {
        for (i=0; i < menu->num_options; i++)
            if (menu->label[i] != NULL)
                free(menu->label[i]);
        if (menu->label != NULL)
            free(menu->label);


        XFreeGC(gDisplay, menu->gc);

        /* Destroy Window and sub-windows
         * i.e. This call destroys bkgd_win and panes[].
         */
        XDestroyWindow(gDisplay, menu->bkgd_win);

        /* Free memory.
         */
        if (menu->pane != NULL)
            free(menu->pane);

        if (menu->callback != NULL)
            free(menu->callback);

        if (menu->font_info != NULL)
            XFreeFont(gDisplay, menu->font_info);
        free(menu);
        }

}   /* end of free_menu() */



/* Initialise the bkgd_win, gc, pane[] and font_info
 * fields of (MENU *)menu.
 * Assumes:
 *      menu->num_options, menu->x, menu->y have been set.
 *      menu->label[] contains the appropriate menu labels.
 * MainWindow is the Window in which the menu appears.
 */
void
init_menu_window_gc_and_font(Window MainWindow, MENU *menu)
{
    char        *font_name = "fixed";
    int         theScreen;
    unsigned long   theBlackPixel, theWhitePixel;
    int         char_count;     /* length of longest menu label */
    int         longest;        /* index of longest menu label */
    int         direction, ascent, descent;
    XCharStruct overall;
    int         winindex, i;
    int         border_width = 4;
    XGCValues   theGCValues;
    unsigned long   valuemask;





    theScreen = DefaultScreen(gDisplay);
    theBlackPixel = BlackPixel( gDisplay, theScreen);
    theWhitePixel = WhitePixel( gDisplay, theScreen);



    /* Access font */
    menu->font_info = XLoadQueryFont(gDisplay,font_name);

    if (menu->font_info == NULL)
        {
        fprintf(stderr, "Can not open font %s in init_menu_window_gc_and_font() in init_X_stuff.c\n", font_name);
        exit(1);
        }


    /* Find longest menu option.
     */
    longest = 0;
    char_count = 0;
    for (i=0; i < menu->num_options; i++)
        if (((int )strlen(menu->label[i])) > char_count)
            {
            longest = i;
            char_count = (int )strlen(menu->label[i]);
            }

    /* Determine the width of each menu pane based
     * on the font size.
     */
    XTextExtents(menu->font_info, menu->label[longest], char_count,
                    &direction, &ascent, &descent, &overall);

    menu->width = overall.width + 20;
    menu->border_width = border_width;

    /* Determine the height of each menu width
     * based in the font size.
     * We need to search through the menu options
     * looking for the one needing the most height/depth.
     * (i.e. an option with a 'd','t' or a 'p','q'
     * needs more height/depth).
     */
    menu->pane_height = 0;
    for (i=0; i < menu->num_options; i++)
        {
        XTextExtents(menu->font_info, menu->label[i],
                            (int )strlen(menu->label[i]), &direction,
                            &ascent, &descent, &overall);
        if (menu->pane_height < (overall.ascent + overall.descent + 8))
            menu->pane_height = overall.ascent + overall.descent + 8;
        }
    menu->height = menu->num_options * menu->pane_height;




    /* Create opaque window at position (menu->x,menu->y) in MainWindow
     */
    menu->bkgd_win = XCreateSimpleWindow(gDisplay, MainWindow, menu->x, menu->y, menu->width, menu->height, border_width, theBlackPixel, theWhitePixel);

    /* Create the a window for each menu option. */
    for (winindex = 0; winindex < menu->num_options; winindex++)
        {
        menu->pane[winindex] = XCreateSimpleWindow(gDisplay, menu->bkgd_win, 0,
                    winindex*menu->height/menu->num_options, menu->width,
                    menu->pane_height, 1, theBlackPixel, theWhitePixel);
        XSelectInput(gDisplay, menu->pane[winindex], ButtonPressMask |
                        ButtonReleaseMask | KeyPressMask |
                        ExposureMask | EnterWindowMask | LeaveWindowMask);
        }

    /* These don't appear until parent (*menu->bkgd_win) is mapped */
    XMapSubwindows(gDisplay, menu->bkgd_win);


    /* Create a graphics context for the menu.
     */
    valuemask = GCForeground | GCBackground;
    theGCValues.foreground = theBlackPixel;
    theGCValues.background = theWhitePixel;
    menu->gc = XCreateGC( gDisplay, menu->bkgd_win, valuemask, &theGCValues );


}   /* end of init_menu_window_gc_and_font() */



/* Finds whether theWindow is one of the panes
 * of (MENU *)menu.
 * Returns a number in the range [0, menu->num_options)
 * if it is.
 * Returns -1 otherwise.
 */
int
menu_option(Window theWindow, MENU *menu) 
{
    int i;

    for (i=0; i < menu->num_options; i++)
        if (theWindow == menu->pane[i])
            return(i);

    return(-1);

}       /* end of menu_option() */



/* Draw one of the menu options.
 * `option' is the option to be drawn.
 * `mode' is set to either NORMAL or HIGHLIGHT.
 * Usually, a menu option is HIGHLIGHTed when it has
 * been selected.
 */
void
paint_menu_pane(int option, MENU *menu, int mode)
{
    int     x, y;
    int     theScreen;
    unsigned long   theBlackPixel, theWhitePixel;



    theScreen = DefaultScreen(gDisplay);
    theBlackPixel = BlackPixel( gDisplay, theScreen);
    theWhitePixel = WhitePixel( gDisplay, theScreen);

    if (mode == HIGHLIGHT)
        {
        XSetWindowBackground(gDisplay, menu->pane[option], theBlackPixel);
        XSetForeground(gDisplay, menu->gc, theWhitePixel);
        }
    else if (mode == NORMAL)
        {
        XSetWindowBackground(gDisplay, menu->pane[option], theWhitePixel);
        XSetForeground(gDisplay, menu->gc, theBlackPixel );
        }
    else
        {
        fprintf(stderr, "Invalid value for arguement `mode' in function paint_menu_pane() in init_X_stuff.c\n");
        exit(1);
        }

    /* clearing repaints the background */
    XClearWindow(gDisplay, menu->pane[option]);

    /* Set the appropriate font.
     */
    XSetFont(gDisplay, menu->gc, menu->font_info->fid);

    x = 10;
    y = menu->font_info->max_bounds.ascent;

    XDrawString(gDisplay, menu->pane[option], menu->gc, x, y,
                menu->label[option], (int )strlen( menu->label[option]) ); 

    XFlush(gDisplay);


}       /* end of paint_menu_pane() */



/* Open a Window with 2 subwindows.
 * The first will contain the string in (char )message[].
 * The second allows the user to type in it.
 *
 * The length of (char )message[] decides the width of the
 * window.
 */
void
open_input_prompt(int x, int y, char *message, Window MainWindow, INPUT_PROMPT **prompt)
{
    INPUT_PROMPT    *alloc_input_prompt(void);
    void            init_input_prompt_fields(INPUT_PROMPT *prompt, Window MainWindow, int x, int y, char *message);


    (*prompt) = alloc_input_prompt();

    init_input_prompt_fields((*prompt), MainWindow, x, y, message);

}   /* end of open_input_prompt() */



INPUT_PROMPT *
alloc_input_prompt(void)
{
    INPUT_PROMPT    *prompt;


    prompt = (INPUT_PROMPT *)malloc( sizeof(INPUT_PROMPT) );

    if (prompt == NULL)
        {
        fprintf(stderr, "malloc failed in alloc_input_prompt() in init_X_stuff.c\n");
        exit(1);
        }

    prompt->message = NULL;
    prompt->font_info = NULL;

    return(prompt);

}   /* end of alloc_input_prompt() */



void
free_input_prompt(INPUT_PROMPT *prompt)
{
    if (prompt != NULL)
        {
        if (prompt->message != NULL)
            free(prompt->message);

        XFreeGC(gDisplay, prompt->gc);

        /* Destroy Window and sub-windows
         */
        XDestroyWindow(gDisplay, prompt->bkgd_win);

        if (prompt->font_info != NULL)
            XFreeFont(gDisplay, prompt->font_info);
        free(prompt);
        }

}   /* end of free_input_prompt() */



void
init_input_prompt_fields(INPUT_PROMPT *prompt, Window MainWindow, int x, int y, char *message)
{
    int         char_count;
    char        *font_name = "fixed";
    int         direction, ascent, descent;
    XCharStruct overall;
    int         border_width = 4;
    int         theScreen;
    unsigned long   theBlackPixel, theWhitePixel;
    XGCValues       theGCValues;
    unsigned long   valuemask;


    prompt->x = x;
    prompt->y = y;
    char_count = strlen(message);
                                        /* +1 for the NULL character. */
    prompt->message = (char *)malloc((char_count+1) * sizeof(char) );
    strcpy(prompt->message, message);


    /* Access font */
    prompt->font_info = XLoadQueryFont(gDisplay,font_name);

    if (prompt->font_info == NULL)
        {
        fprintf(stderr, "Can not open font %s in init_input_prompt_fields() in init_X_stuff.c\n", font_name);
        exit(1);
        }


    /* Determine the width of the window and sub-windows based
     * on the font size.
     */
    XTextExtents(prompt->font_info, prompt->message, char_count,
                    &direction, &ascent, &descent, &overall);

    prompt->width = overall.width + 20;
    prompt->pane_height = overall.ascent + overall.descent + 4;
    prompt->height = 2 * prompt->pane_height;   /* 2 sub-windows */
    prompt->border_width = border_width;



    theScreen = DefaultScreen(gDisplay);
    theBlackPixel = BlackPixel( gDisplay, theScreen);
    theWhitePixel = WhitePixel( gDisplay, theScreen);


    /* Create opaque window at position (prompt->x,prompt->y) in MainWindow
     * MainWindow is the Window in which the prompt window appears.
     */
    prompt->bkgd_win = XCreateSimpleWindow(gDisplay, MainWindow, prompt->x, prompt->y, prompt->width, prompt->height, border_width, theBlackPixel, theWhitePixel);

    /* Create the sub-windows. */
    prompt->message_win = XCreateSimpleWindow(gDisplay, prompt->bkgd_win,
                    0, 0, prompt->width, prompt->pane_height, 1,
                    theBlackPixel, theWhitePixel);
    XSelectInput(gDisplay, prompt->message_win, ButtonPressMask |
                        ButtonReleaseMask | KeyPressMask |
                        ExposureMask | EnterWindowMask | LeaveWindowMask);

    prompt->input_win = XCreateSimpleWindow(gDisplay, prompt->bkgd_win,
                    0, prompt->pane_height, prompt->width, prompt->pane_height,
                    1, theBlackPixel, theWhitePixel);
    XSelectInput(gDisplay, prompt->input_win, ButtonPressMask |
                        ButtonReleaseMask | KeyPressMask |
                        ExposureMask | EnterWindowMask | LeaveWindowMask);

    /* These don't appear until parent (*prompt->bkgd_win) is mapped */
    XMapSubwindows(gDisplay, prompt->bkgd_win);



    /* Create a graphics context for (INPUT_PROMPT *)prompt.
     */
    valuemask = GCForeground | GCBackground;
    theGCValues.foreground = theBlackPixel;
    theGCValues.background = theWhitePixel;
    prompt->gc = XCreateGC( gDisplay, prompt->bkgd_win, valuemask, &theGCValues );

}   /* end of init_input_prompt_fields() */



/* Allows the user to enter a non-negative number as input.
 * Returns the value that the user entered.
 */
double
prompt_user_for_input(INPUT_PROMPT *prompt)
{
    XEvent  report;
    int     x, y;
    char    *valid_characters;
    double  input_value;
    char    input_string[256], buffer[256];
    int     input_index, bufsize = 256;
    KeySym  keysym;
    BOOLEAN end_of_input = FALSE;
    BOOLEAN decimal_point_already_exists = FALSE;
    BOOLEAN valid_character(char c, char *valid_characters);
    void    write_string_in_window(Window win, GC gc, char string[], XFontStruct *font_info);

    valid_characters = "0123456789";
    input_string[0] = '\0';
    input_index = 0;


    XMapWindow( gDisplay, prompt->bkgd_win );

    /* Set the appropriate font.
     */
    XSetFont(gDisplay, prompt->gc, prompt->font_info->fid);

    do
        {
        /* This is a small XEvent Loop to catch the letters
         * that the user types in.
         */
        XNextEvent( gDisplay, &report );
        switch (report.type)
            {
            case Expose :
                if (report.xany.window == prompt->message_win)
                    {
                    /* clearing repaints the background */
                    XClearWindow(gDisplay, prompt->message_win);

                    x = 10;
                    y = prompt->font_info->max_bounds.ascent;

                    /* the string length is necessary because strings for
                     * XDrawString may not be NULL terminated
                     */
                    XDrawString(gDisplay, prompt->message_win, prompt->gc,
                                    x, y, prompt->message,
                                    (int )strlen(prompt->message) ); 

                    XFlush(gDisplay);
                    }
                else if (report.xany.window == prompt->input_win)
                    {
                    /* Re-draw the characters typed in so far.
                     */
                    write_string_in_window(prompt->input_win, prompt->gc, input_string, prompt->font_info);
                    }
                break;
            case KeyPress :
                XLookupString((XKeyEvent *)&report, buffer, bufsize, &keysym, NULL);
                /* If the character is a valid numeral
                 */
                if (valid_character(*buffer, valid_characters) == TRUE)
                    {
                    input_string[input_index] = *buffer;
                    input_string[++input_index] = '\0';
                    write_string_in_window(prompt->input_win, prompt->gc, input_string, prompt->font_info);
                    }
                else if (*buffer == '.')
                    {
                    /* Check if we already have a decimal point
                     */
                    if (decimal_point_already_exists == FALSE)
                        {
                        input_string[input_index] = '.';
                        input_string[++input_index] = '\0';
                        write_string_in_window(prompt->input_win, prompt->gc, input_string, prompt->font_info);
                        decimal_point_already_exists = TRUE;
                        }
                    }
                else if ((keysym == XK_BackSpace) || (keysym == XK_Delete))
                    {
                    /* Check that there are characters to delete.
                     */
                    if (input_index > 0)
                        {
                        /* If last character is decimal point.
                         */
                        if (input_string[input_index-1] == '.')
                            decimal_point_already_exists = FALSE;

                        input_string[--input_index] = '\0';
                        write_string_in_window(prompt->input_win, prompt->gc, input_string, prompt->font_info);
                        }
                    }
                else if (keysym == XK_Escape)
                    {
                    /* They`ve changed their mind
                     * and don't want to change res_radius after all.
                     */
                    XUnmapWindow(gDisplay, prompt->bkgd_win);
                    return((double )-1);    /* Cancel input */
                    }
                else if (keysym == XK_Return)
                    {
                    /* end of input
                     */
                    end_of_input = TRUE;
                    }
                break;
            }
        } while (end_of_input == FALSE);

    /* If they just hit return (without entering a number)
     * then return -1
     * This means that the resolution radius will not be changed.
     */
    if (input_index == 0)
        input_value = (double )-1;
    else
        {
        /* Convert the string the user typed
         * into a number.
         */
        input_value = strtod(input_string, (char **)NULL );
        }

    XUnmapWindow(gDisplay, prompt->bkgd_win);


    return(input_value);


}   /* end of prompt_user_for_input() */



/* Allows the user to enter alphnumeric input.
 * Returns the value that the user entered.
 */
char *
prompt_user_for_alphanumeric_input(INPUT_PROMPT *prompt)
{
    XEvent  report;
    int     x, y;
    char    *valid_characters;
    char    *input_string, buffer[256];
    int     input_index, bufsize = 256;
    KeySym  keysym;
    BOOLEAN end_of_input = FALSE;
    BOOLEAN valid_character(char c, char *valid_characters);
    void    write_string_in_window(Window win, GC gc, char string[], XFontStruct *font_info);

    valid_characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789._-+=";


    /* Allow up to 499 characters in the input.
     * 499 is an arbitrary number.
     */
    input_string = (char *)malloc(500*sizeof(char) );
    if (input_string == NULL)
        {
        fprintf(stderr, "malloc failed in prompt_user_for_alphanumeric_input() in init_X_stuff.c\n");
        exit(1);
        }

    input_string[0] = '\0';
    input_index = 0;


    XMapWindow( gDisplay, prompt->bkgd_win );

    /* Set the appropriate font.
     */
    XSetFont(gDisplay, prompt->gc, prompt->font_info->fid);


    do
        {
        /* This is a small XEvent Loop to catch the letters
         * that the user types in.
         */
        XNextEvent( gDisplay, &report );
        switch (report.type)
            {
            case Expose :
                if (report.xany.window == prompt->message_win)
                    {
                    /* clearing repaints the background */
                    XClearWindow(gDisplay, prompt->message_win);

                    x = 10;
                    y = prompt->font_info->max_bounds.ascent;

                    /* the string length is necessary because strings for
                     * XDrawString may not be NULL terminated
                     */
                    XDrawString(gDisplay, prompt->message_win, prompt->gc,
                                    x, y, prompt->message,
                                    (int )strlen(prompt->message) ); 

                    XFlush(gDisplay);
                    }
                else if (report.xany.window == prompt->input_win)
                    {
                    /* Re-draw the characters typed in so far.
                     */
                    write_string_in_window(prompt->input_win, prompt->gc, input_string, prompt->font_info);
                    }
                break;
            case KeyPress :
                XLookupString((XKeyEvent *)&report, buffer, bufsize, &keysym, NULL);
                /* If the character is a valid alpha-numeric character
                 */
                if (valid_character(*buffer, valid_characters) == TRUE)
                    {
                    input_string[input_index] = *buffer;
                    input_string[++input_index] = '\0';
                    write_string_in_window(prompt->input_win, prompt->gc, input_string, prompt->font_info);
                    }
                else if ((keysym == XK_BackSpace) || (keysym == XK_Delete))
                    {
                    /* Check that there are characters to delete.
                     */
                    if (input_index > 0)
                        {
                        input_string[--input_index] = '\0';
                        write_string_in_window(prompt->input_win, prompt->gc, input_string, prompt->font_info);
                        }
                    }
                else if (keysym == XK_Escape)
                    {
                    /* They`ve changed their mind.
                     */
                    XUnmapWindow(gDisplay, prompt->bkgd_win);
                    free(input_string);
                    return((char *)NULL);   /* Cancel input */
                    }
                else if (keysym == XK_Return)
                    {
                    /* end of input
                     */
                    end_of_input = TRUE;
                    }
                break;
            }
        } while (end_of_input == FALSE);


    XUnmapWindow(gDisplay, prompt->bkgd_win);


    return(input_string);


}   /* end of prompt_user_for_alphanumeric_input() */



void
wait_for_button_press(Window MainWindow, int x, int y)
{
    MESSAGE_WINDOW  *message_window;
    XEvent  report;
    BOOLEAN finish_wait;
    MESSAGE_WINDOW  *alloc_message_window(void);
    void    init_message_window_fields(MESSAGE_WINDOW *message_window, Window MainWindow, int x, int y, int num_strings, ...);
    void    write_message_window_strings(MESSAGE_WINDOW *message_window);
    void    free_message_window(MESSAGE_WINDOW *message_window);


    /* Open a window which asks the user to hit the left mouse button.
     */
    message_window = alloc_message_window();
    init_message_window_fields(message_window, MainWindow, x, y, 1, "Hit Left Mouse Button");
    XMapWindow(gDisplay, message_window->win);


    /* Event Loop
     * Wait for the user to press the
     * left mouse button
     */
    finish_wait = FALSE;
    do
        {
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

}   /* end of wait_for_button_press() */



MESSAGE_WINDOW *
alloc_message_window(void)
{
    MESSAGE_WINDOW  *message_window;


    message_window = (MESSAGE_WINDOW *)malloc( sizeof(MESSAGE_WINDOW) );

    if (message_window == NULL)
        {
        fprintf(stderr, "malloc failed in alloc_message_window() in init_X_stuff.c\n");
        exit(1);
        }

    message_window->num_strings = 0;
    message_window->message = NULL;
    message_window->font_info = NULL;

    return(message_window);

}   /* end of alloc_message_window() */



void
free_message_window(MESSAGE_WINDOW *message_window)
{
    int     i;
    if (message_window != NULL)
        {
        if (message_window->message != NULL)
            {
            for (i=0; i < message_window->num_strings; i++)
                free(message_window->message[i]);
            free(message_window->message);
            }

        XFreeGC(gDisplay, message_window->gc);

        /* Destroy Window and sub-windows
         */
        XDestroyWindow(gDisplay, message_window->win);

        if (message_window->font_info != NULL)
            XFreeFont(gDisplay, message_window->font_info);
        free(message_window);
        }

}   /* end of free_message_window() */



void
init_message_window_fields(MESSAGE_WINDOW *message_window, Window MainWindow, int x, int y, int num_strings, ...)
{
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
        char_count = strlen(string);
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


    /* Create a window at position (x, y) in MainWindow
     * MainWindow is the Window in which the message_window->win appears.
     */
    message_window->win = XCreateSimpleWindow(gDisplay, MainWindow, x, y, message_window->width, message_window->height, border_width, theBlackPixel, theWhitePixel);
    XSelectInput(gDisplay, message_window->win, ButtonPressMask |
                        ButtonReleaseMask | KeyPressMask |
                        ExposureMask | EnterWindowMask | LeaveWindowMask);


    /* Create a graphics context for (MESSAGE_WINDOW *)message_window.
     */
    valuemask = GCForeground | GCBackground;
    theGCValues.foreground = theBlackPixel;
    theGCValues.background = theWhitePixel;
    message_window->gc = XCreateGC( gDisplay, message_window->win, valuemask, &theGCValues );

}   /* end of init_message_window_fields() */



void
quitX(XImage *aXImage, Window MainWindow, GC MainGC, PARENT_MENU *ParentMenu[], int num_parent_menus)
{
    int     i;
    void    free_parent_menu(PARENT_MENU *menu);


    /* Free the XImage structure and the associated data.
     */
    if (ParentMenu != NULL)
        {
        for (i=0; i < num_parent_menus; i++)
            if (ParentMenu[i] != NULL)
                free_parent_menu( ParentMenu[i] );
        free(ParentMenu);
        }

    XDestroyImage(aXImage);
    XFreeGC(gDisplay, MainGC);
    XDestroyWindow(gDisplay, MainWindow);
    XFreeColormap(gDisplay, gColormap);
    XCloseDisplay(gDisplay);

}   /* end of quitX() */



/* Allocate all memory required for a PARENT_MENU
 * with 'num_menus' options.
 * set the num_menus field to 'num_menus'.
 * Return a pointer to the PARENT_MENU.
 */
PARENT_MENU *
alloc_parent_menu(int num_menus)
{
    PARENT_MENU    *parent_menu;
    int     i;

    parent_menu = (PARENT_MENU *)malloc( sizeof(PARENT_MENU) );
    if (parent_menu == NULL)
        {
        fprintf(stderr, "malloc #1 failed in alloc_parent_menu() in init_X_stuff.c\n");
        exit(1);
        }

    parent_menu->num_menus = num_menus;

    /* Allocate space for the labels.
     */
    parent_menu->label = (char **)malloc(num_menus * sizeof(char *) );
    if (parent_menu->label == NULL)
        {
        fprintf(stderr, "malloc #2 failed in alloc_parent_menu() in init_X_stuff.c\n");
        exit(1);
        }

    for (i=0; i < num_menus; i++)
        {
        parent_menu->label[i] = (char *)malloc( 256*sizeof(char) );
        if (parent_menu->label[i] == NULL)
            {
            fprintf(stderr, "malloc #3 failed in alloc_parent_menu() in init_X_stuff.c\n");
            exit(1);
            }

        /* Initialise the label as an empty string.
         */
        parent_menu->label[i][0] = (char )NULL;
        }

    /* Allocate space for the array of Window,
     * one for each parent_menu option.
     */
    parent_menu->pane = (Window *)malloc( num_menus * sizeof(Window) );

    parent_menu->font_info = NULL;

    /* Allocate space for the array of pointers to MENU.
     * i.e. the menu associated with parent_menu option.
     */
    parent_menu->menus = (MENU **)malloc(num_menus * sizeof(MENU *) );
    for (i=0; i < num_menus; i++)
        parent_menu->menus[i] = (MENU *)NULL;

    return(parent_menu);

}   /* end of alloc_parent_menu() */



void
free_parent_menu(PARENT_MENU *parent_menu)
{
    int     i;
    void    free_menu(MENU *menu);

    if (parent_menu != NULL)
        {
        for (i=0; i < parent_menu->num_menus; i++)
            if (parent_menu->label[i] != NULL)
                free(parent_menu->label[i]);
        if (parent_menu->label != NULL)
            free(parent_menu->label);


        XFreeGC(gDisplay, parent_menu->gc);

        /* Destroy Window and sub-windows
         * i.e. This call destroys bkgd_win and panes[].
         */
        XDestroyWindow(gDisplay, parent_menu->bkgd_win);

        /* Free memory.
         */
        for (i=0; i < parent_menu->num_menus; i++)
            {
            if (parent_menu->menus[i] != NULL)
                free_menu(parent_menu->menus[i]);
            }

        if (parent_menu->pane != NULL)
            free(parent_menu->pane);

        if (parent_menu->menus != NULL)
            free(parent_menu->menus);

        if (parent_menu->font_info != NULL)
            XFreeFont(gDisplay, parent_menu->font_info);
        free(parent_menu);
        }

}   /* end of free_parent_menu() */



/* Initialise the bkgd_win, gc, pane[] and font_info
 * fields of (PARENT_MENU *)parent_menu.
 * Assumes:
 *      parent_menu->num_menus, parent_menu->x, parent_menu->y have been set.
 *      parent_menu->label[] contains the appropriate parent_menu labels.
 * MainWindow is the Window in which the parent_menu appears.
 */
void
init_parent_menu_window_gc_and_font(Window MainWindow, PARENT_MENU *parent_menu)
{
    char        *font_name = "fixed";
    int         theScreen;
    unsigned long   theBlackPixel, theWhitePixel;
    int         char_count;     /* length of longest parent_menu label */
    int         longest;        /* index of longest parent_menu label */
    int         direction, ascent, descent;
    XCharStruct overall;
    int         winindex, i;
    int         border_width = 4;
    XGCValues   theGCValues;
    unsigned long   valuemask;





    theScreen = DefaultScreen(gDisplay);
    theBlackPixel = BlackPixel( gDisplay, theScreen);
    theWhitePixel = WhitePixel( gDisplay, theScreen);



    /* Access font */
    parent_menu->font_info = XLoadQueryFont(gDisplay,font_name);

    if (parent_menu->font_info == NULL)
        {
        fprintf(stderr, "Can not open font %s in init_menu_window_gc_and_font() in init_X_stuff.c\n", font_name);
        exit(1);
        }


    /* Find longest label option.
     */
    longest = 0;
    char_count = 0;
    for (i=0; i < parent_menu->num_menus; i++)
        if (((int )strlen(parent_menu->label[i])) > char_count)
            {
            longest = i;
            char_count = (int )strlen(parent_menu->label[i]);
            }

    /* Determine the width of each parent_menu pane based
     * on the font size.
     */
    XTextExtents(parent_menu->font_info, parent_menu->label[longest],
                    char_count, &direction, &ascent, &descent, &overall);

    parent_menu->width = overall.width + 20;
    parent_menu->border_width = border_width;

    /* Determine the height of each parent_menu width
     * based in the font size.
     * We need to search through the parent_menu options
     * looking for the one needing the most height/depth.
     * (i.e. an option with a 'd','t' or a 'p','q'
     * needs more height/depth).
     */
    parent_menu->pane_height = 0;
    for (i=0; i < parent_menu->num_menus; i++)
        {
        XTextExtents(parent_menu->font_info, parent_menu->label[i],
                            (int )strlen(parent_menu->label[i]), &direction,
                            &ascent, &descent, &overall);
        if (parent_menu->pane_height < (overall.ascent + overall.descent + 8))
            parent_menu->pane_height = overall.ascent + overall.descent + 8;
        }
    parent_menu->height = parent_menu->num_menus * parent_menu->pane_height;




    /* Create opaque window at position (parent_menu->x,parent_menu->y)
     * in MainWindow
     */
    parent_menu->bkgd_win = XCreateSimpleWindow(gDisplay, MainWindow, parent_menu->x, parent_menu->y, parent_menu->width, parent_menu->height, border_width, theBlackPixel, theWhitePixel);

    /* Create a window for each label. */
    for (winindex = 0; winindex < parent_menu->num_menus; winindex++)
        {
        parent_menu->pane[winindex] = XCreateSimpleWindow(gDisplay,
                    parent_menu->bkgd_win, 0,
                    winindex*parent_menu->height/parent_menu->num_menus,
                    parent_menu->width, parent_menu->pane_height, 1,
                    theBlackPixel, theWhitePixel);
        XSelectInput(gDisplay, parent_menu->pane[winindex], ButtonPressMask |
                        ButtonReleaseMask | KeyPressMask |
                        ExposureMask | EnterWindowMask | LeaveWindowMask);
        }

    /* These don't appear until background window (*parent_menu->bkgd_win)
     * is mapped
     */
    XMapSubwindows(gDisplay, parent_menu->bkgd_win);


    /* Create a graphics context for the parent_menu.
     */
    valuemask = GCForeground | GCBackground;
    theGCValues.foreground = theBlackPixel;
    theGCValues.background = theWhitePixel;
    parent_menu->gc = XCreateGC( gDisplay, parent_menu->bkgd_win, valuemask, &theGCValues );


}   /* end of init_parent_menu_window_gc_and_font() */



/* Finds whether theWindow is one of the panes
 * of (MENU *)menu.
 * Returns a number in the range [0, parent_menu->num_menus)
 * if it is.
 * Returns -1 otherwise.
 */
int
parent_menu_option(Window theWindow, PARENT_MENU *parent_menu) 
{
    int i;

    for (i=0; i < parent_menu->num_menus; i++)
        if (theWindow == parent_menu->pane[i])
            return(i);

    return(-1);

}       /* end of parent_menu_option() */



/* Draw one of the parent_menu options.
 * `option' is the option to be drawn.
 * `mode' is set to either NORMAL or HIGHLIGHT.
 * Usually, a parent_menu option is HIGHLIGHTed when it has
 * been selected.
 */
void
paint_parent_menu_pane(int option, PARENT_MENU *parent_menu, int mode)
{
    int     x, y;
    int     theScreen;
    unsigned long   theBlackPixel, theWhitePixel;



    theScreen = DefaultScreen(gDisplay);
    theBlackPixel = BlackPixel( gDisplay, theScreen);
    theWhitePixel = WhitePixel( gDisplay, theScreen);

    if (mode == HIGHLIGHT)
        {
        XSetWindowBackground(gDisplay, parent_menu->pane[option],theBlackPixel);
        XSetForeground(gDisplay, parent_menu->gc, theWhitePixel);
        }
    else if (mode == NORMAL)
        {
        XSetWindowBackground(gDisplay, parent_menu->pane[option],theWhitePixel);
        XSetForeground(gDisplay, parent_menu->gc, theBlackPixel );
        }
    else
        {
        fprintf(stderr, "Invalid value for arguement `mode' in function paint_parent_menu_pane() in init_X_stuff.c\n");
        exit(1);
        }

    /* clearing repaints the background */
    XClearWindow(gDisplay, parent_menu->pane[option]);

    /* Set the appropriate font.
     */
    XSetFont(gDisplay, parent_menu->gc, parent_menu->font_info->fid);

    x = 10;
    y = parent_menu->font_info->max_bounds.ascent;

    XDrawString(gDisplay, parent_menu->pane[option], parent_menu->gc, x, y,
                parent_menu->label[option],
                (int )strlen( parent_menu->label[option]) ); 

    XFlush(gDisplay);


}       /* end of paint_parent_menu_pane() */



/* num_menus, string, pointer to function
 */
void
setup_menu_off_parent_menu(PARENT_MENU *parent_menu, Window aWindow,
                            int parent_menu_num, int num_options, ...)
{
    va_list     ap;
    int         i;
    MENU        *menu;
    char        *string;
    func_ptr    menu_callback_function;
    void    init_menu_window_gc_and_font(Window MainWindow, MENU *menu);

    va_start(ap, num_options);
    menu = alloc_menu(num_options);

    /* Locate menu near parent_menu option */
    menu->x = parent_menu->x + 20;
    menu->y = parent_menu->y +
                    (parent_menu_num * parent_menu->pane_height) +
                    (parent_menu->pane_height/2);

    /* Initialise menu labels and callback functions.
     */
    for (i=0; i < num_options; i++)
        {
        (string = va_arg(ap, char *));
        (menu_callback_function = va_arg(ap, func_ptr));

        strcpy( menu->label[i], string);
        menu->callback[i] = menu_callback_function;
        }
    va_end(ap);

    init_menu_window_gc_and_font(aWindow, menu);

    parent_menu->menus[parent_menu_num] = menu;


}   /* end of setup_menu_off_parent_menu() */



/* Allocate memory and initialise the X11 stuff required for
 * the menus.
 * The menus will appear within (Window )MainWindow.
 *
 * At completion, the arguements of type MENU
 * will contain the appropriate menu option labels.
 */
int
setup_menus(Window MainWindow, PARENT_MENU *(*parentmenu[]))
{
    PARENT_MENU *alloc_parent_menu(int num_options);
    void        init_parent_menu_window_gc_and_font(Window MainWindow, PARENT_MENU *parent_menu);
    void    setup_menu_off_parent_menu(PARENT_MENU *parent_menu, Window aWindow, int parent_menu_num, int num_options, ...);
    int callback_to_be_written(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu);
    int callback_DrawCircle(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu);
    int callback_DrawEllipse(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu);
    int     callback_ChangeResolutionRadius(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu);
    int callback_ClearImage(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu);
    int callback_RemoveAnalysis(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu);
    int callback_AddSpeckleNoise(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu);
    int callback_Quit(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu);
    int callback_Chunk(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu);
    int callback_Recognise(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu);
    int callback_Train(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu);
    int callback_Learn(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu);
    int callback_Undo(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu);
    int callback_HelpGeneral(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu);
    int callback_HelpUpWrite(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu);
    int callback_HelpTrain(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu);
    int callback_SHTLines(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu);
    int callback_SHTCircles(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu);
    int callback_SHTEllipses(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu);
    int callback_SHTHelp(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu);
    int callback_RHTLines(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu);
    int callback_RHTCircles(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu);
    int callback_RHTEllipses(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu);
    int callback_RHTHelp(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu);
    int callback_PHTLines(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu);
    int callback_PHTCircles(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu);
    int callback_PHTEllipses(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu);
    int callback_PHTHelp(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu);
    int callback_HHTLines(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu);
    int callback_HHTHelp(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu);
    int callback_to_be_written(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu);



/*      Allocate space for 3 parent menus.     */
    *parentmenu = (PARENT_MENU **)malloc(3 * sizeof(PARENT_MENU *) );



/*      GENERAL IMAGE STUFF                       */
/*      Set up first parent menu with 4 options.  */
    (*parentmenu)[0] = alloc_parent_menu(4);
    (*parentmenu)[0]->x = 0;        /* Put menu in top left hand corner */
    (*parentmenu)[0]->y = 0;


    /* Initialise menu labels and callback functions.
     */
    strcpy( (*parentmenu)[0]->label[0], "Drawing");
    strcpy( (*parentmenu)[0]->label[1], "Image Stuff");
    strcpy( (*parentmenu)[0]->label[2], "Help");
    strcpy( (*parentmenu)[0]->label[3], "Quit");
    init_parent_menu_window_gc_and_font(MainWindow, (*parentmenu)[0]);

    setup_menu_off_parent_menu( (*parentmenu)[0], MainWindow, 0, 2,
                                    "Draw Circle", callback_DrawCircle,
                                    "Draw Ellipse", callback_DrawEllipse);

    setup_menu_off_parent_menu( (*parentmenu)[0], MainWindow, 1, 3,
                                    "Clear Image", callback_ClearImage,
                                    "Remove Analysis", callback_RemoveAnalysis,
                                    "Add Speckle Noise", callback_AddSpeckleNoise);

    setup_menu_off_parent_menu( (*parentmenu)[0], MainWindow, 2, 1,
                                    "Help", callback_HelpGeneral);

    setup_menu_off_parent_menu( (*parentmenu)[0], MainWindow, 3, 1,
                                    "Quit", callback_Quit);






    /* HOUGH STUFF */
    /* set up second parent menu 4 options.
     */
    (*parentmenu)[1] = alloc_parent_menu(4);

    /* Put second parent menu to left of first parent menu.
     */
    (*parentmenu)[1]->x = (*parentmenu)[0]->x +
                            (*parentmenu)[0]->width +
                            2*(*parentmenu)[0]->border_width;
    (*parentmenu)[1]->y = 0;

    /* Initialise menu labels and callback functions.
     */
    strcpy( (*parentmenu)[1]->label[0], "Standard HT");
    strcpy( (*parentmenu)[1]->label[1], "Randomized HT");
    strcpy( (*parentmenu)[1]->label[2], "Probabilistic HT");
    strcpy( (*parentmenu)[1]->label[3], "Hierarchical HT");
    init_parent_menu_window_gc_and_font(MainWindow, (*parentmenu)[1]);

    setup_menu_off_parent_menu( (*parentmenu)[1], MainWindow, 0, 4,
                                    "Lines", callback_SHTLines,
                                    "Circles", callback_SHTCircles,
                                    "Ellipses", callback_SHTEllipses,
                                    "Help", callback_SHTHelp);

    setup_menu_off_parent_menu( (*parentmenu)[1], MainWindow, 1, 4,
                                    "Lines", callback_RHTLines,
                                    "Circles", callback_RHTCircles,
                                    "Ellipses", callback_RHTEllipses,
                                    "Help", callback_RHTHelp);

    setup_menu_off_parent_menu( (*parentmenu)[1], MainWindow, 2, 4,
                                    "Lines", callback_PHTLines,
                                    "Circles", callback_PHTCircles,
                                    "Ellipses", callback_PHTEllipses,
                                    "Help", callback_PHTHelp);

    setup_menu_off_parent_menu( (*parentmenu)[1], MainWindow, 3, 2,
                                    "Lines", callback_HHTLines,
                                    "Help", callback_HHTHelp);




    /* UPWRITE STUFF.
     */
    /* set up third parent menu 3 options.
     */
    (*parentmenu)[2] = alloc_parent_menu(3);
                            /* Put third parent menu to left
                             * of second parent menu.
                             */
    (*parentmenu)[2]->x = (*parentmenu)[1]->x +
                            (*parentmenu)[1]->width +
                            2*(*parentmenu)[1]->border_width;
    (*parentmenu)[2]->y = 0;

    /* Initialise menu labels and callback functions.
     */
    strcpy( (*parentmenu)[2]->label[0], "UpWrite");
    strcpy( (*parentmenu)[2]->label[1], "Teach");
    strcpy( (*parentmenu)[2]->label[2], "Parameters");
    init_parent_menu_window_gc_and_font(MainWindow, (*parentmenu)[2]);

    setup_menu_off_parent_menu( (*parentmenu)[2], MainWindow, 0, 3,
                                    "Chunk", callback_Chunk,
                                    "Recognise", callback_Recognise,
                                    "Help", callback_HelpUpWrite);

    setup_menu_off_parent_menu( (*parentmenu)[2], MainWindow, 1, 4,
                                    "Train", callback_Train,
                                    "Undo", callback_Undo,
                                    "Learn", callback_Learn,
                                    "Help", callback_HelpTrain);

    setup_menu_off_parent_menu( (*parentmenu)[2], MainWindow, 2, 1,
                                    "Resolution Radius", callback_ChangeResolutionRadius);




    return(3);      /* number of parent menus */

}   /* end of setup_menus() */
