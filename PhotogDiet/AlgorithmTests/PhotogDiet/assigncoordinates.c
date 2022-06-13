/*
 * Programmed by:   Daivd John Brasher
 * E-mail:          brasher1@uab.edu
 * Date:            2010
 * 
 * Organization:    Dept. Electrical & Computer Engineering,
 *                  The University of Alabama at Birmingham,
 *                  Birmingham, Alabama,
 *                  USA
 */
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#ifdef _CH_
#pragma package <opencv>
#endif
#define CV_NO_BACKWARD_COMPATIBILITY

#ifndef _EiC
#include "cv.h"
#include "highgui.h"
#endif

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>     /* strlen() */
#include <sys/types.h>
#include <magick/api.h>
//#include "upwrite.h"
#include "geometry.h"
#include <float.h>
#include "nrc\nrutil.h"
#include "nrc.h"

#include "database.h"

float distance_2D( int x1, int x2, int y1, int y2);



#define PI 3.14159265

int find_quad_points(PLACEMAT_POINT *sp, float **distance, int count)
{
	int i,j,k,m;
	int numbers[3];
	int nindex;
	int dindex;
//	int flag;
	float distances[3];
	float tempDist;
	float avgx, avgy, avg;
	FILE *output;
	PLACEMAT_POINT *pp;
	int current_image;
	int countpp;

	current_image = get_current_image();
	pp = get_placemat_points(current_image);
	countpp = get_monument_point_count();

	memset(distances, 0, sizeof(distances));
	memset(numbers, 0, sizeof(numbers));

	for(i=0; i<3; i++)
		distances[i] = FLT_MAX;

	nindex = 0;
	dindex = 0;
	avgx = avgy = 0;
	for(i=0; i<count; i++)
	{
		avgx = avgx + sp[i].x_diameter;
		avgy = avgy + sp[i].y_diameter;
	}
	avgx = avgx / count;
	avgy = avgy / count;
	avg = (avgx + avgy) / 2.0;
	output = fopen("AverageRadii.txt","w+");
	fprintf(output, "AvgX:%f, AvgY:%f, Avg:%f\n", avgx, avgy, avg);
	fclose(output);
// Looking for two sets
	for(k=0; k<2; k++)
	{
// For each point left
		for(i=0; i<count; i++)
		{
			output = fopen("trianglePart.txt","a+");
			fprintf(output, "TrianglePart=%d, i=%d\n", sp[i].triangle_part, i);
			fclose(output);
//			if( sp[i].triangle_part == 0 )
//			{

				for(j=0; j<count; j++)
				{
					if( distance[i][j] < distances[0] && 
						sp[j].quad_part == 0 && 
						distance[i][j]  != 0 )
					{
						distances[2] = distances[1];
						distances[1] = distances[0];
						distances[0] = distance[i][j];
						numbers[2] = numbers[1];
						numbers[1] = numbers[0];
						numbers[0] = j;
					}
					else if( distance[i][j] < distances[1] && 
						sp[j].quad_part == 0 && 
						distance[i][j]  != 0 )
					{
						distances[2] = distances[1];
						distances[1] = distance[i][j];
						numbers[2] = numbers[1];
						numbers[1] = j;
					}
					else if( distance[i][j] < distances[2] && 
						sp[j].quad_part == 0 && 
						distance[i][j]  != 0 )
					{
						distances[2] = distance[i][j];
						numbers[2] = j;
					}
				}
				output = fopen("Distances.txt","a+");
				fprintf(output,"d[0]=%f, d[1]=%f, d[2]=%f\n",
					distances[0], distances[1], distances[2]);
				fclose(output);
				if( distances[0] < 5.0 * avg &&
					distances[1] < 5.0 * avg &&
					distances[2] < 5.0 * avg && 
					distances[0] < FLT_MAX / 2.0 && 
					distances[1] < FLT_MAX / 2.0 &&
					distances[2] < FLT_MAX / 2.0 && 
					collinear(sp[i].photox, sp[i].photoy, 1.0,
						sp[numbers[0]].photox, sp[numbers[0]].photoy, 1.0,
						sp[numbers[2]].photox, sp[numbers[2]].photoy, 1.0) < 1000.0 &&
					collinear(sp[i].photox, sp[i].photoy, 1.0,
						sp[numbers[1]].photox, sp[numbers[1]].photoy, 1.0,
						sp[numbers[2]].photox, sp[numbers[2]].photoy, 1.0) < 1000.0 )
				{
					output = fopen("QuadSearch.txt","a+");
					fprintf(output,"Found a Quad: %d, %d, %d, %d\nShading: %d, %d, %d, %d\n",
						i, numbers[0], numbers[1], numbers[2],
						sp[i].shading, sp[numbers[0]].shading,
						sp[numbers[1]].shading, sp[numbers[2]].shading);
					fclose(output);
					sp[i].quad_part = 1;
					sp[numbers[0]].quad_part = 1;
					sp[numbers[1]].quad_part = 1;
					sp[numbers[2]].quad_part = 1;
					output = fopen("ppstuff.txt","a+");
					fprintf(output,"START\n");
					for( m=0; m<count; m++ )
					{
						fprintf(output,"m=%d, map_id=%d, shading=%d\n",m,pp[m].map_id, pp[m].shading);

						if( pp[m].map_id == 1 || pp[m].map_id == 15 )
						{
							fprintf(output,"\t1 or 15\n");
							if( distance_2D(sp[m].photox, sp[i].photox, sp[m].photoy, sp[i].photoy) <
								distance_2D(sp[m].photox, sp[numbers[2]].photox, sp[m].photoy, sp[numbers[2]].photoy) )
							{
								fprintf(output,"\t\tith point is closer than 3rd (1-15)\n\t\t"
									"Shade1: %d,  Shade2: %d,  Shade3: %d,  Shade4:%d\n",
									pp[i].shading, sp[numbers[0]].shading, sp[numbers[1]].shading, sp[numbers[2]].shading );
								if( pp[i].shading == 100  && 
									sp[numbers[0]].shading == 0 && 
									sp[numbers[1]].shading == 0 &&
									sp[numbers[2]].shading == 100 )
								{
									sp[i].map_id = 3;
									sp[numbers[0]].map_id = 4;
									sp[numbers[1]].map_id = 5;
									sp[numbers[2]].map_id = 6;
								}
								else if(pp[i].shading == 0 &&
									sp[numbers[0]].shading == 100 &&
									sp[numbers[1]].shading == 100 &&
									sp[numbers[2]].shading == 0 )
								{
									sp[i].map_id = 17;
									sp[numbers[0]].map_id = 18;
									sp[numbers[1]].map_id = 19;
									sp[numbers[2]].map_id = 20;
								}
							}
							else
							{
								fprintf(output,"\t\t3rd point is closer than ith (1-15)\n\t\t"
									"Shade1: %d,  Shade2: %d,  Shade3: %d,  Shade4:%d\n",
									pp[i].shading, sp[numbers[0]].shading, sp[numbers[1]].shading, sp[numbers[2]].shading );
								if( pp[numbers[2]].shading == 100 &&
									sp[numbers[1]].shading == 0 && 
									sp[numbers[0]].shading == 0  &&
									sp[i].shading == 100 )
								{
									sp[numbers[2]].map_id = 3;
									sp[numbers[1]].map_id = 4;
									sp[numbers[0]].map_id = 5;
									sp[i].map_id = 6;
								}
								else if(pp[numbers[2]].shading == 0 &&
									sp[numbers[1]].shading == 100 &&
									sp[numbers[0]].shading == 100 &&
									sp[i].shading == 0 )
								{
									sp[numbers[2]].map_id = 17;
									sp[numbers[1]].map_id = 18;
									sp[numbers[0]].map_id = 19;
									sp[i].map_id = 20;
								}
							}
						}
						else if( pp[m].map_id == 8 || pp[m].map_id == 22 )
						{
							if( distance_2D(sp[m].photox, sp[i].photox, sp[m].photoy, sp[i].photoy) <
								distance_2D(sp[m].photox, sp[numbers[2]].photox, sp[m].photoy, sp[numbers[2]].photoy) )
							{
								fprintf(output,"\t\tith point is closer than 3rd (8-22)\n\t\t"
									"Shade1: %d,  Shade2: %d,  Shade3: %d,  Shade4:%d\n",
									pp[i].shading, sp[numbers[0]].shading, sp[numbers[1]].shading, sp[numbers[2]].shading );
								if( pp[i].shading == 100 &&
									sp[numbers[0]].shading == 0 &&
									sp[numbers[1]].shading == 0 && 
									sp[numbers[2]].shading == 100 )
								{
									sp[i].map_id = 6;
									sp[numbers[0]].map_id = 5;
									sp[numbers[1]].map_id = 4;
									sp[numbers[2]].map_id = 3;
								}
								else if(pp[i].shading == 0 &&
									sp[numbers[0]].shading == 100 &&
									sp[numbers[1]].shading == 100 &&
									sp[numbers[2]].shading == 0 )
								{
									sp[i].map_id = 20;
									sp[numbers[0]].map_id = 19;
									sp[numbers[1]].map_id = 18;
									sp[numbers[2]].map_id = 17;
								}
							}						
							else
							{
								fprintf(output,"\t\t3rd point is closer than ith (8-22)\n\t\t"
									"Shade1: %d,  Shade2: %d,  Shade3: %d,  Shade4:%d\n",
									pp[i].shading, sp[numbers[0]].shading, sp[numbers[1]].shading, sp[numbers[2]].shading );
								if( pp[numbers[2]].shading == 100 &&
									sp[numbers[1]].shading == 0 &&
									sp[numbers[0]].shading == 0 &&
									sp[i].shading == 100 )
								{
									sp[numbers[2]].map_id = 6;
									sp[numbers[1]].map_id = 5;
									sp[numbers[0]].map_id = 4;
									sp[i].map_id = 3;
								}
								else if(pp[numbers[2]].shading == 0 &&
									sp[numbers[1]].shading == 100 &&
									sp[numbers[0]].shading == 100 &&
									sp[i].shading == 0 )
								{
									sp[numbers[2]].map_id = 20;
									sp[numbers[1]].map_id = 19;
									sp[numbers[0]].map_id = 18;
									sp[i].map_id = 17;
								}
							}
						}

					}
					fprintf(output,"END\n");
					fclose(output);
				}
				for(m=0; m<3; m++)
					distances[m] = FLT_MAX;
				
			//}
		}
	}
	free(pp);
}

