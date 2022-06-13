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
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "../include/upwrite.h"
#include <time.h>
#include <values.h>


#ifdef COMPUTATION_TIME
clock_t     gStart_time, gEnd_time;
double      gTotal_time;
int         gParity;            /* +1 or -1 */
int         gOverflowAdd;
#endif



/* Function Prototypes.
 */
int callback_Chunk(IMAGE *aImage, XImage *aXImage,
                            Window MainWindow, GC MainGC, MENU *aMenu);
int callback_Recognise(IMAGE *aImage, XImage *aXImage,
                            Window MainWindow, GC MainGC, MENU *aMenu);
int callback_Train(IMAGE *aImage, XImage *aXImage,
                            Window MainWindow, GC MainGC, MENU *aMenu);
int callback_Learn(IMAGE *aImage, XImage *aXImage,
                            Window MainWindow, GC MainGC, MENU *aMenu);
int callback_Undo(IMAGE *aImage, XImage *aXImage,
                            Window MainWindow, GC MainGC, MENU *aMenu);
int callback_RemoveAnalysis(IMAGE *aImage, XImage *aXImage,
                            Window MainWindow, GC MainGC, MENU *aMenu);
int callback_ClearImage(IMAGE *aImage, XImage *aXImage,
                            Window MainWindow, GC MainGC, MENU *aMenu);
int callback_Quit(IMAGE *aImage, XImage *aXImage,
                            Window MainWindow, GC MainGC, MENU *aMenu);
int callback_ChangeResolutionRadius(IMAGE *aImage, XImage *aXImage,
                            Window MainWindow, GC MainGC, MENU *aMenu);
int callback_DrawCircle(IMAGE *aImage, XImage *aXImage,
                            Window MainWindow, GC MainGC, MENU *aMenu);
int callback_DrawEllipse(IMAGE *aImage, XImage *aXImage,
                            Window MainWindow, GC MainGC, MENU *aMenu);
int callback_SHTLines(IMAGE *aImage, XImage *aXImage,
                            Window MainWindow, GC MainGC, MENU *aMenu);
int callback_SHTCircles(IMAGE *aImage, XImage *aXImage,
                            Window MainWindow, GC MainGC, MENU *aMenu);
int callback_SHTEllipses(IMAGE *aImage, XImage *aXImage,
                            Window MainWindow, GC MainGC, MENU *aMenu);
int callback_RHTLines(IMAGE *aImage, XImage *aXImage,
                            Window MainWindow, GC MainGC, MENU *aMenu);
int callback_RHTCircles(IMAGE *aImage, XImage *aXImage,
                            Window MainWindow, GC MainGC, MENU *aMenu);
int callback_RHTEllipses(IMAGE *aImage, XImage *aXImage,
                            Window MainWindow, GC MainGC, MENU *aMenu);
int callback_PHTLines(IMAGE *aImage, XImage *aXImage,
                            Window MainWindow, GC MainGC, MENU *aMenu);
int callback_PHTCircles(IMAGE *aImage, XImage *aXImage,
                            Window MainWindow, GC MainGC, MENU *aMenu);
int callback_PHTEllipses(IMAGE *aImage, XImage *aXImage,
                            Window MainWindow, GC MainGC, MENU *aMenu);
int callback_HHTLines(IMAGE *aImage, XImage *aXImage,
                            Window MainWindow, GC MainGC, MENU *aMenu);
int callback_HelpGeneral(IMAGE *aImage, XImage *aXImage,
                            Window MainWindow, GC MainGC, MENU *aMenu);
int callback_SHTHelp(IMAGE *aImage, XImage *aXImage,
                            Window MainWindow, GC MainGC, MENU *aMenu);
int callback_RHTHelp(IMAGE *aImage, XImage *aXImage,
                            Window MainWindow, GC MainGC, MENU *aMenu);
int callback_PHTHelp(IMAGE *aImage, XImage *aXImage,
                            Window MainWindow, GC MainGC, MENU *aMenu);
int callback_HHTHelp(IMAGE *aImage, XImage *aXImage,
                            Window MainWindow, GC MainGC, MENU *aMenu);
int callback_HelpUpWrite(IMAGE *aImage, XImage *aXImage,
                            Window MainWindow, GC MainGC, MENU *aMenu);
int callback_HelpTrain(IMAGE *aImage, XImage *aXImage,
                            Window MainWindow, GC MainGC, MENU *aMenu);
int callback_AddSpeckleNoise(IMAGE *aImage, XImage *aXImage,
                            Window MainWindow, GC MainGC, MENU *aMenu);
int callback_to_be_written(IMAGE *aImage, XImage *aXImage,
                            Window MainWindow, GC MainGC, MENU *aMenu);




/* Returns 0.
 */
