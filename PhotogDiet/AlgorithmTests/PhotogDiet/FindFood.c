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
#include <math.h>
#include "linklist.h"
#include "database.h"

#include "geometry.h"
FILE *debugger;

//#define VERBOSE_INSTRUMENTATION
IMAGEPOINT lastpoint;
QtyOfColors *lastPixelColor = NULL;
SIMPLEIMAGEPOINT spt[1600*1200];
int edgeptcount=0;
header_t *bloblist;
header_t *pointlist, *pointlist2;
IMAGEPOINT *imagepoint;
int mid_placemat_x, mid_placemat_y;
int photox, photoy;
extern HINSTANCE hInst;
int combinex[12];
int combiney[12];
int combinecount;

int foodx[12];
int foody[12];
int foodcount;

int mergecolors(IplImage *img)
{
	int i;
	int red, green, blue;
	uchar *p;
	if( combinecount < 12 && combinecount > -1)
	{
		p = img->imageData + combiney[combinecount] * img->widthStep;
		red = p[3*combinex[combinecount]+2];
		green = p[3*combinex[combinecount]+1];
		blue = p[3*combinex[combinecount]+0];
	}
	for( i=0; i<combinecount; i++)
	{
		
	}

}

void my_mouse_callback2( int event, int x, int y, int flags, void *param)
{

	IplImage *img = (IplImage*)param;
	char buffer[MAX_PATH];
	wchar_t buff[MAX_PATH];

	switch( event)
	{
	case CV_EVENT_LBUTTONUP:
		sprintf(buffer, "(X,Y) = ( %d, %d )", x, y);
		mbstowcs(buff, buffer, sizeof(buff));
		foodx[foodcount] = x;
		foody[foodcount] = y;
		foodcount++;
		//MessageBox(NULL,buff,L"POINT", MB_OK);

		break;
	}
}

float pointInsideEllipse(header_t *plate, int x, int y)
{
	float x1, y1, answer;
	FILE *out;
	char buffer[MAX_PATH];

	sprintf(buffer, "%s-EllipseEnclosedPoints.txt", get_filename_base());
	out = fopen(buffer,"a+");
	x1 = cos(plate->Theta)*(x-plate->Cx) + sin(plate->Theta)*(y-plate->Cy);
	y1 = -sin(plate->Theta)*(x-plate->Cx) + cos(plate->Theta)*(y-plate->Cy);

	if( plate->R1 >= plate->R2 )
		answer = ((x1*x1) / ((plate->R1/2)*(plate->R1/2))) + ((y1*y1)/((plate->R2/2)*(plate->R2/2)));
	else
		answer = ((x1*x1) / ((plate->R2/2)*(plate->R2/2))) + ((y1*y1)/((plate->R1/2)*(plate->R1/2)));

	fprintf(out, "R1:%f, R2:%f\n", plate->R1, plate->R2);
	fprintf(out,"%f, %f, %f, %d, %d\n", answer,
		plate->Cx, plate->Cy, x, y);
	fclose(out);
	return answer; 

}

int putBLOBInDatabase(header_t *fooditem, char *name)
{
	IplImage *image = 0;
	int photoblob;
	node_t *node1, *node2, *node3;
	FILE *out1, *out3, *out4, *out5, *out6;
	int i,j,k,count;
	header_t *singlepointlist;
	IMAGEPOINT * foodedgepoint;
	CvMoments moments;
	CvHuMoments hu_moments;
	CvMat food;
	CvRect rect;
	FILE *out, *out2;


	double moment_holder[3][2];
	char buffer[MAX_PATH];
	char *p;

	image = cvLoadImage(get_original_filename(), 1);
/*
	strcpy(buffer, get_original_filename());
		p = strrchr(buffer, '\\');
	sprintf(buffer,"Moments-%s.txt", p+1);


	out = fopen(buffer,"a+");
	fprintf(out,"Blob No., Channel,m00, m10, m01, m20, m11, m02, m30, m21, m12, m03, mu20, mu11, mu02, mu30, mu21, mu12, mu03, inv_sqrt_m00, hu1, hu2, hu3, hu4, hu5, hu6, hu7\n");
	fclose(out);

	rect = cvRect(fooditem->average_x-80, fooditem->average_y-80, 
		160, 160);

	out2 = fopen(name,"w+");
	fprintf(out2,"%d,%d, %d, %d\n", rect.x, rect.y, rect.width, rect.height);
	fclose(out2);

	cvSetImageROI(image,rect);*/
#ifdef OLDSTUFF	
	for(i=1; i<=3; i++)
	{
		char buff2[MAX_PATH];
		out = fopen(buffer,"a+");
//m00, m10, m01, m20, m11, m02, m30, m21, m12, m03, mu20, mu11, mu02, mu30, mu21, mu12, mu03, inv_sqrt_m00,hu1, hu2, hu3, hu4, hu5, hu6, hu7; 
		
		cvSetImageCOI(image,i);
		cvMoments(image,&moments,0);	
		moment_holder[i-1][0] = moments.m11;
		moment_holder[i-1][1] = moments.m12;
		cvGetHuMoments(&moments, &hu_moments);	
		fprintf(out,"%s,%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n",
			name, i,
			moments.m00, moments.m10, moments.m01, moments.m20, moments.m11, moments.m02,
			moments.m30, moments.m21, moments.m12, moments.m03, moments.mu20, moments.mu11,
			moments.mu02, moments.mu30, moments.mu21, moments.mu12, moments.mu03,
			moments.inv_sqrt_m00,
			hu_moments.hu1, hu_moments.hu2, hu_moments.hu3, hu_moments.hu4,
			hu_moments.hu5, hu_moments.hu6, hu_moments.hu7);
		fclose(out);
		cvSetImageCOI(image,0);
	}
#endif //OLDSTUFF
//	create_photoblob(get_current_image(),name, 0.0, 0.0, 0.0, 0.0);
/*	cvResetImageROI(image);
	cvReleaseImage(&image);*/

	create_photoblob(get_current_image(),name, 0.0, fooditem->average_x, fooditem->average_y);
	photoblob = get_current_photoblob();

	node1 = fooditem->first;
	for(i=0; i<fooditem->count; i++)
	{
		foodedgepoint = node1->item;
		
		create_blobpoint(photoblob, foodedgepoint->imageX, foodedgepoint->imageY);
		node1 = node1->next;
	}
	return photoblob;
}