int compare_ordered_edge_points( const void *arg1, const void *arg2 )
{
   if( ((ORDERED_EDGE_POINTS *)arg1)->distance_from_edge_start < ((ORDERED_EDGE_POINTS *)arg2)->distance_from_edge_start )
	   return -1;
   else if( ((ORDERED_EDGE_POINTS *)arg1)->distance_from_edge_start > ((ORDERED_EDGE_POINTS *)arg2)->distance_from_edge_start )
	   return 1;
   else
	   return 0;
}

int map_found_edge_points(ORDERED_EDGE_POINTS *edgepts, int count)
{
	if( edgepts[0].point->map_id == 15 && edgepts[1].point->map_id == 16 )
	{
		if( count == 8 )
		{
			update_3D_coordinates(edgepts[2].point->id, 17);
			update_3D_coordinates(edgepts[3].point->id, 18);
			update_3D_coordinates(edgepts[4].point->id, 19);
			update_3D_coordinates(edgepts[5].point->id, 20);
		}
	}
	else if( edgepts[0].point->map_id == 15 && edgepts[1].point->map_id == 13 )
	{
		if( count == 5 )
		{
			update_3D_coordinates(edgepts[2].point->id, 11);
		}
	}
	else if( edgepts[0].point->map_id == 1 && edgepts[1].point->map_id == 2 )
	{
		if( count == 8 )
		{
			update_3D_coordinates(edgepts[2].point->id, 3);
			update_3D_coordinates(edgepts[3].point->id, 4);
			update_3D_coordinates(edgepts[4].point->id, 5);
			update_3D_coordinates(edgepts[5].point->id, 6);
		}
	}
	else if( edgepts[0].point->map_id == 1 && edgepts[1].point->map_id == 9 )
	{
		if( count == 5 )
		{
			update_3D_coordinates(edgepts[2].point->id, 11);
		}
	}
	else if( edgepts[0].point->map_id == 8 && edgepts[1].point->map_id == 7 )
	{
		if( count == 8 )
		{
			update_3D_coordinates(edgepts[2].point->id, 6);
			update_3D_coordinates(edgepts[3].point->id, 5);
			update_3D_coordinates(edgepts[4].point->id, 4);
			update_3D_coordinates(edgepts[5].point->id, 3);
		}
	}
	else if( edgepts[0].point->map_id == 8 && edgepts[1].point->map_id == 10 )
	{
		if( count == 5 )
		{
			update_3D_coordinates(edgepts[2].point->id, 12);
		}
	}
	else if( edgepts[0].point->map_id == 22 && edgepts[1].point->map_id == 14 )
	{
		if( count == 5 )
		{
			update_3D_coordinates(edgepts[2].point->id, 12);
		}
	}
	else if( edgepts[0].point->map_id == 22 && edgepts[1].point->map_id == 21 )
	{
		if( count == 8 )
		{
			update_3D_coordinates(edgepts[2].point->id, 20);
			update_3D_coordinates(edgepts[3].point->id, 19);
			update_3D_coordinates(edgepts[4].point->id, 18);
			update_3D_coordinates(edgepts[5].point->id, 17);
		}
	}

	return 0;
}