int
callback_Chunk(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu)
{
    int         num_chunks;
    char        results_message[100];
    GAUS_LIST   *gaus_list, *chunk_gaus_list, *list;
    void        uncolor_pixels_in_XImage(IMAGE *aImage, XImage *aXImage);
    void        draw_image(XImage *aXImage, Window theWindow, GC theGC);
    GAUS_LIST   *spot_algorithm_2D(IMAGE *aImage, XImage *aXImage, Window theWindow, GC theGC);
    void        assign_pixels_to_gaus_2D(IMAGE *aImage, GAUS_LIST *gaus_list);
    GAUS_LIST   *chunk_ellipses(GAUS_LIST *gaus_list, Window theWindow, GC theGC);
    void    draw_linked_list_of_linked_list_of_gaus_2D(GAUS_LIST *head_linked_list, Window theWindow, GC theGC);
    void    color_pixels_in_XImage(GAUS_LIST *chunk_gaus_list, XImage *aXImage);
    void        free_linked_list_of_gaus(GAUS_LIST *gaus_list);
    void        free_linked_list_of_linked_list_of_gaus(GAUS_LIST *gaus_list);
    void        display_message_and_wait(Window theWindow, int x, int y, int num_strings, ...);



    uncolor_pixels_in_XImage(aImage, aXImage);

    gaus_list = spot_algorithm_2D(aImage, aXImage, MainWindow, MainGC);

    assign_pixels_to_gaus_2D(aImage, gaus_list);

    chunk_gaus_list = chunk_ellipses(gaus_list, MainWindow, MainGC);
    free_linked_list_of_gaus(gaus_list);


    /* Count number of chunks found.
     */
    num_chunks = 0;
    for (list=chunk_gaus_list; list != (GAUS_LIST *)NULL; list=list->next_list)
            num_chunks++;


    draw_image(aXImage, MainWindow, MainGC);
    draw_linked_list_of_linked_list_of_gaus_2D(chunk_gaus_list,
                            MainWindow, MainGC);
    if (num_chunks == 1)
        sprintf(results_message, "Found 1 chunk.");
    else
        sprintf(results_message, "Found %d chunks.", num_chunks);
    display_message_and_wait(MainWindow, 0, 0, 4,
                            "",
                            results_message,
                            "Hit Left Mouse Button to continue",
                            "");



    color_pixels_in_XImage(chunk_gaus_list, aXImage);
    draw_image(aXImage, MainWindow, MainGC);


    free_linked_list_of_linked_list_of_gaus(chunk_gaus_list);



    return(0);

}   /* end of callback_Chunk() */



/* Returns 0.
 */