int findFood(header_t *platelist)
{
	node_t *node1, *node2, *node3;
	FILE *out1, *out3, *out4, *out5, *out6;
	int i,j,k,count;
	header_t *singlepointlist;
	IMAGEPOINT *imgPoint;
	char *buffer[MAX_PATH];
	wchar_t buff[MAX_PATH];
	int flag;
	float answer;
	count = 1;
	foodcount = 0;

	node1 = bloblist->first;
	sprintf(buffer,"%s-FindFood.txt", get_filename_base());
	out1 = fopen(buffer,"w+");
	fprintf(out1, "PMaxX\tPMaxY\tPMinX\tPMinY\t"
			"BMaxX\tBMaxY\tBMinX\tBMinY\t"
			"Cx\tCy\tR1\tR2\tTheta\tBlobRed\tBlobGreen\tBlobBlue\n");
	sprintf(buffer, "%s-PlateEllipse.txt", get_filename_base());
	out3 = fopen(buffer, "w+");
	fprintf(out3, "Cx\tCy\tR1\tR2\tTheta\n");
	fprintf(out3, "%f\t%f\t%f\t%f\t%f\n",
		platelist->Cx, platelist->Cy,
		platelist->R1, platelist->R2,
		platelist->Theta);
	fclose(out3);
	for(i=0; i<bloblist->count; i++)
	{
		flag = 0;
		pointlist = node1->item;
		
		fprintf(out1, "%d\t%d\t%d\t%d"
			"\t%d\t%d\t%d\t%d\t"
			"%f\t%f\t%f\t%f\t%f\t%d\t%d\t%d\n",
			platelist->max_x, platelist->max_y, platelist->min_x,
			platelist->min_y, pointlist->max_x, pointlist->max_y,
			pointlist->min_x, pointlist->min_y,
			platelist->Cx, platelist->Cy,
			platelist->R1, platelist->R2,
			platelist->Theta,
			pointlist->red, pointlist->green, pointlist->blue);

		imgPoint = (IMAGEPOINT *)(pointlist->first->item);
		answer = pointInsideEllipse(platelist, imgPoint->imageX, imgPoint->imageY);
		if( answer < 1.05 )
		{
			fprintf(out1,"FIRST POINT IS INSIDE: %d, %d\n", imgPoint->imageX, imgPoint->imageY);
			flag++;
		}
		else
		{
			fprintf(out1,"FIRST POINT IS OUTSIDE: %d, %d:  ANSWER IS %f\n", imgPoint->imageX, imgPoint->imageY, answer);
		}
		imgPoint = (IMAGEPOINT *)(pointlist->last->item);
		answer = pointInsideEllipse(platelist, imgPoint->imageX, imgPoint->imageY);
		if(  answer < 1.05 )
		{
			fprintf(out1,"LAST POINT IS INSIDE: %d, %d\n", imgPoint->imageX, imgPoint->imageY);
			flag++;
		}
		else
		{
			fprintf(out1,"LAST POINT IS OUTSIDE: %d, %d:   ANSWER IS %f\n", imgPoint->imageX, imgPoint->imageY, answer);
		}
		answer = pointInsideEllipse(platelist, pointlist->average_x, pointlist->average_y );
		if( answer < 1.05 )
		{
			fprintf(out1,"AVERAGE POINT IS INSIDE: %d, %d\n", pointlist->average_x, pointlist->average_y );
			flag++;
		}
		else
		{
			fprintf(out1,"AVERAGE POINT IS OUTSIDE: %d, %d:   ANSWER IS %f\n", pointlist->average_x, pointlist->average_y, answer);
		}
		if( platelist->red == pointlist->red &&
			platelist->blue == pointlist->blue &&
			platelist->green == pointlist->green )
		{
			fprintf(out1,"BLOB IS PLATE\n");
			flag=0;
		}
	/*	if( platelist->max_x > pointlist->max_x &&
			platelist->max_y > pointlist->max_y &&
			platelist->min_x < pointlist->min_x &&
			platelist->min_y < pointlist->min_y )*/
		if( flag >=3 )
		{
			pointlist->isFood = 1;

			sprintf(buffer,"%d", count);	
			pointlist->photoblob_id = putBLOBInDatabase(pointlist, buffer);
			count++;
		}
		else
		{
			
		}
		node1 = node1->next;
	}

	if( count == 9999 )
	{
		int red, green, blue;
		char namew[256];
		uchar *p;
		IplImage *img;

		sprintf(namew,"PICK FOOD: %s", get_current_filename());
		cvNamedWindow(namew,0);
		img = cvLoadImage(get_current_filename(),1);
		cvResizeWindow(namew, img->width, img->height);
		cvShowImage(namew, img);
		cvSetMouseCallback(namew, my_mouse_callback2, (void *)img);
		while( 1 )
		{
			if( cvWaitKey(100) == 27 ) break;
		}
		p = img->imageData + photoy * img->widthStep;
		red = p[3*photox+2];
		green = p[3*photox+1];
		blue = p[3*photox+0];

		sprintf(buffer, "R=%d, G=%d, B=%d", red, green, blue);
		mbstowcs(buff, buffer, sizeof(buff));
		MessageBox(NULL, buff, L"PIXEL COLOR", MB_OK);

		cvReleaseImage(&img);
		cvDestroyWindow(namew);
		node1 = bloblist->first;
		for(i=0; i<bloblist->count; i++)
		{
			flag = 0;
			pointlist = node1->item;
			
			if( pointlist->red == red &&
				pointlist->green == green &&
				pointlist->blue == blue )
			{
				pointlist->isFood = 1;
				sprintf(buffer,"%d", count);		
				putBLOBInDatabase(pointlist, buffer);
				count++;
			}
			node1 = node1->next;
		}
	}
	else if( 1 /*count < 0 /*|| count > 1*/ )
	{
		int red, green, blue;
		uchar *p;
		char namew[256];
		IplImage *img;
		node1 = bloblist->first;
		for(i=0; i<bloblist->count; i++)
		{
			pointlist = node1->item;
			pointlist->isFood = 0;
			delete_photoblob(pointlist->photoblob_id);
			node1 = node1->next;
		}


		sprintf(namew,"PICK FOOD: %s", get_current_filename());
		cvNamedWindow(namew,0);
		img = cvLoadImage(get_current_filename(),1);
		cvResizeWindow(namew, img->width, img->height);
		cvShowImage(namew, img);
		cvSetMouseCallback(namew, my_mouse_callback2, (void *)img);
		while( 1 )
		{
			if( cvWaitKey(100) == 27 ) break;
		}

		for( i=0; i<foodcount; i++)
		{
	//		MessageBox(NULL, L"Loop number n", L"Foodnote loop", MB_OK);
			uchar *p = (uchar *)(img->imageData + foody[i] * img->widthStep );
			red = p[3*foodx[i]+2];
			green = p[3*foodx[i]+1];
			blue = p[3*foodx[i]+0];
///////////////////////////////
			node1 = bloblist->first;
			for(j=0; j<bloblist->count; j++)
			{
				pointlist = node1->item;
				if( pointlist->red == red &&
					pointlist->green == green &&
					pointlist->blue == blue )
				{
					pointlist->isFood = 1;
					sprintf(buffer,"%d", count);		
					putBLOBInDatabase(pointlist, buffer);
					count++;
				}
				node1 = node1->next;
			}
///////////////////////////////
		}
//			cvReleaseImage(img);

		/*********************************
		p = img->imageData + photoy * img->widthStep;
		red = p[3*photox+2];
		green = p[3*photox+1];
		blue = p[3*photox+0];

		sprintf(buffer, "R=%d, G=%d, B=%d", red, green, blue);
		mbstowcs(buff, buffer, sizeof(buff));
		MessageBox(NULL, buff, L"PIXEL COLOR", MB_OK);

		cvReleaseImage(&img);
		cvDestroyWindow(namew);
		node1 = bloblist->first;
		for(i=0; i<bloblist->count; i++)
		{
			flag = 0;
			pointlist = node1->item;
			
			if( pointlist->red == red &&
				pointlist->green == green &&
				pointlist->blue == blue )
			{
				pointlist->isFood = 1;
				sprintf(buffer,"%d", count);		
				putBLOBInDatabase(pointlist, buffer);
				count++;
			}
			node1 = node1->next;
		}
****************************/
	}
	else if( count >= 2 )
	{
#ifdef DUMMYCODE2
// DJB took out for segmentation preprocssing before finding food.
		int red, green, blue;
		uchar *p;
		char namew[256];
		IplImage *img;
		node1 = bloblist->first;
		for(i=0; i<bloblist->count; i++)
		{
			pointlist = node1->item;
			pointlist->isFood = 0;
			delete_photoblob(pointlist->photoblob_id);
			node1 = node1->next;
		}
		combinecount = 0;

		sprintf(namew,"COMBINE OR PICK FOOD: %s", get_current_filename());
		cvNamedWindow(namew,0);
		img = cvLoadImage(get_current_filename(),1);
		cvResizeWindow(namew, img->width, img->height);
		cvShowImage(namew, img);
		cvSetMouseCallback(namew, my_mouse_callback3, (void *)img);
		while( 1 )
		{
			if( cvWaitKey(100) == 27 ) break;
		}
		mergecolors(img);

		p = img->imageData + photoy * img->widthStep;
		red = p[3*photox+2];
		green = p[3*photox+1];
		blue = p[3*photox+0];

		sprintf(buffer, "R=%d, G=%d, B=%d", red, green, blue);
		mbstowcs(buff, buffer, sizeof(buff));
		MessageBox(NULL, buff, L"PIXEL COLOR", MB_OK);

		cvReleaseImage(&img);
		cvDestroyWindow(namew);
		node1 = bloblist->first;
		for(i=0; i<bloblist->count; i++)
		{
			flag = 0;
			pointlist = node1->item;
			
			if( pointlist->red == red &&
				pointlist->green == green &&
				pointlist->blue == blue )
			{
				pointlist->isFood = 1;
				sprintf(buffer,"%d", count);		
				putBLOBInDatabase(pointlist, buffer);
				count++;
			}
			node1 = node1->next;
		}
#endif // DUMMYCODE2
// DJB took out 
	}

	fclose(out1);
}
///////////////////////////////////////////////////////////
int findPlate2(IplImage *image)
{
	node_t *node1, *node2, *node3;
	FILE *out2, *out3, *out4, *out5, *out6, *out7;
	int i,j,k,count;
	header_t *pointlist2;
	char buffer[1024];
	wchar_t buff[1024];
	CvBox2D bBox;
	CvPoint2D32f pt[4];
	CvPoint pts[4];
	CvPoint *parr[1];
	int nCurvePts[1];
	CvContour header;
	CvSeqBlock block;
	int foodindex[12];
	CvSeq *seq;
	CvMat *ptMat;
	int in, out; 
	double val;
	CvMat* vector;
	IplImage* img;
///////////////////////////////////////////////////////////////

vector = cvCreateMat( 1, 3, CV_32SC2 );
CV_MAT_ELEM( *vector, CvPoint, 0, 0 ) = cvPoint(100,100);
CV_MAT_ELEM( *vector, CvPoint, 0, 1 ) = cvPoint(100,200);
CV_MAT_ELEM( *vector, CvPoint, 0, 2 ) = cvPoint(200,100);
img = cvCreateImage( cvSize(300,300), 8, 3 );
cvZero(img);
cvDrawContours( img,
cvPointSeqFromMat(CV_SEQ_KIND_CURVE+CV_SEQ_FLAG_CLOSED,
vector,
&header,
&block),
CV_RGB(255,0,0),
CV_RGB(255,0,0),
0, 3, 8, cvPoint(0,0));
cvSaveImage("TestContourImage.bmp",img,0);
///////////////////////////////////////////////////////////////

//	out2 = fopen("findPlate2.txt", "w+");
//	fprintf(out2,"i,area,sdCX, sdCY,count, PixelCount, Theta, Red, Green, Blue, AreaRatio\n");
	count = 0;
	node1 = bloblist->first;
	ptMat = cvCreateMat(1, 4, CV_32SC2);

/*
IplImage* img = cvCreateImage( cvSize(300,300), 8, 3 );
cvZero(img);
*/
	out7 = fopen("BBoxTest.txt","w+");
	for(i=0; i<bloblist->count; i++)
	{
		in = out = 0;
		pointlist = node1->item;
		bBox.center.x = pointlist->bbox_centerX;
		bBox.center.y = pointlist->bbox_centerY;
		bBox.size.width = pointlist->bbox_sizeX;
		bBox.size.height = pointlist->bbox_sizeY;
		bBox.angle = pointlist->bbox_angle;
		sprintf(buffer,"%f, %f, %f, %f, %f\n",
			pointlist->bbox_centerX,
			pointlist->bbox_centerY,
			pointlist->bbox_sizeX,
			pointlist->bbox_sizeY,
			pointlist->bbox_angle);
		fprintf(out7,"Center: %f, %f;  Size: %f, %f:  Angle: %f\n",
			pointlist->bbox_centerX,
			pointlist->bbox_centerY,
			pointlist->bbox_sizeX,
			pointlist->bbox_sizeY,
			pointlist->bbox_angle);
			
		mbstowcs(buff, buffer, sizeof(buffer));
		//MessageBox(NULL,buff,L"BBox", MB_OK);
		cvBoxPoints(bBox, pt);
		fprintf(out7,"Four Courners are: (%f,%f), (%f,%f), (%f,%f), (%f,%f)\n",
			pt[0].x,pt[0].y,
			pt[1].x,pt[1].y,
			pt[2].x,pt[2].y,
			pt[3].x,pt[3].y);

		CV_MAT_ELEM( *ptMat, CvPoint, 0, 0 ) = cvPoint((int)pt[0].x,(int)pt[0].y);
		CV_MAT_ELEM( *ptMat, CvPoint, 0, 1 ) = cvPoint((int)pt[1].x,(int)pt[1].y);
		CV_MAT_ELEM( *ptMat, CvPoint, 0, 2 ) = cvPoint((int)pt[2].x,(int)pt[2].y);
		CV_MAT_ELEM( *ptMat, CvPoint, 0, 3 ) = cvPoint((int)pt[3].x,(int)pt[3].y);

		seq = cvPointSeqFromMat(CV_SEQ_KIND_CURVE+CV_SEQ_FLAG_CLOSED, ptMat,
			&header, &block);
//		if( seq->total )
//		{
//			sprintf(buffer, "PointSeq Total: %d, Elem Size: %d\n", seq->total, seq->elem_size );
//				mbstowcs(buff, buffer, sizeof(buffer));
//				MessageBox(NULL,buff, L"Error", MB_OK);
//		}

		if( seq == NULL )
			MessageBox(NULL,L"Seq is NULL",L"ERROR",MB_OK);


		cvDrawContours( image, seq, CV_RGB(255,0,0), CV_RGB(255,0,0), 1, 1, 8, cvPoint(0,0));
//		sprintf(buffer, "DrawContours ERROR: %s\n", cvErrorStr( cvGetErrStatus() ));
//		mbstowcs(buff, buffer, sizeof(buffer));
//		MessageBox(NULL,buff, L"Error", MB_OK);
		for(j=0, node2 = bloblist->first; j<bloblist->count; j++, node2 = node2->next)
		{
			if( i != j )
			{
				pointlist2 = node2->item;
				bBox.center.x = pointlist2->bbox_centerX;
				bBox.center.y = pointlist2->bbox_centerY;
				bBox.size.width = pointlist2->bbox_sizeX;
				bBox.size.height = pointlist2->bbox_sizeY;
				bBox.angle = pointlist2->bbox_angle;
				cvBoxPoints(bBox, pt);
				fprintf(out7,"j=%d\t%f\t%f\t%f\t%f\t%f\n",
					j, bBox.center.x, bBox.center.y,
					bBox.size.width, bBox.size.height,
					bBox.angle);
				for(k=0; k<4; k++)
				{
					val = cvPointPolygonTest(seq,pt[k],1);
//		sprintf(buffer, "PointPolugonTest ERROR: %s\n", cvErrorStr( cvGetErrStatus() ));
//		mbstowcs(buff, buffer, sizeof(buffer));
//		MessageBox(NULL,buff, L"Error", MB_OK);
					fprintf(out7,"4-Corners val=%g\n", val);
					if( val > 0 )
						in++;
					else if( val < -1 )
						out++;
				}
				val = cvPointPolygonTest(seq, cvPoint2D32f(pointlist2->average_x, pointlist2->average_y),1);
				fprintf(out7,"Center val=%f\n", val);
				if( val > 1)
					in++;
				else if( val < -1 )
					out++;
				if( in > 4 )
					pointlist->bbox_containedObjects++;
			}
		}
		node1 = node1->next;
	}	
//	MessageBox(NULL,L"I made past the for",L"WOW", MB_OK);

	for(i=0, node1=bloblist->first; i<bloblist->count; i++, node1 = node1->next)
	{
		pointlist2 = node1->item;
//		if(	pointlist2->bbox_containedObjects > 0 )
//		{
			fprintf(out7, "%d\t%d\t%f\n", i, pointlist2->bbox_containedObjects,
				pointlist2->bbox_sizeX * pointlist2->bbox_sizeY);
//		}
	}
//MessageBox(NULL,L"Before the fClose","MADE IT", MB_OK);
	fclose(out7);
//	if( count == 1 )
//	{
///		//char buffer[1024];
//		//sprintf(buffer,c);
///		MessageBox(NULL,L"Found the plate",L"GOOD", MB_OK);
//		findFood(singlepointlist);
//	}
//	else
//	{
//		sprintf(buffer,"Did not find the Plate\nCount=%d", count);
//		mbstowcs(buff, buffer, sizeof(buffer));
//		MessageBox(NULL,buff,L"BAD", MB_OK);
//
//	}
	cvSaveImage("SeqTestImage.jpg",image,0);
	return 0;
}

