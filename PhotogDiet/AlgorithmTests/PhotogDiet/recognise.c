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
  *
 * Modified by:     David John Brasher
 * E-mail:			brasher1@uab.edu
 * Date:			2010
 * Last Modified:	
 * Organization:	University of Alabama Birmingham
 *					Dept. Electrical and Computer Engineering
 */
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "upwrite.h"
#include <time.h>
#include <crtdbg.h>

#include "geometry.h"

#include "window_globals.h"
#include "database.h"
#include "images.h"

extern int upwrite_radius;

#ifdef _DEBUG
#define new DEBUG_CLIENTBLOCK
#endif


int print_gaus(char *name, GAUS_LIST *gaus_list);
int count_ellipse_pixels(GAUS_LIST *gauslist);

IMAGE   *alloc_IMAGE(int x, int y);
void    free_IMAGE(IMAGE *aImage);
void    free_linked_list_of_point(POINT_LIST *point_list);
void    free_linked_list_of_linked_list_of_gaus(GAUS_LIST *gaus_list);

int count_ellipse_pixels(GAUS_LIST *gaus_list)
{
	int ellipse_edge_points;
	GAUS	*gaus;

	ellipse_edge_points = 0;
	for (gaus=gaus_list->head; gaus != NULL; gaus=gaus->next)
	{
		ellipse_edge_points += gaus->point_list->num_elements;
	}
	return ellipse_edge_points;
}

//#define VERBOSE_INSTRUMENTATION
/* Returns 0.
 */
int recognise(char *filename, char *orgfilename)
{
//	double				frank;
	int					shading_factor;
//	double				billy;
	int					maxx, maxy, minx, miny;

	GAUS_LIST           *gaus_list, *chunk_gaus_list, *list;
    POINT               *point;
    CHUNK_LIST          *chunk_list, *final_chunk_list;
    CHUNK               *chunk;
    int                 x, y;
    double              res_radius_min, res_radius_max, res_radius_old;
    double              lklyhd;
    char                label[1000];
    int                 color_index = 0;
    int                 char_count;
	IMAGE				*aImage;
	FILE				*output7;
#ifdef VERBOSE_INSTRUMENTATION
	FILE				*output;
	FILE				*output2;
	FILE				*output3;
    FILE				*output4;
	FILE				*output5;
	FILE				*output6;
#endif //VERBOSE_INSTRUMENTATION
	int j;
	GAUS				*gaus;
	SIMPLE_POINT		*simple_point;
//	SIMPLE_POINT		*edge_point_samples;
	SIMPLE_POINT		center;

	int					ellipse_edge_points;
	int					i;

//    DISPLAY_OBJECT      *d;
    int                 num_objects;
    char                results_message[100];
    DISPLAY_OBJECT_LIST *remove_object_type_from_linked_list_of_display_object(
                                    DISPLAY_OBJECT_LIST *display_object_list,
                                    OBJECT_TYPE type);
    void                uncolor_pixels_in_XImage(IMAGE *aImage);
    void                draw_image();
    GAUS_LIST           *spot_algorithm_2D(IMAGE *aImage);
    void                assign_pixels_to_gaus_2D(IMAGE *aImage,
                                    GAUS_LIST *gaus_list);
    GAUS_LIST           *chunk_ellipses(GAUS_LIST *gaus_list);
    void                draw_linked_list_of_linked_list_of_gaus_2D(
                                    GAUS_LIST *head_linked_list);
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
                                    GAUS_LIST *chunk);
    void                display_label_of_chunk(int x, int y, char *label,
                                    short width,
                                    int ascent, int descent, int color_index);
    CHUNK               *alloc_chunk(void);
    void                free_linked_list_of_gaus(GAUS_LIST *gaus_list);
    void                free_point(POINT *p);
    void                draw_linked_list_of_gaus_2D(GAUS_LIST *gaus_list);
    void                display_message_and_wait(int x, int y, int num_strings, ...);
	IMAGE				*load_image(char *filename);
	int delete_flag;

#ifdef COMPUTATION_TIME
    gStart_time = clock();