int
callback_Recognise(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu)
{
    GAUS_LIST           *gaus_list, *chunk_gaus_list, *list;
    POINT               *point;
    CHUNK_LIST          *chunk_list, *final_chunk_list;
    CHUNK               *chunk;
    int                 x, y;
    double              res_radius_min, res_radius_max, res_radius_old;
    double              lklyhd;
    char                label[1000];
    int                 color_index = 0;
    int                 direction, ascent, descent, char_count;
    XCharStruct         overall;
    DISPLAY_OBJECT      *d;
    int                 num_objects;
    char                results_message[100];
    DISPLAY_OBJECT_LIST *remove_object_type_from_linked_list_of_display_object(
                                    DISPLAY_OBJECT_LIST *display_object_list,
                                    OBJECT_TYPE type);
    void                uncolor_pixels_in_XImage(IMAGE *aImage,
                                    XImage *aXImage);
    void                draw_image(XImage *aXImage, Window theWindow, GC theGC);
    GAUS_LIST           *spot_algorithm_2D(IMAGE *aImage, XImage *aXImage,
                                    Window theWindow, GC theGC);
    void                assign_pixels_to_gaus_2D(IMAGE *aImage,
                                    GAUS_LIST *gaus_list);
    GAUS_LIST           *chunk_ellipses(GAUS_LIST *gaus_list,
                                    Window theWindow, GC theGC);
    void                draw_linked_list_of_linked_list_of_gaus_2D(
                                    GAUS_LIST *head_linked_list,
                                    Window theWindow, GC theGC);
    POINT               *compute_up_to_second_order_geometric_moments(
                                    GAUS_LIST *gaus_list);
    CHUNK_LIST          *group_chunks(CHUNK_LIST **ptr_chunk_list);
    CHUNK_LIST          *put_in_linked_list_of_chunk(CHUNK_LIST *chunk_list,
                                    CHUNK *c);
    void                free_linked_list_of_chunk(CHUNK_LIST *chunk_list);
    POINT               *calculate_zernike_moments(GAUS_LIST *gaus_list,
                                    int max_order);
    double              likelihood_of_point_relative_to_gaus_mixture_model(
                                    POINT *point, GAUS_LIST *gaus_list);
    POINT_LIST          *put_in_linked_list_of_point(POINT_LIST *point_list,
                        POINT *p);
    void                compute_mean_of_chunk(GAUS_LIST *chunk, int *x, int *y);
    DISPLAY_OBJECT      *alloc_display_object(void);
    DISPLAY_OBJECT_LIST *put_in_linked_list_of_display_object(
                                    DISPLAY_OBJECT_LIST *display_oject_list,
                                    DISPLAY_OBJECT *d);
    void                color_pixels_for_this_chunk(int color_index,
                                    GAUS_LIST *chunk, XImage *aXImage);
    void                display_label_of_chunk(int x, int y, char *label,
                                    XFontStruct *font_info, short width,
                                    int ascent, int descent, int color_index,
                                    Window theWindow, GC theGC);
    CHUNK               *alloc_chunk(void);
    void                free_linked_list_of_gaus(GAUS_LIST *gaus_list);
    void                free_point(POINT *p);
    void                draw_linked_list_of_gaus_2D(GAUS_LIST *gaus_list,
                                    Window theWindow, GC theGC);
    void                display_message_and_wait(Window theWindow,
                                    int x, int y, int num_strings, ...);


#ifdef COMPUTATION_TIME
    gStart_time = clock();
#endif



    if ((gResolutionRadius-4) > 1)
        res_radius_min = gResolutionRadius-4;
    else
        res_radius_min = 1;
    res_radius_max = gResolutionRadius+4;

    /* Take a copy of 'gResolutionRadius' */
    res_radius_old = gResolutionRadius;

    /* Chunk the image at a range of resolution radii.
     */
    chunk_list = (CHUNK_LIST *)NULL;
    for (gResolutionRadius=res_radius_min; gResolutionRadius < res_radius_max;
                                                        gResolutionRadius+=0.8)
        {
        /* Remove any upwrite objects from gDisplayObjectList.
         */
        gDisplayObjectList = remove_object_type_from_linked_list_of_display_object(gDisplayObjectList, upwrite);


        uncolor_pixels_in_XImage(aImage, aXImage);
        draw_image(aXImage, MainWindow, MainGC);

        gaus_list = spot_algorithm_2D(aImage, aXImage, MainWindow, MainGC);

        assign_pixels_to_gaus_2D(aImage, gaus_list);

        chunk_gaus_list = chunk_ellipses(gaus_list, MainWindow, MainGC);
        free_linked_list_of_gaus(gaus_list);

        draw_image(aXImage, MainWindow, MainGC);
        draw_linked_list_of_linked_list_of_gaus_2D(chunk_gaus_list,
                                                        MainWindow, MainGC);

        for (list=chunk_gaus_list; list != NULL; list=list->next_list)
            {
            chunk = alloc_chunk();
            chunk->point = compute_up_to_second_order_geometric_moments(list);
            chunk->gaus_list = list;
            chunk_list = put_in_linked_list_of_chunk(chunk_list, chunk);

            }       /* end of 'for (gResolutionRadius...' */

        /* The (GAUS_LIST *) representing each chunk
         * is now stored in a (CHUNK ) in 'chunk_list'
         */
        chunk_gaus_list = (GAUS_LIST *)NULL;
        }

    /* Change 'gResolutionRadius' back to its old value.
     */
    gResolutionRadius = res_radius_old;



    /* Note: group_chunks() modifies 'chunk_list'
     */
    final_chunk_list = group_chunks(&chunk_list);

    free_linked_list_of_chunk(chunk_list);



    if (final_chunk_list == (CHUNK_LIST *)NULL)
        {
        /* No chunks */
        return(0);
        }


    draw_image(aXImage, MainWindow, MainGC);
    num_objects = 0;
    for (chunk=final_chunk_list->head; chunk != NULL; chunk=chunk->next)
        {
        point = calculate_zernike_moments(chunk->gaus_list, 4);
        label[0] = (char )NULL;
        for (gaus_list=gClassList; gaus_list != NULL; gaus_list=gaus_list->next_list)
            {

            /* Does chunk belong to this class?
             */
            lklyhd = likelihood_of_point_relative_to_gaus_mixture_model(point, gaus_list);
            if (lklyhd > 10e-11)
                {
                num_objects++;
                /* Note down the label of the class.
                 * label[] will contain the labels of all classes
                 * that this chunk belongs to
                 * e.g. a circle may be both 'circle' and 'ellipse'.
                 */
                char_count = strlen(label);
                if (char_count > 0)     /* Is there already a word there? */
                    {
                    /* Insert a comma between words
                     */
                    strncat(label, ", ", 999-char_count);
                    char_count+=3;
                    }
                strncat(label, gaus_list->head->label, 999-char_count);
                }
            }   /* end of 'for (gaus_list = gClassList;...' */

        char_count = strlen(label);
        if (char_count > 0)         /* Did we recognise this chunk? */
            {
            /* Record the chunk in the linked list of of
             * objects recognised: 'gDisplayObjectList'
             *
             * Compute the mean on the chunk.
             * Write its label at the mean.
             */
            compute_mean_of_chunk(chunk->gaus_list, &x, &y);


            d = alloc_display_object();
            d->type = upwrite;
            d->obj.upwrite.x = x;
            d->obj.upwrite.y = y;
            d->obj.upwrite.label = (char *)malloc((char_count+1) * sizeof(char) );
            strcpy(d->obj.upwrite.label, label);
            d->obj.upwrite.font_info = XLoadQueryFont(gDisplay, DISPLAY_FONT_NAME);
            /* Determine width, ascent and descent
             * of label (in pixels).
             */
            XTextExtents(d->obj.upwrite.font_info, label, char_count, &direction, &ascent, &descent, &overall);

            if (d->obj.upwrite.font_info == NULL)
                {
                fprintf(stderr, "Can not open font %s in callback_Recognise() in menu_callback_functions.c\n", DISPLAY_FONT_NAME);
                exit(1);
                }

            d->obj.upwrite.width = overall.width;
            d->obj.upwrite.ascent = ascent;
            d->obj.upwrite.descent = descent;
            d->obj.upwrite.color_index = color_index;
            gDisplayObjectList = put_in_linked_list_of_display_object(gDisplayObjectList, d);
            color_pixels_for_this_chunk(color_index, chunk->gaus_list, aXImage);


            /* Draw it */
            XSetForeground(gDisplay, MainGC, gRandomColors[color_index].pixel);
            draw_linked_list_of_gaus_2D(chunk->gaus_list, MainWindow, MainGC);
            display_label_of_chunk(d->obj.upwrite.x, d->obj.upwrite.y,
                                d->obj.upwrite.label,
                                d->obj.upwrite.font_info,
                                d->obj.upwrite.width,
                                d->obj.upwrite.ascent,
                                d->obj.upwrite.descent,
                                d->obj.upwrite.color_index,
                                MainWindow, MainGC);
            XFlush(gDisplay);

            color_index++;
            if (color_index == NUM_RANDOM_COLORS)
                color_index = 0;
            }   /* end of 'if (char_count > 0' */

        free_point(point);

        }   /* end of 'for (chunk=final_chunk_list;...' */


    free_linked_list_of_chunk(final_chunk_list);


    if (num_objects == 1)
        sprintf(results_message, "Found 1 object with UpWrite.");
    else
        sprintf(results_message, "Found %d objects with UpWrite.", num_objects);
    display_message_and_wait(MainWindow, 0, 0, 4,
                            "",
                            results_message,
                            "Hit Left Mouse Button to continue",
                            "");



#ifdef COMPUTATION_TIME
    gEnd_time = clock();
    if (gEnd_time > 0)
        gTotal_time = ((double )gEnd_time - (double )gStart_time) / CLOCKS_PER_SEC;
    else
        {
        gTotal_time = ( 2*(1 + (double )MAXINT) +
                        (double )gEnd_time -
                        (double )gStart_time) / CLOCKS_PER_SEC;
        }
    printf("UpWrite elapsed time: %.2f secs\n", (float )gTotal_time);
#endif


    return(0);

}   /* end of callback_Recognise() */