int assign_edge_points( SIMPLE_POINT *edgepts, int count )
{
	int i,j,k;
	DISTANCE_ANALYSIS distances[8];
	ORDERED_EDGE_POINTS ord_edge_pts[8];

//#define VERBOSE_INSTRUMENTATION

#ifdef VERBOSE_INSTRUMENTATION
	char temp[256];
	FILE *output, *output1;


	sprintf(temp, "SORTED-EDGES-%d-%d-%d-%d.txt", edgepts[0].x, edgepts[0].y,
				edgepts[1].x, edgepts[1].y);
	output1 = fopen(temp, "w+");
	fprintf(output1,"From_x\tFrom_y\tFrom_Shade\tTo_x\tTo_y\tTo_Shade\tDistance\n");

	sprintf(temp, "ASSIGN_EDGE-%d-%d-%d-%d.txt", edgepts[0].x, edgepts[0].y,
				edgepts[1].x, edgepts[1].y);
	output = fopen(temp, "w+");
	
#endif //VERBOSE_INSTRUMENTATION
	if( count > 8 )
		return(0);
	k=0;
	for( i=0; i<count; i++)
	{
		if( i != 0 )
		{
#ifdef VERBOSE_INSTRUMENTATION
			fprintf(output, "%d\t%d\t%d\t%d\t%f\n", edgepts[0].x, edgepts[0].y,
				edgepts[i].x, edgepts[i].y, distance_2D(edgepts[0].x, edgepts[i].x, 
				edgepts[0].y, edgepts[i].y));
#endif //VERBOSE_INSTRUMENTATION
		}
		ord_edge_pts[i].distance_from_edge_start = distance_2D(edgepts[0].x, 
			edgepts[i].x, edgepts[0].y, edgepts[i].y);
		if( ord_edge_pts[i].distance_from_edge_start == FLT_MAX )
			ord_edge_pts[i].distance_from_edge_start = 0;
		ord_edge_pts[i].point = &edgepts[i];
		k++;
	}
	qsort(ord_edge_pts, k, sizeof(ORDERED_EDGE_POINTS), compare_ordered_edge_points);

	distances[0].from = &edgepts[0];
	distances[0].to = ord_edge_pts[0].point;
	distances[0].distance = ord_edge_pts[0].distance_from_edge_start;
	for( j=1; j<k; j++)
	{
		distances[j].from = ord_edge_pts[j-1].point;
		distances[j].to = ord_edge_pts[j].point;
		distances[j].distance = distance_2D(distances[j].from->x, distances[j].to->x,
			distances[j].from->y, distances[j].to->y);
	}
	// try to assign point coordinates to found points

	
#ifdef VERBOSE_INSTRUMENTATION
	for( i=0; i<k; i++)
	{
		fprintf(output1,"%d\t%d\t%d\t%d\t%d\t%d\t%f\n", distances[i].from->x, distances[i].from->y,
			distances[i].from->shading,
			distances[i].to->x,  distances[i].to->y, 
			distances[i].to->shading, distances[i].distance);
	}
	fclose(output1);
	fclose(output);
#endif //VERBOSE_INSTRUMENTATION

	return map_found_edge_points(ord_edge_pts, k);
}