////////////////////////////////////////////////////////////
float distance_2Df( float x1, float x2, float y1, float y2)
{
	float sq;

	if( x1 == x2 && y1 == y2 )
		return FLT_MAX;
	sq = (float)sqrt(pow((x2 - x1),2.0) + pow((y2 - y1),2.0));
	return sq;
}
///////////////////////////////////////////////////////////
typedef struct ellipseStack 
{
	header_t *ellipseData;
	float sumStdDev;
} ELLIPSESTACK;

#define PLATESAMPLES	26

int findPlate(IplImage *image)
{
	node_t *node1, *node2, *node3;
	FILE *out, *out3, *out4, *out5, *out6;
	int i,j,k,count;
	header_t **singlepointlist, *temp, *stack[PLATESAMPLES];
	char buffer[1024];
	wchar_t buff[1024];
	int holder;
	header_t initEllipse;
	float leastSum;

	singlepointlist = malloc(sizeof(header_t *));
	sprintf(buffer,"%s-findPlate.txt", get_filename_base());
	out = fopen(buffer, "w+");
	fprintf(out,"i,area,sdCX, sdCY,count, PixelCount, Theta, Red, Green, Blue, AreaRatio, PixelCountRatio, Cx, Cy\n");
	count = 0;
	node1 = bloblist->first;
	
	initEllipse.sdCx = 10000;
	initEllipse.sdCy = 10000;
	initEllipse.sdR1 = 10000;
	initEllipse.sdR2 = 10000;
	initEllipse.sdTheta = 10000;

	for(i=0; i<bloblist->count; i++)
	{
		pointlist = node1->item;
		fprintf(out, "%d, %f, %f, %f, %d, %d, "
			"%f, %d, %d, %d, %f, "
			"%f, %f, %f\n", i, 
			pointlist->area, pointlist->sdCx, pointlist->sdCy,
			pointlist->count, pointlist->pixel_count,
			pointlist->sdTheta, pointlist->red, pointlist->green, pointlist->blue,
			pointlist->area / (image->height * image->width ),
			(float)pointlist->pixel_count / (float)(image->height * image->width ),
			pointlist->Cx, pointlist->Cy); 	

		if( i==0 )
		{
			for(j=0; j<PLATESAMPLES; j++ )
			{		
				stack[j] = &initEllipse;
			}
		}
		for(j=0; j<PLATESAMPLES; j++ )
		{
			if( stack[j]->sdCx + stack[j]->sdCy +
				stack[j]->sdR1 + stack[j]->sdR2 /*+
				stack[j]->sdTheta*/ > 
						pointlist->sdCx +
						pointlist->sdCy +
						pointlist->sdR1 +
						pointlist->sdR2 /*+
						pointlist->sdTheta*/ )
			{
				k = PLATESAMPLES-1;
				while( k > j )
				{
					stack[k] = stack[k-1];
					k--;
				}
				stack[j] = pointlist;
				j = PLATESAMPLES+1;
			}
		}
#ifdef OLDER
		else if( leastSum > (pointlist->sdCx +
						pointlist->sdCy +
						pointlist->sdR1 +
						pointlist->sdR2 +
						pointlist->sdTheta) )
/*		if( (pointlist->sdCx + pointlist->sdCy) < 32.0 &&
			(pointlist->area / (image->height * image->width )) > 0.08 &&
			(pointlist->area / (image->height * image->width )) < 0.245 )// &&*/
			//((float)(pointlist->pixel_count) / (float)(image->height * image->width )) > 0.13)
		{
			count++;
			leastList = pointlist;
			leastSum = pointlist->sdCx +
						pointlist->sdCy +
						pointlist->sdR1 +
						pointlist->sdR2 +
						pointlist->sdTheta;
			
/*			singlepointlist = (header_t **)realloc(singlepointlist, count * sizeof(header_t *));
			for(j=count-1; j<count; j++)
				singlepointlist[j] = malloc(sizeof(header_t *));
			fprintf(out,"%d item matches criteria\n", i);
			pointlist->isPlate = 1;
			singlepointlist[count-1] = pointlist;
			temp = pointlist;*/
		}
#endif //OLDER
		node1 = node1->next;
	}

	sprintf(buffer,"%s-SmallestSTDDEV.txt", get_filename_base());
	out6 = fopen(buffer, "w+");
	fprintf(out6, "StackNumber\tSum\n");

	for(j=0; j<PLATESAMPLES; j++)
		fprintf(out6, "%d\t%f\n", j, stack[j]->sdCx + stack[j]->sdCy +
				stack[j]->sdR1 + stack[j]->sdR2 +
				stack[j]->sdTheta );
	fclose(out6);
	//return;
	count = 0;
	for( j=0; j<PLATESAMPLES; j++)
	{
		if ( ((stack[j]->area / (image->height * image->width )) > 0.068 ) &&
			( (stack[j]->area / (image->height * image->width )) < 0.245 ) &&
			(((float)(stack[j]->pixel_count) / (float)(image->height * image->width )) > 0.03) )
		{
			temp = stack[j];
			j=PLATESAMPLES+1;
			count = 1;
		}
	
	}
	holder = count;
	fclose(out);
//	temp = singlepointlist[0];
	if (count == 0 /*|| count != 0*/ )
	{

		int red, green, blue;
		uchar *p;
		int flag;
		IplImage *img;
		char name[MAX_PATH];

//		free(singlepointlist);	
		MessageBox(NULL,L"ERROR - NO PLATES FOUND",L"ERROR", MB_OK);
		sprintf(name, "PICK PLATE - %s", get_current_filename());

		cvNamedWindow(name,0);

		img = cvLoadImage(get_current_filename(),1);
		cvResizeWindow(name, img->width, img->height);
		cvShowImage(name, img);
		cvSetMouseCallback(name, my_mouse_callback2, (void *)img);
		while( 1 )
		{
			if( cvWaitKey(100) == 27 ) break;
		}
		p = img->imageData + photoy * img->widthStep;
		red = p[3*photox+2];
		green = p[3*photox+1];
		blue = p[3*photox+0];

		sprintf(buffer, "R=%d, G=%d, B=%d", red, green, blue);
		mbstowcs(buff, buffer, sizeof(buff));
		MessageBox(NULL, buff, L"PIXEL COLOR", MB_OK);

		cvReleaseImage(&img);
		cvDestroyWindow(name);
		node1 = bloblist->first;
		count = 0;
		for(i=0; i<bloblist->count; i++)
		{
			flag = 0;
			pointlist = node1->item;
			
			if( pointlist->red == red &&
				pointlist->green == green &&
				pointlist->blue == blue )
			{
				temp = pointlist;
				//count = 1;
//				pointlist->isFood = 1;
//				sprintf(buffer,"%d", count);		
//				putBLOBInDatabase(pointlist, buffer);
				count++;
			}
			node1 = node1->next;
		}

		//return -1;
	}
/*	if( count > 1 )
	{
		for(i=0; i<count; i++)
		{
			if( temp->pixel_count < singlepointlist[i]->pixel_count /*&&
				distance_2df(singlepointlist[i]->Cx, image->width/2, singlepointlist[i]->Cy, image->height/2) < 
				distance_2df(temp->Cx, image->width/2, temp->Cy, image->height/2) )
				temp = singlepointlist[i];
		}
		count = 1;
	}*/
	if( temp->R1 > temp->R2 )
		cvEllipse(image,cvPoint(temp->Cx, temp->Cy), cvSize(temp->R1/2, temp->R2/2), temp->Theta, 0, 360, cvScalar(255,0,0,1), 4, 8, 0);
	else
		cvEllipse(image,cvPoint(temp->Cx, temp->Cy), cvSize(temp->R2/2, temp->R1/2), temp->Theta, 0, 360, cvScalar(255,0,0,1), 4, 8, 0);

	sprintf(buffer, "%s-PlateEllipse.png", get_filename_base());
	cvSaveImage(buffer, image, 0);
	if( count == 1 )
	{
		//char buffer[1024];
		//sprintf(buffer,c);
//		MessageBox(NULL,L"Found the plate",L"GOOD", MB_OK);
		findFood(temp);
	}
	else
	{
		sprintf(buffer,"Did not find the Plate\nCount=%d", count);
		mbstowcs(buff, buffer, sizeof(buffer));
		MessageBox(NULL,buff,L"BAD", MB_OK);

	}
	//for( i=0; i<holder; i++)
//		free(singlepointlist[i]);
	free(singlepointlist);	
	return 0;
}