/* Returns 1.
 */
int
callback_Train(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu)
{
    GAUS_LIST   *gaus_list, *chunk_gaus_list, *chunk;
    POINT       *point;
    void        uncolor_pixels_in_XImage(IMAGE *aImage, XImage *aXImage);
    void        draw_image(XImage *aXImage, Window theWindow, GC theGC);
    GAUS_LIST   *spot_algorithm_2D(IMAGE *aImage, XImage *aXImage, Window theWindow, GC theGC);
    void        assign_pixels_to_gaus_2D(IMAGE *aImage, GAUS_LIST *gaus_list);
    GAUS_LIST   *chunk_ellipses(GAUS_LIST *gaus_list, Window theWindow, GC theGC);
    void    draw_linked_list_of_linked_list_of_gaus_2D(GAUS_LIST *head_linked_list, Window theWindow, GC theGC);
    void    color_pixels_in_XImage(GAUS_LIST *chunk_gaus_list, XImage *aXImage);
    POINT   *calculate_zernike_moments(GAUS_LIST *gaus_list, int max_order);
    void        free_linked_list_of_gaus(GAUS_LIST *gaus_list);
    void        free_linked_list_of_linked_list_of_gaus(GAUS_LIST *gaus_list);
    POINT_LIST  *put_in_linked_list_of_point(POINT_LIST *point_list, POINT *p);



    uncolor_pixels_in_XImage(aImage, aXImage);
    draw_image(aXImage, MainWindow, MainGC);

    gaus_list = spot_algorithm_2D(aImage, aXImage, MainWindow, MainGC);

    assign_pixels_to_gaus_2D(aImage, gaus_list);

    chunk_gaus_list = chunk_ellipses(gaus_list, MainWindow, MainGC);
    free_linked_list_of_gaus(gaus_list);

    draw_image(aXImage, MainWindow, MainGC);
    draw_linked_list_of_linked_list_of_gaus_2D(chunk_gaus_list,
                                        MainWindow, MainGC);

    color_pixels_in_XImage(chunk_gaus_list, aXImage);
    draw_image(aXImage, MainWindow, MainGC);


    /* 'gOldCountTrainingPoint' contains the number
     * of points in the linked list of training points,
     * before the most recent points have been added.
     *
     * If we choose the 'Undo' menu option,
     * we shall use this to remove recent additions
     * to the linked list of training points.
     */
    if (gTrainingPointList != NULL)
        gOldCountTrainingPoint = gTrainingPointList->num_elements;
    else
        gOldCountTrainingPoint = 0;


    for (chunk=chunk_gaus_list; chunk != NULL; chunk=chunk->next_list)
        {
        point = calculate_zernike_moments(chunk, 4);
        gTrainingPointList = put_in_linked_list_of_point(gTrainingPointList, point);
        }


    free_linked_list_of_linked_list_of_gaus(chunk_gaus_list);



    return(1);

}   /* end of callback_Train() */



/* Returns 0.
 */
