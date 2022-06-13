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
//#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winuser.h>
//#undef WIN32_LEAN_AND_MEAN
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
//#include <errno.h>

/************RUNNING FOR LAB TESTS***************/
//#define PERCENTSAMPLE	0.95
//#define PLATECUTOFF		0.51
//#define STDDEVMULT		50.0
#define SAMPLESIZE		30
#define PERCENTSAMPLE	0.90
#define PLATECUTOFF		0.00
#define STDDEVMULTUP	6.0
#define STDDEVMULTDOWN	3.0

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

#include "assigncoordinates.h"
#include "images.h"
#include "window_globals.h"
#include <commctrl.h>

typedef struct LISTY
{
    int count;
    char **data;
} List;

struct threadargs
{
	HWND hWnd;
	HWND hStatusBar;
	HWND hToolBar;
};

typedef struct threadargs THREADARGS;
typedef struct threadargs * PTHREADARGS;

extern THREADARGS threadArgs;

extern wchar_t status_bar1[2048];
extern wchar_t status_bar2[2048];
extern wchar_t status_bar3[2048];

extern qhull_path[MAX_PATH];

typedef struct pointfile_names {
	char *name;
	int blobID;
	int foodItemID;
} POINTFILE_NAMES;

float meanz, stddevz, minZ, maxZ;

void AddStringToList(List *list,const char *string)
{
    int c =list->count;
    list->data=(char**)realloc(list->data,sizeof(char*)*(c+1));

    ++list->count;

    list->data[c]=malloc(strlen(string)+1);
    
    strcpy(list->data[c],string);
}

int samplePoints(char *filename, int foodItemID, int volumetype)
{

	char buffer[1024];
	FILE *output, *output2;
	FILE *input;
	int count;
	int array1[256];
	float volume[SAMPLESIZE];
	int array2[256];
	float vol_avg;
	time_t time1;
	int i,j,k;
	List list;
	char **pointList;
	time1 = time(NULL);
	srand((unsigned int)time1);

	vol_avg = 0;

	input = fopen(filename,"r+");
	output = fopen("Filtertemp.txt","w+");
	count = 0;
	while( fgets(buffer, sizeof(buffer), input) )
	{
		fprintf(output,"%s", buffer);
		count++;
	}
	fclose(output);
	fclose(input);
	list.count = 0;
	list.data = (char **)malloc(sizeof(char *));
	input = fopen("Filtertemp.txt","r");
	while( fgets(buffer, sizeof(buffer), input) )
	{
		AddStringToList(&list, buffer);
	}
	fclose(input);

	output = fopen("DJBstuffList.txt","w+");
	for(i=0; i<list.count; i++)
	{
		fprintf(output,"%d:%s\n", i, list.data[i]);
	}
	fclose(output);

	output2 = fopen("DaveVolumes.txt","a+");
	fprintf(output2, "FOOD ITEM ID = %d, StdZ=%f, MeanZ=%f, MinZ=%f, "
					"MaxZ=%f, Percent Sample = %2.2f, Plate Height = %0.2f, "
					"StdDevUp = %0.2f, StdDevDown = %0.2f, MealID = %d, SampleSize=%d\n", 
					foodItemID, stddevz, meanz, minZ, maxZ, 
					PERCENTSAMPLE, PLATECUTOFF, STDDEVMULTUP, 
					STDDEVMULTDOWN, getMealIDfromFoodItem(foodItemID),
					SAMPLESIZE);
		
	fclose(output2);
	for( i=0; i<SAMPLESIZE; i++)
	{
		int index;
		output = fopen(filename, "w+");
		fprintf(output,"3\n");
		if( PERCENTSAMPLE == 1.0 )
			k=count;
		else
			k = (int)((float)count*PERCENTSAMPLE);
		fprintf(output,"%d\n", k);

		if( PERCENTSAMPLE == 1.0 )
		{
			for(j=0; j<count; j++)
			{
				fprintf(output, "%s", list.data[j]);
			}
		}
		else
		{
			for(j=0; j<k; j++)
			{
				index = (int)((float)((float)rand()/(float)RAND_MAX) * (float)count);
				if( index == count )
					index--;
				//fprintf(output,"Count:%d, k:%d, index:%d\n",
				//	count, k, index);
				fprintf(output, "%s", list.data[index]);
			}
		}
		fclose(output);
		wrightGeomViewPointFile(filename, "GEMOUT.TXT");

		volume[i] = calculateVolume(filename);
		output2 = fopen("DaveVolumes.txt","a+");
		fprintf(output2, "Sample VOLUME[i]=%f\n", volume[i]);
		fclose(output2);
	}
	for( i=0; i<SAMPLESIZE; i++ )
	{
		vol_avg = vol_avg + volume[i];
		output2 = fopen("DaveVolumes.txt","a+");
		fprintf(output2, "ADDING VALUE Average=%10.8f, Volume[i]=%f\n", vol_avg, volume[i]);
		fclose(output2);
	}
	output2 = fopen("DaveVolumes.txt","a+");
	fprintf(output2, "BEFORE Average=%10.8f\n", (double)vol_avg);
	fclose(output2);

	vol_avg = vol_avg / SAMPLESIZE;
	
	output2 = fopen("DaveVolumes.txt","a+");
	fprintf(output2, "VolumeAverage=%10.8f\n", vol_avg);
	fclose(output2);
	
	update_fooditem_volume(foodItemID, (double)vol_avg, volumetype);
}

int sortRayIntersects(void *item1, void *item2)
{
	RAYINTERSECT_PARAMS *ray1, *ray2;
	
	ray1 = (RAYINTERSECT_PARAMS *)item1;
	ray2 = (RAYINTERSECT_PARAMS *)item2;

	if( ray1->blobpoint1ID > ray2->blobpoint1ID )
		return 1;
	else if ( ray1->blobpoint1ID < ray2->blobpoint1ID )
		return -1;
	else if ( ray1->blobpoint1ID == ray2->blobpoint1ID )
	{
		if( ray1->distanceFromCamera > ray2->distanceFromCamera )
			return 1;
		else if ( ray1->distanceFromCamera < ray2->distanceFromCamera )
			return -1;
		else if ( ray1->distanceFromCamera == ray2->distanceFromCamera )
			return 0;
	}
}

int wrightGeomViewPointFile(char *inputfile, char *outputfile)
{
	char name2[MAX_PATH];
	char name3[MAX_PATH];
	char *p;
	FILE *geomview, *input, *sphere;
	char buffer[1024];
	wchar_t buff[MAX_PATH];

	float x,y,z;
	int count;
	int i,j,k;

	strcpy(buffer, inputfile);
	p = strrchr(buffer, '.');
	if( p != NULL)
		*p = '\0';
	sprintf(name2, "GEOM-%s.vect", buffer);
//	sprintf(name3, "SPEHERE-%s.sphere", buffer);
	geomview = fopen(name2,"w+");
//	sphere = fopen(name3,"w+");

	mbstowcs(buff, buffer, sizeof(buffer));
	if( geomview == NULL )
	{
		MessageBox(NULL, buff, L"geomview did not open", MB_ICONHAND);
	}
/*	if(sphere == NULL )
	{
		MessageBox(NULL, buff, L"sphere file did not open", "MB_ICONHAND");
	}

	fprintf(sphere, "SPHERE\n");
*/
	input = fopen(inputfile, "r");
	fgets(buffer,sizeof(buffer), input);
	fgets(buffer,sizeof(buffer), input);
	count = 0;
	while( fgets(buffer, sizeof(buffer), input) )
	{		
		count++;
	}
	fclose(input);
	fprintf(geomview,"VECT\n");
	fprintf(geomview,"%d %d %d\n\n", count, count, 1);
	for(k=0; k<count; k++)
	{
		fprintf(geomview,"1 ");
	}
	fprintf(geomview,"\n");
	for(k=0; k<count; k++)
	{
		if( k == 0 )
			fprintf(geomview,"1 ");
		else
			fprintf(geomview,"0 ");
	}
	fprintf(geomview,"\n");
	input = fopen(inputfile, "r");
	fgets(buffer,sizeof(buffer), input);
	fgets(buffer,sizeof(buffer), input);
	while( fgets(buffer, sizeof(buffer), input) )
	{
		sscanf(buffer, "%f %f %f\n", &x, &y, &z);
/// FOR ALL POINTS
		fprintf(geomview,"%f %f %f\n", x, y, z);
//		fprintf(sphere,"0.0625 %f %f %f\n", x, y, z);
/// ENDFOR

	}

	fprintf(geomview,"\n");
	fprintf(geomview,"%f %f %f %f\n", 0.0, 1.0, 0.0, 0.5); 
	fclose(geomview);
	fclose(input);
//	fclose(sphere);
}

float calculateVolume(char *inputfile)
{
	char buffer[1024];
	FILE *input, *output;
	int count;
	float area;
	float volume;
	float dummy1;

	sprintf(buffer,"%s\\qconvex FS <%s >holder.txt", qhull_path, inputfile);
	system(buffer);
	input = fopen("holder.txt","r");
	fgets(buffer, sizeof(buffer), input);
	fscanf(input,"%d %f %f", &count, &area, &volume);
	
	volume = volume * 0.0692640693;
	
	output = fopen("DaveVolumes.txt","a+");
	fprintf(output,"DJB!!!Volume = %f\n", volume);
	fclose(output);

	return volume;
}