int fitEllipses(IplImage *image)
{
	node_t *node1, *node2, *node3;
	FILE *out1, *out3, *out4, *out5, *out6, *out7;
	int i,j,k;
	int count;
	CvScalar mean1, stddev1;
	CvScalar mean2, stddev2;
	CvScalar mean3, stddev3;
	CvScalar mean4, stddev4;
	CvScalar mean5, stddev5;
	CvMat * edge98;
	CvMat * mCx;
	CvMat * mCy;
	CvMat * mR1;
	CvMat * mR2;
	CvMat * mTheta;
	char buffer[MAX_PATH];
	CvBox2D rEllipse;
	CvBox2D bBox;
	CvPoint2D32f pt[4];
	CvPoint pts[4];
	CvPoint *parr[1];
	int nCurvePts[1];

	int number;
	float Cx, Cy, R1, R2, Theta;
	char *p;
	srand(17641);
	Cx=Cy=R1=R2=Theta=0;
//#define VERBOSE_INSTRUMENTATION
#ifdef VERBOSE_INSTRUMENTATION
	sprintf(buffer, "%s-CollapsedEllipseData.txt", get_filename_base());
	out4 = fopen(buffer, "w+");
	fprintf(out4,"Red,Green,Blue, Cx, Cy, R1, R2, Theta,PixelCount,Area,MinX,MinY,MaxX,MaxY,Ax,Ay\n");

	strcpy(buffer, get_current_filename());
	p = strrchr(buffer, '\\');
//	if( p != NULL )
//		strcpy(buffer, p+1);
	sprintf(buffer,"EllipseAnalysis-%s.txt", p+1);
	out5 = fopen(buffer,"w+");
	fprintf(out5,"Red,Green,Blue,AvgCx, AvgCy,AvgR1,AvgR2,AvgTheta,StdCx,StdCy,StdR1,StdR2,StdTheta,PixelCount,Area,%% of Image,Edge Pixels\n");
	fclose(out5);

	out7 = fopen("BoundingBoxes.txt","w+");
	fprintf(out7, "SizeX\tSizeY\tCenterX\tCenterY\tAngle\tRed\tGreen\tBlue\tPt1X\tPt1Y\tPt2X\tPt2Y\tPt3X\tPt3Y\tPt4X\tPt4Y\tBox Area\n");

	out3 = fopen("AnalyzeEllipses.txt","a+");
#endif //VERBOSE_INSTRUMENTATION
	node1 = bloblist->first;
	for(i=0; i<bloblist->count; i++)
	{
		char buffer1[MAX_PATH];
//		char buffer2[MAX_PATH];

		pointlist = node1->item;
		mCx = cvCreateMat(6, 1, CV_32FC1);
		mCy = cvCreateMat(6, 1, CV_32FC1);
		mR1 = cvCreateMat(6, 1, CV_32FC1);
		mR2 = cvCreateMat(6, 1, CV_32FC1);
		mTheta = cvCreateMat(6, 1, CV_32FC1);
	
		sprintf(buffer1,"FitEllipses-Colors-%d-%d-%d.txt", pointlist->red, pointlist->green, pointlist->blue);
#ifdef VERBOSE_INSTRUMENTATION		
		out1 = fopen(buffer1, "w+");
		fclose(out1);
		out1 = fopen(buffer1, "a+");
#endif //VERBOSE_INSTRUMENTATION
/////////////////////////////////////////
		node2 = pointlist->first;
//////////////////////////////////////////////////////////
		edge98 = cvCreateMat(pointlist->count, 1, CV_32SC2);
		count=0;
		for(j=0; j<pointlist->count; j++)
		{
			IMAGEPOINT *img;
			img = node2->item;
			CV_MAT_ELEM(*edge98, CvPoint, j, 0) = cvPoint(img->imageX, img->imageY);
			count++;
			node2 = node2->next;
			//node2 = node2->next;
		}
		if( pointlist->isFood == 1)
		{
			IplImage *img;
			//MessageBox(NULL,L"Cloning", "STATUS", MB_OK);
			img = cvCloneImage(image);
			//MessageBox(NULL,L"SettingZero", "STATUS", MB_OK);
			cvSetZero(img);
			//MessageBox(NULL,L"Saving", "STATUS", MB_OK);
			cvSaveImage("BlankImage.jpg",img,1);
			//MessageBox(NULL,L"Releasing", "STATUS", MB_OK);
			cvReleaseImage(img);
		}
		bBox = cvMinAreaRect2(edge98, NULL);
		cvBoxPoints(bBox, pt);
		pointlist->area = bBox.size.height * bBox.size.width;
		pointlist->bbox_angle = bBox.angle;
		pointlist->bbox_centerX = bBox.center.x;
		pointlist->bbox_centerY = bBox.center.y;
		pointlist->bbox_sizeX = bBox.size.width;
		pointlist->bbox_sizeY = bBox.size.height;
#ifdef VERBOSE_INSTRUMENTATION		
		fprintf(out7, "%f\t%f\t%f\t%f\t%f\t%d\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\n",
			bBox.size.width,
			bBox.size.height,
			bBox.center.x,
			bBox.center.y,
			bBox.angle,
			pointlist->red,
			pointlist->green,
			pointlist->blue,
			pt[0].x, pt[0].y,
			pt[1].x, pt[1].y,
			pt[2].x, pt[2].y,
			pt[3].x, pt[3].y,
			pointlist->area
			);
#endif //VERBOSE_INSTRUMENTATION		

		for( j=0; j<4; j++)
		{
			pts[j].x = (int)pt[j].x;
			pts[j].y = (int)pt[j].y;
		}
//CvPoint  curve1[]={10,10,  10,100,  100,100,  100,10};
//CvPoint  curve2[]={30,30,  30,130,  130,130,  130,30,  150,10};
//CvPoint* curveArr[2]={curve1, curve2};
//int      nCurvePts[1]={4};
		nCurvePts[0] = 4;


		parr[0] = pts;
// Line below will draw minimum size bounding boxes around all blobs in the image
		cvPolyLine(image, parr, nCurvePts, 1, 1, cvScalar(0,255,0,3), 4, 8, 0);

		
//		cvPolyLine(img,   curveArr,nCurvePts,nCurves,isCurveClosed,cvScalar(0,255,255),lineWidth);
		rEllipse = cvFitEllipse2(edge98);
#ifdef VERBOSE_INSTRUMENTATION
		fprintf(out3, "Color, %d, %d, %d, Center, %f, %f, Size, %f, %f, Angle, %f\n",
			pointlist->red, pointlist->green, pointlist->blue,
			rEllipse.center.x,
			rEllipse.center.y,
			rEllipse.size.width,
			rEllipse.size.height,
			rEllipse.angle);
#endif
		CV_MAT_ELEM(*mCx, float, 0,0) = (float)rEllipse.center.x;
		CV_MAT_ELEM(*mCy, float, 0,0) = (float)rEllipse.center.y;
		CV_MAT_ELEM(*mR1, float, 0,0) = (float)rEllipse.size.width;
		CV_MAT_ELEM(*mR2, float, 0,0) = (float)rEllipse.size.height;
		CV_MAT_ELEM(*mTheta, float, 0,0) = (float)rEllipse.angle-90.0f;


		Cx = Cx + rEllipse.center.x;
		Cy = Cy + rEllipse.center.y;
		R1 = R1 + rEllipse.size.width;
		R2 = R2 + rEllipse.size.height;
		Theta = Theta + rEllipse.angle;
		if( rEllipse.size.height > 10*image->height || rEllipse.size.width > 10*image->width )
		{}
		else
		{
			rEllipse.angle = (float)90.0 - rEllipse.angle;
			cvEllipseBox(image, rEllipse,  CV_RGB(255,0,0), 1, 8, 0);
		}
		cvReleaseMat(&edge98);
		node2 = pointlist->first;
		edge98 = cvCreateMat(23, 1, CV_32SC2);
		count = 0;
		number=0;
		for(j=0; j<23; j++)
		{
			IMAGEPOINT *img;
//			number = (int)((double)(rand()/(double)RAND_MAX)*pointlist->count);			
			number = number + (int)((float)pointlist->count / 23.0);			
			for( k=0, node3 = pointlist->first; k<number; node3 = node3->next, k++)
			{
				img = node3->item;
			}
			CV_MAT_ELEM(*edge98, CvPoint, count, 0) = cvPoint(img->imageX, img->imageY);
			count++;
		}
		rEllipse = cvFitEllipse2(edge98);
#ifdef VERBOSE_INSTRUMENTATION
		fprintf(out3, "Color, %d, %d, %d, Center, %f, %f, Size, %f, %f, Angle, %f\n",
			pointlist->red, pointlist->green, pointlist->blue,
			rEllipse.center.x,
			rEllipse.center.y,
			rEllipse.size.width,
			rEllipse.size.height,
			rEllipse.angle);
#endif //VERBOSE_INSTRUMENTATION
		CV_MAT_ELEM(*mCx, float, 1,0) = (float)rEllipse.center.x;
		CV_MAT_ELEM(*mCy, float, 1,0) = (float)rEllipse.center.y;
		CV_MAT_ELEM(*mR1, float, 1,0) = (float)rEllipse.size.width;
		CV_MAT_ELEM(*mR2, float, 1,0) = (float)rEllipse.size.height;
		CV_MAT_ELEM(*mTheta, float, 1,0) = (float)rEllipse.angle-90.0f;

		Cx = Cx + rEllipse.center.x;
		Cy = Cy + rEllipse.center.y;
		R1 = R1 + rEllipse.size.width;
		R2 = R2 + rEllipse.size.height;
		Theta = Theta + rEllipse.angle;
		if( rEllipse.size.height > 10*image->height || rEllipse.size.width > 10*image->width )
		{}
		else
		{
			rEllipse.angle = (float)90.0 - rEllipse.angle;
//			cvEllipseBox(image, rEllipse, CV_RGB(0,255,0), 1, 8, 0);
		}
		cvReleaseMat(&edge98);
		node2 = pointlist->first;
		edge98 = cvCreateMat(7, 1, CV_32SC2);
		count = 0;
		number = 0;
		for(j=0; j<7; j++)
		{
			IMAGEPOINT *img;
//			number = (int)((double)(rand()/(double)RAND_MAX)*pointlist->count);			
			number = number + (int)((float)pointlist->count / 7.0);			
			for( k=0, node3 = pointlist->first; k<number; node3 = node3->next, k++)
			{
				img = node3->item;
			}
			CV_MAT_ELEM(*edge98, CvPoint, count, 0) = cvPoint(img->imageX, img->imageY);
			count++;
				}
		rEllipse = cvFitEllipse2(edge98);
#ifdef VERBOSE_INSTRUMENTATION
		fprintf(out3, "Color, %d, %d, %d, Center, %f, %f, Size, %f, %f, Angle, %f\n",
			pointlist->red, pointlist->green, pointlist->blue,
			rEllipse.center.x,
			rEllipse.center.y,
			rEllipse.size.width,
			rEllipse.size.height,
			rEllipse.angle);
#endif //VERBOSE_INSTRUMENTATION
		CV_MAT_ELEM(*mCx, float, 2,0) = (float)rEllipse.center.x;
		CV_MAT_ELEM(*mCy, float, 2,0) = (float)rEllipse.center.y;
		CV_MAT_ELEM(*mR1, float, 2,0) = (float)rEllipse.size.width;
		CV_MAT_ELEM(*mR2, float, 2,0) = (float)rEllipse.size.height;
		CV_MAT_ELEM(*mTheta, float, 2,0) = (float)rEllipse.angle-90.0f;

		Cx = Cx + rEllipse.center.x;
		Cy = Cy + rEllipse.center.y;
		R1 = R1 + rEllipse.size.width;
		R2 = R2 + rEllipse.size.height;
		Theta = Theta + rEllipse.angle;
		if( rEllipse.size.height > 10*image->height || rEllipse.size.width > 10*image->width )
		{}
		else
		{
			rEllipse.angle = (float)90.0 - rEllipse.angle;
			cvEllipseBox(image, rEllipse, CV_RGB(255,255,0), 1, 8, 0);
		}
		cvReleaseMat(&edge98);
		node2 = pointlist->first;
		edge98 = cvCreateMat(11, 1, CV_32SC2);
		count = 0;
		number = 0;
		for(j=0; j<11; j++)
		{
			IMAGEPOINT *img;
//			number = (int)((double)(rand()/(double)RAND_MAX)*pointlist->count);			
			number = number + (int)((float)pointlist->count / 11.0);			
			for( k=0, node3 = pointlist->first; k<number; node3 = node3->next, k++)
			{
				img = node3->item;
			}
			CV_MAT_ELEM(*edge98, CvPoint, count, 0) = cvPoint(img->imageX, img->imageY);
			count++;
		}
		rEllipse = cvFitEllipse2(edge98);
#ifdef VERBOSE_INSTRUMENTATION
		fprintf(out3, "Color, %d, %d, %d, Center, %f, %f, Size, %f, %f, Angle, %f\n",
			pointlist->red, pointlist->green, pointlist->blue,
			rEllipse.center.x,
			rEllipse.center.y,
			rEllipse.size.width,
			rEllipse.size.height,
			rEllipse.angle);
#endif //VERBOSE_INSTRUMENTATION
		CV_MAT_ELEM(*mCx, float, 3,0) = (float)rEllipse.center.x;
		CV_MAT_ELEM(*mCy, float, 3,0) = (float)rEllipse.center.y;
		CV_MAT_ELEM(*mR1, float, 3,0) = (float)rEllipse.size.width;
		CV_MAT_ELEM(*mR2, float, 3,0) = (float)rEllipse.size.height;
		CV_MAT_ELEM(*mTheta, float, 3,0) = (float)rEllipse.angle-90.0f;

		Cx = Cx + rEllipse.center.x;
		Cy = Cy + rEllipse.center.y;
		R1 = R1 + rEllipse.size.width;
		R2 = R2 + rEllipse.size.height;
		Theta = Theta + rEllipse.angle;
		if( rEllipse.size.height > 10*image->height || rEllipse.size.width > 10*image->width )
		{}
		else
		{
			rEllipse.angle = (float)90.0 - rEllipse.angle;
			cvEllipseBox(image, rEllipse, CV_RGB(255,0,255), 1, 8, 0);
		}
		cvReleaseMat(&edge98);
		node2 = pointlist->first;
		edge98 = cvCreateMat(13, 1, CV_32SC2);
		count = 0;
		number = 0;
		for(j=0; j<13; j++)
		{
			IMAGEPOINT *img;
//			number = (int)((double)(rand()/(double)RAND_MAX)*pointlist->count);			
			number = number + (int)((float)pointlist->count / 13.0);			
			for( k=0, node3 = pointlist->first; k<number; node3 = node3->next, k++)
			{
				img = node3->item;
			}
			CV_MAT_ELEM(*edge98, CvPoint, count, 0) = cvPoint(img->imageX, img->imageY);
			count++;
		}
		rEllipse = cvFitEllipse2(edge98);
#ifdef VERBOSE_INSTRUMENTATION
		fprintf(out3, "Color, %d, %d, %d, Center, %f, %f, Size, %f, %f, Angle, %f\n",
			pointlist->red, pointlist->green, pointlist->blue,
			rEllipse.center.x,
			rEllipse.center.y,
			rEllipse.size.width,
			rEllipse.size.height,
			rEllipse.angle);
#endif //VERBOSE_INSTRUMENTATION
		CV_MAT_ELEM(*mCx, float, 4,0) = (float)rEllipse.center.x;
		CV_MAT_ELEM(*mCy, float, 4,0) = (float)rEllipse.center.y;
		CV_MAT_ELEM(*mR1, float, 4,0) = (float)rEllipse.size.width;
		CV_MAT_ELEM(*mR2, float, 4,0) = (float)rEllipse.size.height;
		CV_MAT_ELEM(*mTheta, float, 4,0) = (float)rEllipse.angle-90.0f;

		Cx = Cx + rEllipse.center.x;
		Cy = Cy + rEllipse.center.y;
		R1 = R1 + rEllipse.size.width;
		R2 = R2 + rEllipse.size.height;
		Theta = Theta + rEllipse.angle;
		if( rEllipse.size.height > 10*image->height || rEllipse.size.width > 10*image->width )
		{}
		else
		{
			rEllipse.angle = (float)90.0 - rEllipse.angle;
//			cvEllipseBox(image, rEllipse, CV_RGB(0,255,255), 1, 8, 0);
		}
		cvReleaseMat(&edge98);
		node2 = pointlist->first;
		edge98 = cvCreateMat(27, 1, CV_32SC2);
		count = 0;
		number = 0;
		for(j=0; j<27; j++)
		{
			IMAGEPOINT *img;
//			number = (int)((double)(rand()/(double)RAND_MAX)*pointlist->count);			
			number = number + (int)((float)pointlist->count / 27.0);			
			for( k=0, node3 = pointlist->first; k<number; node3 = node3->next, k++)
			{
				img = node3->item;
			}
			CV_MAT_ELEM(*edge98, CvPoint, count, 0) = cvPoint(img->imageX, img->imageY);
			count++;
		}
		rEllipse = cvFitEllipse2(edge98);
#ifdef VERBOSE_INSTRUMENTATION
		fprintf(out3, "Color, %d, %d, %d, Center, %f, %f, Size, %f, %f, Angle, %f\n",
			pointlist->red, pointlist->green, pointlist->blue,
			rEllipse.center.x,
			rEllipse.center.y,
			rEllipse.size.width,
			rEllipse.size.height,
			rEllipse.angle);
#endif //VERBOSE_INSTRUMENTATION
		CV_MAT_ELEM(*mCx, float, 5,0) = (float)rEllipse.center.x;
		CV_MAT_ELEM(*mCy, float, 5,0) = (float)rEllipse.center.y;
		CV_MAT_ELEM(*mR1, float, 5,0) = (float)rEllipse.size.width;
		CV_MAT_ELEM(*mR2, float, 5,0) = (float)rEllipse.size.height;
		CV_MAT_ELEM(*mTheta, float, 5,0) = (float)rEllipse.angle-90.0f;

		Cx = Cx + rEllipse.center.x;
		Cy = Cy + rEllipse.center.y;
		R1 = R1 + rEllipse.size.width;
		R2 = R2 + rEllipse.size.height;
		Theta = Theta + rEllipse.angle;
		if( rEllipse.size.height > 10*image->height || rEllipse.size.width > 10*image->width )
		{}
		else
		{
			rEllipse.angle = (float)90.0 - rEllipse.angle;
//			cvEllipseBox(image, rEllipse, CV_RGB(0,255,255), 1, 8, 0);
		}
		Cx = Cx / (float)6.0;
		Cy = Cy / (float)6.0;
		R1 = R1 / (float)6.0;
		R2 = R2 / (float)6.0;
		Theta = Theta / (float)6.0;

		cvAvgSdv(mCx, &mean1, &stddev1, NULL);
		cvAvgSdv(mCy, &mean2, &stddev2, NULL);
		cvAvgSdv(mR1, &mean3, &stddev3, NULL);
		cvAvgSdv(mR2, &mean4, &stddev4, NULL);
		cvAvgSdv(mTheta, &mean5, &stddev5, NULL);
//#define VERBOSE_INSTRUMENTATION
#ifdef VERBOSE_INSTRUMENTATION

		fprintf(out4,"%d,%d,%d, %f,%f, %f,%f, %f,%d, %f, %d, %d, %d, %d, %d,%d\n",
			pointlist->red,
			pointlist->green,
			pointlist->blue,
			Cx,
			Cy,
			R1,
			R2,
			90.0 - Theta,
			pointlist->pixel_count,
			(float)CV_PI*R1*R2,
			pointlist->min_x,
			pointlist->min_y,
			pointlist->max_x,
			pointlist->max_y,
			pointlist->average_x,
			pointlist->average_y);

		strcpy(buffer, get_current_filename());
		p = strrchr(buffer, '\\');
//		if( p != NULL )
//			strcpy(buffer, p+1);
		sprintf(buffer,"EllipseAnalysis-%s.txt", p+1);
		out5 = fopen(buffer,"a+");
		fprintf(out5,"%d,%d,%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%d,%f,%f,%d\n",
			pointlist->red, pointlist->green, pointlist->blue,
			mean1.val[0], mean2.val[0], mean3.val[0], mean4.val[0], mean5.val[0],
			stddev1.val[0], stddev2.val[0], stddev3.val[0], stddev4.val[0], stddev5.val[0],
			pointlist->pixel_count, (float)CV_PI*(mean3.val[0]/2.0f)*(mean4.val[0]/2.0f), 
			((float)CV_PI*(mean3.val[0]/2.0f)*(mean4.val[0]/2.0f))/ (float)(image->height*image->width),
			pointlist->count);

		fclose(out5);
#endif //VERBOSE_INSTRUMENTATION
		pointlist->Cx = (float)mean1.val[0];
		pointlist->Cy = (float)mean2.val[0];
		pointlist->R1 = (float)mean3.val[0];
		pointlist->R2 = (float)mean4.val[0];
		pointlist->Theta = (float)mean5.val[0];
		pointlist->sdCx = (float)stddev1.val[0];
		pointlist->sdCy = (float)stddev2.val[0];
		pointlist->sdR1 = (float)stddev3.val[0];
		pointlist->sdR2 = (float)stddev4.val[0];
		pointlist->sdTheta = (float)stddev5.val[0];
		pointlist->area = (float)((float)CV_PI*(mean3.val[0]/2.0f)*(mean4.val[0]/2.0f));
		pointlist->percentOfImage = (float)(((float)CV_PI*(mean3.val[0]/2.0f)*(mean4.val[0]/2.0f))/ (float)(image->height*image->width));

		cvReleaseMat(&mCx);
		cvReleaseMat(&mCy);
		cvReleaseMat(&mR1);
		cvReleaseMat(&mR2);
		cvReleaseMat(&mTheta);

		node1 = node1->next;
#ifdef VERBOSE_INSTRUMENTATION
		fclose(out1);
#endif //VERBOSE_INSTRUMENTATION
	}

#ifdef VERBOSE_INSTRUMENTATION
	fclose(out3);
	fclose(out4);
	fclose(out7);
#endif //VERBOSE_INSTRUMENTATION

	cvSaveImage("EllipseFound.pbm", image, 0);
#undef VERBOSE_INSTRUMENTATION
return 0;
}