int
callback_Learn(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu)
{
    GAUS_LIST   *gaus_list;
    GAUS        *gaus;
    INPUT_PROMPT    *prompt;
    char        *label;
    void        display_message_and_wait(Window theWindow, int x, int y, int num_strings, ...);
    void        open_input_prompt(int x, int y, char *message, Window MainWindow, INPUT_PROMPT **prompt);
    char        *prompt_user_for_alphanumeric_input(INPUT_PROMPT *prompt);
    GAUS_LIST   *em_algorithm(int num_gaus, POINT_LIST *point_list,
                                            Window theWindow, GC theGC);
    void        output_model(char *label, GAUS_LIST *gaus_list);
    void        output_training_data(GAUS_LIST *gaus_list);
    GAUS_LIST   *put_in_linked_list_of_linked_list_of_gaus(
                                            GAUS_LIST *head_list,
                                            GAUS_LIST *list);
    void        free_linked_list_of_point(POINT_LIST *point_list);
    void        free_linked_list_of_gaus(GAUS_LIST *gaus_list);
    void        free_input_prompt(INPUT_PROMPT *prompt);
    void        display_message_and_wait(Window theWindow, int x, int y, int num_strings, ...);



    if ((gTrainingPointList == NULL) || (gTrainingPointList->head == NULL))
        {
        display_message_and_wait(MainWindow, aImage->x / 4, 0, 5,
                            "Before selecting 'Learn',",
                            "you need to 'Train' the system.",
                            "",
                            "Hit Left Mouse Button to continue",
                            "");
        return(0);
        }


    open_input_prompt(0, 0, "Enter label for this class", MainWindow, &prompt);


    label = prompt_user_for_alphanumeric_input(prompt);

    /* Keep prompting them until they type something.
     */
    while ((label == (char *)NULL) || (label[0] == (char )NULL))
        {
        open_input_prompt(0, 0, "You must enter a label for the class", MainWindow, &prompt);
        label = prompt_user_for_alphanumeric_input(prompt);
        }



    /* We initialise the E.M. algorithm with 15 gaussians.
     * This is an arbitrary number, but seems a good
     * first guess.
     * Some of the gaussians may be discarded by em_algorithm()
     */
    gaus_list = em_algorithm(15, gTrainingPointList, MainWindow, MainGC);

    if ((gaus_list == (GAUS_LIST *)NULL) || (gaus_list->head == (GAUS *)NULL))
        {
        display_message_and_wait(MainWindow, 0, 0, 7,
                            "",
                            "Learning failed.",
                            "",
                            "Try again with more training examples.",
                            "",
                            "Hit Left Mouse Button to continue",
                            "");
        return(0);
        }

    /* Output the linked list of GAUS in 'gaus_list'
     * in a form that can later be used for recognition.
     */
    output_model(label, gaus_list);



    /* Output the training data and the gaussian mixture model
     * in a format the the xgobi program can read.
     * The data is output to the files:
     *      tmp.dat
     *      tmp.colors
     *      tmp.glyphs
     *      tmp.lines
     *      tmp.linecolors
     */
    output_training_data(gaus_list);


    /* Record the object's label in each gaus that
     * is in gaus_list.
     */
    for (gaus=gaus_list->head; gaus != NULL; gaus=gaus->next)
        {
        gaus->label = (char *)malloc((1+strlen(label)) * sizeof(char) );
        if (gaus->label == NULL)
            {
            fprintf(stderr, "malloc failed in callback_Learn() in menu_callback_functions.c\n");
            exit(1);
            }
        strcpy(gaus->label, label);
        }

    /* Add the new object to the list 'gClassList' */
    gClassList = put_in_linked_list_of_linked_list_of_gaus(gClassList,
                                                            gaus_list);


    free_linked_list_of_point(gTrainingPointList);
    gTrainingPointList = (POINT_LIST *)NULL;
    free_input_prompt(prompt);
    free(label);



    return(0);

}   /* end of callback_Learn() */



/* Returns 1.
 */
int
callback_Undo(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu)
{
    POINT_LIST  *remove_last_entries_from_point_list(POINT_LIST *point_list, int n);


    if ((gTrainingPointList == NULL) || (gTrainingPointList->head == NULL))
        {
        /* No Training points to remove.
         * Do nothing.
         */
        }
    else
        {
        /* Remove recent additions to the
         * linked list of training points.
         */
        gTrainingPointList = remove_last_entries_from_point_list(gTrainingPointList, gTrainingPointList->num_elements - gOldCountTrainingPoint);
        }



    return(1);

}   /* end of callback_Undo() */



/* Returns 0.
 */
int
callback_RemoveAnalysis(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu)
{
    void    free_linked_list_of_display_object(DISPLAY_OBJECT_LIST *display_object_list);
    void    uncolor_pixels_in_XImage(IMAGE *aImage, XImage *aXImage);

    free_linked_list_of_display_object(gDisplayObjectList);
    gDisplayObjectList = NULL;
    uncolor_pixels_in_XImage(aImage, aXImage);

    return(0);

}   /* End of callback_RemoveAnalysis() */



/* Returns 0.
 */
int
callback_ClearImage(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu)
{
    void    free_linked_list_of_display_object(DISPLAY_OBJECT_LIST *display_object_list);
    void    clear_image(IMAGE *aImage, XImage *aXImage);

    free_linked_list_of_display_object(gDisplayObjectList);
    gDisplayObjectList = NULL;
    clear_image(aImage, aXImage);

    return(0);

}   /* End of callback_ClearImage() */



/* Returns -1.
 */
int
callback_Quit(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu)
{

    return(-1);

}   /* end of callback_Quit() */



/* Returns 0.
 */
int
callback_ChangeResolutionRadius(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu)
{
    INPUT_PROMPT    *prompt;
    double      input_val;
    void    open_input_prompt(int x, int y, char *message, Window MainWindow, INPUT_PROMPT **prompt);
    double  prompt_user_for_input(INPUT_PROMPT *prompt);
    void    free_input_prompt(INPUT_PROMPT *prompt);



    open_input_prompt(aMenu->x, aMenu->y, "Enter Resolution Radius", MainWindow, &prompt);


    input_val = prompt_user_for_input(prompt);
    if (input_val != -1)
        gResolutionRadius = input_val;


    free_input_prompt(prompt);

    return(0);

}   /* End of callback_ChangeResolutionRadius() */



int
callback_DrawCircle(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu)
{
    void    draw_image(XImage *aXImage, Window theWindow, GC theGC);
    void    drag_out_a_circle(IMAGE *aImage, XImage *aXImage, Window theWindow, GC theGC);

    draw_image(aXImage, MainWindow, MainGC);
    XFlush(gDisplay);

    drag_out_a_circle(aImage, aXImage, MainWindow, MainGC);

    return(0);

}   /* end of callback_DrawCircle() */



int
callback_DrawEllipse(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu)
{
    void    draw_image(XImage *aXImage, Window theWindow, GC theGC);
    void    drag_out_an_ellipse(IMAGE *aImage, XImage *aXImage, Window theWindow, GC theGC);

    draw_image(aXImage, MainWindow, MainGC);
    XFlush(gDisplay);

    drag_out_an_ellipse(aImage, aXImage, MainWindow, MainGC);

    return(0);

}   /* end of callback_DrawEllipse() */