#endif


	aImage = load_image(filename);
    gResolutionRadius = upwrite_radius;

    if ((gResolutionRadius-4) > 1)
        res_radius_min = gResolutionRadius-2.00;
    else
        res_radius_min = 1;
    res_radius_max = gResolutionRadius+4.00;

    /* Take a copy of 'gResolutionRadius' */
    res_radius_old = gResolutionRadius;

    /* Chunk the image at a range of resolution radii.
     */
    chunk_list = (CHUNK_LIST *)NULL;
    for (gResolutionRadius=res_radius_min; gResolutionRadius <= res_radius_max;
                                                        gResolutionRadius+=0.800)
        {

#ifdef VERBOSE_INSTRUMENTATION
		output5 = fopen("Ellipses.txt","w+");
		fclose(output5);
		output3 = fopen("SPOT_2D_RUNS.txt", "a+");
		fprintf(output3, "Running Spot2D again - gResolutionRadius=%f\n", gResolutionRadius);
#endif //VERBOSE_INSTRUMENTATION
        gaus_list = spot_algorithm_2D(aImage);
#ifdef VERBOSE_INSTRUMENTATION
		fprintf(output3, "After Running Spot2D\n");
		fclose(output3);
#endif //VERBOSE_INSTRUMENTATION

		if( gaus_list )
		{
#ifdef VERBOSE_INSTRUMENTATION
			output = fopen("GAUS_LIST.txt","a+");
			fprintf(output, "****GAUS_LIST START****\n");
			fprintf(output,"num_elements = %d\n", gaus_list->num_elements);
			if( gaus_list->head )
			{
				fprintf(output,"HEAD: num_points=%f\n", gaus_list->head->num_points);
			}
			if( gaus_list->tail)
			{
				fprintf(output,"TAIL: num_points=%f\n", gaus_list->tail->num_points);
			}
			fprintf(output, "----END OF GAUS_LIST----\n");
#endif //VERBOSE_INSTRUMENTATION
		}
        assign_pixels_to_gaus_2D(aImage, gaus_list);
// DJB:  Chunk ellipses needs to be a mutli-threaded finction.  It is highly processor intensive
//       This is one of the places which would need to be addressed for performance.
        chunk_gaus_list = chunk_ellipses(gaus_list);
        free_linked_list_of_gaus(gaus_list);
		

//      draw_image(aXImage, MainWindow, MainGC);
//        draw_linked_list_of_linked_list_of_gaus_2D(chunk_gaus_list);

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
			free_IMAGE(aImage);
			return(0);
        }


//  draw_image(aXImage, MainWindow, MainGC);
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
#ifdef VERBOSE_INSTRUMENTATION
	    output4 = fopen("Likely Hood Of A Match.txt", "a+");
	    fprintf(output4,"lklyhd=%g\n", lklyhd);
	    fclose(output4);