int count_colors(IplImage *image)
{
	int x,y;
	int i;
	node_t *node1;
//#define VERBOSE_INSTRUMENTATION
#ifdef VERBOSE_INSTRUMENTATION
	FILE *out;
#endif //VERBOSE_INSTRUMENTATION

	for( y=0; y<image->height; y++)
	{
		uchar *p = (uchar *)(image->imageData + y * image->widthStep );
		for( x=0; x<image->width; x++)
		{
			node1 = bloblist->first;
			for(i=0; i<bloblist->count; i++)
			{
				pointlist = node1->item;
				if( pointlist->red == p[3*x+2] &&
					pointlist->green == p[3*x+1] &&
					pointlist->green == p[3*x+1] )
				{
					pointlist->pixel_count++;
				}
				node1 = node1->next;
			}
			
		}
	}
#ifdef VERBOSE_INSTRUMENTATION
	out = fopen("ColorCount.txt","w+");
#endif VERBOSE_INSTRUMENTATION

	node1 = bloblist->first;
	for(i=0; i<bloblist->count; i++)
	{
		pointlist = node1->item;
#ifdef VERBOSE_INSTRUMENTATION

		fprintf(out,"R:%d, G:%d, B:%d, Count:%d\n",
			pointlist->red, pointlist->green,
			pointlist->blue, pointlist->pixel_count);
#endif //#ifdef VERBOSE_INSTRUMENTATION

		node1 = node1->next;
	}
#ifdef VERBOSE_INSTRUMENTATION
	fclose(out);
#endif //VERBOSE_INSTRUMENTATIO
//#undef VERBOSE_INSTRUMENTATION

	return 0;
}