int
callback_SHTLines(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu)
{
    void    find_lines_with_SHT(IMAGE *aImage, XImage *aXImage, Window theWindow, GC theGC);


#ifdef COMPUTATION_TIME
    gStart_time = clock();
#endif


    find_lines_with_SHT(aImage, aXImage, MainWindow, MainGC);


#ifdef COMPUTATION_TIME
    gEnd_time = clock();
    if (gEnd_time > 0)
        gTotal_time = ((double )gEnd_time - (double )gStart_time) / CLOCKS_PER_SEC;
    else
        {
        gTotal_time = ( 2*(1 + (double )MAXINT) +
                        (double )gEnd_time -
                        (double )gStart_time) / CLOCKS_PER_SEC;
        }
    printf("SHT Lines elapsed time: %.2f secs\n", (float )gTotal_time);
#endif

    return(0);

}   /* end of callback_SHTLines() */



int
callback_SHTCircles(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu)
{
    void    find_circles_with_SHT(IMAGE *aImage, XImage *aXImage, Window theWindow, GC theGC);


#ifdef COMPUTATION_TIME
    gStart_time = clock();
#endif



    find_circles_with_SHT(aImage, aXImage, MainWindow, MainGC);



#ifdef COMPUTATION_TIME
    gEnd_time = clock();
    if (gEnd_time > 0)
        gTotal_time = ((double )gEnd_time - (double )gStart_time) / CLOCKS_PER_SEC;
    else
        {
        gTotal_time = ( 2*(1 + (double )MAXINT) +
                        (double )gEnd_time -
                        (double )gStart_time) / CLOCKS_PER_SEC;
        }
    printf("SHT Circles elapsed time: %.2f secs\n", (float )gTotal_time);
#endif


    return(0);

}   /* end of callback_SHTCircles() */



int
callback_SHTEllipses(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu)
{
    void    find_ellipses_with_SHT(IMAGE *aImage, XImage *aXImage, Window theWindow, GC theGC);


#ifdef COMPUTATION_TIME
    gOverflowAdd = 0;
    gParity = 1;
    gStart_time = clock();
#endif



    find_ellipses_with_SHT(aImage, aXImage, MainWindow, MainGC);



#ifdef COMPUTATION_TIME
    gEnd_time = clock();
    if (gEnd_time > 0)
        {
        gTotal_time = ( ((double )gOverflowAdd) * 2*(1 + (double )MAXINT) +
                        (double )gEnd_time -
                        (double )gStart_time)
                                            / CLOCKS_PER_SEC;
        }
    else
        {
        gTotal_time =   ( ((double )gOverflowAdd+1) * 2*(1+(double )MAXINT) +
                        (double )gEnd_time -
                        (double )gStart_time)
                                            / CLOCKS_PER_SEC;
        }
    printf("SHT Ellipses elapsed time: %.2f secs\n", (float )gTotal_time);
#endif


    return(0);

}   /* end of callback_SHTEllipses() */



int
callback_RHTLines(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu)
{
    void    find_lines_with_RHT(IMAGE *aImage, XImage *aXImage, Window theWindow, GC theGC);


#ifdef COMPUTATION_TIME
    gStart_time = clock();
#endif



    find_lines_with_RHT(aImage, aXImage, MainWindow, MainGC);



#ifdef COMPUTATION_TIME
    gEnd_time = clock();
    if (gEnd_time > 0)
        gTotal_time = ((double )gEnd_time - (double )gStart_time) / CLOCKS_PER_SEC;
    else
        {
        gTotal_time = ( 2*(1 + (double )MAXINT) +
                        (double )gEnd_time -
                        (double )gStart_time) / CLOCKS_PER_SEC;
        }
    printf("RHT Lines elapsed time: %.2f secs\n", (float )gTotal_time);
#endif


    return(0);

}   /* end of callback_RHTLines() */



int
callback_RHTCircles(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu)
{
    void    find_circles_with_RHT(IMAGE *aImage, XImage *aXImage, Window theWindow, GC theGC);


#ifdef COMPUTATION_TIME
    gStart_time = clock();
#endif



    find_circles_with_RHT(aImage, aXImage, MainWindow, MainGC);



#ifdef COMPUTATION_TIME
    gEnd_time = clock();
    if (gEnd_time > 0)
        gTotal_time = ((double )gEnd_time - (double )gStart_time) / CLOCKS_PER_SEC;
    else
        {
        gTotal_time = ( 2*(1 + (double )MAXINT) +
                        (double )gEnd_time -
                        (double )gStart_time) / CLOCKS_PER_SEC;
        }
    printf("RHT Circles elapsed time: %.2f secs\n", (float )gTotal_time);
#endif


    return(0);

}   /* end of callback_RHTCircles() */



int
callback_RHTEllipses(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu)
{
    void    find_ellipses_with_RHT(IMAGE *aImage, XImage *aXImage, Window theWindow, GC theGC);


#ifdef COMPUTATION_TIME
    gStart_time = clock();
#endif



    find_ellipses_with_RHT(aImage, aXImage, MainWindow, MainGC);



#ifdef COMPUTATION_TIME
    gEnd_time = clock();
    if (gEnd_time > 0)
        gTotal_time = ((double )gEnd_time - (double )gStart_time) / CLOCKS_PER_SEC;
    else
        {
        gTotal_time = ( 2*(1 + (double )MAXINT) +
                        (double )gEnd_time -
                        (double )gStart_time) / CLOCKS_PER_SEC;
        }
    printf("RHT Ellipses elapsed time: %.2f secs\n", (float )gTotal_time);
#endif


    return(0);

}   /* end of callback_RHTEllipses() */



