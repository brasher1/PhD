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

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "linklist.h"
#include "database.h"

#ifdef _CH_
#pragma package <opencv>
#endif
#define CV_NO_BACKWARD_COMPATIBILITY

#ifndef _EiC
#include "cv.h"
#include "highgui.h"
#endif

int openCVsegmentation(char *filename)
{
	IplImage *image, *clone;
	char *p;
	char name[MAX_PATH];
	CvMemStorage *storage;
	CvSeq *comp;
	int i,j,k;
	storage = cvCreateMemStorage(0);
	comp = NULL;

	image = cvLoadImage(filename,1);
	clone = cvCloneImage(image);

	cvPyrMeanShiftFiltering(image, clone, 5, 40, 2, cvTermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 5, 1));


	strcpy(name, filename);

	p = strrchr(name, '.');
	*p = '\0';
	strcat(name, "-PyrMeanShiftSegmentation.png");
	cvSaveImage(name, clone, 0);

	for( i=0; i<9000; i+=1000 )
	{
		for( j=0; j<225; j+=75)
		{
			for(k=2; k<6; k+=2)
			{
				cvPyrSegmentation(image, clone, storage, &comp, k, i, j);

				*p = '\0';
				sprintf(p, "-PyrSeg-%d-%d-%d.png", k, i, j );
	//			strcat(name, "-PyrSegmentation.png");
				cvSaveImage(name, clone, 0);
				cvReleaseMemStorage(&storage);
			}
		}
	}
	cvReleaseImage(&clone);
	cvReleaseImage(&image);
}

int undistortImage(char *filename)
{
	CvMat * intrinsic_matrix;
	CvMat * distortion_matrix;
	CAMERA_INTERNALS cameraInt;
	IplImage *gray, *t, *image;

	get_camera_internal_params(&cameraInt, get_current_image());
	intrinsic_matrix = cvCreateMat(3,3,CV_32FC1);
	distortion_matrix = cvCreateMat(5,1,CV_32FC1);

	CV_MAT_ELEM(*intrinsic_matrix, float, 0,0) = cameraInt.fx;
	CV_MAT_ELEM(*intrinsic_matrix, float, 0,1) = 0.0;
	CV_MAT_ELEM(*intrinsic_matrix, float, 0,2) = cameraInt.cx;

	CV_MAT_ELEM(*intrinsic_matrix, float, 1,0) = 0.0;
	CV_MAT_ELEM(*intrinsic_matrix, float, 1,1) = cameraInt.fy;
	CV_MAT_ELEM(*intrinsic_matrix, float, 1,2) = cameraInt.cy;

	CV_MAT_ELEM(*intrinsic_matrix, float, 2,0) = 0.0;
	CV_MAT_ELEM(*intrinsic_matrix, float, 2,1) = 0.0;
	CV_MAT_ELEM(*intrinsic_matrix, float, 2,2) = 1.0;


	CV_MAT_ELEM(*distortion_matrix, float, 0,0) = cameraInt.k1;
	CV_MAT_ELEM(*distortion_matrix, float, 1,0) = cameraInt.k2;
	CV_MAT_ELEM(*distortion_matrix, float, 2,0) = cameraInt.p1;
	CV_MAT_ELEM(*distortion_matrix, float, 3,0) = cameraInt.p2;
	CV_MAT_ELEM(*distortion_matrix, float, 4,0) = cameraInt.k3;

	image = cvLoadImage(filename, 0);
	cvShowImage("Original Image", image);
	cvSaveImage("OriginalImage.png", image, 0);

//	mapx = cvCreateImage( cvGetSize(image), IPL_DEPTH_32F, 1);
	t = cvCloneImage(image);
//	cvShowImage("Cloned Image", t);

	gray = cvCreateImage( cvGetSize(image), 8, 1);
	//cvShowImage("Gray-scale Image", gray);

	cvUndistort2(image, gray, intrinsic_matrix, distortion_matrix);
	cvShowImage("Undistorted Image", gray);
	cvSaveImage("UndistortedImage.png", gray, 0);

	cvWaitKey(0);

	cvReleaseMat(&intrinsic_matrix);
	cvReleaseMat(&distortion_matrix);
	cvReleaseImage(&image);
	cvReleaseImage(&t);
	cvReleaseImage(&gray);
}


int collect_blob_properties(void *data, long data_size)
{
//	cvDFT(
	// TODO: Retrieve blobs in image

	// TODO: For each blob in image
	
	//		TODO: Calculate DFT
	//				calc entropy
	//				calc energy
	//				calc inertia
	//
	//			store values in database
	//  END FOR
}

int group_blobs_by_properties(int image_id1, int image_id2, int image_id3)
{
	// TODO:  For each image
	//			retrieve entropy, energy and inertia
	//   END FOR

	// TODO: For each unmatched blob in each image
	//          find closest match in other images
	//   END FOR

	// TODO: Store matched blobs as same food item in database
	//
}

#ifdef TRUE2
CvMat* A = cvCreateMat( M1, N1, CVg32F );
CvMat* B = cvCreateMat( M2, N2, A->type );
// it is also possible to have only abs(M2-M1)+1 \times abs(N2-N1)+1
// part of the full convolution result
CvMat* conv = cvCreateMat( A->rows + B->rows - 1, A->cols + B->cols - 1,
A->type );
// initialize A and B
...
int dftgM = cvGetOptimalDFTSize( A->rows + B->rows - 1 );
int dftgN = cvGetOptimalDFTSize( A->cols + B->cols - 1 );
CvMat* dftgA = cvCreateMat( dft\_M, dft\_N, A->type );
CvMat* dftgB = cvCreateMat( dft\_M, dft\_N, B->type );
CvMat tmp;
// copy A to dftgA and pad dft\_A with zeros
cvGetSubRect( dftgA, &tmp, cvRect(0,0,A->cols,A->rows));
cvCopy( A, &tmp );
cvGetSubRect( dftgA, &tmp, cvRect(A->cols,0,dft\_A->cols - A->cols,A->rows));
cvZero( &tmp );
// no need to pad bottom part of dftgA with zeros because of
// use nonzerogrows parameter in cvDFT() call below
cvDFT( dftgA, dft\_A, CV\_DXT\_FORWARD, A->rows );
// repeat the same with the second array
cvGetSubRect( dftgB, &tmp, cvRect(0,0,B->cols,B->rows));
cvCopy( B, &tmp );
cvGetSubRect( dftgB, &tmp, cvRect(B->cols,0,dft\_B->cols - B->cols,B->rows));
cvZero( &tmp );
// no need to pad bottom part of dftgB with zeros because of
// use nonzerogrows parameter in cvDFT() call below
cvDFT( dftgB, dft\_B, CV\_DXT\_FORWARD, B->rows );
cvMulSpectrums( dftgA, dft\_B, dft\_A, 0 /* or CV\_DXT\_MUL\_CONJ to get
correlation rather than convolution */ );
cvDFT( dftgA, dft\_A, CV\_DXT\_INV\_SCALE, conv->rows ); // calculate only
// the top part
cvGetSubRect( dftgA, &tmp, cvRect(0,0,conv->cols,conv->rows) );
142 CHAPTER 1. CXCORE
cvCopy( &tmp, conv );
#endif