int pointFilter(char *name, int foodItemID, float upperfactor, float lowerfactor, float plateheight, int filtertype)
{
	float meanX, meanY, meanZ;
	int count,k;
	float stddevX, stddevY, stddevZ;
	float x,y,z;
	FILE *input, *output, *output2;
	char buffer[2048];
	wchar_t buff[2048];
	char name2[MAX_PATH];
	char name3[MAX_PATH];
	float area, volume;
	char *p;
	float maxx, minx, maxy, miny, maxz, minz;
	FILE *geomview;
	float upperlimit, lowerlimit;
	float temp;
	int s;

	wrightGeomViewPointFile(name, "GEOMOUT.txt");
	input = fopen(name,"r+");
	fgets(buffer, sizeof(buffer), input);
	fgets(buffer, sizeof(buffer), input);

	meanX=meanY=meanZ=count=0;
	minx = miny = minz = FLT_MAX;
	maxx = maxy = maxz = -FLT_MAX;
	while( fgets(buffer, sizeof(buffer), input) )
	{
		sscanf(buffer, "%f %f %f\n", &x, &y, &z);
		meanX = meanX + x;
		meanY = meanY + y;
		meanZ = meanZ + z;
		if( x < minx )
			minx = x;
		if( x > maxx )
			maxx = x;
		if( y < miny )
			miny = y;
		if( y > maxy )
			maxy = y;
		if( z < minz )
			minZ = minz = z;
		if( z > maxz )
			maxZ = maxz = z;
		
		count++;
	}

	meanX = meanX / count;
	meanY = meanY / count;
	meanZ = meanZ / count;
	fclose(input);

	input = fopen(name,"r+");
	fgets(buffer, sizeof(buffer), input);
	fgets(buffer, sizeof(buffer), input);
	stddevX=stddevY=stddevZ=0;

	while( fgets(buffer, sizeof(buffer), input) )
	{
		sscanf(buffer, "%f %f %f\n", &x, &y, &z);
		stddevX = stddevX + pow((x - meanX),2.0);
		stddevY = stddevY + pow((y - meanY),2.0);
		stddevZ = stddevZ + pow((z - meanZ),2.0);
	}
	stddevX = sqrt(stddevX / count);
	stddevY = sqrt(stddevY / count);
	stddevZ = sqrt(stddevZ / count);
	fclose(input);

	sprintf(name2,"%s-Mean-StdDev.txt", name);
	output2 = fopen(name2,"w+");
	fprintf(output2,"Dim\tMean\tStdDev\tMin\tMax\nX\t%f\t%f\t%f\t%f\nY\t%f\t%f\t%f\t%f\nZ\t%f\t%f\t%f\t%f\n",
		meanX, stddevX, minx, maxx, meanY, stddevY, miny, maxy, meanZ, stddevZ, minz, maxz);
	meanz = meanZ;
	stddevz = stddevZ;

	fclose(output2);

	update_fooditem_volume(foodItemID, (double)stddevz, STDDEV);
	update_fooditem_volume(foodItemID, (double)meanz, MEAN);

//	sprintf(buffer,"MeanZ=%f\nStdDevZ=%f", meanZ, stddevZ);
//	mbstowcs(buff, buffer, sizeof(buffer));
//	MessageBox(NULL, buff, L"Mean/StdDev", MB_OK);

	input = fopen(name,"r+");
	fgets(buffer, sizeof(buffer), input);
	fgets(buffer, sizeof(buffer), input);
	sprintf(buffer, "%s", name);
	p = strrchr(name, '.');
	if( p != NULL )
		p = '\0';
	sprintf(name2,"%s-Filter-%02.0f-%02.0f-%02.2f.txt", name, upperfactor, lowerfactor, plateheight);
	output = fopen(name2, "w+");

	count = 0;
	upperlimit = upperfactor*stddevZ;
	lowerlimit = lowerfactor*stddevZ;

	while( fgets(buffer, sizeof(buffer), input) )
	{

		sscanf(buffer, "%f %f %f\n", &x, &y, &z);

		if( /*z < meanZ + (0.95 * stddevZ) &&
			z > meanZ - (0.95 * stddevZ) &&
			z > 0.5 )*/
			(z < meanZ + upperlimit) &&
			(z > meanZ - lowerlimit ) && 
			(z >= plateheight ) )
//			&&*/
//			(z < maxz * 0.95 ) &&
//			(x < maxx * 0.95 ) &&
//			(y < maxy * 0.95 ) 	)
		{
			fprintf(output,"%f %f %f\n", x, y, z);
			count++;
		}
	}

	fclose(input);
	fclose(output);
	
	samplePoints(name2, foodItemID, filtertype);
/*
	input = fopen(name2,"r+");
	output = fopen("Filtertemp.txt","w+");
	count = 0;
	while( fgets(buffer, sizeof(buffer), input) )
	{
		fprintf(output,"%s", buffer);
		count++;
	}
	fclose(output);
	fclose(input);

	input = fopen("Filtertemp.txt","r+");
	output = fopen(name2, "w+");
	fprintf(output,"3\n");
	fprintf(output,"%d\n", count);
	while( fgets(buffer, sizeof(buffer), input) )
	{
		fprintf(output,"%s", buffer);
	}
	fclose(input);
	fclose(output);
	wrightGeomViewPointFile(name2);

/////////////////
	volume = calculateVolume(name2);
	///////////////
//	update_fooditem_volume(foodItemID, volume);
	fclose(input);
	*/
	return(0);
}

int buildQHullFiles(POINTFILE_NAMES *pfNames, int count)
{
	int i,j;
	FILE *output, *input, *out2;
	char name[MAX_PATH];
	char str[1024];
	char dummy[256];
	output = fopen("pfNames.txt","w+");
	for(i=0; i<count; i++)
		fprintf(output,"%s, %d, %d\n", 
			pfNames[i].name, pfNames[i].foodItemID, pfNames[i].foodItemID);
	fclose(output);
	out2 = fopen("trace.txt","w+");
	for(i=0; i<count; i++)
	{
		fprintf(out2,"i=%d\n", i);
		if( pfNames[i].foodItemID != 0 )
		{
			fprintf(out2,"Inside foodItemID !=0:  pfNames[i].foodItemID=%d\n", pfNames[i].foodItemID);
			sprintf(name, "QHULL%06d.txt", pfNames[i].foodItemID);
			output = fopen(name, "w+");
			fprintf(output,"3\nXXX\n");
			fclose(output);
			fprintf(out2,"Calling Inner J-Loop\n");
			for(j=0; j<count; j++)
			{
				if( pfNames[i].foodItemID == pfNames[j].foodItemID )
				{
					fprintf(out2,"Inside foodItemID=%d i=j:  i=%d, j=%d\n", pfNames[j].foodItemID, i,j);
					output = fopen(name, "a+");
					input = fopen(pfNames[j].name, "r");
					while( fgets(str, sizeof(str), input) != NULL )
					{
						fprintf(output, "%s", str);
					}
					fclose(input);
					fclose(output);
					if( j!=i)
						pfNames[j].foodItemID = 0;
				}
				else
				{
					fprintf(out2,"foodItemID's were different, %d != %d\n", pfNames[i].foodItemID, pfNames[j].foodItemID);
				}
			}
			histogramPointCloud(name, pfNames[i].foodItemID);
			pointFilter(name, pfNames[i].foodItemID, 6, 3, PLATECUTOFF, STDMEAN63);
			pointFilter(name, pfNames[i].foodItemID, 5, 2, PLATECUTOFF, STDMEAN52);
			pointFilter(name, pfNames[i].foodItemID, 4, 1, PLATECUTOFF, STDMEAN41);
			pointFilter(name, pfNames[i].foodItemID, 3, 0, PLATECUTOFF, STDMEAN30);
			pointFilter(name, pfNames[i].foodItemID, 6, 2, PLATECUTOFF, STDMEAN62);
			pointFilter(name, pfNames[i].foodItemID, 5, 1, PLATECUTOFF, STDMEAN51);
			pointFilter(name, pfNames[i].foodItemID, 3, 1, PLATECUTOFF, STDMEAN31);
			pointFilter(name, pfNames[i].foodItemID, 2, 2, PLATECUTOFF, STDMEAN22);
			pointFilter(name, pfNames[i].foodItemID, 2, 1, PLATECUTOFF, STDMEAN21);
			//fclose(output);
		}
	}
	fclose(out2);
}