#endif //VERBOSE_INSTRUMENTATION   /*1.75e-12*/
            if (lklyhd > 1.75e-12)
                {
                num_objects++;
                /* Note down the label of the class.
                 * label[] will contain the labels of all classes
                 * that this chunk belongs to
                 * e.g. a circle may be both 'circle' and 'ellipse'.
                 */
                char_count = (int)strlen(label);
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

        char_count = (int)strlen(label);
        if (char_count > 0)         /* Did we recognise this chunk? */
            {
            /* Record the chunk in the linked list of of
             * objects recognised: 'gDisplayObjectList'
             *
             * Compute the mean on the chunk.
             * Write its label at the mean.
             */
            compute_mean_of_chunk(chunk->gaus_list, &x, &y);
			center.x = x; center.y = y;
			ellipse_edge_points = count_ellipse_pixels(chunk->gaus_list);
			simple_point = malloc((size_t)(ellipse_edge_points * sizeof(SIMPLE_POINT)));

#ifdef VERBOSE_INSTRUMENTATION
			output6 = fopen("good_chunks.txt","a+");
			output5 = fopen("Ellipses.txt", "a+");
#endif //VERBOSE_INSTRUMENTATION
			i=0;

			minx = miny = INT_MAX;	
			maxx = maxy = 0;
			j = create_monumentpoint(x, y, 0.0, 0.0, 0.0, 0,(double)0);

			for (gaus=chunk->gaus_list->head; gaus != NULL; gaus=gaus->next)
			{
#ifdef VERBOSE_INSTRUMENTATION
				fprintf(output6,"NEXT GAUS\nnum_points=%f\n", gaus->num_points);
				fprintf(output6,"POINT_LIST: num_points=%d\n", gaus->point_list->num_elements);
#endif //VERBOSE_INSTRUMENTATION
				for (point=gaus->point_list->head; point != NULL; point=point->next)
				{
					create_edgepoint((int)(point->x[1]),(int)(point->x[2]),0,0,0);
					simple_point[i].x = (int)point->x[1];
					if( (int)point->x[1] < minx )
						minx = (int)point->x[1];
					else if ( (int)point->x[1] > maxx )
						maxx = (int)point->x[1];

					simple_point[i].y = (int)point->x[2];
					if( (int)point->x[2] < miny )
						miny = (int)point->x[2];
					else if ( (int)point->x[2] > maxy )
						maxy = (int)point->x[2];

					i++;
#ifdef VERBOSE_INSTRUMENTATION
					fprintf(output6,"EDGE POINT: x=%f, y=%f\n", point->x[1], point->x[2]);
#endif //VERBOSE_INSTRUMENTATION

				}
			}
			delete_flag = 0;

			if( i==0 && j != 0 )
			{
				delete_monument_point(j);
				delete_flag = 1;
			}
			else if( 1==2 ) //Lines, circles and ellipses for upwrite algorithm
			{
				center.x = x; center.y = y;
				shading_factor = shade_detection(orgfilename, &center, simple_point);
				draw_monument_point(filename, x,y, shading_factor, simple_point, ellipse_edge_points);
				output7 = fopen("foundElements.txt", "a+");
				if( output7 )
				{
					fprintf(output7, "Type: %s, X: %d, Y: %d\n",
						chunk->gaus_list->head->label, x, y);
					fclose(output7);
				}

			}				
//#ifdef MONUMENTPOINT_DISCRIMINATOR
			else if( ( (double)(maxx-minx)/aImage->x < .020 || (double)(maxx-minx)/aImage->x > .05 ||
				(double)(maxy-miny)/aImage->y < .020 || (double)(maxy-miny)/aImage->y > .065) )
			{
				delete_monument_point(j);
				delete_flag = 1;
			}
			else if( fabs(((float)(maxx-minx) / (float)(maxy-miny))) > 1.6|| 
				fabs(((float)(maxx-minx) / (float)(maxy-miny))) < .2 )
			{
				delete_monument_point(j);
				delete_flag = 1;
			}
//#endif //MONUMENTPOINT_DISCRIMINATOR
			else
			{
				center.x = x; center.y = y;
				shading_factor = shade_detection(orgfilename, &center, simple_point);
				update_monumentpoint_diameter(j, maxx-minx, maxy-miny);
				update_shading_factor(j, shading_factor);
				draw_monument_point(filename, x,y, shading_factor, simple_point, ellipse_edge_points);
				delete_flag = 0;
			}
#ifdef VERBOSE_INSTRUMENTATION
			fprintf(output6, "%s\nx=%d, y=%d\nmin_x=%d, min_y=%d\nmax_x=%d, max_y=%d\ntotal edge points=%d\n", label, x, y, 
				chunk->gaus_list->min_x, chunk->gaus_list->min_y, chunk->gaus_list->max_x, chunk->gaus_list->max_y, ellipse_edge_points);
			fprintf(output5, "%s\nDELETE FLAG = %d\nx=%d, y=%d\nmin_x=%d, min_y=%d\nmax_x=%d, max_y=%d\ntotal edge points=%d\nWidth=%d, Height=%d\nmaxx=%d, maxy=%d, minx=%d, miny=%d\nImage_x=%d, Image_y=%d\n\n", 
				label, delete_flag, x, y, 
				chunk->gaus_list->min_x, chunk->gaus_list->min_y, chunk->gaus_list->max_x, chunk->gaus_list->max_y, ellipse_edge_points,
				chunk->gaus_list->max_x - chunk->gaus_list->min_x, chunk->gaus_list->max_y - chunk->gaus_list->min_y, maxx, maxy, minx, miny,
				aImage->x, aImage->y);
#endif //VERBOSE_INSTRUMENTATION


/*			edge_point_samples = (SIMPLE_POINT *)malloc((size_t)(8*sizeof(SIMPLE_POINT)));

			for( i=0; i<8; i++)
			{
				edge_point_samples[0].x = simple_point[(i/8)*(ellipse_edge_points)].x;
				edge_point_samples[0].y = simple_point[(i/8)*(ellipse_edge_points)].y;
			}
			free(edge_point_samples);
*/
			free(simple_point);

#ifdef VERBOSE_INSTRUMENTATION			
			fprintf(output6,"\n");
			fclose(output6);
			fclose(output5);
#endif //VERBOSE_INSTRUMENTATION
            }   /* end of 'if (char_count > 0' */
	        free_point(point);

        }   /* endof 'for (chunk=final_chunk_list;...' */

    free_linked_list_of_chunk(final_chunk_list);

    if (num_objects == 1)
        sprintf(results_message, "Found 1 object with UpWrite.");
    else
        sprintf(results_message, "Found %d objects with UpWrite.", num_objects);

	free_IMAGE(aImage);
    free_linked_list_of_point(gTrainingPointList);

    free_linked_list_of_linked_list_of_gaus(gClassList);
//    free_linked_list_of_display_object(gDisplayObjectList);

  //  free_linked_list_of_display_object(gDisplayObjectList);
    return(0);

}   /* end of callback_Recognise() */

#undef VERBOSE_INSTRUMENTATION