int group_by_slope(int pt1x, int pt1y, int pt1_map_id, int pt1_shading, int pt2x, int pt2y, int pt2_map_id, int pt2_shading, PLACEMAT_POINT *sp, int count)
{
	int i,j;
	float slope;
//	int slope_groups[8];
	float **collinear_det;
	float d;
	int *indx;
//	int n;
	float det;
	int number;
	SIMPLE_POINT collinear_pts[22];

#define VERBOSE_INSTRUMENTATION

#ifdef VERBOSE_INSTRUMENTATION
	char temp[MAX_PATH];
	FILE *output;

	sprintf(temp, "%s-DET-%d-%d-%d-%d.txt", get_filename_base(), pt1x, pt1y, pt2x, pt2y);

//	sprintf(temp, "DET%d-%d-%d-%d.txt", pt1x, pt1y, pt2x, pt2y);
	output = fopen(temp, "w+");
#endif //VERBOSE_INSTRUMENTATION

	if( pt2x - pt1x == 0 )
		return 0;

	slope = (float)(pt2y - pt1y)/(float)(pt2x - pt1x);
	
	collinear_det = matrix(1,3,1,3);
	indx = ivector(1,9);

#ifdef VERBOSE_INSTRUMENTATION
	fprintf(output, "%d\t%d\n%d\t%d\n", pt1x, pt1y, pt2x, pt2y);
#endif //VERBOSE_INSTRUMENTATION
	collinear_pts[0].shading = pt1_shading;
	collinear_pts[0].map_id = pt1_map_id;
	collinear_pts[0].x = pt1x;
	collinear_pts[0].y = pt1y;
	collinear_pts[1].shading = pt2_shading;
	collinear_pts[1].map_id = pt2_map_id;
	collinear_pts[1].x = pt2x;
	collinear_pts[1].y = pt2y;

	number = 2;
	for( i=0; i<count; i++)
	{
		if( pt1x != sp[i].photox && pt1y != sp[i].photoy && pt2x != sp[i].photox && pt2y != sp[i].photoy )
		{
			//det = 1;
			collinear_det[1][1] = (float)pt1x; collinear_det[1][2] = (float)pt1y; collinear_det[1][3] = 1.0f;
			collinear_det[2][1] = (float)pt2x; collinear_det[2][2] = (float)pt2y; collinear_det[2][3] = 1.0f;
			collinear_det[3][1] = (float)sp[i].photox; collinear_det[3][2] = (float)sp[i].photoy; collinear_det[3][3] = 1.0f;
			ludcmp(collinear_det, 3, indx, &d);
			det = collinear_det[1][1];
			for( j=2; j<=3; j++)
				det *= collinear_det[j][j];
			if( fabs(det) < 1500 )
			{
				memcpy( &collinear_pts[number], &sp[i], sizeof(SIMPLE_POINT));
#ifdef VERBOSE_INSTRUMENTATION
				fprintf(output, "%d\t%d\t%f\n", sp[i].photox, sp[i].photoy, det);
#endif //VERBOSE_INSTRUMENTATION
				number++;
			}
		}
	}
#ifdef VERBOSE_INSTRUMENTATION
	fclose(output);
#endif //VERBOSE_INSTRUMENTATION
	if( number > 2 )
	{
		assign_edge_points( collinear_pts, number );
	}

	free_matrix(collinear_det, 1,3,1,3);
	free_vector(indx, 1,9);
#undef VERBOSE_INSTRUMENTATION
	return 0;
}

int assign_other_triangles_to_map(TRIANGLE *t)
{
	int i;
	int j;

	for(i=0; i<4; i++)
	{
		if( t[i].point1_shading == 100 && t[i].point2_shading == 0 && t[i].point3_shading == 0 )
		{
			for(j=0; j<4; j++)
			{
				if( j != i )
				{
					if( fabs(t[i].slope12 - t[j].slope12)/fabs(t[i].slope12) < 0.05 || fabs(t[i].slope12 - t[j].slope31)/fabs(t[i].slope12) < 0.1)
					{
						if( t[j].point1_map_id == 15 )
						{
							t[i].point2_map_id = 9;
							t[i].point3_map_id = 2;
						}
						else if( t[j].point1_map_id == 8 )
						{
							t[i].point2_map_id = 2;
							t[i].point3_map_id = 9;
						}
					}
				}
			}
		}
		if( t[i].point1_shading == 0 && t[i].point2_shading == 100 && t[i].point3_shading == 100 )
		{
			for(j=0; j<4; j++)
			{
				if( j != i )
				{
					if( fabs(t[i].slope12 - t[j].slope12 )/fabs(t[i].slope12) < 0.1 || fabs(t[i].slope12 - t[j].slope31 )/fabs(t[i].slope12) < 0.1 )
					{
						if( t[j].point1_map_id == 22 )
						{
							t[i].point2_map_id = 7;
							t[i].point3_map_id = 10;
						}
						else if( t[j].point1_map_id == 1 )
						{
							t[i].point2_map_id = 10;
							t[i].point3_map_id = 7;
						}
					}
				}
			}
		}
	}
	return 0;
}

int assign_triangle_to_map(TRIANGLE *t)
{
	if( t->point1_shading == 100 && ((t->point2_shading == 100 && t->point3_shading == 0) || (t->point2_shading == 0 && t->point3_shading == 100)) )
	{
		t->point1_map_id = 22;
		if( t->point2_shading == 0 )
		{
			t->point2_map_id = 21;
			t->point3_map_id = 14;
		}
		else
		{
			t->point2_map_id = 14;
			t->point3_map_id = 21;
		}
	}
	else if( t->point1_shading == 0 && ((t->point2_shading == 100 && t->point3_shading == 0) || (t->point2_shading == 0 && t->point3_shading == 100)) )
	{
		t->point1_map_id = 15;
		if( t->point2_shading == 100 )
		{
			t->point2_map_id = 13;
			t->point3_map_id = 16;
		}
		else
		{
			t->point2_map_id = 16;
			t->point3_map_id = 13;
		}
	}
	else if( t->point1_shading == 100 && t->point2_shading == 0 && t->point3_shading == 0 )
	{
		t->point1_map_id = 1;
	}
	else if( t->point1_shading == 0 && t->point2_shading == 100 && t->point3_shading == 100 )
	{
		t->point1_map_id = 8;
	}
	return 0;
}

int collinear(float pt1x, float pt1y, float pt1z,
				float pt2x, float pt2y, float pt2z,
				float pt3x, float pt3y, float pt3z)
{
	CvMat *mat;
	double det;
	char buffer[2048];
//	wchar_t buff[2048];

	mat = cvCreateMat(3,3,CV_32FC1);

	CV_MAT_ELEM(*mat, float, 0,0) = pt1x;
	CV_MAT_ELEM(*mat, float, 0,1) = pt1y;
	CV_MAT_ELEM(*mat, float, 0,2) = pt1z;

	CV_MAT_ELEM(*mat, float, 1,0) = pt2x;
	CV_MAT_ELEM(*mat, float, 1,1) = pt2y;
	CV_MAT_ELEM(*mat, float, 1,2) = pt2z;

	CV_MAT_ELEM(*mat, float, 2,0) = pt3x;
	CV_MAT_ELEM(*mat, float, 2,1) = pt3y;
	CV_MAT_ELEM(*mat, float, 2,2) = pt3z;

	det = cvDet(mat);
#ifdef VERBOSE_INSTRUMENTATION	
	sprintf(buffer,"Point 1=(%f,%f,%f)\nPoint 2=(%f,%f,%f)\nPoint3=(%f,%f,%f)\nDet=%f",
		pt1x, pt1y, pt1z, pt2x, pt2y, pt2z, pt3x, pt3y, pt3z, det);
	mbstowcs(buff, buffer, sizeof(buff));
	MessageBox(NULL,buff,L"Det in collinear", MB_OK);
#endif //VERBOSE_INSTRUMENTATION
	if( fabs(det) < 1000 )
	{
		return 1;
	}
	return 0;
}