int calculateFoodVolumePoints(MATCHING_BLOBS *mb, int count)
{
	BLOBPOINT_PARAMS *blobPoints;
	BLOBPOINT_PARAMS *currentBP;
	BLOBPOINT_PARAMS *nextBP;
	POINTFILE_NAMES *pfNames;
	POINT_3D pt1, pt2, pt3, pt4;
	RAYINTERSECT_PARAMS *rayIntersects;
	INTERSECT_POINT_3D *ip3DTemp;

	PHOTOG_PARAMS *currentPP;
	PHOTOG_PARAMS *nextPP;
	int countOfRays;
	int i,k;
	int m,n,p,s;
	int blobNumber;
	int totalProcessed;
	int countOfBlobPoints;
	int countOfCurrentBlobPoints;
	int countOfNextBlobPoints;
//	MSG msg;
	MSG message;
	char buffer[1024];
	wchar_t buff[1024];
	float stddevX;
	float stddevY;
	float stddevZ;
	float meanX;
	float meanY;
	float meanZ;
	float factordown;
	float factorup;
	FILE *output, *geomview, *tracer, *output2;
	char name[256];
	char name2[256];
	int j;
	float desiredSecondPoint;
	int rayIndexSecondPoint;

	pfNames = (POINTFILE_NAMES *)malloc(count*sizeof(POINTFILE_NAMES));
	memset(pfNames, 0, sizeof(pfNames));
	currentBP = NULL;

	rayIntersects = NULL;
	for( i=0; i<count; i++)
	{
		wsprintf(status_bar3,L"Loop to calculate FoodVolumePoints %d", i);
		PostMessage(threadArgs.hStatusBar, SB_SETTEXT, 3, (LPARAM)(LPWSTR)status_bar3);
/*		if (PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}*/
		sprintf(name, "VolumePoints-%06d", mb[i].blob_id);
		for(m=0; m<count; m++)
		{
			if( mb[i].foodItemID == mb[m].foodItemID && m != i )
			{
				sprintf(name, "%s-%06d", name, mb[m].blob_id);
			}
		}
		sprintf(name2,"All%s.txt", name);
		strcat(name, ".txt");
		pfNames[i].name = (char *)malloc(sizeof(name));
		strcpy(pfNames[i].name, name);
		pfNames[i].blobID = mb[i].blob_id;
		pfNames[i].foodItemID = mb[i].foodItemID;

//		mbstowcs(buff, name, sizeof(buffer));
//		MessageBox(NULL,buff, "FILENAME", MB_OK);
		output = fopen(name,"w+");
		output2 = fopen(name2,"w+");
//		fprintf(output,"X\tY\tZ\tDistance\tBLOBPOINT 1\tBLOBPOINT 2\tBLOB 1\tBLOB 2\tCam 1X\tCam 1Y\tCam 1Z\tDist to Cam\n");
		fprintf(output2,"X\tY\tZ\tDistance\tBLOBPOINT 1\tBLOBPOINT 2\tBLOB 1\tBLOB 2\tCam 1X\tCam 1Y\tCam 1Z\tDist to Cam\n");

		fclose(output);
		fclose(output2);

		countOfRays = 0;
		currentBP = getBlobPointsForRays(mb[i].foodItemID, mb[i].blob_id);
		countOfCurrentBlobPoints = get_current_blobpoint_count();	
		if( countOfCurrentBlobPoints == 0 )
			MessageBox(NULL,L"countOfCurrentBlobPoints == 0",L"ERROR getting blob points", MB_ICONHAND);

		currentPP = getCameraForRays(mb[i].blob_id);
		for(j=0; j<count; j++)
		{

			if( mb[i].foodItemID == mb[j].foodItemID && i != j)
			{
				nextPP = getCameraForRays(mb[j].blob_id);
				nextBP = getBlobPointsForRays(mb[j].foodItemID, mb[j].blob_id);
				countOfNextBlobPoints = get_current_blobpoint_count();		
				if( countOfNextBlobPoints == 0 )
					MessageBox(NULL,L"countOfNextBlobPoints == 0",L"ERROR getting blob points", MB_ICONHAND);
				rayIntersects = (RAYINTERSECT_PARAMS *)realloc(rayIntersects, 
					sizeof(RAYINTERSECT_PARAMS)*countOfCurrentBlobPoints*countOfNextBlobPoints );
				if( rayIntersects == NULL )
				{
					MessageBox(NULL,L"Cannot allocate memory in calculateVolumePoints",L"MEMORY ALLOCATION ERROR", MB_ICONHAND);
				}
//				for(m=0; m<countOfCurrentBlobPoints; m+=5)  //DJB: 8/22/2011
				for(m=0; m<countOfCurrentBlobPoints; m+=1)
				{
					currentPP->image_x = currentBP[m].imageX;
					currentPP->image_y = currentBP[m].imageY;
					createRay(currentPP);
/*					if (PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
					{
						TranslateMessage(&message);
						DispatchMessage(&message);
					}*/
					for(n=0; n<countOfNextBlobPoints; n+=2)
					{
						nextPP->image_x = nextBP[n].imageX;
						nextPP->image_y = nextBP[n].imageY;
						createRay(nextPP);
						pt1.x = currentPP->camera_x;
						pt1.y = currentPP->camera_y;
						pt1.z = currentPP->camera_z;
						pt2.x = currentPP->world_x;
						pt2.y = currentPP->world_y;
						pt2.z = currentPP->world_z;

						pt3.x = nextPP->camera_x;
						pt3.y = nextPP->camera_y;
						pt3.z = nextPP->camera_z;
						pt4.x = nextPP->world_x;
						pt4.y = nextPP->world_y;
						pt4.z = nextPP->world_z;

						ip3DTemp = closest_point_between_2_lines(&pt1, &pt2, &pt3, &pt4);						
/*						sprintf(buffer, "X:%f,Y:%f,Z:%f-X:%f,Y:%f,Z:%f-X:%f,Y:%f,Z:%f-X:%f,Y:%f,Z:%f", 
							pt1.x, pt1.y,pt1.z,
							pt2.x, pt2.y,pt2.z,
							pt3.x, pt3.y,pt3.z,
							pt4.x, pt4.y,pt4.z);

						mbstowcs(status_bar3,  buffer, sizeof(status_bar3));
						PostMessage(threadArgs.hStatusBar, SB_SETTEXT, 3, (LPARAM)(LPWSTR)status_bar3);
						if (PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
						{
							TranslateMessage(&message);
							DispatchMessage(&message);
						}*/
//						if( ip3DTemp->distance < 0.01 && ip3DTemp->z > 0 )  DJB 8/22/2011
						if( ip3DTemp->distance < 0.05 && ip3DTemp->z > 0 )
						{
							rayIntersects[countOfRays].intersectX = ip3DTemp->x;
							rayIntersects[countOfRays].intersectY = ip3DTemp->y;
							rayIntersects[countOfRays].intersectZ = ip3DTemp->z;
							rayIntersects[countOfRays].intersectDistance = ip3DTemp->distance;
							rayIntersects[countOfRays].cameraX = currentBP[i].cameraX;
							rayIntersects[countOfRays].cameraY = currentBP[i].cameraX;
							rayIntersects[countOfRays].cameraZ = currentBP[i].cameraX;
							rayIntersects[countOfRays].distanceFromCamera = 
								sqrt(pow(currentPP->camera_x-ip3DTemp->x,2.0) +
									pow(currentPP->camera_y-ip3DTemp->y,2.0) +
									pow(currentPP->camera_z-ip3DTemp->z,2.0));
							rayIntersects[countOfRays].blobpoint1ID = currentBP[m].blobPointID;
							rayIntersects[countOfRays].blobpoint2ID = nextBP[n].blobPointID;
							rayIntersects[countOfRays].blob1ID = mb[i].blob_id;
							rayIntersects[countOfRays].blob2ID = mb[j].blob_id;
							countOfRays++;
						}
						sprintf(buffer,
							
							"CountOfRays:%d", countOfRays);
						mbstowcs(status_bar2,  buffer, sizeof(status_bar2));
						PostMessage(threadArgs.hStatusBar, SB_SETTEXT, 2, (LPARAM)(LPWSTR)status_bar2);




						if( countOfRays > 10000 )
						{
							//MessageBox(NULL, L"Loads of Rays", L"ERROR", MB_OK);
						}
						if (PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
						{
							TranslateMessage(&message);
							DispatchMessage(&message);
						}
						free(ip3DTemp);
					}

				}
#ifdef SAFETY
				sprintf(buffer, "QSorting...", countOfRays);
				mbstowcs(status_bar2,  buffer, sizeof(status_bar3));
				PostMessage(threadArgs.hStatusBar, SB_SETTEXT, 3, (LPARAM)(LPWSTR)status_bar3);
				if (PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
				{
					TranslateMessage(&message);
					DispatchMessage(&message);
				}
				output = fopen(name,"a+");
				meanX = meanY = meanZ = 0;
				for(s=0; s<countOfRays; s++)
				{
					meanX = meanX + rayIntersects[s].intersectX;
					meanY = meanY + rayIntersects[s].intersectY;
					meanZ = meanZ + rayIntersects[s].intersectZ;
				}				
				meanX = meanX / countOfRays;
				meanY = meanY / countOfRays;
				meanZ = meanZ / countOfRays;
				stddevX = stddevY = stddevZ = 0;
				for(s=0; s<countOfRays; s++)
				{
					stddevX = stddevX + pow((rayIntersects[s].intersectX - meanX),2.0);
					stddevY = stddevY + pow((rayIntersects[s].intersectY - meanY),2.0);
					stddevZ = stddevZ + pow((rayIntersects[s].intersectZ - meanZ),2.0);
				}
				stddevX = sqrt(stddevX / countOfRays);
				stddevY = sqrt(stddevY / countOfRays);
				stddevZ = sqrt(stddevZ / countOfRays);
				factordown = 4.0;
				factorup = 4.0;
				for(s=0; s<countOfRays; s++)
				{
/*					if( (rayIntersects[s].intersectX < meanX + (factorup*stddevX)) &&
						(rayIntersects[s].intersectX > meanX - (factordown*stddevX)) &&

						(rayIntersects[s].intersectY < meanY + (factorup*stddevY)) &&
						(rayIntersects[s].intersectY > meanY - (factordown*stddevY)) &&

						(rayIntersects[s].intersectZ < meanZ + (factorup*stddevZ)) &&
						(rayIntersects[s].intersectZ > meanZ - (factordown*stddevZ)) && 
						(rayIntersects[s].intersectZ >= 0.5) )
					{*/
					if( rayIntersects[s].intersectZ >= 0.25)
					{
						fprintf(output,"%f\t%f\t%f\t%f\t%d\t%d\t%d\t%d\t%f\t%f\t%f\t%f\n",
							rayIntersects[s].intersectX,
							rayIntersects[s].intersectY,
							rayIntersects[s].intersectZ,
							rayIntersects[s].intersectDistance,
							rayIntersects[s].blobpoint1ID,
							rayIntersects[s].blobpoint2ID, 
							mb[i].blob_id, mb[j].blob_id,
							currentPP->camera_x,
							currentPP->camera_y,
							currentPP->camera_z,
							rayIntersects[s].distanceFromCamera);
	/*					fprintf(output,"%f\t%f\t%f\n",
							rayIntersects[s].intersectX,
							rayIntersects[s].intersectY,
							rayIntersects[s].intersectZ);*/
					}
				}
#endif //SAFETY
				free(nextBP);
			}
		}
////////////////////////////////////
		qsort( rayIntersects, countOfRays, sizeof(RAYINTERSECT_PARAMS), sortRayIntersects);
		output = fopen(name,"a+");
		output2 = fopen(name2,"a+");
		meanX = meanY = meanZ = 0;
		for(s=0; s<countOfRays; s++)
		{
			meanX = meanX + rayIntersects[s].intersectX;
			meanY = meanY + rayIntersects[s].intersectY;
			meanZ = meanZ + rayIntersects[s].intersectZ;
		}				
		meanX = meanX / countOfRays;
		meanY = meanY / countOfRays;
		meanZ = meanZ / countOfRays;
		stddevX = stddevY = stddevZ = 0;
		for(s=0; s<countOfRays; s++)
		{
			stddevX = stddevX + pow((rayIntersects[s].intersectX - meanX),2.0);
			stddevY = stddevY + pow((rayIntersects[s].intersectY - meanY),2.0);
			stddevZ = stddevZ + pow((rayIntersects[s].intersectZ - meanZ),2.0);
		}
		stddevX = sqrt(stddevX / countOfRays);
		stddevY = sqrt(stddevY / countOfRays);
		stddevZ = sqrt(stddevZ / countOfRays);

		for(s=0; s<countOfRays; s++)
		{
			fprintf(output2,"%f\t%f\t%f\t%f\t%d\t%d\t%d\t%d\t%f\t%f\t%f\t%f\n",
				rayIntersects[s].intersectX,
				rayIntersects[s].intersectY,
				rayIntersects[s].intersectZ,
				rayIntersects[s].intersectDistance,
				rayIntersects[s].blobpoint1ID,
				rayIntersects[s].blobpoint2ID, 
				rayIntersects[s].blob1ID, 
				rayIntersects[s].blob2ID, 
				currentPP->camera_x,
				currentPP->camera_y,
				currentPP->camera_z,
				rayIntersects[s].distanceFromCamera);
		}
		for(s=0; s<countOfRays; s++)
		{
			int temp, count, ctr1, ctr2;
			int rays;
			temp = ctr1 = ctr2 = s;
			count = 0; rays = 1;
			while( rayIntersects[temp].blobpoint1ID == rayIntersects[temp+1].blobpoint1ID )
			{
				if( rayIntersects[temp].blob2ID != rayIntersects[temp+1].blob2ID)
					rays++;
				temp++;
			}
			if( rays > 1 )
			{
				while( rayIntersects[ctr1].blobpoint2ID == rayIntersects[ctr1+1].blobpoint2ID )
					ctr1++;
				ctr2 = ctr1 + 1;
				desiredSecondPoint = rayIntersects[temp].intersectZ;
				rayIndexSecondPoint = temp;
				while( rayIntersects[ctr2].blobpoint2ID == rayIntersects[ctr2+1].blobpoint2ID )
				{
					if( desiredSecondPoint > rayIntersects[ctr2+1].intersectZ )
					{
						desiredSecondPoint = rayIntersects[ctr2+1].intersectZ;
						rayIndexSecondPoint = ctr2+1;
					}
					ctr2++;
				}
				if( rayIntersects[rayIndexSecondPoint].intersectZ >= 0.05 )
				{
/*					fprintf(output,"%f\t%f\t%f\t%f\t%d\t%d\t%d\t%d\t%f\t%f\t%f\t%f\n",
						rayIntersects[rayIndexSecondPoint].intersectX,
						rayIntersects[rayIndexSecondPoint].intersectY,
						rayIntersects[rayIndexSecondPoint].intersectZ,
						rayIntersects[rayIndexSecondPoint].intersectDistance,
						rayIntersects[rayIndexSecondPoint].blobpoint1ID,
						rayIntersects[rayIndexSecondPoint].blobpoint2ID, 
						rayIntersects[rayIndexSecondPoint].blob1ID, 
						rayIntersects[rayIndexSecondPoint].blob2ID, 
						currentPP->camera_x,
						currentPP->camera_y,
						currentPP->camera_z,
						rayIntersects[rayIndexSecondPoint].distanceFromCamera);*/
					fprintf(output,"%f\t%f\t%f\n",
						rayIntersects[rayIndexSecondPoint].intersectX,
						rayIntersects[rayIndexSecondPoint].intersectY,
						rayIntersects[rayIndexSecondPoint].intersectZ);
				}
			}
			s = temp+1;
		}
		fclose(output);
		fclose(output2);
		free(rayIntersects);
		rayIntersects = NULL;
////////////////////////////////////
		free(currentBP);
	}
	buildQHullFiles(pfNames, count);

	for(i=0; i<count; i++)
	{
		if( pfNames[i].name != NULL )
			free(pfNames[i].name);
	}
//	if( currentBP )
//		free(currentBP);
	free(pfNames);
}

int shootFoodEdgeRays(MATCHING_BLOBS *mb, int count)
{
	BLOBPOINT_PARAMS *blobPoints;
	BLOBPOINT_PARAMS *currentBP;
	BLOBPOINT_PARAMS *nextBP;
	POINT_3D pt1, pt2, pt3, pt4;
	RAYINTERSECT_PARAMS *rayIntersects;
//	INTERSECT_POINT_3D *ip3DTemp;

	PHOTOG_PARAMS *pp;
	int i,k;
	int m,n;
	int blobNumber;
	int totalProcessed;
	int countOfBlobPoints;
	int countOfCurrentBlobPoints;
	int countOfNextBlobPoints;

#ifdef VERBOSE_INSTRUMENTATION
	FILE *output; 
#endif
	FILE *geomview;
	char name[256];
	int j;
	i=0;
	totalProcessed = 0;

//	while(totalProcessed < count)
//	{
	if( count <=0 )
	{
		MessageBox(NULL,L"No Blobs to Shoot Points For",L"ERROR", MB_ICONHAND);
	}	
	for(i=0; i<count; i++)
	{
#ifdef VERBOSE_INSTRUMENTATION
		sprintf(name,"RaysAroundFood%04d%04d.txt",mb[i].foodItemID,mb[i].blob_id);
		output = fopen(name,"w+");
		if( output == NULL )
		{
			int err;
			char buffer[1024];
			wchar_t buff[1024];
			_get_errno( &err);
		
			sprintf(buffer, "Error open GeomFile in ShootFoodRays: %d - %s", err, strerror(err));
			mbstowcs(buff, buffer, strlen(buffer)+1);

			MessageBox(NULL, buff, L"ERROR", MB_ICONHAND);
		}
		
		fprintf(output,"CameraX\tCameraY\tCameraZ\tWorldX\tWorldY\tWorldZ\n");
#endif //VERBOSE_INSTRUMENTATION
		blobPoints = getBlobPointsForRays(mb[i].foodItemID, mb[i].blob_id);	
		if( blobPoints == NULL )
		{
			MessageBox(NULL, L"No blobPoints for ray generation", L"ERROR", MB_ICONHAND);
			return(-1);
		}

		countOfBlobPoints = get_current_blobpoint_count();		
		pp = getCameraForRays(mb[i].blob_id);
		if( pp==NULL )
		{
			MessageBox(NULL, L"No data from the Camera", L"ERROR", MB_ICONHAND);
		}

		for(j=0; j<countOfBlobPoints; j++)
		{
			pp->image_x = blobPoints[j].imageX;
			pp->image_y = blobPoints[j].imageY;
			
			createRay(pp);
#ifdef VERBOSE_INSTRUMENTATION
			k = fprintf(output,"%f\t%f\t%f\t%f\t%f\t%f\n",
				pp->camera_x, pp->camera_y, pp->camera_z,
				pp->world_x, pp->world_y, pp->world_z);
#endif //VERBOSE_INSTRUMENTATION
			blobPoints[j].cameraX = pp->camera_x;
			blobPoints[j].cameraY = pp->camera_y;
			blobPoints[j].cameraZ = pp->camera_z;
			blobPoints[j].worldX = pp->world_x;
			blobPoints[j].worldY = pp->world_y;
			blobPoints[j].worldZ = pp->world_z;
			update_blobpoint(mb[i].blob_id, pp->image_x, pp->image_y, 
						pp->world_x, 
						pp->world_y, pp->world_z, blobPoints[j].blobPointID);
		}
#ifdef VERBOSE_INSTRUMENTATION
		fclose(output);
#endif //VERBOSE_INSTRUMENTATION
///////////////////////////////////////////////////
#define VERBOSE_INSTRUMENTATION
///////////////////////////////////////////////////
#ifdef VERBOSE_INSTRUMENTATION
		sprintf(name,"RaysAroundFood%04d%04d.vect",mb[i].foodItemID,mb[i].blob_id);
		geomview = fopen(name,"w+");
		if( geomview == NULL )
		{
			int err;
			char buffer[1024];
			wchar_t buff[1024];
			_get_errno( &err);
			
			sprintf(buffer, "Error open GeomFile: %d - %s", err, strerror(err));
			mbstowcs(buff, buffer, strlen(buffer)+1);

			MessageBox(NULL, buff, L"ERROR", MB_ICONHAND);
		}
		fprintf(geomview,"VECT\n");
		fprintf(geomview,"%d %d %d\n\n", countOfBlobPoints, countOfBlobPoints*2, 1);
		for(k=0; k<countOfBlobPoints; k++)
		{
			fprintf(geomview,"2 ");
		}
		fprintf(geomview,"\n");
		for(k=0; k<countOfBlobPoints; k++)
		{
			if( k == 0 )
				fprintf(geomview,"1 ");
			else
				fprintf(geomview,"0 ");
		}
		fprintf(geomview,"\n");
		for(k=0; k<countOfBlobPoints; k++)
		{
			fprintf(geomview,"%f %f %f %f %f %f\n", 
			blobPoints[k].cameraX, 
			blobPoints[k].cameraY,
			blobPoints[k].cameraZ,
			blobPoints[k].worldX,
			blobPoints[k].worldY, 
			blobPoints[k].worldZ);
		}
		fprintf(geomview,"%f %f %f %f\n", 1.0, 0.0, 0.0, 0.0); 
		fclose(geomview);
#endif //VERBOSE_INSTRUMENTATION
#undef VERBOSE_INSTRUMENTATION
		free(blobPoints);
		free(pp);
		totalProcessed++;
	}
	/////////////////////////////////////////////////////
}
#define VERBOSE_INSTRUMENTATION

int matchFoodItems(int meal_id, int grouptype_id)
{
	FILE *output, *geomview;
	char name[MAX_PATH];
	PHOTOG_PARAMS *blobs;
	int count;
	int i,j;//k,m;
//	char buffer[2048];
//	wchar_t buff[2048];
	int oneBlobFlag;


	POINT_3D pt1, pt2, pt3, pt4;
	int countOfRayIntersects;
	INTERSECT_POINT_3D *ip3D;
	INTERSECT_POINT_3D *ip3DTemp;
	BLOB_DIRECTION *blobDir;
	int countDirs;
	MATCHING_BLOBS *matchingBLOBS;
	BLOBS_IN_IMAGE *blobsInImage;
	int last_blob_id;
	int imageCount;
	int foodItemID;
	int total_blobs;
	int groupTypeMealID;
	char buffer[2048];
	wchar_t buff[2048];
	
	int group_number = 0;
	int current_blob_number = 0;
//	FOOD_BLOB_GROUP *foodBlobGroup;
	imageCount = 0;
	countDirs = 0;
	ip3D = NULL;
	last_blob_id = 0;
	matchingBLOBS = NULL;
	blobsInImage = NULL;
	countOfRayIntersects = 0;

//	MessageBox(NULL,L"Before calling database",L"BLOB Centers",MB_OK);
	blobs = getBLOBCenters(meal_id, grouptype_id);
	count = get_meal_blob_count();
	if( count == 0 )
	{
		MessageBox(NULL, L"No Blobs in Database",L"ERROR", MB_ICONHAND);
		return -1;
	}
//	MessageBox(NULL,L"After calling database",L"BLOB Centers",MB_OK);

	for( i=0; i<count; i++)
	{
		groupTypeMealID = blobs[0].grouptype_meal_id;

		if( blobs[i].image_id != last_blob_id )
		{
			imageCount++;
			blobsInImage = (BLOBS_IN_IMAGE *)realloc(blobsInImage, sizeof(BLOBS_IN_IMAGE)*imageCount);
			blobsInImage[imageCount-1].image_id = blobs[i].image_id;
			blobsInImage[imageCount-1].count_of_blobs=1;
			last_blob_id = blobs[i].image_id;
		}
		else if (blobs[i].image_id == last_blob_id )
		{
			blobsInImage[imageCount-1].count_of_blobs++;
			last_blob_id = blobs[i].image_id;
		}
	}
	if( imageCount == 1 )
	{	
		MessageBox(NULL, L"Cannot work with one Image",L"Matching Food Items Error",MB_ICONHAND);
		free(blobsInImage);
		return -1;
	}
//	else
//		MessageBox(NULL,L"imageCount != 1", L"HERE", MB_OK);
	output = fopen("BlobCountsInImages.txt","a+");
	fprintf(output,"START\n");
	for(i=0; i<imageCount; i++)
	{
		fprintf(output,"Image: %d, CountOfBLOBS: %d\n",
			blobsInImage[i].image_id,
			blobsInImage[i].count_of_blobs);
		if( i != 0 )
		{
			if( blobsInImage[i].count_of_blobs != blobsInImage[i-1].count_of_blobs )
			{
				MessageBox(NULL, L"All foods not found in every image",L"Matching Food Items Error",MB_ICONHAND);
				free(blobsInImage);
				fclose(output);
				return -1;
			}
		}
	}
//	MessageBox(NULL,L"After the First For Loop",L"wow", MB_OK);

	total_blobs = imageCount * (blobsInImage[0].count_of_blobs);

	matchingBLOBS = malloc(sizeof(MATCHING_BLOBS)*total_blobs);

	blobDir = malloc(sizeof(BLOB_DIRECTION)*count);
	
	for( i=0; i<count; i++)
	{
		createRay(&blobs[i]);
		blobs[i].i = blobs[i].camera_x - blobs[i].world_x;
		blobs[i].j = blobs[i].camera_y - blobs[i].world_y;
		blobs[i].k = blobs[i].camera_z - blobs[i].world_z;
		matchingBLOBS[i].blob_id = blobs[i].blob_id;
		matchingBLOBS[i].group_number = 0;
	}
	for( i=0; i<count; i++)
	{
		float param;
		matchingBLOBS[i].blob_id = blobs[i].blob_id;
		blobDir[countDirs].new_z1 = 1.5f;		
		param = (blobDir[countDirs].new_z1 - blobs[i].camera_z)/(blobs[i].k);
		matchingBLOBS[i].x = blobs[i].camera_x + (blobs[i].i * param);
		matchingBLOBS[i].y = blobs[i].camera_y + (blobs[i].j * param);
		matchingBLOBS[i].group_number = 0;
		for(j=i; j<count; j++)
		{
			float parameter;

			if( blobs[i].image_id == blobs[j].image_id && i != j)
			{
				blobDir[countDirs].blob1_id = blobs[i].blob_id;
				blobDir[countDirs].blob2_id = blobs[j].blob_id;
				blobDir[countDirs].image_id1 = blobs[i].image_id;
				blobDir[countDirs].image_id2 = blobs[j].image_id;

				matchingBLOBS[j].blob_id = blobs[j].blob_id;

				blobDir[countDirs].new_z1 = 1.5f;
				blobDir[countDirs].new_z2 = 1.5f;

				parameter = (blobDir[countDirs].new_z2 - blobs[j].camera_z)/(blobs[j].k);
				blobDir[countDirs].new_x2 = blobs[j].camera_x + (blobs[j].i * parameter);
				blobDir[countDirs].new_y2 = blobs[j].camera_y + (blobs[j].j * parameter);

				parameter = (blobDir[countDirs].new_z1 - blobs[i].camera_z)/(blobs[i].k);
				blobDir[countDirs].new_x1 = blobs[i].camera_x + (blobs[i].i * parameter);
				blobDir[countDirs].new_y1 = blobs[i].camera_y + (blobs[i].j * parameter);

				matchingBLOBS[j].x = blobDir[countDirs].new_x2;
				matchingBLOBS[j].y = blobDir[countDirs].new_y2;

				blobDir[countDirs].i_vector = blobDir[countDirs].new_x1 - blobDir[countDirs].new_x2;
				blobDir[countDirs].j_vector = blobDir[countDirs].new_y1 - blobDir[countDirs].new_y2;

				blobDir[countDirs].angle1to2 = atan2(blobDir[countDirs].j_vector, blobDir[countDirs].i_vector);

//				matchingBLOBS[i].angle1 = blobDir[countDirs].angle1to2;
				if( blobDir[countDirs].angle1to2 < 0 )
				{
					blobDir[countDirs].angle2to1 = blobDir[countDirs].angle1to2 + PI;
				}
				else
				{
					blobDir[countDirs].angle2to1 = blobDir[countDirs].angle1to2 - PI;
				}
//				matchingBLOBS[j].angle2 = blobDir[countDirs].angle2to1;

				countDirs++;
			}
		}
	}
	group_number = 0;
	foodItemID = 0;
//MessageBox(NULL,L"Checking matchingBlobs",L"wow", MB_OK);
	oneBlobFlag = 1;
	for(i=0; i<imageCount; i++)
	{
		if( blobsInImage[i].count_of_blobs != 1 )
		{
			oneBlobFlag = 0;
		}
	}

	if( oneBlobFlag  == 0 )
	{
		for( i=0; i<count; i++)
		{
			if( matchingBLOBS[i].group_number == 0 )
			{
				group_number++;
				matchingBLOBS[i].group_number = group_number;
				foodItemID = createFoodItem(groupTypeMealID, group_number);
				matchingBLOBS[i].foodItemID = foodItemID;
				updateBlobToFood(foodItemID, matchingBLOBS[i].blob_id);
			}
			for(j=i; j<count; j++)
			{
				if( i != j && matchingBLOBS[j].group_number == 0 )
				{
					if( matchingBLOBS[i].x + 3.0 >= matchingBLOBS[j].x &&
						matchingBLOBS[i].x - 3.0 <= matchingBLOBS[j].x &&
						matchingBLOBS[i].y + 3.0 >= matchingBLOBS[j].y &&
						matchingBLOBS[i].y - 3.0 <= matchingBLOBS[j].y)
					{
						matchingBLOBS[j].group_number = group_number;
						matchingBLOBS[j].foodItemID = foodItemID;
						updateBlobToFood(foodItemID, matchingBLOBS[j].blob_id);
					}
				}
			}
		}
	}
	else if( oneBlobFlag == 1 )
	{
		i=0;
		matchingBLOBS[i].group_number = group_number;
		foodItemID = createFoodItem(groupTypeMealID, group_number);
		matchingBLOBS[i].foodItemID = foodItemID;
		updateBlobToFood(foodItemID, matchingBLOBS[i].blob_id);
		for( i=1; i<count; i++)
		{
			matchingBLOBS[i].group_number = group_number;
			matchingBLOBS[i].foodItemID = foodItemID;
			updateBlobToFood(foodItemID, matchingBLOBS[i].blob_id);
		}
	}
#ifdef DUMMY
	group_number = 0;
	foodBlobGroup = (FOOD_BLOB_GROUP *)malloc(sizeof(FOOD_BLOB_GROUP)*blobsInImage[0].count_of_blobs);
	memset(foodBlobGroup, 0, sizeof(foodBlobGroup));

	for( i=0; i<count; i++)
	{
		if( group_number == 0 )
		{
			group_number++;
			foodBlobGroup[i].foodItemID = create_food_item(group_number);
			update_BlobToFood(foodBlobGroup[i].foodItemID, matchingBLOBS[i].blob_id);
		}
		for(j=i; j<count; j++)
		{
			if( i != j and matchingBLOBS[j].group_number == group_number )
			{
				update_BlobToFood(foodBlobGroup[i].foodItemID, matchingBLOBS[j].blob_id);
			}
		}
	}
#endif //DUMMY
#ifdef VERBOSE_INSTRUMENTATION
	sprintf(buffer,"M%d-GT%d-MatchingBlobs.txt", meal_id, grouptype_id);
	output = fopen(buffer,"w+");
	fprintf(output,"BLOB ID\tGroup Number\tX\tY\tFoodItemID\n");
	for( i=0; i<count; i++)
	{
		fprintf(output,"%d\t%d\t%f\t%f\t%d\n", matchingBLOBS[i].blob_id, matchingBLOBS[i].group_number,
			matchingBLOBS[i].x, matchingBLOBS[i].y,
			matchingBLOBS[i].foodItemID);
	}
	fclose(output);
#endif //VERBOSE_INSTRUMENTATION
#ifdef VERBOSE_INSTRUMENTATION
	sprintf(name,"%s-camera2blobs.vect", get_filename_base());
	//sprintf(name,"e:\\temp\\camera2blobs.vect"); 
	geomview = fopen(name,"w+");
	if( geomview == NULL )
	{
		int err;
		_get_errno( &err);
		
		sprintf(buffer, "Error open GeomFile: %d - %s", err, strerror(err));
		mbstowcs(buff, buffer, strlen(buffer)+1);

		MessageBox(NULL, buff, L"ERROR", MB_ICONHAND);

	}
	fprintf(geomview,"VECT\n");
	fprintf(geomview,"%d %d %d\n\n", total_blobs, total_blobs*imageCount*2, imageCount);
	for(i=0; i<total_blobs; i++)
	{
		fprintf(geomview,"2 ");
	}
	fprintf(geomview,"\n");
	for(i=0; i<total_blobs*imageCount; i++)
	{
		if( i+1 % 3 ==  1 )
			fprintf(geomview,"1 ");
		else
			fprintf(geomview,"0 ");
	}
	fprintf(geomview,"\n");

	for(i=0; i<total_blobs*imageCount; i++)
	{
		fprintf(geomview,"%f %f %f %f %f %f\n", 
			blobs[i].camera_x, blobs[i].camera_y, blobs[i].camera_z,
			blobs[i].world_x, blobs[i].world_y, blobs[i].world_z);
	}
	for(i=0; i<imageCount; i++)
	{
		if( i%3 == 0 )
			fprintf(geomview,"%f %f %f %f\n", 
				1.0,0.0,0.0, 1.0);
		else if( i%3 == 1 )
			fprintf(geomview,"%f %f %f %f\n", 
				0.0,1.0,0.0, 1.0);
		else if( i%3 == 2 )
			fprintf(geomview,"%f %f %f %f\n", 
				0.0,0.0,1.0, 1.0);

	}
	fclose(geomview);
	
	for(i=0; i<total_blobs; i++)
	{
		blobs[i].i = blobs[i].camera_x - blobs[i].world_x;
		blobs[i].j = blobs[i].camera_y - blobs[i].world_y;
		blobs[i].k = blobs[i].camera_z - blobs[i].world_z;
	}
#endif //VERBOSE_INSTRUMENTATION


#ifdef VERBOSE_INSTRUMENTATION
	output = fopen("AnglesBetweenBlobs.txt","w+");
	fprintf(output,"IMAGE 1\tBLOB 1\tB1 X\tB1 Y\tIMAGE 2\tBLOB 2\tB2 X\tB2 Y\tANGLE1to2\tANGLE2to1\n");
	for(i=0; i<countDirs; i++)
	{
		fprintf(output,"%d\t%d\t%f\t%f\t%d\t%d\t%f\t%f\t%f\t%f\n",
			blobDir[i].image_id1, blobDir[i].blob1_id, blobDir[i].new_x1, blobDir[i].new_y1, 
			blobDir[i].image_id2, blobDir[i].blob2_id, blobDir[i].new_x2, blobDir[i].new_y2, 
			blobDir[i].angle1to2, blobDir[i].angle2to1);
	}
	fclose(output);

	output = fopen("BLOB RAYS.txt","w+");
	for( i=0; i<count; i++)
	{
		fprintf(output, "%f\t%f\t%f\t%f\t%f\t%f\n",
			blobs[i].camera_x, blobs[i].camera_y, blobs[i].camera_z,
			blobs[i].world_x, blobs[i].world_y, blobs[i].world_z);
	}
	fclose(output);
#endif //VERBOSE_INSTRUMENTATION
#ifdef VERBOSE_INSTRUMENTATION
	for(i=0; i<count; i++)
	{
		for(j=i+1; j<count; j++)
		{
			if( blobs[i].image_id != blobs[j].image_id )
			{
				pt1.x = blobs[i].camera_x;
				pt1.y = blobs[i].camera_y;
				pt1.z = blobs[i].camera_z;
				pt2.x = blobs[i].world_x;
				pt2.y = blobs[i].world_y;
				pt2.z = blobs[i].world_z;

				pt3.x = blobs[j].camera_x;
				pt3.y = blobs[j].camera_y;
				pt3.z = blobs[j].camera_z;
				pt4.x = blobs[j].world_x;
				pt4.y = blobs[j].world_y;
				pt4.z = blobs[j].world_z;
				ip3DTemp = closest_point_between_2_lines(&pt1, &pt2, &pt3, &pt4);
				countOfRayIntersects++;
				ip3D = (INTERSECT_POINT_3D *)realloc(ip3D, 	countOfRayIntersects*sizeof(INTERSECT_POINT_3D));
				ip3D[countOfRayIntersects-1].ray1_id = i;
				ip3D[countOfRayIntersects-1].ray2_id = j;
				ip3D[countOfRayIntersects-1].x = ip3DTemp->x;
				ip3D[countOfRayIntersects-1].y = ip3DTemp->y;
				ip3D[countOfRayIntersects-1].z = ip3DTemp->z;
				ip3D[countOfRayIntersects-1].distance = ip3DTemp->distance;
				free(ip3DTemp);
			}
		}
	}

	output = fopen("Intersects.txt","w+");
	fprintf(output,"%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n",
		"BLOB 1", "BLOB 2", "Ray 1 Cam X", "Ray 1 Cam Y", "Ray 1 Cam Z",
		"Ray 1 World X", "Ray 1 World Y", "Ray 1 World Z",
		"Ray 2 Cam X", "Ray 2 Cam Y", "Ray 2 Cam Z",
		"Ray 2 World X", "Ray 2 World Y", "Ray 2 World Z",
		"Intersect X", "Intersect Y", "Intersect Z", "Missed Distance");

	for( i=0; i<countOfRayIntersects; i++)
	{
		fprintf(output, "Image(%d)Blob(%d)\tImage(%d)Blob(%d)\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\n",
			blobs[ip3D[i].ray1_id].image_id,blobs[ip3D[i].ray1_id].blob_id,
			blobs[ip3D[i].ray2_id].image_id,blobs[ip3D[i].ray2_id].blob_id,

			blobs[ip3D[i].ray1_id].camera_x, blobs[ip3D[i].ray1_id].camera_y, blobs[ip3D[i].ray1_id].camera_z,
			blobs[ip3D[i].ray1_id].world_x, blobs[ip3D[i].ray1_id].world_y, blobs[ip3D[i].ray1_id].world_z,
			blobs[ip3D[i].ray2_id].camera_x, blobs[ip3D[i].ray2_id].camera_y, blobs[ip3D[i].ray2_id].camera_z,
			blobs[ip3D[i].ray2_id].world_x, blobs[ip3D[i].ray2_id].world_y, blobs[ip3D[i].ray2_id].world_z,
			ip3D[i].x, ip3D[i].y, ip3D[i].z, ip3D[i].distance); 
	}
	fclose(output);
#endif //VERBOSE_INSTRUMENTATION
	free(ip3D);
	free(blobs);
	free(blobDir);
	shootFoodEdgeRays(matchingBLOBS, count);
	calculateFoodVolumePoints(matchingBLOBS, count);
	free(matchingBLOBS);
}

POINT_3D *createRayPoint(PHOTOG_PARAMS *pp)
{
	POINT_3D *p;

	CvMat *Rinv;
	CvMat *R;
	CvMat *W;
	CvMat *T;
	CvMat *J;
	CvMat *uv;
	CvMat *co;

	CV_FUNCNAME( "calculateImagePoint" );
	cvSetErrMode( CV_ErrModeSilent );

	__BEGIN__;

	p = malloc(sizeof(POINT_3D));

	R = cvCreateMat(3,3, CV_32FC1);
	T = cvCreateMat(3,1, CV_32FC1);
	co = cvCreateMat(3,1, CV_32FC1);
	J = cvCreateMat(3,1, CV_32FC1);
	W = cvCreateMat(3,1, CV_32FC1);
	uv = cvCreateMat(3,1, CV_32FC1);
	Rinv = cvCreateMat(3,3, CV_32FC1);

	CV_MAT_ELEM(*T, float, 0,0) = pp->t1;
	CV_MAT_ELEM(*T, float, 1,0) = pp->t2;
	CV_MAT_ELEM(*T, float, 2,0) = pp->t3;

	CV_MAT_ELEM(*R, float, 0,0) = pp->r11;
	CV_MAT_ELEM(*R, float, 0,1) = pp->r12;
	CV_MAT_ELEM(*R, float, 0,2) = pp->r13;
	CV_MAT_ELEM(*R, float, 1,0) = pp->r21;
	CV_MAT_ELEM(*R, float, 1,1) = pp->r22;
	CV_MAT_ELEM(*R, float, 1,2) = pp->r23;
	CV_MAT_ELEM(*R, float, 2,0) = pp->r31;
	CV_MAT_ELEM(*R, float, 2,1) = pp->r32;
	CV_MAT_ELEM(*R, float, 2,2) = pp->r33;

	CV_MAT_ELEM(*uv, float, 0,0) = (float)((pp->image_x - pp->cx)/pp->fx)*36.0f;
	CV_MAT_ELEM(*uv, float, 1,0) = (float)((pp->image_y - pp->cy)/pp->fy)*36.0f;
	CV_MAT_ELEM(*uv, float, 2,0) = 1.0f*36.0f;

	cvInv(R,Rinv, 0);
	cvSave("Rinvmatrix.xml", Rinv,"Rinv-MATRIX", "The Rinv Matrix", cvAttrList(0,0));

	cvMatMul(Rinv, T, co);
	cvSave("comatrix.xml", co,"co-MATRIX", "The co Matrix", cvAttrList(0,0));
	cvSave("Tmatrix.xml", T,"T-MATRIX", "The T Matrix", cvAttrList(0,0));

	cvSub(uv, T, J, 0);
	cvSave("Jmatrix.xml", J,"J-MATRIX", "The J Matrix", cvAttrList(0,0));
	cvSave("uvmatrix.xml", uv,"uv-MATRIX", "The uv Matrix", cvAttrList(0,0));	
	cvMatMul(Rinv, J, W);
	cvSave("Worldmatrix.xml", W,"World-Point", "World-Point", cvAttrList(0,0));

	p->x = CV_MAT_ELEM(*W, float, 0,0);
	p->y = CV_MAT_ELEM(*W, float, 1,0);
	p->z = CV_MAT_ELEM(*W, float, 2,0);

	cvReleaseMat(&R);
	cvReleaseMat(&T);
	cvReleaseMat(&W);
	cvReleaseMat(&J);
	cvReleaseMat(&Rinv);
	cvReleaseMat(&uv);
	cvReleaseMat(&co);
	
	__END__;
	return(p);
}

int createRays(int photoblob_id)
{
	CvMat *camera;
	CvMat *Rt;
	CvMat *Rtinv;
	CvMat *Q;
	CvMat *Qinv;
	CvMat *arr;
	CvMat *A;
	CvMat *R;
	CvMat *Rinv;
	CvMat *Rtrans;
	CvMat *U;
	CvMat *W;
	CvMat *T;
	CvMat *M;
	CvMat *J;
	CvMat *img;
	double det;
//	float x,y,z;
	CvMat *I;

	CvMat *camerainv;
	CvMat *C;
	CvMat *K;

	CV_FUNCNAME( "createRays" );
	cvSetErrMode( CV_ErrModeSilent );

	__BEGIN__;

	Rt = cvCreateMat(3, 4, CV_32FC1);
	Rtinv = cvCreateMat(3, 4, CV_32FC1);

	camera = cvCreateMat(3, 3, CV_32FC1);
	camerainv = cvCreateMat(3, 3, CV_32FC1);
	Q = cvCreateMat(3,4, CV_32FC1);
	Qinv = cvCreateMat(4, 3, CV_32FC1);
	arr = cvCreateMat(3,1, CV_32FC1);
	A = cvCreateMat(4, 4, CV_32FC1);
	R = cvCreateMat(3,3, CV_32FC1);
	Rinv = cvCreateMat(3,3, CV_32FC1);
	Rtrans = cvCreateMat(3,3, CV_32FC1);
	U = cvCreateMat(4,1, CV_32FC1);
	W = cvCreateMat(4,1, CV_32FC1);
	M = cvCreateMat(3,3, CV_32FC1);
	T = cvCreateMat(3,1, CV_32FC1);
	J = cvCreateMat(3,1, CV_32FC1);
	C = cvCreateMat(3,1, CV_32FC1);
	img = cvCreateMat(3,1, CV_32FC1);
	K = cvCreateMat(3,1, CV_32FC1);
	I = cvCreateMat(3,1, CV_32FC1);

	CV_MAT_ELEM(*Rt, float, 0,0) = -0.968324f;
	CV_MAT_ELEM(*Rt, float, 0,1) = 0.006806f;
	CV_MAT_ELEM(*Rt, float, 0,2) = 0.249604f;
	CV_MAT_ELEM(*Rt, float, 0,3) = 10.55015f;
	CV_MAT_ELEM(*Rt, float, 1,0) = 0.011771f;
	CV_MAT_ELEM(*Rt, float, 1,1) = 0.999761f;
	CV_MAT_ELEM(*Rt, float, 1,2) = 0.018405f;
	CV_MAT_ELEM(*Rt, float, 1,3) = -5.346645f;
	CV_MAT_ELEM(*Rt, float, 2,0) = -0.249419f;
	CV_MAT_ELEM(*Rt, float, 2,1) = 0.02076f;
	CV_MAT_ELEM(*Rt, float, 2,2) = -0.968173f;
	CV_MAT_ELEM(*Rt, float, 2,3) = 99.40504f;

	CV_MAT_ELEM(*T, float, 0,0) = 10.55015f;
	CV_MAT_ELEM(*T, float, 1,0) = -5.346645f;
	CV_MAT_ELEM(*T, float, 2,0) = 99.40504f;

	CV_MAT_ELEM(*R, float, 0,0) = -0.968324f;
	CV_MAT_ELEM(*R, float, 0,1) = 0.006806f;
	CV_MAT_ELEM(*R, float, 0,2) = 0.249604f;
	CV_MAT_ELEM(*R, float, 1,0) = 0.011771f;
	CV_MAT_ELEM(*R, float, 1,1) = 0.999761f;
	CV_MAT_ELEM(*R, float, 1,2) = 0.018405f;
	CV_MAT_ELEM(*R, float, 2,0) = -0.249419f;
	CV_MAT_ELEM(*R, float, 2,1) = 0.02076f;
	CV_MAT_ELEM(*R, float, 2,2) = -0.968173f;

	CV_MAT_ELEM(*camera, float, 0,0) = 6921.581f;
	CV_MAT_ELEM(*camera, float, 0,1) = 0.0f;
	CV_MAT_ELEM(*camera, float, 0,2) = 0.0f;
	CV_MAT_ELEM(*camera, float, 1,0) = 0.0f;
	CV_MAT_ELEM(*camera, float, 1,1) = 6806.268f;
	CV_MAT_ELEM(*camera, float, 1,2) = 0.0f;
	CV_MAT_ELEM(*camera, float, 2,0) = 780.4959f;
	CV_MAT_ELEM(*camera, float, 2,1) = 581.5905f;
	CV_MAT_ELEM(*camera, float, 2,2) = 1.0;
	
	CV_MAT_ELEM(*arr, float, 0,0) = 0.0;
	CV_MAT_ELEM(*arr, float, 1,0) = 0.0;
	CV_MAT_ELEM(*arr, float, 2,0) = 0.0;
//	CV_MAT_ELEM(*arr, float, 3,0) = 1.0;


	CV_MAT_ELEM(*J, float, 0,0) = 1537.0;
	CV_MAT_ELEM(*J, float, 1,0) = 1065.0;
	CV_MAT_ELEM(*J, float, 2,0) = 1.0;

	CV_CALL(cvMatMul(camera, Rt, Q));
	cvSave("Qmatrix.xml", Q,"Q-MATRIX", "The Q Matrix", cvAttrList(0,0));
	cvSave("Rmatrix.xml", R,"R-MATRIX", "The R Matrix", cvAttrList(0,0));
	
	CV_CALL(cvInvert(Q, Qinv, CV_SVD));
	cvSave("Qinvmatrix.xml", Qinv,"Qinv-MATRIX", "The Qinv Matrix", cvAttrList(0,0));

	CV_CALL(cvInvert(R, Rinv, CV_SVD));
	cvSave("Rinvmatrix.xml", Rinv,"Rinv-MATRIX", "The Rinv Matrix", cvAttrList(0,0));

	CV_CALL(cvInvert(camera, camerainv, CV_SVD));
	cvSave("camerainvmatrix.xml", camerainv,"Camerainv-MATRIX", "The Camerainv Matrix", cvAttrList(0,0));

	CV_CALL(cvTranspose(R, Rtrans));
	cvSave("Rtransmatrix.xml", Rtrans,"Rtrans-MATRIX", "The Rtrans Matrix", cvAttrList(0,0));

	CV_CALL(cvMatMul(Qinv, arr, U));
	cvSave("Umatrix.xml", U,"U-MATRIX", "The U Matrix", cvAttrList(0,0));

	CV_CALL(cvMatMul(Qinv, J, W));
	cvSave("Wmatrix.xml", W,"W-MATRIX", "The W Matrix", cvAttrList(0,0));
///////////////////////////////////////
	CV_CALL(cvMatMul(Rinv, camerainv, M));
	cvSave("Mmatrix.xml", M,"M-MATRIX", "The M Matrix", cvAttrList(0,0));

	CV_CALL(cvMatMul(M, J, K));
	cvSave("Wmatrix.xml", W,"W-MATRIX", "The W Matrix", cvAttrList(0,0));

	CV_CALL(cvAdd(T, K, C,0));
	cvSave("Cmatrix.xml", C,"C-MATRIX", "The C Matrix", cvAttrList(0,0));
	
//	CV_CALL(cvAdd(W, T, J,0));
//	cvSave("Jmatrix.xml", J,"J-MATRIX", "The J Matrix", cvAttrList(0,0));

	//	CV_CALL(cvMatMul(Q, arr,  A));
//	cvSave("A.xml", A,"A-MATRIX", "The A Matrix", cvAttrList(0,0));
	CV_CALL(cvMatMul(Rinv, camerainv, M));
	cvSave("Mmatrix.xml", M,"M-MATRIX", "The M Matrix", cvAttrList(0,0));

	CV_CALL(cvMatMul(M, J, K));
	cvSave("Kmatrix.xml", K,"K-MATRIX", "The K Matrix", cvAttrList(0,0));
//////////////////////////////////////////
	


/////////////////////////////////////



	__END__;

	CV_MAT_ELEM(*camera, float, 0,0) = 76.01f;
	CV_MAT_ELEM(*camera, float, 0,1) = -10.66f;
	CV_MAT_ELEM(*camera, float, 0,2) = 354.4f;
	CV_MAT_ELEM(*camera, float, 1,0) = 65.46f;
	CV_MAT_ELEM(*camera, float, 1,1) = -5.31f;
	CV_MAT_ELEM(*camera, float, 1,2) = 255.0f;
	CV_MAT_ELEM(*camera, float, 2,0) = 0.0f;
	CV_MAT_ELEM(*camera, float, 2,1) = 12.25f;
	CV_MAT_ELEM(*camera, float, 2,2) = 0.0f;

	det = cvDet(camera);

	cvReleaseMat(&camera);
	cvReleaseMat(&Rt);
	cvReleaseMat(&Q);
	cvReleaseMat(&Qinv);

	return 0;
}

IMAGE_2D *calculateImagePoint(int monumentPointID)
{
	PHOTOG_PARAMS *pp;

	float xp, yp, x, y;
	float xpp, ypp;
	float r;
	float u;
	float v;
	IMAGE_2D *p;

//	CvMat *camera;
//	CvMat *Rt;
	CvMat *Rinv;
//	CvMat *Q;
//	CvMat *Qinv;
//	CvMat *arr;
//	CvMat *A;
	CvMat *R;

//	CvMat *Rtrans;
//	CvMat *U;
	CvMat *W;
	CvMat *T;
//	CvMat *M;
	CvMat *J;
	CvMat *P;
	CvMat *img;
	CvMat *I;
	char buffer[1024];
	wchar_t buff[1024];

	CV_FUNCNAME( "calculateImagePoint" );
	cvSetErrMode( CV_ErrModeSilent );
	pp = getPhotogParams(monumentPointID);

	__BEGIN__;

	p = malloc(sizeof(IMAGE_2D));

	R = cvCreateMat(3,3, CV_32FC1);
	T = cvCreateMat(3,1, CV_32FC1);
	J = cvCreateMat(3,1, CV_32FC1);
	P = cvCreateMat(3,1, CV_32FC1);
	W = cvCreateMat(3,1, CV_32FC1);
	I = cvCreateMat(3,1, CV_32FC1);
	img = cvCreateMat(3,1, CV_32FC1);
	Rinv = cvCreateMat(3,3, CV_32FC1);

	CV_MAT_ELEM(*T, float, 0,0) = pp->t1;
	CV_MAT_ELEM(*T, float, 1,0) = pp->t2;
	CV_MAT_ELEM(*T, float, 2,0) = pp->t3;
	cvSave("Tmatrix.xml", T,"T-MATRIX", "The T Matrix", cvAttrList(0,0));

	CV_MAT_ELEM(*R, float, 0,0) = pp->r11;
	CV_MAT_ELEM(*R, float, 0,1) = pp->r12;
	CV_MAT_ELEM(*R, float, 0,2) = pp->r13;
	CV_MAT_ELEM(*R, float, 1,0) = pp->r21;
	CV_MAT_ELEM(*R, float, 1,1) = pp->r22;
	CV_MAT_ELEM(*R, float, 1,2) = pp->r23;
	CV_MAT_ELEM(*R, float, 2,0) = pp->r31;
	CV_MAT_ELEM(*R, float, 2,1) = pp->r32;
	CV_MAT_ELEM(*R, float, 2,2) = pp->r33;
	cvSave("Rmatrix.xml", R,"R-MATRIX", "The R Matrix", cvAttrList(0,0));

	CV_MAT_ELEM(*P, float, 0,0) = pp->world_x;
	CV_MAT_ELEM(*P, float, 1,0) = pp->world_y;
	CV_MAT_ELEM(*P, float, 2,0) = pp->world_z;
	cvSave("Pmatrix.xml", P,"P-MATRIX", "The P Matrix", cvAttrList(0,0));
	
	CV_CALL(cvMatMul(R, P, J));
	cvSave("Jmatrix.xml", J,"J-MATRIX", "The J Matrix", cvAttrList(0,0));
	
	CV_CALL(cvAdd(J, T, W,0));
	cvSave("Wmatrix.xml", W,"W-MATRIX", "The W Matrix", cvAttrList(0,0));

	xp = CV_MAT_ELEM(*W, float, 0,0)/CV_MAT_ELEM(*W, float, 2,0);
	yp = CV_MAT_ELEM(*W, float, 1,0)/CV_MAT_ELEM(*W, float, 2,0);
	
	r = xp*xp + yp*yp;

	xpp = xp * (1.0f + (pp->k1*r) + (pp->k2*r*r) + (pp->k3*r*r*r)) + (2.0f*pp->p1*xp*yp) + (pp->p2*(r*r + 2.0f*xp*xp));
	ypp = yp * (1.0f + (pp->k1*r) + (pp->k2*r*r) + (pp->k3*r*r*r)) + (pp->p1*(r+2.0f*yp*yp)) + (2.0f*pp->p2*xp*yp);

	u = pp->fx*xpp + pp->cx;
	v = pp->fy*ypp + pp->cy;

	sprintf(buffer, "Stored Point: (%d, %d)\nCalculated Point: (%f, %f)\nMonumentPoint ID: %d",
		pp->image_x, pp->image_y, u, v, monumentPointID);
	mbstowcs(buff, buffer, strlen(buffer)+1);

	MessageBox(NULL, buff, L"ReProject Calculation", MB_OK);
return p;
	
	p->x = (int)xpp; p->y = (int)ypp;
///////////////////////////////////////////
	cvInv(R,Rinv, 0);
	cvSave("Rinvmatrix.xml", Rinv,"Rinv-MATRIX", "The Rinv Matrix", cvAttrList(0,0));

	CV_MAT_ELEM(*I, float, 0,0) = (float)(pp->image_x);
	cvSave("Imatrix.xml", I,"I-MATRIX", "The I Matrix", cvAttrList(0,0));

	xp = (1537 - pp->cx)/pp->fx;
	yp = (1065 - pp->cy)/pp->fy;

	x = xp;
	y = yp;

	CV_MAT_ELEM(*img, float, 0,0) = x;
	CV_MAT_ELEM(*img, float, 1,0) = y;
	CV_MAT_ELEM(*img, float, 2,0) = 1.0;

	cvMatMul(Rinv, img, W);
	cvSave("W2matrix.xml", W,"W-MATRIX", "The W Matrix", cvAttrList(0,0));

	cvSub(W, T, P,0);
	cvSave("P2matrix.xml", P,"P-MATRIX", "The P Matrix", cvAttrList(0,0));
	cvSave("T2matrix.xml", T,"T-MATRIX", "The T Matrix", cvAttrList(0,0));

//	sprintf(buffer, "3D Point: (%f, %f, %f)",
//		CV_MAT_ELEM(*P, float, 0,0), CV_MAT_ELEM(*P, float, 1,0), CV_MAT_ELEM(*P, float, 2,0));
//	mbstowcs(buff, buffer, strlen(buffer)+1);

//	MessageBox(NULL, buff, L"3D point from Mon Point 14306", MB_OK);



///////////////////////////////////////////
	free(pp);

	cvReleaseMat(&R);
	cvReleaseMat(&T);
	cvReleaseMat(&W);
	cvReleaseMat(&P);
	cvReleaseMat(&J);
	__END__;

	return(p);

}

int findFoodbyRays(int meal_id)
{
	return 0;
}

IMAGE_2D *validatePhotoG(int monumentPointID)
{
	PHOTOG_PARAMS *pp;
	float det;
	IMAGE_2D *p;

	CvMat *Rinv;
	CvMat *R;
	CvMat *W;
	CvMat *T;
	CvMat *J;
	CvMat *test;
	CvMat *uv;
	CvMat *co;

	char buffer[1024];
	wchar_t buff[1024];

	CV_FUNCNAME( "calculateImagePoint" );
	cvSetErrMode( CV_ErrModeSilent );
	pp = getPhotogParams(monumentPointID);

	__BEGIN__;

	p = malloc(sizeof(IMAGE_2D));

	R = cvCreateMat(3,3, CV_32FC1);
	T = cvCreateMat(3,1, CV_32FC1);
	co = cvCreateMat(3,1, CV_32FC1);
	J = cvCreateMat(3,1, CV_32FC1);
	W = cvCreateMat(3,1, CV_32FC1);
	uv = cvCreateMat(3,1, CV_32FC1);
	test = cvCreateMat(3,3, CV_32FC1);
	Rinv = cvCreateMat(3,3, CV_32FC1);

	CV_MAT_ELEM(*T, float, 0,0) = pp->t1;
	CV_MAT_ELEM(*T, float, 1,0) = pp->t2;
	CV_MAT_ELEM(*T, float, 2,0) = pp->t3;

	CV_MAT_ELEM(*R, float, 0,0) = pp->r11;
	CV_MAT_ELEM(*R, float, 0,1) = pp->r12;
	CV_MAT_ELEM(*R, float, 0,2) = pp->r13;
	CV_MAT_ELEM(*R, float, 1,0) = pp->r21;
	CV_MAT_ELEM(*R, float, 1,1) = pp->r22;
	CV_MAT_ELEM(*R, float, 1,2) = pp->r23;
	CV_MAT_ELEM(*R, float, 2,0) = pp->r31;
	CV_MAT_ELEM(*R, float, 2,1) = pp->r32;
	CV_MAT_ELEM(*R, float, 2,2) = pp->r33;

	CV_MAT_ELEM(*uv, float, 0,0) = (float)((pp->image_x - pp->cx)/pp->fx)*26.948436f;
	CV_MAT_ELEM(*uv, float, 1,0) = (float)((pp->image_y - pp->cy)/pp->fy)*26.948436f;
	CV_MAT_ELEM(*uv, float, 2,0) = 1.0f*26.948436f;

	cvInv(R,Rinv, 0);
	cvSave("Rinvmatrix.xml", Rinv,"Rinv-MATRIX", "The Rinv Matrix", cvAttrList(0,0));

	cvMatMul(Rinv, T, co);
	cvSave("comatrix.xml", co,"co-MATRIX", "The co Matrix", cvAttrList(0,0));
	cvSave("Tmatrix.xml", T,"T-MATRIX", "The T Matrix", cvAttrList(0,0));

	cvSub(uv, T, J, 0);
	cvSave("Jmatrix.xml", J,"J-MATRIX", "The J Matrix", cvAttrList(0,0));
	cvSave("uvmatrix.xml", uv,"uv-MATRIX", "The uv Matrix", cvAttrList(0,0));	
	cvMatMul(Rinv, J, W);
	cvSave("Worldmatrix.xml", W,"World-Point", "World-Point", cvAttrList(0,0));

	CV_MAT_ELEM(*test, float, 0,0) = 	/*pp->t1;*/1.0f*CV_MAT_ELEM(*co, float, 0,0);
	CV_MAT_ELEM(*test, float, 0,1) = 	/*pp->t2;*/1.0f*CV_MAT_ELEM(*co, float, 1,0);
	CV_MAT_ELEM(*test, float, 0,2) = 	/*pp->t3;*/1.0f*CV_MAT_ELEM(*co, float, 2,0);
	
	CV_MAT_ELEM(*test, float, 1,0) = 	pp->world_x;
	CV_MAT_ELEM(*test, float, 1,1) = 	pp->world_y;
	CV_MAT_ELEM(*test, float, 1,2) = 	pp->world_z;

	CV_MAT_ELEM(*test, float, 2,0) = 	CV_MAT_ELEM(*W, float, 0,0);
	CV_MAT_ELEM(*test, float, 2,1) = 	CV_MAT_ELEM(*W, float, 1,0);
	CV_MAT_ELEM(*test, float, 2,2) = 	CV_MAT_ELEM(*W, float, 2,0);

	cvSave("Testmatrix.xml", test,"Test-MATRIX", "The Test Matrix", cvAttrList(0,0));
	det = (float)cvDet(test);
	
	sprintf(buffer, "Image Pt: (%d, %d)\nUndistorited Image Point: (%f, %f)\nKnown 3D Point: (%f, %f, %f)\nDerived 3D Point: (%f, %f, %f)\nCamera Optical Center: (%f, %f, %f)\nDet: %f\nMonument Point ID: %d",
		pp->image_x, pp->image_y, 
		1.0, 1.0,
		pp->world_x, pp->world_y, pp->world_z,
		CV_MAT_ELEM(*W, float, 0,0),
		CV_MAT_ELEM(*W, float, 1,0),
		CV_MAT_ELEM(*W, float, 2,0),
		1.0f*CV_MAT_ELEM(*co, float, 0,0),
		1.0f*CV_MAT_ELEM(*co, float, 1,0),
		1.0f*CV_MAT_ELEM(*co, float, 2,0),
		det,
		monumentPointID);

	mbstowcs(buff, buffer, strlen(buffer)+1);

	MessageBox(NULL, buff, L"ReProject Calculation", MB_OK);
///////////////////////////////////////////
	free(pp);

	cvReleaseMat(&R);
	cvReleaseMat(&T);
	cvReleaseMat(&W);
	cvReleaseMat(&J);
	cvReleaseMat(&Rinv);
	cvReleaseMat(&test);
	cvReleaseMat(&uv);
	cvReleaseMat(&co);
	
	__END__;
	return(p);
}


int createRay(PHOTOG_PARAMS *pp)
{
//	float det;

	CvMat *Rinv;
	CvMat *R;
	CvMat *W;
	CvMat *T;
	CvMat *J;
//	CvMat *test;
	CvMat *uv;
	CvMat *co;

//	char buffer[1024];
//	wchar_t buff[1024];
	R = cvCreateMat(3,3, CV_32FC1);
	if( R==NULL )
	{
		MessageBox(NULL,L"R was not allocated in CreateRay",L"ERROR", MB_ICONHAND);
	}
	T = cvCreateMat(3,1, CV_32FC1);
	if( T==NULL )
	{
		MessageBox(NULL,L"T was not allocated in CreateRay",L"ERROR", MB_ICONHAND);
	}
	co = cvCreateMat(3,1, CV_32FC1);
	if( co==NULL )
	{
		MessageBox(NULL,L"co was not allocated in CreateRay",L"ERROR", MB_ICONHAND);
	}
	J = cvCreateMat(3,1, CV_32FC1);
	if( J==NULL )
	{
		MessageBox(NULL,L"J was not allocated in CreateRay",L"ERROR", MB_ICONHAND);
	}
	W = cvCreateMat(3,1, CV_32FC1);
	if( W==NULL )
	{
		MessageBox(NULL,L"W was not allocated in CreateRay",L"ERROR", MB_ICONHAND);
	}
	uv = cvCreateMat(3,1, CV_32FC1);
	if( uv==NULL )
	{
		MessageBox(NULL,L"uv was not allocated in CreateRay",L"ERROR", MB_ICONHAND);
	}
/*
	test = cvCreateMat(3,3, CV_32FC1);
	if( test==NULL )
	{
		MessageBox(NULL,L"test was not allocated in CreateRay",L"ERROR", MB_ICONHAND);
	}*/
	Rinv = cvCreateMat(3,3, CV_32FC1);
	if( Rinv==NULL )
	{
		MessageBox(NULL,L"Rinv was not allocated in CreateRay",L"ERROR", MB_ICONHAND);
	}

	CV_MAT_ELEM(*T, float, 0,0) = pp->t1;
	CV_MAT_ELEM(*T, float, 1,0) = pp->t2;
	CV_MAT_ELEM(*T, float, 2,0) = pp->t3;

	CV_MAT_ELEM(*R, float, 0,0) = pp->r11;
	CV_MAT_ELEM(*R, float, 0,1) = pp->r12;
	CV_MAT_ELEM(*R, float, 0,2) = pp->r13;
	CV_MAT_ELEM(*R, float, 1,0) = pp->r21;
	CV_MAT_ELEM(*R, float, 1,1) = pp->r22;
	CV_MAT_ELEM(*R, float, 1,2) = pp->r23;
	CV_MAT_ELEM(*R, float, 2,0) = pp->r31;
	CV_MAT_ELEM(*R, float, 2,1) = pp->r32;
	CV_MAT_ELEM(*R, float, 2,2) = pp->r33;

	cvInv(R,Rinv, 0);

	cvMatMul(Rinv, T, co);
	
	CV_MAT_ELEM(*uv, float, 0,0) = (float)((pp->image_x - pp->cx)/pp->fx)*125.0f;
	CV_MAT_ELEM(*uv, float, 1,0) = (float)((pp->image_y - pp->cy)/pp->fy)*125.0f;
	CV_MAT_ELEM(*uv, float, 2,0) = 1.0f*125.0f;

	cvSub(uv, T, J, 0);

	cvMatMul(Rinv, J, W);

#ifdef VERBOSE_INSTRUMENTATION1
	cvSave("Worldmatrix.xml", W,"World-Point", "World-Point", cvAttrList(0,0));
	cvSave("Rinvmatrix.xml", Rinv,"Rinv-MATRIX", "The Rinv Matrix", cvAttrList(0,0));
	cvSave("Jmatrix.xml", J,"J-MATRIX", "The J Matrix", cvAttrList(0,0));
	cvSave("uvmatrix.xml", uv,"uv-MATRIX", "The uv Matrix", cvAttrList(0,0));	
	cvSave("Tmatrix.xml", T,"T-MATRIX", "The T Matrix", cvAttrList(0,0));
	cvSave("comatrix.xml", co,"co-MATRIX", "The co Matrix", cvAttrList(0,0));
#endif //VERBOSE_INSTRUMENTATION	

	pp->world_x = 	CV_MAT_ELEM(*W, float, 0,0);
	pp->world_y = 	CV_MAT_ELEM(*W, float, 1,0);
	pp->world_z = 	CV_MAT_ELEM(*W, float, 2,0);

/*
	CV_MAT_ELEM(*te  st, float, 0,0) = 	1.0f*CV_MAT_ELEM(*co, float, 0,0);
	CV_MAT_ELEM(*test, float, 0,1) = 	1.0f*CV_MAT_ELEM(*co, float, 1,0);
	CV_MAT_ELEM(*test, float, 0,2) = 	1.0f*CV_MAT_ELEM(*co, float, 2,0);
	
	CV_MAT_ELEM(*test, float, 1,0) = 	pp->world_x;
	CV_MAT_ELEM(*test, float, 1,1) = 	pp->world_y;
	CV_MAT_ELEM(*test, float, 1,2) = 	pp->world_z;

	CV_MAT_ELEM(*test, float, 2,0) = 	CV_MAT_ELEM(*W, float, 0,0);
	CV_MAT_ELEM(*test, float, 2,1) = 	CV_MAT_ELEM(*W, float, 1,0);
	CV_MAT_ELEM(*test, float, 2,2) = 	CV_MAT_ELEM(*W, float, 2,0);

	cvSave("Testmatrix.xml", test,"Test-MATRIX", "The Test Matrix", cvAttrList(0,0));
	det = (float)cvDet(test);
	
	sprintf(buffer, "Image Pt: (%d, %d)\nUndistorited Image Point: (%f, %f)\nKnown 3D Point: (%f, %f, %f)\nDerived 3D Point: (%f, %f, %f)\nCamera Optical Center: (%f, %f, %f)\nDet: %f\nMonument Point ID: %d",
		pp->image_x, pp->image_y, 
		1.0, 1.0,
		pp->world_x, pp->world_y, pp->world_z,
		CV_MAT_ELEM(*W, float, 0,0),
		CV_MAT_ELEM(*W, float, 1,0),
		CV_MAT_ELEM(*W, float, 2,0),
		1.0f*CV_MAT_ELEM(*co, float, 0,0),
		1.0f*CV_MAT_ELEM(*co, float, 1,0),
		1.0f*CV_MAT_ELEM(*co, float, 2,0),
		det,
		monumentPointID);

	mbstowcs(buff, buffer, strlen(buffer)+1);

	MessageBox(NULL, buff, L"ReProject Calculation", MB_OK);*/
///////////////////////////////////////////

	cvReleaseMat(&R);
	cvReleaseMat(&T);
	cvReleaseMat(&W);
	cvReleaseMat(&J);
	cvReleaseMat(&Rinv);
//	cvReleaseMat(&test);
	cvReleaseMat(&uv);
	cvReleaseMat(&co);

	return(0);
}

#define NUM_BINS 30
#define NUM_POINTS 4096

int histogramPointCloud(char *name, int foodItemID)
{
	FILE *input;
	FILE *output;
	char buffer[1024];
	float zPoints[NUM_POINTS][4];
	float lowerbound, upperbound;
	int zCount, countbelowplate;// = 0;
	float step;
	int i,j;
	float x,y,z,maxz, minz;
	int bin[NUM_BINS];

	input = fopen(name,"r+");
	fgets(buffer, sizeof(buffer), input);
	fgets(buffer, sizeof(buffer), input);
	minz = FLT_MAX;
	maxz = -FLT_MAX;
	memset(bin, 0, sizeof(bin));
	zCount = 0;
	while( fgets(buffer, sizeof(buffer), input) )
	{
		sscanf(buffer, "%f %f %f\n", &x, &y, &z);
		zPoints[zCount][0] = z;
		zPoints[zCount][1] = x;
		zPoints[zCount][2] = y;
		zPoints[zCount][3] = 0;

		zCount++;
		if( z < minz )
			minz = z;
		if( z > maxz )
			maxz = z;
	}
	fclose(input);
	step = (float)(maxz - minz) / (float)NUM_BINS;


	for(i=0; i<zCount; i++)
	{
		for(j=0; j<NUM_BINS; j++)
		{
			if( zPoints[i][0] > j*step && zPoints[i][0] <= (j+1)*step )
			{
				bin[j]++;
				j = NUM_BINS;
			}
		}
	}

	output = fopen( "histogram.txt", "w");
	fprintf(output, "Bin, Histogram, Lowerbound, Upperbound\n");
	for(j=0; j<NUM_BINS; j++)
	{
		fprintf(output, "bin[%d] %d, %f, %f\n", j, bin[j], j*step, (j+1)*step);
	}
	fclose(output);

	sprintf(buffer, "HistogramCulledPoints-FoodID%d.txt", foodItemID);
	output = fopen(buffer,"w+");

	for(j=1; j<=4; j++)
	{
		float factor;
		sprintf(buffer, "HistogramCulledPoints-FoodID%d-%d.txt", foodItemID,j);
		output = fopen(buffer,"w+");
		factor = j * 0.05;
		lowerbound = maxz * factor;
		upperbound = maxz * (1.0-factor);
		countbelowplate = 0;

		for(i=0; i<zCount; i++)
		{
			if( zPoints[i][0] <= lowerbound || zPoints[i][0] >= upperbound )
				zPoints[i][3] = -1.0;
			else if( zPoints[i][0] <= PLATECUTOFF )
			{
				zPoints[i][3] = -2.0;
				countbelowplate++;
			}
			else
				zPoints[i][3] = 0.0;
		}

		update_fooditem_volume(foodItemID, (double)countbelowplate, POINTSBELOWPLATE);
		update_fooditem_volume(foodItemID, (double)zCount, TOTALPOINTS);
		update_fooditem_volume(foodItemID, PLATECUTOFF, PLATEHEIGHT);
		update_fooditem_volume(foodItemID, (double)maxz, MAXZ);
		update_fooditem_volume(foodItemID, (double)minz, MINZ);

		for(i=0; i<zCount; i++)
		{
			if( zPoints[i][3] == 0 )
				fprintf(output,"%f %f %f\n", zPoints[i][2], zPoints[i][1], zPoints[i][0]);
	//		else
	//			fprintf(output,"%f %f %f REMOVED-OUTOFBOUNDS\n", zPoints[i][2], zPoints[i][1], zPoints[i][0]);

		}
//fprintf(output, "UpperBound=%f,  LowerBound=%f\n", upperbound, lowerbound);
		fclose(output);


		switch(j+13)
		{
		case FIVEPERCENT:
			samplePoints(buffer, foodItemID, FIVEPERCENT);
			break;
		case TENPERCENT:
			samplePoints(buffer, foodItemID, TENPERCENT);
			break;
		case FIFTEENPERCENT:
			samplePoints(buffer, foodItemID, FIFTEENPERCENT);
			break;
		case TWENTYPERCENT:
			samplePoints(buffer, foodItemID, TWENTYPERCENT);
			break;
		}
	}
}
