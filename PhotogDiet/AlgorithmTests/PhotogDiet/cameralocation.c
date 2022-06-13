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
#include <string.h>
#include <time.h>

#include "geometry.h"
#include "database.h"
#include "photog.h"

int locate_camera(char *filename)
{
	int	current_image;
	PLACEMAT_POINT *pp;
	int count, valid_points;
	int i;
	CAMERA_INTERNALS camera_int;
	float z;

	CvMat *world_points;
	CvMat *image_points;
	CvMat *point_counts;
	

	CvMat *intrinsic_parameters;
	CvMat *distortion_coefficients;
	CvMat *rotation_vector;
	CvMat *translation_vector;
	CvMat *rotation_matrix;
	CvMat *Rinv;
	CvMat *CL;
	CvMat *T;

#ifdef VERBOSE_INSTRUMENTATION
	FILE *output1;
	FILE *output2;
	FILE *output3;
#endif //VERBOSE_INSTRUMENTATION

	current_image = get_current_image();
	
	// GET PLACEMAT POINTS
	pp = get_placemat_points(current_image);
	count = get_monument_point_count();
	valid_points = 0;
	for( i=0; i<count; i++)
	{
		if( pp[i].world_true )
		{
			valid_points++;
		}
	}
	if( count == 0 )
	{
		//MessageBox(NULL,L"NO MONUMENT POINTS",L"ERROR", MB_ICONHAND);
		return(-1);
	}
	if( valid_points < 4 )
	{
		//MessageBox(NULL,L"NOT ENOUGH POINTS FOR A SOLUTION",L"ERROR", MB_ICONHAND);
		free(pp);
		return(-1);
	}
	world_points = cvCreateMat(valid_points , 3, CV_32FC1);
	image_points = cvCreateMat(valid_points , 2, CV_32FC1);
	point_counts = cvCreateMat(1, 1, CV_32SC1);

	valid_points = 0;
	for( i=0; i<count; i++)
	{
		if( pp[i].world_true )
		{

			CV_MAT_ELEM(*world_points, float, valid_points,0) = (float)(pp[i].worldx);
			CV_MAT_ELEM(*world_points, float, valid_points,1) = (float)(pp[i].worldy);
			CV_MAT_ELEM(*world_points, float, valid_points,2) = (float)(pp[i].worldz);

			CV_MAT_ELEM(*image_points, float, valid_points,0) = (float)(pp[i].photox);
			CV_MAT_ELEM(*image_points, float, valid_points,1) = (float)(pp[i].photoy);
			valid_points++;
		}
	}

	CV_MAT_ELEM(*point_counts, int, 0,0) = valid_points;

	// GET CAMERA DISTORTION AND INTRINSIC VECTORS
	intrinsic_parameters = cvCreateMat(3, 3, CV_32FC1);
	distortion_coefficients = cvCreateMat(1,5, CV_32FC1);
	get_camera_internal_params(&camera_int, get_current_image());
	
	CV_MAT_ELEM(*intrinsic_parameters, float, 0,0) = camera_int.fx;
	CV_MAT_ELEM(*intrinsic_parameters, float, 0,1) = 0.0;
	CV_MAT_ELEM(*intrinsic_parameters, float, 0,2) = camera_int.cx;
	CV_MAT_ELEM(*intrinsic_parameters, float, 1,0) = 0.0;
	CV_MAT_ELEM(*intrinsic_parameters, float, 1,1) = camera_int.fy;
	CV_MAT_ELEM(*intrinsic_parameters, float, 1,2) = camera_int.cy;
	CV_MAT_ELEM(*intrinsic_parameters, float, 2,0) = 0.0;
	CV_MAT_ELEM(*intrinsic_parameters, float, 2,1) = 0.0;
	CV_MAT_ELEM(*intrinsic_parameters, float, 2,2) = 1.0;

	CV_MAT_ELEM(*distortion_coefficients, float, 0,0) = camera_int.k1;	
	CV_MAT_ELEM(*distortion_coefficients, float, 0,1) = camera_int.k2;	
	CV_MAT_ELEM(*distortion_coefficients, float, 0,2) = camera_int.p1;	
	CV_MAT_ELEM(*distortion_coefficients, float, 0,3) = camera_int.p2;
	CV_MAT_ELEM(*distortion_coefficients, float, 0,4) = camera_int.k3;

	cvSave("World_Points4.xml", world_points,"WORLD-POINTS", "My World Points", cvAttrList(0,0));
	cvSave("image_points4.xml", image_points, "IMAGE-POINTS", "My Image Points", cvAttrList(0,0));
	cvSave("intrinsic_parameters4.xml", intrinsic_parameters, "INTRINSIC_MATRIX", "stuff", cvAttrList(0,0));
	cvSave("distortion_coefficients4.xml", distortion_coefficients, "DISTORTION_COEFF", "stuff", cvAttrList(0,0));

	rotation_vector = cvCreateMat(1,3, CV_32FC1);
	Rinv = cvCreateMat(3,3, CV_32FC1);
	translation_vector = cvCreateMat(1,3, CV_32FC1);
	CL = cvCreateMat(3,1, CV_32FC1);
	T = cvCreateMat(3,1, CV_32FC1);

	cvFindExtrinsicCameraParams2(world_points, image_points, 
		intrinsic_parameters, distortion_coefficients, rotation_vector, translation_vector,0);

	cvSave("Rotation4.xml", rotation_vector,"ROTATION", "My rotation", cvAttrList(0,0));
	cvSave("Translation4.xml", translation_vector, "TRANSLATION", "My translations", cvAttrList(0,0));
	
	rotation_matrix = cvCreateMat(3,3,CV_32FC1);

	cvRodrigues2(rotation_vector, rotation_matrix, NULL);
	cvSave("rotation_matrix.xml", rotation_matrix, "ROTATION", "My rotation matrix", cvAttrList(0,0));
	
	cvInv(rotation_matrix, Rinv, CV_SVD);
	cvSave("Rinv_matrix.xml", Rinv, "ROTATION", "My inverse rotation matrix", cvAttrList(0,0));

	CV_MAT_ELEM(*T, float, 0,0) = CV_MAT_ELEM(*translation_vector, float, 0,0);
	CV_MAT_ELEM(*T, float, 1,0) = CV_MAT_ELEM(*translation_vector, float, 0,1);
	CV_MAT_ELEM(*T, float, 2,0) = CV_MAT_ELEM(*translation_vector, float, 0,2);

	cvMatMul(Rinv, T, CL);
	cvSave("CL_matrix.xml", CL, "CameraLocation", "My camera location", cvAttrList(0,0));

	CV_MAT_ELEM(*CL, float, 0,0) = CV_MAT_ELEM(*CL, float, 0,0) * (-1.0f);
	CV_MAT_ELEM(*CL, float, 1,0) = CV_MAT_ELEM(*CL, float, 1,0) * (-1.0f);
	CV_MAT_ELEM(*CL, float, 2,0) = CV_MAT_ELEM(*CL, float, 2,0) * (-1.0f);

	update_camera_location(current_image, 
		CV_MAT_ELEM(*rotation_matrix, float, 0,0),
		CV_MAT_ELEM(*rotation_matrix, float, 0,1),
		CV_MAT_ELEM(*rotation_matrix, float, 0,2),
		CV_MAT_ELEM(*rotation_matrix, float, 1,0),
		CV_MAT_ELEM(*rotation_matrix, float, 1,1),
		CV_MAT_ELEM(*rotation_matrix, float, 1,2),
		CV_MAT_ELEM(*rotation_matrix, float, 2,0),
		CV_MAT_ELEM(*rotation_matrix, float, 2,1),
		CV_MAT_ELEM(*rotation_matrix, float, 2,2),

		CV_MAT_ELEM(*translation_vector, float, 0,0),
		CV_MAT_ELEM(*translation_vector, float, 0,1),
		CV_MAT_ELEM(*translation_vector, float, 0,2),
		
		CV_MAT_ELEM(*CL, float, 0,0),
		CV_MAT_ELEM(*CL, float, 1,0),
		CV_MAT_ELEM(*CL, float, 2,0));

	z = CV_MAT_ELEM(*CL, float, 2,0);

	free(pp);


	cvReleaseMat(&world_points);
	cvReleaseMat(&image_points);
	cvReleaseMat(&point_counts);
	cvReleaseMat(&CL);
	cvReleaseMat(&T);
	cvReleaseMat(&Rinv);
	cvReleaseMat(&intrinsic_parameters);
	cvReleaseMat(&distortion_coefficients);
	cvReleaseMat(&rotation_vector);
	cvReleaseMat(&translation_vector);
	cvReleaseMat(&rotation_matrix);

	if( z < 10.0 )
		return(-1);
	return(0);
}

