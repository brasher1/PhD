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
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "../include/upwrite.h"
#include "../include/globals.h"



/* Function Prototypes
 */
int     main(int argc, char *argv[]);
void    XEventLoop(Window MainWindow, GC MainGC, PARENT_MENU *ParentMenu[], int num_parent_menus, IMAGE *aImage, XImage *aXImage, int argc, char *argv[]);





int
main(int argc, char *argv[])
{
    IMAGE   *aImage;
    XImage  *aXImage;
    Window  MainWindow;
    GC      MainGC;
    PARENT_MENU     **ParentMenu;   /* array of pointers to PARENT MENU */
    int     num_parent_menus;
    int     x, y;

    void    init_upwrite(void);
    IMAGE   *alloc_IMAGE(int x, int y);
    IMAGE   *load_pbm_image(char filename[]);
    void    setup_window_gc(IMAGE *aImage, char win_name[], Window *win, GC *gc);
    void    setup_colormap(Window theWindow);
    XImage  *setup_XImage(IMAGE *aImage, Window theWindow);
    int     setup_menus(Window MainWindow, PARENT_MENU *(*parent_menu[]));
    int     get_pixel_value(IMAGE *aImage, int x, int y);
    void    set_XImage_grey_pixel_value(XImage *aXImage, int x, int y, int val);
    void    free_IMAGE(IMAGE *aImage);
    void    clear_image(IMAGE *aImage, XImage *Ximage);
    void    sobel_edge_detect(double threshold, IMAGE *aImage, XImage *aXImage);
    void    XEventLoop(Window MainWindow, GC MainGC, PARENT_MENU *ParentMenu[], int num_parent_menus, IMAGE *aImage, XImage *aXImage, int argc, char *argv[]);
    void    free_linked_list_of_point(POINT_LIST *point_list);
    void    free_linked_list_of_linked_list_of_gaus(GAUS_LIST *gaus_list);
    void    free_linked_list_of_display_object(DISPLAY_OBJECT_LIST *display_object_list);
    void    quitX(XImage *aXImage, Window MainWindow, GC MainGC, PARENT_MENU *ParentMenu[], int num_parent_menus);



    /* Initialse gResolutionRadius, drand48()
     */
    init_upwrite();


    if (argc == 2)
        {
        /* Load image file.
         */
        aImage = load_pbm_image(argv[1]);

        /* Set up X stuff.
         */
        setup_window_gc(aImage, "UpWrite and Hough Algorithms", &MainWindow, &MainGC);
        setup_colormap(MainWindow);
        aXImage = setup_XImage(aImage, MainWindow);

        /* Copy aImage into aXImage.
         */
        for (x=0; x < aImage->x; x++)
            for (y=0; y < aImage->y; y++)
                set_XImage_grey_pixel_value(aXImage, x, y, get_pixel_value(aImage,x,y));
        }
    else
        {
        /* Begin with blank image.
         */

        /* Allocate memory for the IMAGE.
        * This also specifies the size of the window.
        */
        aImage = alloc_IMAGE(WINDOW_SIZE_X, WINDOW_SIZE_Y);

        /* Set up X stuff.
         */
        setup_window_gc(aImage, "UpWrite and Hough Algorithms", &MainWindow, &MainGC);
        setup_colormap(MainWindow);
        aXImage = setup_XImage(aImage, MainWindow);
        clear_image(aImage, aXImage);
        }


    num_parent_menus = setup_menus(MainWindow, &ParentMenu);



    XEventLoop(MainWindow, MainGC, ParentMenu, num_parent_menus, aImage, aXImage, argc, argv);


    /* Shut down
     */
    free_IMAGE(aImage);
    free_linked_list_of_point(gTrainingPointList);
    free_linked_list_of_linked_list_of_gaus(gClassList);
    free_linked_list_of_display_object(gDisplayObjectList);
    quitX(aXImage, MainWindow, MainGC, ParentMenu, num_parent_menus);


    return(1);

}   /* end of main() */


