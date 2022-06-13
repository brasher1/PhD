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
//#include <windows.h>
//#undef WIN32_LEAN_AND_MEAN
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

#include "stdafx.h"
#include <process.h>

#include "PhotogDiet.h"
#include "ImageMagickInterface.h"
#include <commctrl.h>
#include <crtdbg.h>
#include <magick/api.h>
#include <fcntl.h>
#include <share.h>
#include <errno.h>
#include <io.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

int segment_an_image(char *, float, float, int);

INT_PTR CALLBACK MonumentPointDlgProc(HWND, UINT, WPARAM, LPARAM);

extern "C"
{
#include "database.h"
#include "assigncoordinates.h"
#include "images.h"
#include "database_globals.h"
#include "window_globals.h"

struct threadargs
{
	HWND hWnd;
	HWND hStatusBar;
	HWND hToolBar;
	int	 mode;		//where to start in photog process-if other than beginning.
					// insterted for stalled or images with errors.
					// defines are in 
};



typedef struct threadargs THREADARGS;
typedef struct threadargs * PTHREADARGS;

extern THREADARGS threadArgs;

extern wchar_t status_bar1[2048];
extern wchar_t status_bar2[2048];
extern wchar_t status_bar3[2048];
extern char execpath[MAX_PATH];
}
extern HINSTANCE hInst;
PTHREADARGS pArgs;
int photox, photoy;

INT_PTR CALLBACK MonumentPointDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hWndDlg;
	TCHAR tc[1000];
	int length;
	int pointNumber;
	int id;

	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
        hWndDlg = GetDlgItem(hDlg, IDC_POINTPICKER);
        for(int Count = 1; Count < 22; Count++)
        {
			wchar_t str[5];
			wsprintf(str, L"%d", Count);
            SendMessage(hWndDlg,
                        CB_ADDSTRING,
                        0,
                        reinterpret_cast<LPARAM>((LPCTSTR)str));
        }		
		hWndDlg = GetDlgItem(hDlg, IDC_PHOTOX);
		_stprintf(tc, TEXT("%d"), photox); ;
		SendMessage(hWndDlg, WM_SETTEXT, 0, (LPARAM)tc);
		hWndDlg = GetDlgItem(hDlg, IDC_PHOTOY);
		_stprintf(tc, TEXT("%d"), photoy);
		SendMessage(hWndDlg, WM_SETTEXT, 0, (LPARAM)tc);
		return (INT_PTR)TRUE;
	case IDC_DELETEPOINT:
		MessageBox(NULL,L"Delete Point",L"Delete Point",MB_OK);
		
		break;
	case IDD_SAVEPOINT:

		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		if( LOWORD(wParam) == IDD_SAVEPOINT )
		{
			hWndDlg = GetDlgItem(hDlg, IDC_POINTPICKER);
			GetWindowText(hWndDlg, tc, sizeof(tc));
			pointNumber = _wtoi(tc);
			id = getClosestMonumentPoint(pointNumber, photox, photoy); 
			if(  id > 0 )
			{
				update_3D_coordinates( id, pointNumber);
				//MessageBox(NULL, L"Updated Coordinates",L"HERE", MB_OK);
			}
			else
			{
				MessageBox(NULL, L"Did not get ID back for MP",L"ERROR", MB_ICONHAND);
			}
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		if( LOWORD(wParam) == IDC_DELETEPOINT )
		{
			hWndDlg = GetDlgItem(hDlg, IDC_POINTPICKER);
			GetWindowText(hWndDlg, tc, sizeof(tc));
			pointNumber = _wtoi(tc);
			deleteClosestMonumentPoint(pointNumber, photox, photoy);
			MessageBox(NULL,L"Delete Point",L"Delete Point",MB_OK);
		}
	}
	return (INT_PTR)FALSE;
}