int
callback_PHTLines(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu)
{
    void    find_lines_with_PHT(IMAGE *aImage, XImage *aXImage, Window theWindow, GC theGC);


#ifdef COMPUTATION_TIME
    gStart_time = clock();
#endif



    find_lines_with_PHT(aImage, aXImage, MainWindow, MainGC);



#ifdef COMPUTATION_TIME
    gEnd_time = clock();
    if (gEnd_time > 0)
        gTotal_time = ((double )gEnd_time - (double )gStart_time) / CLOCKS_PER_SEC;
    else
        {
        gTotal_time = ( 2*(1 + (double )MAXINT) +
                        (double )gEnd_time -
                        (double )gStart_time) / CLOCKS_PER_SEC;
        }
    printf("PHT Lines elapsed time: %.2f secs\n", (float )gTotal_time);
#endif


    return(0);

}   /* end of callback_PHTLines() */



int
callback_PHTCircles(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu)
{
    void    find_circles_with_PHT(IMAGE *aImage, XImage *aXImage, Window theWindow, GC theGC);


#ifdef COMPUTATION_TIME
    gStart_time = clock();
#endif


    find_circles_with_PHT(aImage, aXImage, MainWindow, MainGC);


#ifdef COMPUTATION_TIME
    gEnd_time = clock();
    if (gEnd_time > 0)
        gTotal_time = ((double )gEnd_time - (double )gStart_time) / CLOCKS_PER_SEC;
    else
        {
        gTotal_time = ( 2*(1 + (double )MAXINT) +
                        (double )gEnd_time -
                        (double )gStart_time) / CLOCKS_PER_SEC;
        }
    printf("PHT Circles elapsed time: %.2f secs\n", (float )gTotal_time);
#endif


    return(0);

}   /* end of callback_PHTCircles() */



int
callback_PHTEllipses(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu)
{
    void    find_ellipses_with_PHT(IMAGE *aImage, XImage *aXImage, Window theWindow, GC theGC);


#ifdef COMPUTATION_TIME
    gStart_time = clock();
#endif



    find_ellipses_with_PHT(aImage, aXImage, MainWindow, MainGC);



#ifdef COMPUTATION_TIME
    gEnd_time = clock();
    if (gEnd_time > 0)
        gTotal_time = ((double )gEnd_time - (double )gStart_time) / CLOCKS_PER_SEC;
    else
        {
        gTotal_time = ( 2*(1 + (double )MAXINT) +
                        (double )gEnd_time -
                        (double )gStart_time) / CLOCKS_PER_SEC;
        }
    printf("PHT Ellipses elapsed time: %.2f secs\n", (float )gTotal_time);
#endif


    return(0);

}   /* end of callback_PHTEllipses() */



int
callback_HHTLines(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu)
{
    void    find_lines_with_HHT(IMAGE *aImage, XImage *aXImage, Window theWindow, GC theGC);


#ifdef COMPUTATION_TIME
    gStart_time = clock();
#endif


    find_lines_with_HHT(aImage, aXImage, MainWindow, MainGC);


#ifdef COMPUTATION_TIME
    gEnd_time = clock();
    if (gEnd_time > 0)
        gTotal_time = ((double )gEnd_time - (double )gStart_time) / CLOCKS_PER_SEC;
    else
        {
        gTotal_time = ( 2*(1 + (double )MAXINT) +
                        (double )gEnd_time -
                        (double )gStart_time) / CLOCKS_PER_SEC;
        }
    printf("HHT Lines elapsed time: %.2f secs\n", (float )gTotal_time);
#endif


    return(0);

}   /* end of callback_HHTLines() */



int
callback_HelpGeneral(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu)
{
    void    display_message_and_wait(Window theWindow, int x, int y, int num_strings, ...);


    display_message_and_wait(MainWindow, 0, 0, 25,
                            "Help General",
                            "",
                            "Draw Circle:",
                            "Draw Ellipse:",
                            "   Use left mouse button to drag out a circle/ellipse.",
                            "",
                            "Remove Analysis:",
                            "   Discard any objects recognised, but",
                            "   don't clear the image.",
                            "   It has the effect of removing any",
                            "   colored objects from the screen, leaving only",
                            "   the black and white image.",
                            "   You will use this if you wish to compare",
                            "   different recognition algorithms on the",
                            "   same image",
                            "",
                            "Add Speckle Noise:",
                            "   Randomly invert pixels in the image",
                            "",
                            "The circle and number in the top right-hand",
                            "corner of the screen show the resolution radius",
                            "used with the UpWrite.",
                            "",
                            "Hit Left Mouse Button to continue",
                            "");


    return(0);

}   /* end of callback_HelpGeneral() */



int
callback_SHTHelp(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu)
{
    void    display_message_and_wait(Window theWindow, int x, int y, int num_strings, ...);


    display_message_and_wait(MainWindow, 0, 0, 14,
                            "Help - Standard Hough Transform",
                            "",
                            "Perform Standard Hough Transform",
                            "to find a specific type of object.",
                            "",
                            "Lines:     Find straight lines.",
                            "           Time rating: Quick.",
                            "Circles:   First find circle centres, then radius.",
                            "           Time rating: Quick.",
                            "Ellipses:  Find centres, then orientation and size.",
                            "           Time rating: Slow (minutes - hours).",
                            "",
                            "Hit Left Mouse Button to continue",
                            "");


    return(0);

}   /* end of callback_SHTHelp() */



int
callback_RHTHelp(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu)
{
    void    display_message_and_wait(Window theWindow, int x, int y, int num_strings, ...);


    display_message_and_wait(MainWindow, 0, 0, 14,
                            "Help - Randomized Hough Transform",
                            "",
                            "Perform Randomized Hough Transform",
                            "to find a specific type of object.",
                            "",
                            "Lines:   Find straight lines.",
                            "         Time rating: Quick.",
                            "Circles: Find circles.",
                            "         Time rating: Quick.",
                            "Ellipses:",
                            "         Time rating: Medium.",
                            "",
                            "Hit Left Mouse Button to continue",
                            "");


    return(0);

}   /* end of callback_RHTHelp() */