void
XEventLoop(Window MainWindow, GC MainGC, PARENT_MENU *ParentMenu[], int num_parent_menus, IMAGE *aImage, XImage *aXImage, int argc, char *argv[])
{
    XEvent  report, next_event;
    int     choice, i, j;
    int     return_val;
    BOOLEAN first_expose;
    BOOLEAN found_menu_flag, found_it_flag;
    MENU    *CurrentMenu;
    BOOLEAN display_number_training_examples_flag;
    void    draw_image(XImage *aXImage, Window theWindow, GC theGC);
    void    display_instructions(Window MainWindow);
    int     parent_menu_option(Window theWindow, PARENT_MENU *menu);
    void    paint_parent_menu_pane(int option, PARENT_MENU *parent_menu, int mode);
    int     menu_option(Window theWindow, MENU *menu);
    void    paint_menu_pane(int option, MENU *menu, int mode);
    void    drag_out_a_line(IMAGE *aImage, XImage *aXImage, Window theWindow, GC theGC);
    void    free_hand_draw(IMAGE *aImage, XImage *aXImage, Window theWindow, GC theGC);
    void    display_number_training_examples(Window MainWindow, GC MainGC);


    first_expose = TRUE;
    display_number_training_examples_flag = FALSE;
    CurrentMenu = NULL;

    /* Event Loop */

    while (1)
        {
        XNextEvent( gDisplay, &report );
        switch (report.type)
            {
            case Expose :
                if (report.xany.window == MainWindow)
                    {
                    if (report.xexpose.count != 0)
                        {
                        /* Unless this is the last contiguous
                         * expose, don't draw the window.
                         */
                        break;
                        }
                    else
                        {
                        if (XPending(gDisplay) > 0)
                            {
                            XPeekEvent(gDisplay, &next_event);
                            if ((next_event.type == Expose) && (next_event.xany.window == MainWindow))
                                {
                                /* We don't want to re-draw window
                                 * twice in a row, so check if next
                                 * event is an Expose for MainWindow.
                                 * If it is an Expose, then ignore
                                 * the current Expose event.
                                 *
                                 * This happens when the 4 menu windows
                                 * are unmapped. It generates 4 Expose
                                 * events.
                                 */
                                break;      /* out of 'case Expose :' */
                                }
                            }

                        /* Display instructions the first time
                         * that the window appears.
                         */
                        if (first_expose == TRUE)
                            {
                            display_instructions(MainWindow);
                            first_expose = FALSE;
                            }
                        draw_image(aXImage, MainWindow, MainGC);
                        if (display_number_training_examples_flag == TRUE)
                            {
                            display_number_training_examples(MainWindow, MainGC);
                            display_number_training_examples_flag = FALSE;
                            }
                        XFlush(gDisplay);
                        }
                    }   /* end of 'if (report.xany.window == MainWindow)' */
                else
                    {
                    /* Parent Menus.
                     */
                    found_it_flag = FALSE;
                    for (i=0; i < num_parent_menus; i++)
                        {
                        choice = parent_menu_option(report.xany.window, ParentMenu[i]);
                        if (choice != -1)
                            {
                            paint_parent_menu_pane(choice, ParentMenu[i], NORMAL);
                            found_it_flag = TRUE;
                            break;      /* out of for-loop */
                            }
                        }

                    /* Menus.
                     */
                    if (found_it_flag == FALSE)
                        {
                        for (i=0; i < num_parent_menus; i++)
                            {
                            for (j=0; j < ParentMenu[i]->num_menus; j++)
                                {
                                choice = menu_option(report.xany.window, ParentMenu[i]->menus[j]);
                                if (choice != -1)
                                    {
                                    paint_menu_pane(choice, ParentMenu[i]->menus[j], NORMAL);
                                    found_it_flag = TRUE;
                                    break;      /* out of for-loop */
                                    }
                                }
                            if (found_it_flag == TRUE)
                                break;      /* out of for-loop */
                            }   /* end of 'for (i=0; ...' */

                        }   /* end of 'if (not_parent_menu_flag == TRUE)' */
                    }
                break;

            case ButtonPress :
                if (report.xbutton.button == Button1)
                    {
                    drag_out_a_line(aImage, aXImage, MainWindow, MainGC);
                    }
                else if (report.xbutton.button == Button2)
                    {
                    free_hand_draw(aImage, aXImage, MainWindow, MainGC);
                    }
                else if (report.xbutton.button == Button3)
                    {
                    XUngrabPointer(gDisplay, CurrentTime);
                    for (i=0; i < num_parent_menus; i++)
                        XMapWindow(gDisplay, ParentMenu[i]->bkgd_win);
                    }
                break;

            case ButtonRelease :
                if (report.xbutton.button == Button3)
                    {


                    /* UnMap the parent menus
                     */
                    for (i=0; i < num_parent_menus; i++)
                        XUnmapWindow(gDisplay, ParentMenu[i]->bkgd_win);

                    /* If a menu was mapped when button was released,
                     * find if we were in one of its menu options.
                     */
                    found_menu_flag = FALSE;
                    if (CurrentMenu != NULL)
                        {
                        choice = menu_option(report.xany.window, CurrentMenu);
                        if (choice != -1)
                            found_menu_flag = TRUE;
                        XUnmapWindow(gDisplay, CurrentMenu->bkgd_win);
                        }

                    /* Call the callback function.
                     */
                    if (found_menu_flag == TRUE)
                        {
                        return_val = (CurrentMenu->callback)[choice](aImage, aXImage, MainWindow, MainGC, CurrentMenu);
                        if (return_val == 1)
                            display_number_training_examples_flag = TRUE;
                        else if (return_val == -1)
                            return;     /* out of XEventLoop() */
                        }

                    CurrentMenu = NULL;
                    }

                break;

            case EnterNotify :
                /* If we entered one of the parent menu options,
                 * then highlight it and map the associated menu.
                 */
                found_it_flag = FALSE;
                for (i=0; i < num_parent_menus; i++)
                    {
                    choice = parent_menu_option(report.xcrossing.window, ParentMenu[i]);
                    if (choice != -1)
                        {
                        /* Unmap the currently mapped menu
                         * Don't unmap if it corresponds
                         * to the menu to be mapped.
                         */
                        if ((CurrentMenu != NULL) && (CurrentMenu != ParentMenu[i]->menus[choice]))
                            XUnmapWindow(gDisplay, CurrentMenu->bkgd_win);

                        paint_parent_menu_pane(choice, ParentMenu[i], HIGHLIGHT);
                        /* Map the appropriate menu.
                         * Don't map it if it is already mapped.
                         */
                        if (CurrentMenu != ParentMenu[i]->menus[choice])
                            {
                            XMapWindow(gDisplay, ParentMenu[i]->menus[choice]->bkgd_win);
                            XRaiseWindow(gDisplay, ParentMenu[i]->menus[choice]->bkgd_win);
                            CurrentMenu = ParentMenu[i]->menus[choice];
                            }
                        found_it_flag = TRUE;
                        break;  /* out of for-loop */
                        }
                    }

                if (found_it_flag == FALSE)
                    {
                    for (i=0; i < num_parent_menus; i++)
                        {
                        for (j=0; j < ParentMenu[i]->num_menus; j++)
                            {
                            choice = menu_option(report.xcrossing.window, ParentMenu[i]->menus[j]);
                            if (choice != -1)
                                {
                                paint_menu_pane(choice, ParentMenu[i]->menus[j], HIGHLIGHT);
                                found_it_flag = TRUE;
                                break;      /* out of for-loop */
                                }
                            }
                        if (found_it_flag == TRUE)
                            break;      /* out of for-loop */
                        }
                    }   /* end of 'if (found_it_flag == FALSE)' */
                break;

            case LeaveNotify :
                /* If we left one of the menu options,
                 * then UNhighlight it.
                 */
                found_it_flag = FALSE;
                for (i=0; i < num_parent_menus; i++)
                    {
                    choice = parent_menu_option(report.xcrossing.window, ParentMenu[i]);
                    if (choice != -1)
                        {
                        paint_parent_menu_pane(choice, ParentMenu[i], NORMAL);
                        found_it_flag = TRUE;
                        break;
                        }
                    }

                if (found_it_flag == FALSE)
                    {
                    for (i=0; i < num_parent_menus; i++)
                        {
                        for (j=0; j < ParentMenu[i]->num_menus; j++)
                            {
                            choice = menu_option(report.xcrossing.window, ParentMenu[i]->menus[j]);
                            if (choice != -1)
                                {
                                paint_menu_pane(choice, ParentMenu[i]->menus[j], NORMAL);
                                found_it_flag = TRUE;
                                break;
                                }
                            }
                        if (found_it_flag == TRUE)
                            break;      /* out of for-loop */
                        }
                    }   /* end of 'if (found_it_flag == FALSE)' */
                break;


            }   /* end of 'switch (report.type)' */
        }   /* end of 'while (1)' */


}   /* end of XEventLoop() */