int validate_triangle( TRIANGLE *t, float point_width)
{
	char buffer[1024];
	wchar_t buff[2048];
	float slope1, slope2, slope3;

	slope1 = (float)(t->pt1y - t->pt2y)/(float)(t->pt1x - t->pt2x);
	slope2 = (float)(t->pt1y - t->pt3y)/(float)(t->pt1x - t->pt3x);
	slope3 = (float)(t->pt3y - t->pt2y)/(float)(t->pt3x - t->pt2x);
/*
	sprintf(buffer, "slopes: %f, %f, %f\n%d, %d\n%d, %d\n%d, %d", 
		slope1, slope2, slope3, t->pt1x, t->pt1y, t->pt2x, t->pt2y,
		t->pt3x, t->pt3y);
	mbstowcs(buff, buffer, sizeof(buffer));
	MessageBox(NULL, buff, L"SLOPES", MB_OK);
*/
	if( (fabs((slope1 -  slope3 )/ fabs(slope3)) < 0.05 && fabs((slope2 -  slope3 )/ fabs(slope3)) < 0.05) ||
		(fabs(slope1) < 0.001 && fabs(slope2) < 0.001 && fabs(slope3) < 0.001) )
	{
#ifdef VERBOSE_INSTRUMENTATION
		sprintf(buffer, "SLOPE FAILURE: %f, %f, %f\n%d, %d\n%d, %d\n%d, %d", 
			slope1, slope2, slope3, t->pt1x, t->pt1y, t->pt2x, t->pt2y,
			t->pt3x, t->pt3y);
		mbstowcs(buff, buffer, sizeof(buffer));
		MessageBox(NULL, buff, L"SLOPES", MB_OK);
#endif //VERBOSE_INSTRUMENTATION
		t->point1_id = t->point2_id = t->point3_id = 0;
		memset(t, 0, sizeof(TRIANGLE));
		return 0;
	}

	if( t->distance23 > 3.0*t->distance12 || t->distance23 > 3.0*t->distance31 )
	{
#ifdef VERBOSE_INSTRUMENTATION
		sprintf(buffer, "DISTANCE FAILURE: %f, %f, %f\n%d, %d\n%d, %d\n%d, %d", 
			t->distance12, t->distance23, t->distance31, t->pt1x, t->pt1y, t->pt2x, t->pt2y,
			t->pt3x, t->pt3y);
		mbstowcs(buff, buffer, sizeof(buffer));
		MessageBox(NULL, buff, L"DISTANCE", MB_OK);
#endif //VERBOSE_INSTRUMENTATION
		t->point1_id = t->point2_id = t->point3_id = 0;
		memset(t, 0, sizeof(TRIANGLE));
		return 0;
	}
	if( t->distance12 > 3.0*point_width || t->distance23 > 3.0*point_width || t->distance31 > 3.0*point_width )
	{
#ifdef VERBOSE_INSTRUMENTATION
		sprintf(buffer, "POINT WIDTH FAILURE: %f, %f, %f\nPoint Width=%f\n%d, %d\n%d, %d\n%d, %d", 
			t->distance12, t->distance23, t->distance31, point_width, t->pt1x, t->pt1y, t->pt2x, t->pt2y,
			t->pt3x, t->pt3y);
		mbstowcs(buff, buffer, sizeof(buffer));
		MessageBox(NULL, buff, L"DISTANCE", MB_OK);
#endif //VERBOSE_INSTRUMENTATION
		t->point1_id = t->point2_id = t->point3_id = 0;
		memset(t, 0, sizeof(TRIANGLE));
		return 0;
	}
	if( collinear((float)t->pt1x, (float)t->pt1y, (float)1.0, (float)t->pt2x, (float)t->pt2y, 
		(float)1.0, (float)t->pt3x, (float)t->pt3y, (float)1.0 ) )
	{
#ifdef VERBOSE_INSTRUMENTATION
		sprintf(buffer, "DETERMINENT FAILURE: %f, %f, %f\nPoint Width=%f\n%d, %d\n%d, %d\n%d, %d", 
			t->distance12, t->distance23, t->distance31, point_width, t->pt1x, t->pt1y, t->pt2x, t->pt2y,
			t->pt3x, t->pt3y);
		mbstowcs(buff, buffer, sizeof(buffer));
		MessageBox(NULL, buff, L"DETERMINENT", MB_OK);
#endif //VERBOSE_INSTRUMENTATION	
		t->point1_id = t->point2_id = t->point3_id = 0;
		memset(t, 0, sizeof(TRIANGLE));
		return 0;
	}
	return 1;
}

int check_for_duplicates(TRIANGLE *t)
{
	if( t[0].point1_id == t[1].point1_id )
		memset(&t[1], 0, sizeof(TRIANGLE));
	if( t[0].point1_id == t[2].point1_id )
		memset(&t[2], 0, sizeof(TRIANGLE));
	if( t[0].point1_id == t[3].point1_id )
		memset(&t[3], 0, sizeof(TRIANGLE));
	if( t[1].point1_id == t[2].point1_id )
		memset(&t[2], 0, sizeof(TRIANGLE));
	if( t[1].point1_id == t[3].point1_id )
		memset(&t[3], 0, sizeof(TRIANGLE));
	if( t[2].point1_id == t[3].point1_id )
		memset(&t[3], 0, sizeof(TRIANGLE));
	return 0;
}