void my_mouse_callback( int event, int x, int y, int flags, void *param)
{

	IplImage *img = (IplImage*)param;
	char buffer[MAX_PATH];
	wchar_t buff[MAX_PATH];

	switch( event)
	{
	case CV_EVENT_LBUTTONUP:
		sprintf(buffer, "(X,Y) = ( %d, %d )", x, y);
		mbstowcs(buff, buffer, sizeof(buff));
		photox = x;
		photoy = y;
		//MessageBox(NULL,buff,L"POINT", MB_OK);
		DialogBox(hInst, MAKEINTRESOURCE(IDD_FIXMONUMENTPOINT), 
			(HWND)cvGetWindowHandle(get_monument_point_filename()), 
			MonumentPointDlgProc);
		break;
	}
}

unsigned Counter; 
unsigned __stdcall RunFullProcess( void* pArguments )
{
	char buffer[_MAX_PATH];

	char *p;
	pArgs = (PTHREADARGS)pArguments;
	wchar_t w_message[512];


	switch( pArgs->mode )
	{
	case WHOLE:
		p = get_current_filename();
		if( create_image(p) == -1)
		{
			MessageBox(pArgs->hWnd, L"Image Not Created", L"Error", MB_ICONEXCLAMATION); 
			mbstowcs(w_message, get_current_filename(), strlen(get_current_filename())+1);
			wsprintf(status_bar1, L"Image NOT Created: %s\0\0", w_message);
			PostMessage(pArgs->hStatusBar, SB_SETTEXT, 2, (LPARAM)(LPWSTR)status_bar1);
		}
		else
		{
			mbstowcs(w_message, get_current_filename(), strlen(get_current_filename())+1);
			wsprintf(status_bar1, L"Image Record Created: %s\0\0", w_message);
			PostMessage(pArgs->hStatusBar, SB_SETTEXT, 2, (LPARAM)(LPWSTR)status_bar1);
		}
		//goto end;
	case CANNY:
		mbstowcs(w_message, get_current_filename(), strlen(get_current_filename())+1);
		wsprintf(status_bar1, L"Doing Canny: %s\0\0", w_message);
		PostMessage(pArgs->hStatusBar, SB_SETTEXT, 2, (LPARAM)(LPWSTR)status_bar1);
		OpenCVcanny(get_current_filename());
		//PostMessage(pArgs->hWnd, WM_PAINT, 0,0);
		mbstowcs(w_message, get_current_filename(), strlen(get_current_filename())+1);
		wsprintf(status_bar1, L"Canny Complete: %s\0\0", w_message);
		PostMessage(pArgs->hStatusBar, SB_SETTEXT, 2, (LPARAM)(LPWSTR)status_bar1);

	case NEGATEIMAGE:
		//_CrtDumpMemoryLeaks();
		Negate_Image(get_current_filename());
		//MessageBox(NULL, L"After Negate", L"HERE", MB_OK);
		//PostMessage(pArgs->hWnd, WM_PAINT, 0,0);
		mbstowcs(w_message, get_current_filename(), strlen(get_current_filename())+1);
		wsprintf(status_bar1, L"Negated Image/Starting Upwrite: %s\0\0", w_message);
		PostMessage(pArgs->hStatusBar, SB_SETTEXT, 2, (LPARAM)(LPWSTR)status_bar1);

	case UPWRITE:
		//////////////////////////////////////////////////////////////
	//	recognise(get_current_filename(), get_original_filename());
		sprintf(buffer, "UpWrite.exe \"%s\" \"%s\" \"%s\" %d", execpath, get_current_filename(), get_original_filename(),
			get_current_image());
		system(buffer);
		/////////////////////////////////////////////////////////////////

		mbstowcs(w_message, get_current_filename(), strlen(get_current_filename())+1);
		wsprintf(status_bar1, L"UpWrite Done/Assigning Coordinates: %s\0\0", w_message);
		PostMessage(pArgs->hStatusBar, SB_SETTEXT, 2, (LPARAM)(LPWSTR)status_bar1);
		//PostMessage(pArgs->hWnd, WM_PAINT, 0,0);
		//_CrtDumpMemoryLeaks();
		//MessageBox(NULL,L"Assigning Coordinates",L"HErE", MB_OK);

	case ASSIGNCOORDDINATES:
		if( assign_coordinates(get_current_filename()) < 0 )
		{
			MessageBox(pArgs->hWnd, L"World Coordinates were not assigned", L"Error in AssignCoordinates", MB_ICONERROR);

			mbstowcs(w_message, get_current_filename(), strlen(get_current_filename())+1);
			wsprintf(status_bar1, L"No World Coordinates Assigned: %s\0\0", w_message);
			PostMessage(pArgs->hStatusBar, SB_SETTEXT, 2, (LPARAM)(LPWSTR)status_bar1);

			//goto end;
		}
	case REVIEWPOINTASSIGNMENT:
		mbstowcs(w_message, get_current_filename(), strlen(get_current_filename())+1);
		wsprintf(status_bar1, L"%s\0\0", w_message);
		PostMessage(pArgs->hStatusBar, SB_SETTEXT, 2, (LPARAM)(LPWSTR)status_bar1);
		PostMessage(pArgs->hWnd, IDM_SHOWUPWRITEIMAGE, 0,0);
	case LOCATECAMERA:
		//MessageBox(NULL,L"AFTER Assigning Coordinates",L"HErE", MB_OK);
		//MessageBox(NULL,L"Locating Camera",L"HERE", MB_OK);
		if( locate_camera(get_current_filename()) < 0 )
		{
			IplImage *img;
			cvNamedWindow(get_monument_point_filename(),0);

			img = cvLoadImage(get_monument_point_filename());
			cvResizeWindow(get_monument_point_filename(), img->width, img->height);
			cvShowImage(get_monument_point_filename(), img);
			cvSetMouseCallback(get_monument_point_filename(), my_mouse_callback, (void *)img);
			while( 1 )
			{
				if( cvWaitKey(100) == 27 ) break;
			}
			cvReleaseImage(&img);
			cvDestroyWindow(get_monument_point_filename());
			if( locate_camera(get_current_filename()) >= 0 )
			{
				
			}
			else
			{
				goto end;
			}
		//	MessageBox(pArgs->hWnd, L"Not Enough Monument Points for a Solution", L"Error in LocateCamera", MB_ICONERROR);
	//	MessageBox(NULL,L"Going to EnD",L"ERROR", MB_OK); 
	//		mbstowcs(w_message, get_current_filename(), strlen(get_current_filename())+1);
	//		wsprintf(status_bar1, L"Not enough Monument Points: %s\0\0", w_message);
	//		PostMessage(pArgs->hStatusBar, SB_SETTEXT, 2, (LPARAM)(LPWSTR)status_bar1);
		}
	//	MessageBox(NULL,L"AFTER LOCATING CAMERA",L"HErE", MB_OK);		
	//
	case SAVEIMAGEPBM:
		strcpy( buffer, get_current_filename());
		set_current_filename(buffer);
		save_cvImageAsPBM(NULL);
		//PostMessage(pArgs->hWnd, WM_PAINT, 0,0);
		mbstowcs(w_message, get_current_filename(), strlen(get_current_filename())+1);
		wsprintf(status_bar1, L"Saved an image copy: %s\0\0", w_message);
		PostMessage(pArgs->hStatusBar, SB_SETTEXT, 2, (LPARAM)(LPWSTR)status_bar1);

	case SEGMENTATION:
		PostMessage(pArgs->hWnd, ID_SEGMENT_IMAGE, 0, 0);
		mbstowcs(w_message, get_current_filename(), strlen(get_current_filename())+1);
		wsprintf(status_bar1, L"Segmenting Image: %s\0\0", w_message);
		PostMessage(pArgs->hStatusBar, SB_SETTEXT, 2, (LPARAM)(LPWSTR)status_bar1);

		segment_an_image(get_current_filename(), 0.8f, 250.0f, 9000);
		//PostMessage(pArgs->hWnd, WM_PAINT, 0,0);
		mbstowcs(w_message, get_current_filename(), strlen(get_current_filename())+1);
		wsprintf(status_bar1, L"Segmenting Complete: %s\0\0", w_message);
		PostMessage(pArgs->hStatusBar, SB_SETTEXT, 2, (LPARAM)(LPWSTR)status_bar1);
	case SAVEIMAGEPPM:
		save_cvSegmentationImageAsPPM((float)0.8f, (float)250, 9000);
		//PostMessage(pArgs->hWnd, WM_PAINT, 0,0);
	/*	mbstowcs(w_message, get_current_filename(), strlen(get_current_filename())+1);
		wsprintf(status_bar1, L"Filling plate white: %s\0\0", w_message);
		PostMessage(pArgs->hStatusBar, SB_SETTEXT, 2, (LPARAM)(LPWSTR)status_bar1);
//DJB MUST COMBINE SEGMENTED FOOD HERE
		
///////////////////////////////////////
		
		fill_plate_white(get_current_filename());*/
		fixSegmentation();

		//PostMessage(pArgs->hWnd, WM_PAINT, 0,0);
	case FINDFOOD:
		mbstowcs(w_message, get_current_filename(), strlen(get_current_filename())+1);
		wsprintf(status_bar1, L"Finding Food: %s\0\0", w_message);
		PostMessage(pArgs->hStatusBar, SB_SETTEXT, 2, (LPARAM)(LPWSTR)status_bar1);

		FindFood(get_current_filename());
		//PostMessage(pArgs->hWnd, WM_PAINT, 0,0);
		mbstowcs(w_message, get_current_filename(), strlen(get_current_filename())+1);
		wsprintf(status_bar1, L"Found Food: %s\0\0", w_message);
		PostMessage(pArgs->hStatusBar, SB_SETTEXT, 2, (LPARAM)(LPWSTR)status_bar1);
	default:
		save_cvImageAsPBM(get_current_filename());
		//PostMessage(pArgs->hWnd, WM_PAINT, 0,0);
	}
	
end:
	mbstowcs(w_message, get_current_filename(), strlen(get_current_filename())+1);
	wsprintf(status_bar1, L"%s", w_message);
	PostMessage(pArgs->hStatusBar, SB_SETTEXT, 2, (LPARAM)(LPWSTR)status_bar1);
	wsprintf(status_bar2, L"DONE");
	PostMessage(pArgs->hStatusBar, SB_SETTEXT, 3, (LPARAM)(LPWSTR)status_bar2);
	//MessageBox(NULL,L"ENDING THREAD", L"HErE", MB_OK);
	_endthreadex( 0 );
    return 0;
} 