int write_edge_samples2()
{
#ifdef VERBOSE_INSTRUMENTATION
	node_t *node1, *node2;
	FILE *out1, *out2;
	int i,j;

	node1 = bloblist->first;
	for(i=0; i<bloblist->count; i++)
	{
		char buffer1[MAX_PATH];
		char buffer2[MAX_PATH];

		pointlist = node1->item;
		sprintf(buffer1,"TestEllipseEdges-X-Colors-%d-%d-%d.txt", pointlist->red, pointlist->green, pointlist->blue);
		sprintf(buffer2,"TestEllipseEdges-Y-Colors-%d-%d-%d.txt", pointlist->red, pointlist->green, pointlist->blue);

		out1 = fopen(buffer1, "w+");
		out2 = fopen(buffer2, "w+");
		fclose(out1);
		fclose(out2);
		out1 = fopen(buffer1, "a+");
		out2 = fopen(buffer2, "a+");
/////////////////////////////////////////
		node2 = pointlist->first;
//////////////////////////////////////////////////////////
		fprintf(out1, "x%02d=[",1);
		fprintf(out2, "y%02d=[",1);
		for(j=0; j<pointlist->count; j++)
		{
			IMAGEPOINT *img;
			img = node2->item;
			if( j % 98 == 0 )
			{
				fprintf(out1,"%d; ", img->imageX);
				fprintf(out2,"%d; ", img->imageY); 
			}
			node2 = node2->next;
		}
		fprintf(out1, "];\n",i);
		fprintf(out2, "];\n",i);
		fprintf(out1, "x%02d=[",2);
		fprintf(out2, "y%02d=[",2);
		node2 = pointlist->first;
		for(j=0; j<pointlist->count; j++)
		{
			IMAGEPOINT *img;
			img = node2->item;
			if( j % 67 == 0 )
			{
				fprintf(out1,"%d; ", img->imageX);
				fprintf(out2,"%d; ", img->imageY); 
			}
			node2 = node2->next;
		}
		fprintf(out1, "];\n",i);
		fprintf(out2, "];\n",i);


		fprintf(out1, "x%02d=[",3);
		fprintf(out2, "y%02d=[",3);
		node2 = pointlist->first;
		for(j=0; j<pointlist->count; j++)
		{
			IMAGEPOINT *img;
			img = node2->item;
			if( j % 53 == 0 )
			{
				fprintf(out1,"%d; ", img->imageX);
				fprintf(out2,"%d; ", img->imageY); 
			}
			node2 = node2->next;
		}
		fprintf(out1, "];\n",i);
		fprintf(out2, "];\n",i);

		fprintf(out1, "x%02d=[",4);
		fprintf(out2, "y%02d=[",4);
		node2 = pointlist->first;
		for(j=0; j<pointlist->count; j++)
		{
			IMAGEPOINT *img;
			img = node2->item;
			if( j % 17 == 0 )
			{
				fprintf(out1,"%d; ", img->imageX);
				fprintf(out2,"%d; ", img->imageY); 
			}
			node2 = node2->next;
		}
		fprintf(out1, "];\n",i);
		fprintf(out2, "];\n",i);

		fprintf(out1, "x%02d=[",5);
		fprintf(out2, "y%02d=[",5);
		node2 = pointlist->first;
		for(j=0; j<pointlist->count; j++)
		{
			IMAGEPOINT *img;
			img = node2->item;
			if( j % 9 == 0 )
			{
				fprintf(out1,"%d; ", img->imageX);
				fprintf(out2,"%d; ", img->imageY); 
			}
			node2 = node2->next;
		}
		fprintf(out1, "];\n",i);
		fprintf(out2, "];\n",i);

		fprintf(out1, "x%02d=[",6);
		fprintf(out2, "y%02d=[",6);
		node2 = pointlist->first;
		for(j=0; j<pointlist->count; j++)
		{
			IMAGEPOINT *img;
			img = node2->item;
			if( j % 5 == 0 )
			{
				fprintf(out1,"%d; ", img->imageX);
				fprintf(out2,"%d; ", img->imageY); 
			}
			node2 = node2->next;
		}
		fprintf(out1, "];\n",i);
		fprintf(out2, "];\n",i);
		fclose(out1);
		fclose(out2);

		node1 = node1->next;
	}
#endif //VERBOSE_INSTRUMENTATION

	return 0;
}

int write_edge_samples()
{
#ifdef VERBOSE_INSTRUMENTATION
	node_t *node1, *node2;
	FILE *out1, *out2, *out3, *out4, *out5, *out6,
		*out7, *out8, *out9, *out10, *out11, *out12;
	int i,j;

	node1 = bloblist->first;
	for(i=0; i<bloblist->count; i++)
	{
		char buffer1[MAX_PATH];
		char buffer2[MAX_PATH];
		char buffer3[MAX_PATH];
		char buffer4[MAX_PATH];
		char buffer5[MAX_PATH];
		char buffer6[MAX_PATH];
		char buffer7[MAX_PATH];
		char buffer8[MAX_PATH];
		char buffer9[MAX_PATH];
		char buffer10[MAX_PATH];
		char buffer11[MAX_PATH];
		char buffer12[MAX_PATH];

		pointlist = node1->item;
		sprintf(buffer1,"EllipseEdges-X-05-Colors-%d-%d-%d.txt", pointlist->red, pointlist->green, pointlist->blue);
		sprintf(buffer2,"EllipseEdges-X-09-Colors-%d-%d-%d.txt", pointlist->red, pointlist->green, pointlist->blue);
		sprintf(buffer3,"EllipseEdges-X-17-Colors-%d-%d-%d.txt", pointlist->red, pointlist->green, pointlist->blue);
		sprintf(buffer4,"EllipseEdges-X-53-Colors-%d-%d-%d.txt", pointlist->red, pointlist->green, pointlist->blue);
		sprintf(buffer5,"EllipseEdges-X-67-Colors-%d-%d-%d.txt", pointlist->red, pointlist->green, pointlist->blue);
		sprintf(buffer6,"EllipseEdges-X-98-Colors-%d-%d-%d.txt", pointlist->red, pointlist->green, pointlist->blue);
		sprintf(buffer7,"EllipseEdges-Y-05-Colors-%d-%d-%d.txt", pointlist->red, pointlist->green, pointlist->blue);
		sprintf(buffer8,"EllipseEdges-Y-09-Colors-%d-%d-%d.txt", pointlist->red, pointlist->green, pointlist->blue);
		sprintf(buffer9,"EllipseEdges-Y-17-Colors-%d-%d-%d.txt", pointlist->red, pointlist->green, pointlist->blue);
		sprintf(buffer10,"EllipseEdges-Y-53-Colors-%d-%d-%d.txt", pointlist->red, pointlist->green, pointlist->blue);
		sprintf(buffer11,"EllipseEdges-Y-67-Colors-%d-%d-%d.txt", pointlist->red, pointlist->green, pointlist->blue);
		sprintf(buffer12,"EllipseEdges-Y-98-Colors-%d-%d-%d.txt", pointlist->red, pointlist->green, pointlist->blue);

		out1 = fopen(buffer1, "w+");
		out2 = fopen(buffer2, "w+");
		out3 = fopen(buffer3, "w+");
		out4 = fopen(buffer4, "w+");
		out5 = fopen(buffer5, "w+");
		out6 = fopen(buffer6, "w+");
		out7 = fopen(buffer7, "w+");
		out8 = fopen(buffer8, "w+");
		out9 = fopen(buffer9, "w+");
		out10 = fopen(buffer10, "w+");
		out11 = fopen(buffer11, "w+");
		out12 = fopen(buffer12, "w+");

		node2 = pointlist->first;
		for(j=0; j<pointlist->count; j++)
		{
			IMAGEPOINT *img;
			img = node2->item;
			if( j % 98 == 0 )
			{
				fprintf(out6,"%d; ", img->imageX);
				fprintf(out12,"%d; ", img->imageY); 
			}
			else if( j % 67 == 0 )
			{
				fprintf(out5,"%d; ", img->imageX);
				fprintf(out11,"%d; ", img->imageY); 
			}
			else if( j % 53 == 0 )
			{
				fprintf(out4,"%d; ", img->imageX);
				fprintf(out10,"%d; ", img->imageY); 
			}
			else if( j % 17 == 0 )
			{
				fprintf(out3,"%d; ", img->imageX);
				fprintf(out9,"%d; ", img->imageY); 
			}
			else if( j % 9 == 0 )
			{
				fprintf(out2,"%d; ", img->imageX);
				fprintf(out8,"%d; ", img->imageY); 
			}
			else if( j % 5 == 0 )
			{
				fprintf(out1,"%d; ", img->imageX);
				fprintf(out7,"%d; ", img->imageY); 
			}
			node2 = node2->next;
		}
		fclose(out1);
		fclose(out2);
		fclose(out3);
		fclose(out4);
		fclose(out5);
		fclose(out6);
		fclose(out7);
		fclose(out8);
		fclose(out9);
		fclose(out10);
		fclose(out11);
		fclose(out12);

		node1 = node1->next;
	}
#endif //VERBOSE_INSTRUMENTATION
	return 0;
}

int remove_pixels(IMAGEPOINT colors[], int count, header_t *pointlist)
{
	int i,j;
	node_t *node1;//, *node2, *node3;
//	long total_x, total_y;
//	int max_x, max_y, min_x, min_y;
	IMAGEPOINT *img;
//#define VERBOSE_INSTRUMENTATION
#ifdef VERBOSE_INSTRUMENTATION
	FILE *output, *output2, *output3;
	char buffer[MAX_PATH];
#endif //VERBOSE_INSTRUMENTATION
	int total;

	node1 = pointlist->first;
#ifdef VERBOSE_INSTRUMENTATION
	sprintf(buffer,"containedcolors-%d-%d-%d.txt", 
pointlist->red, pointlist->green, pointlist->blue);
	output3 = fopen(buffer, "w+");

	sprintf(buffer,"dumpthecompare-%d-%d-%d.txt",
		pointlist->red, pointlist->green, pointlist->blue);
	output = fopen(buffer, "w+");

	sprintf(buffer,"removed_pixels-%d-%d-%d.txt",
		pointlist->red, pointlist->green, pointlist->blue);
	output2 = fopen(buffer, "w+");
#endif //VERBOSE_INSTRUMENTATION
#ifdef VERBOSE_INSTRUMENTATION
	for(i=0; i<count; i++)
	{
		fprintf(output3,"%d,%d,%d\n",colors[i].red, colors[i].green,
			colors[i].blue);
	}
	fclose(output3);
#endif //VERBOSE_INSTRUMENTATION
	
	total = pointlist->count;
	for(i=0; i<total; i++)
	{
		node_t *temp;
		img = node1->item;
#ifdef VERBOSE_INSTRUMENTATION
		fprintf(output, "%d,%d,%d,%d,%d,%d,%d,%d\n", 
			pointlist->red, pointlist->green, pointlist->blue,
			img->imageX, img->imageY,
			img->connectedred, img->connectedgreen, img->connectedblue);
#endif //VERBOSE_INSTRUMENTATION
		temp = node1->next;
		for(j=0; j<count; j++)
		{
			if( img->connectedblue == colors[j].blue &&
				img->connectedgreen == colors[j].green &&
				img->connectedred == colors[j].red )
			{
				if( img->imageX != 0 && img->imageY != 0 )
				{
//#ifdef VERBOSE_INSTRUMENTATION
					fprintf(debugger,"REMOVED: j=%d, ConnectedColor:%d,%d,%d, XY:%d,%d\n", j,
						img->connectedred,
						img->connectedgreen,
						img->connectedblue,
						img->imageX, img->imageY);
//#endif //VERBOSE_INSTRUMENTATION
					GlobalFree((IMAGEPOINT *)del_any_node(node1));
						break;
				}
			}
			else
			{
#ifdef VERBOSE_INSTRUMENTATION
					fprintf(debugger,"SAVED: j=%d, ConnectedColor:%d,%d,%d, XY:%d,%d\n", j,
						img->connectedred,
						img->connectedgreen,
						img->connectedblue,
						img->imageX, img->imageY);
#endif //VERBOSE_INSTRUMENTATION
			}
		}
//		if( temp == NULL )
//			break;
		node1 = temp;
	}
#ifdef VERBOSE_INSTRUMENTATION
	fclose(output);
	fclose(output2);
	
#endif //VERBOSE_INSTRUMENTATION
//#undef VERBOSE_INSTRUMENTATION
	if(pointlist->count == 0)

	return 0;
}