int fill_slopes_distances(TRIANGLE *t)
{
	t->slope12 = (double)(t->pt2y - t->pt1y) / (double)(t->pt2x - t->pt1x);
	t->slope23 = (double)(t->pt3y - t->pt2y) / (double)(t->pt3x - t->pt2x);
	t->slope31 = (double)(t->pt1y - t->pt3y) / (double)(t->pt1x - t->pt3x);

	t->distance12 = distance_2D(t->pt1x, t->pt2x, t->pt1y, t->pt2y);
	t->distance23 = distance_2D(t->pt3x, t->pt2x, t->pt3y, t->pt2y);
	t->distance31 = distance_2D(t->pt1x, t->pt3x, t->pt1y, t->pt3y);

	// using law of cosines
	t->angle1 = acos((pow(t->distance23, 2.0) - pow(t->distance12,2.0) - pow(t->distance31,2.0))/(-2.0*t->distance12*t->distance31)) * 180.0 / PI;
	t->angle2 = acos((pow(t->distance31, 2.0) - pow(t->distance12,2.0) - pow(t->distance23,2.0))/(-2.0*t->distance23*t->distance12)) * 180.0 / PI;
	t->angle3 = acos((pow(t->distance12, 2.0) - pow(t->distance31,2.0) - pow(t->distance23,2.0))/(-2.0*t->distance23*t->distance31)) * 180.0 / PI;

	return 0;
}

int fill_triangle( TRIANGLE *t, PLACEMAT_POINT *pt1, SIMPLE_POINT *pt2, SIMPLE_POINT *pt3)
{
	float dist1, dist2, dist3;
	// From point 1 to point 2
	dist1 = distance_2D(pt1->photox, pt2->x, pt1->photoy, pt2->y);
	// From point 2 to point 3
	dist2 = distance_2D(pt3->x, pt2->x, pt3->y, pt2->y);
	// From point 1 to point 3
	dist3 = distance_2D(pt1->photox, pt3->x, pt1->photoy, pt3->y);

	// if corner angle is point2
	if( dist1 < dist3 && dist2 < dist3 )
	{
		t->point1_id = pt2->id;
		t->pt1x = pt2->x;
		t->pt1y = pt2->y;
		t->point1_shading = pt2->shading;
		t->point1_map_id = pt2->map_id;

		t->point2_id = pt1->id;
		t->pt2x = pt1->photox;
		t->pt2y = pt1->photoy;
		t->point2_shading = pt1->shading;
		t->point2_map_id = pt1->map_id;

		t->point3_id = pt3->id;
		t->pt3x = pt3->x;
		t->pt3y = pt3->y;
		t->point3_shading = pt3->shading;
		t->point3_map_id = pt3->map_id;
	}
	// if corner angle is point3
	if( dist2 < dist1 && dist3 < dist1 )
	{
		t->point1_id = pt3->id;
		t->pt1x = pt3->x;
		t->pt1y = pt3->y;
		t->point1_shading = pt3->shading;
		t->point1_map_id = pt3->map_id;

		t->point2_id = pt1->id;
		t->pt2x = pt1->photox;
		t->pt2y = pt1->photoy;
		t->point2_shading = pt1->shading;
		t->point2_map_id = pt1->map_id;

		t->point3_id = pt2->id;
		t->pt3x = pt2->x;
		t->pt3y = pt2->y;
		t->point3_shading = pt2->shading;
		t->point3_map_id = pt2->map_id;
	}
	// if corner angle is point1
	if( dist1 < dist2 && dist3 < dist2 )
	{
		t->point1_id = pt1->id;
		t->pt1x = pt1->photox;
		t->pt1y = pt1->photoy;
		t->point1_shading = pt1->shading;
		t->point1_map_id = pt1->map_id;

		t->point2_id = pt2->id;
		t->pt2x = pt2->x;
		t->pt2y = pt2->y;
		t->point2_shading = pt2->shading;
		t->point2_map_id = pt2->map_id;

		t->point3_id = pt3->id;
		t->pt3x = pt3->x;
		t->pt3y = pt3->y;
		t->point3_shading = pt3->shading;
		t->point3_map_id = pt3->map_id;
	}
	return 0;
}

int find_max_x(PLACEMAT_POINT *sp, int m)
{
	int	temp_x;
	int point_id;
	int i;

	temp_x = sp[0].photox;
	point_id = 0;
	for( i=1; i<m; i++)
	{
		if( sp[i].photox > temp_x )
		{
			temp_x = sp[i].photox;
			point_id = i;
		}
	}
	return point_id;
}
int find_max_y(PLACEMAT_POINT *sp, int m)
{
	int	temp_y;
	int point_id;
	int i;

	temp_y = sp[0].photoy;
	point_id = 0;
	for( i=1; i<m; i++)
	{
		if( sp[i].photoy > temp_y )
		{
			temp_y = sp[i].photoy;
			point_id = i;
		}
	}
	return point_id;
}
int find_min_x(PLACEMAT_POINT *sp, int m)
{
	int	temp_x;
	int point_id;
	int i;

	temp_x = sp[0].photox;
	point_id = 0;
	for( i=1; i<m; i++)
	{
		if( sp[i].photox < temp_x )
		{
			temp_x = sp[i].photox;
			point_id = i;
		}
	}
	return point_id;
}
int find_min_y(PLACEMAT_POINT *sp, int m)
{
	int	temp_y;
	int point_id;
	int i;

	temp_y = sp[0].photoy;
	point_id = 0;
	for( i=1; i<m; i++)
	{
		if( sp[i].photoy < temp_y )
		{
			temp_y = sp[i].photoy;
			point_id = i;
		}
	}
	return point_id;
}
float distance_2D( int x1, int x2, int y1, int y2)
{
	float sq;

	if( x1 == x2 && y1 == y2 )
		return FLT_MAX;
	sq = (float)sqrt(pow((x2 - x1),2.0) + pow((y2 - y1),2.0));
	return sq;
}