int
callback_PHTHelp(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu)
{
    void    display_message_and_wait(Window theWindow, int x, int y, int num_strings, ...);


    display_message_and_wait(MainWindow, 0, 0, 16,
                            "Help - Probabilistic Hough Transform",
                            "",
                            "Perform Probabilistic Hough Transform",
                            "to find a specific type of object.",
                            "Uses same algorithms as standard Hough Transform",
                            "But only samples one tenth of the image's pixels.",
                            "",
                            "Lines:   Find straight lines.",
                            "         Time rating: Quick.",
                            "Circles: First find circle centres, then radius.",
                            "         Time rating: Quick.",
                            "Ellipses:",
                            "         Time rating: Slow (minutes - hours).",
                            "",
                            "Hit Left Mouse Button to continue",
                            "");


    return(0);

}   /* end of callback_PHTHelp() */



int
callback_HHTHelp(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu)
{
    void    display_message_and_wait(Window theWindow, int x, int y, int num_strings, ...);


    display_message_and_wait(MainWindow, 0, 0, 10,
                            "Help - Hierarchical Hough Transform",
                            "",
                            "Perform Hierarchical Hough Transform",
                            "to find straight lines segment.",
                            "Computes Hough Transform on small sub-images,",
                            "then combine the results.",
                            "Time rating: Quick.",
                            "",
                            "Hit Left Mouse Button to continue",
                            "");


    return(0);

}   /* end of callback_HHTHelp() */



int
callback_HelpUpWrite(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu)
{
    void    display_message_and_wait(Window theWindow, int x, int y, int num_strings, ...);


    display_message_and_wait(MainWindow, 0, 0, 24,
                            "Help For UpWrite Algorithm",
                            "",
                            "Chunk:",
                            "   This will segment the image into separate objects",
                            "   Each object is drawn in a different color.",
                            "Recognise:",
                            "   Segment image into separate objects and",
                            "   perform recognition on each object.",
                            "   Chunking is performed several times for the",
                            "   the recognition, each time at a different",
                            "   resolution radius.",
                            "",
                            "   Data for the recognition is taken from any",
                            "   files in the current directory with the",
                            "   .upwritedata postfix.",
                            "   Each file contains a description of one type",
                            "   of object (e.g. line, circle or ellipse)",
                            "",
                            "The number and circle in the top right-hand",
                            "corner of the screen shows the current",
                            "resolution radius.",
                            "",
                            "Hit Left Mouse Button to continue",
                            "");


    return(0);

}   /* end of callback_HelpUpWrite() */



int
callback_HelpTrain(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu)
{
    void    display_message_and_wait(Window theWindow, int x, int y, int num_strings, ...);


    display_message_and_wait(MainWindow, 0, 0, 38,
                            "Help Training",
                            "",
                            "To recognise a new type of object,",
                            "1. Draw examples of the object.",
                            "   You can draw several on the screen simultaneously.",
                            "   Select the 'Train' menu option.",
                            "2. Clear image, draw more examples",
                            "   of the object and select 'Train' again.",
                            "3. Do this many times",
                            "   To train the system on ellipses,",
                            "   we drew over 1000 of them",
                            "4. If you make a mistake, select 'Undo'.",
                            "   This will discard the most recent image",
                            "   that you trained with.",
                            "5. Once you have finished training, select the",
                            "   'Learn' menu option.",
                            "   You will be asked for a label for the object.",
                            "   A file with the .upwritedata postfix will be",
                            "   created, containing a description of the object.",
                            "   This will automatically be loaded next time this",
                            "   program is run.",
                            "",
                            "Each training object is represented as a point",
                            "in 7-D space.",
                            "The cluster of points representing your training",
                            "set is modelled using a gaussian mixture model.",
                            "The training objects that you draw will be written",
                            "to a series of files, with the prefix 'tmp.'",
                            "Use a program that displays high dimensional data",
                            "to view these files. The files are written in a",
                            "format appropriate for the xgobi program",
                            "(search the WWW for source-code for xgobi).",
                            "Each gaussian in the gaussian mixture model is",
                            "represented by several red lines, one for each",
                            "eigenvector.",
                            "",
                            "Hit Left Mouse Button to continue",
                            "");


    return(0);

}   /* end of callback_HelpTrain() */



/* Returns 0.
 */
int
callback_AddSpeckleNoise(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu)
{
    INPUT_PROMPT    *prompt;
    double      input_val;
    void    open_input_prompt(int x, int y, char *message, Window MainWindow, INPUT_PROMPT **prompt);
    double  prompt_user_for_input(INPUT_PROMPT *prompt);
    void    free_input_prompt(INPUT_PROMPT *prompt);
    void    add_speckle_noise(double proportion_noise, IMAGE *aImage,
                                XImage *aXImage, Window theWindow, GC theGC);



    open_input_prompt(aMenu->x, aMenu->y, "% speckle noise (e.g. 0.5)", MainWindow, &prompt);


    input_val = prompt_user_for_input(prompt);
    if (input_val != -1)
        add_speckle_noise(input_val/100.0, aImage, aXImage, MainWindow, MainGC);


    free_input_prompt(prompt);

    return(0);

}   /* End of callback_AddSpeckleNoise() */



int
callback_to_be_written(IMAGE *aImage, XImage *aXImage, Window MainWindow, GC MainGC, MENU *aMenu)
{
    fprintf(stderr, "To be written\n");

    return(0);
}