int clean_interior_pixels()
{
	int i,j;
	node_t *node1, *node2;//, *node3;
//	long total_x;
	//int max_y, min_x;
//	IMAGEPOINT *img;
#ifdef VERBOSE_INSTRUMENTATION
	FILE *output, *output2, *output3;
#endif //VERBOSE_INSTRUMENTATION
	IMAGEPOINT contained_colors[75];
	int count;

	node1 = bloblist->first;
	for(i=0; i<bloblist->count; i++)
	{
		count = 0;
		pointlist = node1->item;
		node2 = bloblist->first;
		for(j=0; j<bloblist->count, node2 != NULL; j++)
		{
			if( j != i && node2 != NULL )
			{
				fprintf(debugger,"CleanInterior: i=%d, j=%d\n", i, j);
				pointlist2 = node2->item;
				if( pointlist2->max_x < pointlist->max_x &&
					pointlist2->max_y < pointlist->max_y &&
					pointlist2->min_x > pointlist->min_x &&
					pointlist2->min_y > pointlist->min_y )
				{
					contained_colors[count].red = pointlist2->red;
					contained_colors[count].green = pointlist2->green;
					contained_colors[count].blue = pointlist2->blue;
					count++;
				}			
			}
			if( node2 != NULL )
				node2 = node2->next;
		}
		remove_pixels(contained_colors, count, pointlist);
		node1 = node1->next;
	}
	return 0;
}

int calculate_blob_centers()
{
	int i,j,k;
	node_t *node1, *node2;
	long total_x, total_y;
	int max_x, max_y, min_x, min_y;
	IMAGEPOINT *img;
#ifdef VERBOSE_INSTRUMENTATION
	FILE *output, *output2, *output3;
#endif //VERBOSE_INSTRUMENTATION
	node1 = bloblist->first;

	for(i=0; i<bloblist->count; i++)
	{
		total_x = total_y = 0;
		max_x = max_y = (int)0;
		min_x = min_y = (int)2e6;

		pointlist = node1->item;
		node2 = pointlist->first;
		for(j=0; j<pointlist->count; j++)
		{
			img = node2->item;
#ifdef VERBOSE_INSTRUMENTATION
			if( i==0 )
			{
				output = fopen("listofitem1.txt", "a+");
				fprintf(output, "%02x, %02x, %02x, %d, %d, %d, %d, %d\n", pointlist->red, pointlist->green, pointlist->blue,
					pointlist->count, pointlist->average_x, pointlist->average_y,img->imageX, img->imageY);
				fclose(output);
			}
#endif //VERBOSE_INSTRUMENTATION
			if( img->imageX > max_x )
				max_x = img->imageX;
			if( img->imageY > max_y )
				max_y = img->imageY;
			if( img->imageX < min_x )
				min_x = img->imageX;
			if( img->imageY < min_y )
				min_y = img->imageY;


			total_x = total_x + img->imageX;
			total_y = total_y + img->imageY;
			node2 = node2->next;
		}
		pointlist->average_x = total_x / pointlist->count;
		pointlist->average_y = total_y / pointlist->count;
		pointlist->max_x = max_x;
		pointlist->max_y = max_y;
		pointlist->min_x = min_x;
		pointlist->min_y = min_y;
		node1 = node1->next;
	}
#ifdef VERBOSE_INSTRUMENTATION
	output = fopen("LinkedListOfEdges.txt", "w+");
	fprintf(output,"Red,Green,Blue,Count,Avg X,Avg Y,Max X,Max Y,Min X,Min Y,PM Mid X,PM Mid Y,Ext X,Ext Y,Dist Avg-Mid,Blob Area\n");
#endif //VERBOSE_INSTRUMENTATION
	node1 = bloblist->first;
	for(i=0; i<bloblist->count; i++)
	{
		total_x = total_y = 0;
		max_x = max_y = (int)0;
		min_x = min_y = (int)2e6;

		pointlist = node1->item;
#ifdef VERBOSE_INSTRUMENTATION
		fprintf(output,"%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d,%d,%d,%f,%ld\n",
			pointlist->red, 
			pointlist->green,
			pointlist->blue,
			pointlist->count,
			pointlist->average_x,
			pointlist->average_y,
			pointlist->max_x,
			pointlist->max_y,
			pointlist->min_x,
			pointlist->min_y,
			mid_placemat_x,
			mid_placemat_y,
			pointlist->max_x - pointlist->min_x,
			pointlist->max_y - pointlist->min_y,
			(double)sqrt((double)((pointlist->average_x - mid_placemat_x)*(pointlist->average_x - mid_placemat_x)) + 
						(double)((pointlist->average_y - mid_placemat_y)*(pointlist->average_y - mid_placemat_y))),
			(long)(pointlist->max_x - pointlist->min_x) * (long)(pointlist->max_y - pointlist->min_y)
			);
#endif //VERBOSE_INSTRUMENTATION
		node1 = node1->next;
	}
#ifdef VERBOSE_INSTRUMENTATION
	fclose(output);
#endif //VERBOSE_INSTRUMENTATION
	clean_interior_pixels();
////////////////////////////////////////////////////////
#ifdef VERBOSE_INSTRUMENTATION
	node1 = bloblist->first;
	for(i=0; i<bloblist->count; i++)
	{
		char buffer[MAX_PATH];
		char buffer2[MAX_PATH];
		char buffer3[MAX_PATH];

		pointlist = node1->item;
		sprintf(buffer,"EdgePoints-%d-X-Colors-%d-%d-%d.txt",i, pointlist->red, pointlist->green, pointlist->blue);
		sprintf(buffer2,"EdgePoints-%d-Y-Colors-%d-%d-%d.txt",i, pointlist->red, pointlist->green, pointlist->blue);
		sprintf(buffer3,"EdgePoints-%d-Colors-%d-%d-%d.txt",i, pointlist->red, pointlist->green, pointlist->blue);

		output = fopen(buffer, "w+");
		output2 = fopen(buffer2, "w+");
		output3 = fopen(buffer3, "w+");
		node2 = pointlist->first;
		for(j=0; j<pointlist->count; j+=(pointlist->count / 32))
		{
			IMAGEPOINT *img;
			img = node2->item;
			fprintf(output,"%d; ", img->imageX);
			fprintf(output2,"%d; ", img->imageY); 
			fprintf(output3,"%d, %d\n", img->imageX, img->imageY);

			for(k=0; k<(pointlist->count / 33); k++)
				node2 = node2->next;
		}
		fclose(output);
		fclose(output2);
		fclose(output3);
		node1 = node1->next;
	}
#endif //VERBOSE_INSTRUMENTATION	
/////////////////////////////////////////////////////
	return 0;
}

int pick_food_items()
{
	int current_image;
	PLACEMAT_POINT *sp;
	int count;
	int i;
	int totalx, totaly;
	int mid_placemat_coord_x, mid_placemat_coord_y;
#ifdef VERBOSE_INSTRUMENTATION
	FILE *output;
#endif //VERBOSE_INSTRUMENTATION
	int good_pts;
	
	current_image = get_current_image();
//	current_image = 800;
	sp = get_placemat_points(current_image);
	count = get_monument_point_count();
	if( sp == NULL )
		return -1;
	totalx = totaly = 0;
	good_pts = 0;
	for(i=0; i<count; i++)
	{
		good_pts++;
		totalx = totalx + sp[i].photox;
		totaly = totaly + sp[i].photoy;
	}
	mid_placemat_coord_x = totalx / good_pts;
	mid_placemat_coord_y = totaly / good_pts;
#ifdef VERBOSE_INSTRUMENTATION
	output = fopen("MidMonumentPoints.txt","w+");
	fprintf(output,"%d, %d\n", mid_placemat_coord_x, mid_placemat_coord_y);
	fclose(output);
#endif //VERBOSE_INSTRUMENTATION

	mid_placemat_x = mid_placemat_coord_x;
	mid_placemat_y = mid_placemat_coord_y;

	free(sp);
	return 0;
}

int findDuplicateBlobPoint(header_t *list, int x, int y)
{
	int i;
	node_t *node;
	IMAGEPOINT *pt;
	FILE *out;

	node = list->first;
	for(i=0; i<list->count; i++)
	{
		pt = node->item;
		if( x == pt->imageX && y == pt->imageY )
		{
//#define VERBOSE_INSTRUMENTATION
#ifdef VERBOSE_INSTRUMENTATION
			out = fopen("FoundADuplicate.txt", "a+");
			fprintf(out, "New XY,%d, %d,  ExistingXY, %d, %d, Color, %d, %d, %d\n",
				x,y, pt->imageX, pt->imageY, list->red, list->green, list->blue);
			fclose(out);
#endif //VERBOSE_INSTRUMENTATION
//#undef VERBOSE_INSTRUMENTATION
			return 1;
		}
		node = node->next;
	}
	return 0;
}

int addPixelToEdge(uchar red, uchar green, uchar blue, int x, int y,
				   uchar connectedred, uchar connectedgreen, uchar connectedblue)
{
	node_t *node;
	int i;
	int flag = FALSE;

	if( bloblist->count == 0 )  // First blobpoint in list
	{
		pointlist = create_header();
		insert_node(pointlist, bloblist);
		pointlist->blue = blue;
		pointlist->green = green;
		pointlist->red = red;
		imagepoint = create_imagepoint_item();
		imagepoint->imageX = x;
		imagepoint->imageY = y;
		imagepoint->connectedred = connectedred;
		imagepoint->connectedgreen = connectedgreen;
		imagepoint->connectedblue = connectedblue;
		append_node(imagepoint, pointlist);
	}
	else  // look through all other blob points and see if 
	{
		node = bloblist->first;
		for(i=0; i < bloblist->count; i++)
		{
			pointlist = node->item;
			if( pointlist->blue == blue && pointlist->green == green && pointlist->red == red )
			{
//				if( findDuplicateBlobPoint(pointlist, x, y) == 0)
		//		{
					imagepoint = create_imagepoint_item();
					imagepoint->imageX = x;
					imagepoint->imageY = y;
					imagepoint->connectedred = connectedred;
					imagepoint->connectedgreen = connectedgreen;
					imagepoint->connectedblue = connectedblue;
					append_node(imagepoint, pointlist);
					flag = TRUE;
		//		}
			}
			node = node->next;
		}		
		if( flag == FALSE)
		{
			pointlist = create_header();
			insert_node(pointlist, bloblist);
			pointlist->blue = blue;
			pointlist->green = green;
			pointlist->red = red;
			imagepoint = create_imagepoint_item();
			imagepoint->imageX = x;
			imagepoint->imageY = y;
			imagepoint->connectedred = connectedred;
			imagepoint->connectedgreen = connectedgreen;
			imagepoint->connectedblue = connectedblue;
			append_node(imagepoint, pointlist);
		}
	}
	return 0;
}