int find_closest_two_points(PLACEMAT_POINT *point_list, int count, PLACEMAT_POINT *sp, SIMPLE_POINT *near1, SIMPLE_POINT *near2)
{
	int i;
	float distance1, distance2;
	float dist;

	if( sp->photox == point_list[0].photox && sp->photoy == point_list[0].photoy )
		distance1 = FLT_MAX;
	else
		distance1 = distance_2D(sp->photox, point_list[0].photox, sp->photoy, point_list[0].photoy);
	distance2 = FLT_MAX;
	for( i=0; i<count; i++)
	{
		if( (dist=distance_2D(sp->photox, point_list[i].photox, sp->photoy, point_list[i].photoy )) < distance1 )
		{
			distance2 = distance1;
			near2->x = near1->x;
			near2->y = near1->y;
			near2->id = near1->id;
			near2->shading = near1->shading;
			near2->map_id = near1->map_id;

			distance1 = dist;
			near1->x = point_list[i].photox;
			near1->y = point_list[i].photoy;
			near1->id = point_list[i].id;
			near1->shading = point_list[i].shading;
			near1->map_id = point_list[i].map_id;
			point_list[i].triangle_part = 1;
		}
		else if( (dist=distance_2D(sp->photox, point_list[i].photox, sp->photoy, point_list[i].photoy )) < distance2 )
		{
			distance2 = dist;
			near2->x = point_list[i].photox;
			near2->y = point_list[i].photoy;
			near2->id = point_list[i].id;
			near2->shading = point_list[i].shading;
			near2->map_id = point_list[i].map_id;
			point_list[i].triangle_part = 1;
		}
	}
	return (0);
}

int compare_distances( const void *arg1, const void *arg2 )
{
   if( ((POINT_PROPS *)arg1)->distance < ((POINT_PROPS *)arg2)->distance )
	   return -1;
   else if( ((POINT_PROPS *)arg1)->distance > ((POINT_PROPS *)arg2)->distance )
	   return 1;
   else
	   return 0;
}

int compare_slopes( const void *arg1, const void *arg2 )
{
   if( ((POINT_PROPS *)arg1)->slope < ((POINT_PROPS *)arg2)->slope)
	   return -1;
   else if( ((POINT_PROPS *)arg1)->slope > ((POINT_PROPS *)arg2)->slope)
	   return 1;
   else
	   return 0;
}


void max_simple_point_array(SIMPLE_POINT *sp, int count)
{
	int i;
	for(i=0; i<count; i++)
	{
		sp[i].x = INT_MAX;
		sp[i].y = INT_MAX;
		sp[i].shading = INT_MAX;
	}
}

void null_simple_point_array(SIMPLE_POINT *sp, int count)
{
	int i;
	for(i=0; i<count; i++)
	{
		sp[i].x = 0;
		sp[i].y = 0;
		sp[i].shading = 0;
	}
}