HANDLE executeFullProcess(void *pData)
{ 
    HANDLE hThread;
    unsigned threadID;

    // Create the work thread.
    hThread = (HANDLE)_beginthreadex( NULL, 0, &RunFullProcess, (void *)pData, 0, &threadID );

	//Wait until Finished
	WaitForSingleObject( hThread, INFINITE );
	// Destroy the thread object.

	return hThread;
}


int doBatchProcess(char *filename)
{
	FILE *input;
	char buffer[MAX_PATH];
	int meal_id;
	int i;
	char *p;
	int flag;

	input = fopen(filename,"r");
	if( input == NULL )
	{
		return -1;
	}
	flag = 0;
	while( fgets(buffer, sizeof(buffer), input) )
	{
		i = _strnicmp(buffer, "MEAL", 4);

		if( i == 0 )
		{
			if( flag == 1 )
			{
				matchFoodItems(/*meal_id*/ meal_id, /* grouptype */ BEFORE);
				flag = 0;
			}
			meal_id = createMealRecords(1/*person ID*/, BEFORE);
		}
		else
		{
			flag = 1;
			p = strrchr(buffer, '\n');
			if( p!=NULL )
				*p = '\0';
			if( strlen(buffer) != 0 )
			{
				set_current_filename(buffer);
				set_filename_base(buffer);
				set_original_filename(buffer);
//				openCVsegmentation(buffer);
				threadArgs.mode = WHOLE;

				executeFullProcess(&threadArgs);			
			}
		}
	}
	fclose(input);
	return 0;
}