int FindFood(char *filename)
{
	CvFont font;
	QtyOfColors qc[256];
	char buffer[MAX_PATH];
#ifdef VERBOSE_INSTRUMENTATION
	FILE *output2;
	FILE *output3;
#endif //VERBOSE_INSTRUMENTATION
	FILE *outputEdgePoints;
	int count=0;
	int i,j,x,y;
	int width=0;
	CvPoint pt1;
	node_t *node, *node2;
	char blobNumber[256];

	
	IplImage *image = 0;
	IplImage *image2 = 0;
	bloblist = create_header();
	pointlist = NULL;
//MessageBox(NULL,L"Inside FindFood",L"HERE", MB_OK);

	debugger = fopen("Debug.txt","w+");
#ifdef VERBOSE_INSTRUMENTATION
	output3 = fopen("AddingPixels.txt","w+");
	//fclose(output3);
#endif //VERBOSE_INSTRUMENTATION
	memset(qc, 0, sizeof(qc));

	if( (image = cvLoadImage( filename, 1)) == 0 )
        return -1;	
	
	lastpoint.imageX = 0;
	lastpoint.imageY = 0;
	lastpoint.red = *(uchar *)(image->imageData + 2 );
	lastpoint.green = *(uchar *)(image->imageData + 1 );
	lastpoint.blue = *(uchar *)(image->imageData + 0 );

//	output = fopen("testaddingPixels2.txt", "w+");
	for( i=0; i<image->height; i++)
	{
		uchar *p = (uchar *)(image->imageData + i * image->widthStep );
		width = 0;
		for( j=0; j<image->width; j++)
		{
			count++;
			if( i==0 || j==0 || j==image->width-1 || i==image->height )
			{
#ifdef VERBOSE_INSTRUMENTATION
				fprintf(output3, "CP,%d,%d,%d, LP,%d,%d,%d, XY, %d,%d\n",
					p[3*j+2],p[3*j+1],p[3*j+0],lastpoint.red, lastpoint.green, lastpoint.blue,j,i);
#endif //VERBOSE_INSTRUMENTATION
				addPixelToEdge(p[3*j+2],p[3*j+1],p[3*j+0],j,i, lastpoint.red, lastpoint.green, lastpoint.blue);
			}
			else if ( lastpoint.red == p[3*j+2] && lastpoint.green == p[3*j+1] && lastpoint.blue == p[3*j+0] )
			{}
			else
			{
				if( width != 1 )
					addPixelToEdge(lastpoint.red, lastpoint.green, lastpoint.blue, lastpoint.imageX, lastpoint.imageY,
					p[3*j+2],p[3*j+1],p[3*j+0]);
				addPixelToEdge(p[3*j+2],p[3*j+1],p[3*j+0],j,i,lastpoint.red, lastpoint.green, lastpoint.blue);
			}
			width++;
			lastpoint.red = p[3*j+2];
			lastpoint.green = p[3*j+1];
			lastpoint.blue = p[3*j+0];
			lastpoint.imageX = j;
			lastpoint.imageY = i;

			//AddPixel(qc, p[3*j+2], p[3*j+1], p[3*j+0], 256, j,i);
//			fprintf(output, "%d, R:%02x, G:%02x, B:%02x, x:%d, y:%d\n", count, p[3*j+2], p[3*j+1], p[3*j+0], j,i);
		}
	}	
	lastpoint.imageX = 0;
	lastpoint.imageY = 0;
	lastpoint.red = *(uchar *)(image->imageData + 2 );
	lastpoint.green = *(uchar *)(image->imageData + 1 );
	lastpoint.blue = *(uchar *)(image->imageData + 0 );
	for( x=0; x<image->width; x++)
	{
		width = 0;
		for( y=0; y<image->height; y++)
		{
			uchar *p = (uchar *)(image->imageData + (x * 3) + (y * image->widthStep));
			count++;
			if( x==0 || y==0 || x==image->width-1 || y==image->height )
			{
#ifdef VERBOSE_INSTRUMENTATION
				fprintf(output3, "CP,%d,%d,%d, LP,%d,%d,%d, XY, %d,%d\n",
					p[2],p[1],p[0],lastpoint.red, lastpoint.green, lastpoint.blue,j,i);
#endif //VERBOSE_INSTRUMENTATION
				addPixelToEdge(p[2],p[1],p[0],x,y, lastpoint.red, lastpoint.green, lastpoint.blue);
			}
			else if( lastpoint.red == p[2] && lastpoint.green == p[1] && lastpoint.blue == p[0] )
			{}
			else
			{
				if( width != 1 )
					addPixelToEdge(lastpoint.red, lastpoint.green, lastpoint.blue, lastpoint.imageX, lastpoint.imageY,p[2],p[1],p[0]);
				addPixelToEdge(p[2],p[1],p[0],x,y, lastpoint.red, lastpoint.green, lastpoint.blue);
			}
			width++;
			lastpoint.red = p[2];
			lastpoint.green = p[1];
			lastpoint.blue = p[0];
			lastpoint.imageX = x;
			lastpoint.imageY = y;

			//AddPixel(qc, p[3*j+2], p[3*j+1], p[3*j+0], 256, j,i);
//			fprintf(output, "%d, R:%02x, G:%02x, B:%02x, x:%d, y:%d\n", count, p[3*j+2], p[3*j+1], p[3*j+0], j,i);
		}
	}	
//	MessageBox(NULL,L"Before Opening File",L"HERE", MB_OK);
	if( (image2 = cvLoadImage( get_original_filename(), 1)) == 0 )
	{
		MessageBox(NULL,L"Could not open original File",L"ERROR", MB_ICONHAND);
        return -1;	
	}
	node = bloblist->first;
	for(i=0; i < bloblist->count; i++)
	{
		pointlist = node->item;
		node2 = pointlist->first;
		for(j=0; j < pointlist->count; j++)
		{
			imagepoint = node2->item;
			pt1.x = imagepoint->imageX;
			pt1.y = imagepoint->imageY;
			node2 = node2->next;
			cvLine(image2, pt1, pt1, CV_RGB(0,255,0),1,8,0);
		}
		node = node->next;
	}	
//	MessageBox(NULL,L"Finished drawing Lines",L"HERE", MB_OK);
	sprintf(buffer,"%s-EdgeOriginalFile.png", get_filename_base());
//	MessageBox(NULL,L"Calling Save Image",L"HERE", MB_OK);

	cvSaveImage(buffer, image2,0);

//	fclose(output);
	fprintf(debugger,"Calling PickFood");
	pick_food_items();
	fprintf(debugger,"Calling CalcBlobCenters");
	calculate_blob_centers();
	fprintf(debugger,"Calling WriteEdgeSamples");
	write_edge_samples2();
	fprintf(debugger,"Calling CountColors");
	count_colors(image);
#ifdef VERBOSE_INSTRUMENTATION
	output2 = fopen("AnalyzeEllipses.txt","w+");
	fclose(output2);
#endif //VERBOSE_INSTRUMENTATION
	fprintf(debugger,"Calling FitEllipses");
	fitEllipses(image);
	fprintf(debugger,"Calling FindPlate");
	findPlate(image);
//	findPlate2(image);
	node = bloblist->first;
	for(i=0; i < bloblist->count; i++)
	{
		pointlist = node->item;
		node2 = pointlist->first;
		for(j=0; j < pointlist->count; j++)
		{
			imagepoint = node2->item;
			pt1.x = imagepoint->imageX;
			pt1.y = imagepoint->imageY;
			node2 = node2->next;
			cvLine(image, pt1, pt1, CV_RGB(imagepoint->connectedred, imagepoint->connectedgreen, imagepoint->connectedblue),1,8,0);
		}
		node = node->next;
	}		
	cvSaveImage("testimage.ppm", image,0);

	cvInitFont(&font, CV_FONT_HERSHEY_TRIPLEX, 1, 2, 0,2,8);

	node = bloblist->first;
	count = 1;
	for(i=0; i < bloblist->count; i++)
	{
		IMAGEPOINT *imgpt;
		pointlist = node->item;
		if( pointlist->isFood )
		{
			IplImage *img;
			sprintf(blobNumber, "%d(%d,%d,%d)", count, pointlist->red,
				pointlist->green, pointlist->blue);
			pt1.x = pointlist->average_x; pt1.y = pointlist->average_y;
			cvPutText(image,blobNumber,pt1,&font, CV_RGB(255,255,255));
			count++;
			//MessageBox(NULL,L"Cloning", "STATUS", MB_OK);
			img = cvCloneImage(image);
			//MessageBox(NULL,L"SettingZero", "STATUS", MB_OK);
			cvSetZero(img);
			//MessageBox(NULL,L"SettingOne", "STATUS", MB_OK);
			
			cvSet(img,cvScalar(255,255,255,0), NULL);
		
			node2 = pointlist->first;
//////////////////////////////////////////////////////////
			//edge98 = cvCreateMat(pointlist->count, 1, CV_32SC2);
			count=0;
			sprintf(buffer, "%s.CSV", get_original_filename());
			outputEdgePoints = fopen(buffer, "w+");
			for(j=0; j<pointlist->count; j++)
			{
//				IMAGEPOINT *img;
				imgpt = node2->item;
				cvDrawLine(img, cvPoint( imgpt->imageX, imgpt->imageY), cvPoint( imgpt->imageX, imgpt->imageY),
					cvScalar(0,0,0,0),1,8,0);
				fprintf(outputEdgePoints, "%d,%d\n", imgpt->imageX, imgpt->imageY );
//				CV_MAT_ELEM(*edge98, CvPoint, j, 0) = cvPoint(img->imageX, img->imageY);
//				count++;
				node2 = node2->next;
				//node2 = node2->next;
			}

			fclose(outputEdgePoints);
			sprintf(buffer, "%s.PPM", get_original_filename());

			
			//MessageBox(NULL,L"Saving", "STATUS", MB_OK);
			cvSaveImage(buffer,img,0);
			//MessageBox(NULL,L"Releasing", "STATUS", MB_OK);
			cvReleaseImage(&img);
			
		}
		node = node->next;
	}		
	sprintf(buffer,"%s-BLOBImage.jpg", get_filename_base());
	cvSaveImage(buffer,image,0);

	cvReleaseImage(&image);
	cvReleaseImage(&image2);

	j = bloblist->count;
	node = bloblist->first;
	for( i=0; i<j, node!=NULL; i++)
	{
		pointlist = node->item;
		while( pointlist->count > 0 )
			GlobalFree((IMAGEPOINT *)del_first_node(pointlist));
//		destroy_list(pointlist);
		node = node->next;
	}
	destroy_list(bloblist);
	fclose(debugger);
#ifdef VERBOSE_INSTRUMENTATION
	fclose(output3);
#endif //VERBOSE_INSTRUMENTATION

	return 0;
}

int FindFood2(char *filename)
{
	CvFont font;
	QtyOfColors qc[256];
#ifdef VERBOSE_INSTRUMENTATION
	FILE *output2;
	FILE *output3;
#endif //VERBOSE_INSTRUMENTATION
	int count=0;
	int i,j,x,y;
	int width=0;
	CvPoint pt1;
	node_t *node, *node2;
	char blobNumber[256];
	IplImage *g_gray;
	CvMemStorage *g_storage;
	CvSeq *contours;
	int contourcount;
	IplImage *image;
		char buffer[1048];
	wchar_t buff[1048];
	contours = 0;
	image = 0;


	bloblist = create_header();
	pointlist = NULL;

	debugger = fopen("Debug.txt","w+");
#ifdef VERBOSE_INSTRUMENTATION
	output3 = fopen("AddingPixels.txt","w+");
	//fclose(output3);
#endif //VERBOSE_INSTRUMENTATION
	memset(qc, 0, sizeof(qc));
	g_storage = cvCreateMemStorage(0);
	
	OpenCVcanny(filename);

	if( (image = cvLoadImage( get_current_filename(), 1)) == 0 )
        return -1;	
	g_gray = cvCreateImage( cvGetSize(image), 8, 1);
	cvCvtColor(image, g_gray, CV_BGR2GRAY);
	
	cvSaveImage("test.jpg",g_gray, 0);
	contourcount = cvFindContours(g_gray, g_storage, &contours, sizeof(CvContour), CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0));
	cvZero(g_gray);
/*
	sprintf(buffer, "Contours Found = %d", contourcount);
	mbstowcs(buff, buffer, sizeof(buffer));
	MessageBox(NULL,buff, L"Count of Contours", MB_OK);
*/
	for( ; contours != 0; contours = contours->h_next )
	{
		CvScalar color = CV_RGB( 0, 255, 0 );
		/* replace CV_FILLED with 1 to see the outlines */
		cvDrawContours( g_gray, contours, cvScalarAll(255), color, CV_FILLED, 1, 8, cvPoint(0,0) );
	}
//	MessageBox(NULL,L"Drawing Contours",L"Drawing", MB_OK);
//	cvDrawContours(g_gray, contours, cvScalarAll(255), cvScalarAll(255), 100, 1, 8, cvPoint(0,0));
//	}
	cvShowImage("Contours", g_gray);
	cvWaitKey(0);
	return(0);
}