int assign_coordinates(char *filename)
{
	IplImage *image;
	int	current_image;
	PLACEMAT_POINT *sp;
	int count;
	float **slope;
	float **distance;
	int i,j,m;
	double minimums[26];
	SIMPLE_POINT minimum_coordinates[64];
	POINT_PROPS pprops[512];
	TRIANGLE tri[12];
//	int max_x, max_y, min_x, min_y;
	SIMPLE_POINT near1, near2;
	float average_diameter, nearness_cutoff;
	FILE *temp;
	CvPoint pt1, pt2;
	CvFont font;
	char coordinates[256];
//	char *p;
	char buffer[MAX_PATH];
	wchar_t buff[MAX_PATH];
 
//	PLACEMAT_POINT arr_by_slope[8][4];
#define DUMMY
#ifdef DUMMY //VERBOSE_INSTRUMENTATION
	FILE *output1;
	FILE *output2;
	FILE *output3;
#endif //VERBOSE_INSTRUMENTATION

	current_image = get_current_image();
	sp = get_placemat_points(current_image);
	count = get_monument_point_count();

	if( count == 3 )
	{
		MessageBox(NULL,"Only 3 Points do not make a solution",L"ERROR", MB_ICONHAND);
		free(sp);
		return -1;
	}
	if( count == 0 )
	{
		sprintf(buffer,"No Monument Points-CI:%d, sp=%p", current_image, sp);
		mbstowcs(buff, buffer, sizeof(buffer));
		MessageBox(NULL,buff,L"ERROR-assign_coordinates", MB_ICONHAND);
		return(-1);
	}
    if( (image = cvLoadImage( filename, 1)) == 0 )
        return -1;

	for( i=0,j=0; i<count; i++)
	{
		average_diameter = (float)(sp[i].x_diameter) + (float)(sp[i].y_diameter);
		j+=2;
	}
	average_diameter = average_diameter / j;
	nearness_cutoff = average_diameter * 3;

	slope = (float**)malloc(count * sizeof(float*));
    distance = (float**)malloc(count * sizeof(float*));
    for(i = 0; i < count; i++)
	{
	    slope[i] = (float*)malloc(count * sizeof(float));
	    distance[i] = (float*)malloc(count * sizeof(float));
	}
	// calculate the slopes and distance from every point to every other point
	for( i = 0; i<count; i++)
	{
		for( j=0; j<count; j++)
		{
			if( i==j )
			{
				slope[i][j] = FLT_MIN;
			}
			if( sp[i].photox == sp[j].photox )
			{
				slope[i][j] = FLT_MAX;
			}
			else
			{
				slope[i][j] = (float)(sp[i].photoy-sp[j].photoy)/(float)(sp[i].photox - sp[j].photox);
			}
			distance[i][j] = (float)sqrt(pow((sp[i].photox - sp[j].photox),2.0) + pow((sp[i].photoy - sp[j].photoy),2.0));
		}
	}


	for(i=0; i<26; i++)
		minimums[i] = DBL_MAX;

	max_simple_point_array(&minimum_coordinates[0], 26);
	m = 0;
	for( i=0; i<count; i++)
	{
		for( j=0; j<i; j++)
		{
			pprops[m].distance = distance[i][j];
			pprops[m].slope = slope[i][j];
			pprops[m].pointfrom = i;
			pprops[m].pointto = j;
			pprops[m].pointfrom_id = sp[i].id;
			pprops[m].pointto_id = sp[j].id;
			m++;
		}
	}
/*	
	max_x = find_max_x(sp, count);
	max_y = find_max_y(sp, count);
	min_x = find_min_x(sp, count);
	min_y = find_min_y(sp, count);
*/
	memset(tri, 0, sizeof(tri));

	m = 0;
	for( i=0; i<count; i++)
	{
		if( sp[i].triangle_part == 0 )
		{
			find_closest_two_points(sp, count, &sp[i], &near1, &near2);
			fill_triangle(&tri[m], &sp[i], &near1, &near2);
			fill_slopes_distances(&tri[m]);
			if( validate_triangle(&tri[m], (sp[i].x_diameter+sp[i].y_diameter)/2.0 ) )
			{
				m++;
				sp[i].triangle_part = 1;
			}
		}
	}


/*
	find_closest_two_points(sp, count, &sp[max_y], &near1, &near2);
	fill_triangle(&tri[1], &sp[max_y], &near1, &near2);
	fill_slopes_distances(&tri[1]);
	validate_triangle(&tri[1]);
	
	find_closest_two_points(sp, count, &sp[min_x], &near1, &near2);
	fill_triangle(&tri[2], &sp[min_x], &near1, &near2);
	fill_slopes_distances(&tri[2]);
	validate_triangle(&tri[2]);
	
	find_closest_two_points(sp, count, &sp[min_y], &near1, &near2);
	fill_triangle(&tri[3], &sp[min_y], &near1, &near2);
	fill_slopes_distances(&tri[3]);
	validate_triangle(&tri[3]);
*/
	for( i=0; i<m; i++ )
	{
		if( tri[i].point1_id )
		{
			assign_triangle_to_map(&tri[i]);
		}
	}

	//assign the other triangle corner points by
	//looking at the other triangles
	assign_other_triangles_to_map(tri);


	check_for_duplicates(tri);

	for( i=0; i<m; i++ )
	{
		if( tri[i].point1_map_id > 0 && tri[i].point1_map_id <= 22 )
			update_3D_coordinates(tri[i].point1_id, tri[i].point1_map_id );
		if( tri[i].point2_map_id > 0 && tri[i].point2_map_id <= 22 )
			update_3D_coordinates(tri[i].point2_id, tri[i].point2_map_id );
		if( tri[i].point3_map_id > 0 && tri[i].point3_map_id <= 22 )
			update_3D_coordinates(tri[i].point3_id, tri[i].point3_map_id );
	}
	// Look for the groups of 4 points together on the top
	// and bottom of the placemat
	find_quad_points(sp, distance, count);
	for( i=0; i<count; i++ )
	{
		if( sp[i].map_id != 0 )
		{
			update_3D_coordinates(sp[i].id, sp[i].map_id );
		}
	}
//	free(sp);
//	sp = get_image_monument_points(current_image);

	for( i=0; i<4; i++ )
	{
		if( tri[i].point1_id )
		{
			group_by_slope(tri[i].pt1x, tri[i].pt1y, tri[i].point1_map_id, tri[i].point1_shading, tri[i].pt2x, tri[i].pt2y, tri[i].point2_map_id, tri[i].point2_shading, sp, count);
			group_by_slope(tri[i].pt1x, tri[i].pt1y, tri[i].point1_map_id, tri[i].point1_shading, tri[i].pt3x, tri[i].pt3y, tri[i].point3_map_id, tri[i].point3_shading, sp, count);
		}
	}
	qsort(pprops, m, sizeof(POINT_PROPS), compare_distances);
	
	qsort(pprops, m, sizeof(POINT_PROPS), compare_slopes);

//#define DUMMY	
#ifdef DUMMY
	output1 = fopen("slope.txt", "w+");
	output2 = fopen("distance.txt", "w+");
	output3 = fopen("pprops.txt", "w+");

	for( i = 0; i<count; i++)
	{
		fprintf(output1, "%d\t%d\t%d\t", sp[i].photox, sp[i].photoy, sp[i].shading);
		fprintf(output2, "%d\t%d\t%d\t", sp[i].photox, sp[i].photoy, sp[i].shading);

		for( j=0; j<count; j++)
		{
			fprintf(output1,"%g\t", slope[i][j]);
			fprintf(output2,"%g\t", distance[i][j]);
		}
		fprintf(output1, "\n");
		fprintf(output2,"\n");
	}
	fprintf(output3, "Slope\tDistance\tFrom PT\tFrom PT id\tTo PT\tTo PT id\n");
	for( i=0; i<250; i++)
	{
		fprintf(output3, "%10.2f\t%10.2f\t%d\t%d\t%d\t%d\n", 
			pprops[i].slope, pprops[i].distance,
			pprops[i].pointfrom, pprops[i].pointfrom_id,
			pprops[i].pointto, pprops[i].pointto_id);
	}
///  FREE ALL RESOURCES
	fclose(output1);
	fclose(output2);
	fclose(output3);
#endif   //VERBOSE_INSTRUMENTATION 
	cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 2.0, 1.5,0,3,8);
	free(sp);
	current_image = get_current_image();
	sp = get_placemat_points(current_image);
	count = get_monument_point_count();

	cvSaveImage("MonumentColors.png",image,0);
	cvReleaseImage(&image);
	image = cvLoadImage("MonumentColors.png", 1);
	temp = fopen("quadupdates.txt","w+");
	for( i=0; i<count; i++ )
	{
		fprintf(temp,"sp[%d].id=%d,  sp[%d].map_id=%d\n", i, sp[i].id, i, sp[i].map_id);
		if( sp[i].map_id != 0 )
		{
			pt1.x = sp[i].photox-12; pt1.y = sp[i].photoy-12;
			sprintf(coordinates, "%d", sp[i].map_id);
			cvPutText(image,coordinates,pt1,&font, cvScalarAll(0,255,0,0.5));
			update_3D_coordinates(sp[i].id, sp[i].map_id );
		}
	}
	sprintf(buffer, "%s-MonumentPoints.png", get_filename_base());
	set_monument_point_filename(buffer);
	cvSaveImage(buffer,image, 0);
	cvReleaseImage(&image);
	fclose(temp);

	  for(i = 0; i < count; i++)  
	  {
		  if( slope[i] )
			  free(slope[i]);
		  if(distance[i])
			free(distance[i]);
	  }
	  if( slope )
		  free(slope);
	  if( distance)
		free(distance);

	free(sp);
	return 0;
}
