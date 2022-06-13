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
// PhotogDiet.cpp : Defines the entry point for the application.
//


#include "stdafx.h"

#include <process.h>

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include "PhotogDiet.h"
#include "ImageMagickInterface.h"
#include <commctrl.h>
#include <crtdbg.h>
#include <magick/api.h>
#include <Shobjidl.h>
#include <sqltypes.h>

#include <cxcore.h>

COMDLG_FILTERSPEC rgSpec[] =
{ 
    { L"JPG Files", L"*.jpg;*.jpeg" },
    { L"All Files", L"*.*" }
};

COMDLG_FILTERSPEC rgSpec2[] =
{ 
    { L"TXT Files", L"*.txt" },
    { L"All Files", L"*.*" }
};

extern "C"
{
#include "database.h"
#include "assigncoordinates.h"
#include "images.h"
#include "database_globals.h"
#include "window_globals.h"
#include "geometry.h"

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

THREADARGS threadArgs;

ULONG_PTR  gdiplusToken;

}




HANDLE hLogFile;


#ifdef _DEBUG
#define new DEBUG_CLIENTBLOCK
#endif

int segment_an_image(char *filename, float sigma, float k, int min_size);
void canny2(float sigma, float tlow, float thigh, unsigned char **edge, char *filename);
LRESULT CALLBACK NewWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
int load_preferences();


wchar_t w_current_file[MAX_PATH];
char	current_file[MAX_PATH];
int		partwidths[] = {0, 200, 800, -1};
wchar_t	exepath[MAX_PATH];
//char	execpath[MAX_PATH];
double		mag_factor;

#define MAX_LOADSTRING 100
extern "C"
{
	wchar_t status_bar1[2048];
	wchar_t status_bar2[2048];
	wchar_t status_bar3[2048];
	char	execpath[MAX_PATH];
}

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
HWND hWndStatus;
HWND hWndToolbar;
// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	Preferences(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_PHOTOGDIET, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

    hLogFile = CreateFile(L"c:\\log.txt", GENERIC_WRITE, 
      FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, 
      FILE_ATTRIBUTE_NORMAL, NULL);
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, hLogFile);

	
	// Get current flag
	int tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );

	// Turn on leak-checking bit
	tmpFlag |= _CRTDBG_LEAK_CHECK_DF;
	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	cvSetErrMode( CV_ErrModeSilent);

	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	// Initialize GDI+.
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PHOTOGDIET));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if( msg.message == WM_QUIT )
			break;
	}

	_CrtDumpMemoryLeaks();
	tmpFlag &= ~_CRTDBG_CHECK_CRT_DF;

	// Set flag to the new value
	_CrtSetDbgFlag( tmpFlag );

    CloseHandle(hLogFile);
	//GdiplusShutdown(gdiplusToken);	



	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PHOTOGDIET));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_PHOTOGDIET);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;
	wchar_t *p;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);


   if (!hWnd)
   {
      return FALSE;
   }

   	threadArgs.hWnd = hWnd;

	GetModuleFileName(NULL, exepath, MAX_PATH);
    p = wcsrchr(exepath, '\\');
	*p = '\0';
	
	wcstombs(execpath, exepath, MAX_PATH);

//	input_classes(execpath);
	initialize_db();
	mag_factor = 1;

	load_preferences();

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	wchar_t szBuf[256];
//	SIMPLE_POINT *sp;
	char *p;
//	char buffer[MAX_PATH];

	switch (message)
	{
					
   case WM_CREATE:
      hWndStatus = CreateWindowEx( 
            0L,                              // no extended styles
            STATUSCLASSNAME,                 // status bar
            (LPCWSTR)"",                              // no text 
            WS_CHILD | WS_BORDER | WS_VISIBLE,  // styles
            -100, -100, 10, 10,              // x, y, cx, cy
            hWnd,                            // parent window
            (HMENU)100,                      // window ID
            hInst,                           // instance
            NULL);                           // window data

      if (hWndStatus == NULL)
         MessageBox (NULL, (LPCWSTR)"Status Bar not created!", NULL, MB_OK );
		
	  SendMessage(hWndStatus, SB_SETPARTS, (WPARAM)4, (LPARAM)&partwidths[0]);
	  
      hWndToolbar = CreateWindowEx( 
            0L,                              // no extended styles
            TOOLBARCLASSNAME,                 // status bar
            (LPCWSTR)"",                              // no text 
            WS_CHILD | TBSTYLE_WRAPABLE,  // styles
            100,100,100,100,              // x, y, cx, cy
            hWnd,                            // parent window
            (HMENU)100,                      // window ID
            hInst,                           // instance
            NULL);                           // window data

      if (hWndStatus == NULL)
         MessageBox (NULL, (LPCWSTR)"Tool Bar not created!", NULL, MB_OK );

	  threadArgs.hStatusBar = hWndStatus;
	  threadArgs.hToolBar = hWndToolbar;

	  break;
	  
   case WM_MOUSEMOVE:
      wsprintf(szBuf, TEXT("X,Y: %d, %d"), LOWORD(lParam), HIWORD(lParam));
      SendMessage(hWndStatus, SB_SETTEXT, 1, (LPARAM)(LPSTR)szBuf);
      break;
	case WM_SIZE:
		{
//			HWND hTool;//, hStatus;
			//RECT rcTool, rcStatus;
			//int  iStatusHeight, iToolHeight;

			// Size toolbar and get height
//			hTool = GetDlgItem(hwnd, IDC_MAIN_TOOL);
//			SendMessage(hTool, TB_AUTOSIZE, 0, 0);

			//GetWindowRect(hTool, &rcTool);
			//iToolHeight = rcTool.bottom - rcTool.top;

			// Size status bar and get height
			//hStatus = GetDlgItem(hwnd, IDC_MAIN_STATUS);
			SendMessage(hWndStatus, WM_SIZE, 0, 0);

			//GetWindowRect(hStatus, &rcStatus);
			//iStatusHeight = rcStatus.bottom - rcStatus.top;

			//MessageBox(hwnd,"iEditHeightj","WndProc", 1);
			NewWndProc(hWndStatus, message, wParam, lParam);
		}
		break;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_NEXT:
			mag_factor = mag_factor * 2.0;
			break;
		case VK_PRIOR:
			{
				mag_factor = mag_factor * 0.5;
			}
			break;
		}
		::InvalidateRect(hWnd,NULL, TRUE);
		break;
	case ID_SEGMENT_IMAGE:
		
		break;

	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		case IDM_SHOWUPWRITEIMAGE:
			MessageBox(NULL,L"I am in ShowImage",L"Hey", MB_OK);
			break;
		case ID_ALGORITHM_PICKMEALPHOTOS:

			break;
		case ID_ALGORITHM_OPE:
			OpenCVcanny(current_file);
			break;
		case ID_ALGORITHM_COMPLETE32787:
			threadArgs.mode = WHOLE;
			set_current_grouptype_meal(1);
			executeFullProcess(&threadArgs);
			break;
		case ID_ALGORITHM_RUNFROMCAMERALOC:
			{
				char *p;
				char buffer[MAX_PATH];
				int images[] = {1531, 1520, 1525, 1532, 1538 };
				for(int i=0; i<sizeof(images)/sizeof(int); i++)
				{
					set_current_image(images[i]);
					p = getImageName(get_current_image());
					strcpy(buffer, p);
					set_current_filename(buffer);
					set_filename_base(buffer);
					set_original_filename(buffer);
					free(p);
					threadArgs.mode = LOCATECAMERA;
					executeFullProcess(&threadArgs);
				}
			}
			break;
		case ID_ALGORITHM_COMPLETE:
			{
			char buffer[MAX_PATH];
			if( create_image(current_file) == -1)
			{
				//PostMessage(hWnd, WM_COMMAND,  
			}
            mbstowcs(w_current_file, current_file, strlen(get_current_filename()));
			wsprintf(szBuf, TEXT("Calling Canny: %s"), w_current_file);
			SendMessage(hWndStatus, SB_SETTEXT, 2, (LPARAM)(LPSTR)szBuf);

			OpenCVcanny(current_file);

			_CrtDumpMemoryLeaks();
			mbstowcs(w_current_file, get_current_filename(), strlen(get_current_filename()));
			wsprintf(szBuf, TEXT("NegateImage: %s"), w_current_file);
			SendMessage(hWndStatus, SB_SETTEXT, 2, (LPARAM)(LPSTR)szBuf);

			Negate_Image(get_current_filename());

            mbstowcs(w_current_file, get_current_filename(), strlen(get_current_filename()));
			wsprintf(szBuf, TEXT("Calling UpWrite for Ellipses: %s"), w_current_file);
			SendMessage(hWndStatus, SB_SETTEXT, 2, (LPARAM)(LPSTR)szBuf);

			sprintf(buffer, "UpWrite.exe \"%s\" \"%s\" \"%s\" %d", execpath, get_current_filename(), get_original_filename(),
				get_current_image());
			system(buffer);

			_CrtDumpMemoryLeaks();
            mbstowcs(w_current_file, get_current_filename(), strlen(get_current_filename()));
            mbstowcs(w_current_file, get_current_filename(), strlen(get_current_filename()));
			wsprintf(szBuf, TEXT("Calling Assign Coordinates: %s"), w_current_file);
			SendMessage(hWndStatus, SB_SETTEXT, 2, (LPARAM)(LPSTR)szBuf);
	
			if( assign_coordinates(get_current_filename()) < 0 )
			{
				MessageBox(hWnd, L"World Coordinates were not assigned", L"Error in AssignCoordinates", MB_ICONERROR);
				break;
			}

            mbstowcs(w_current_file, get_current_filename(), strlen(get_current_filename()));
			wsprintf(szBuf, TEXT("Calling Locate Camera: %s"), w_current_file);
			SendMessage(hWndStatus, SB_SETTEXT, 2, (LPARAM)(LPSTR)szBuf);

			if( locate_camera(get_current_filename()) < 0 )
			{
				MessageBox(hWnd, L"Not Enough Monument Points for a Solution", L"Error in LocateCamera", MB_ICONERROR);
				break;
			}

            mbstowcs(w_current_file, get_current_filename(), strlen(get_current_filename()));
			wsprintf(szBuf, TEXT("Segmenting the image: %s"), w_current_file);
			SendMessage(hWndStatus, SB_SETTEXT, 2, (LPARAM)(LPSTR)szBuf);
			
			strcpy( buffer, get_current_filename());
/*			for(int k=4; k<=5; k+=2)
			{
				for( int l=500; l<=600; l+=200 )
				{
					for(int m=4000; m<=5000; m+=1000 )
					{
						set_current_filename(buffer);
						segment_an_image(get_current_filename(), 0.1f*(float)k, (float)l, m);
						save_cvImageAsJPG();
					}
				}
			}
*/
			set_current_filename(buffer);
			save_cvImageAsPBM(NULL);
			segment_an_image(get_current_filename(), 0.8f, 250.0f, 9000);
			save_cvSegmentationImageAsPPM((float)0.8f, (float)250, 9000);

			fill_plate_white(get_current_filename());

			FindFood(get_current_filename());

			mbstowcs(w_current_file, get_current_filename(), strlen(get_current_filename()));
			wsprintf(szBuf, TEXT("Calling Canny: %s"), w_current_file);
			SendMessage(hWndStatus, SB_SETTEXT, 2, (LPARAM)(LPSTR)szBuf);
#ifdef NOTNEEDED			
			OpenCVcanny(get_current_filename());
			
			_CrtDumpMemoryLeaks();
			mbstowcs(w_current_file, get_current_filename(), strlen(get_current_filename()));
			wsprintf(szBuf, TEXT("NegateImage: %s"), w_current_file);
			SendMessage(hWndStatus, SB_SETTEXT, 2, (LPARAM)(LPSTR)szBuf);

			Negate_Image(get_current_filename());
#endif //NOENEEDED	
			wsprintf(szBuf, TEXT("Done"));
			SendMessage(hWndStatus, SB_SETTEXT, 3, (LPARAM)(LPSTR)szBuf);
			wsprintf(szBuf, TEXT("%s"),w_current_file);
			SendMessage(hWndStatus, SB_SETTEXT, 2, (LPARAM)(LPSTR)szBuf);
			save_cvImageAsPBM(get_current_filename());

		
			::InvalidateRect(hWnd,NULL, TRUE);

			_CrtDumpMemoryLeaks();
		}
			break;
		case ID_EDIT_PREFERENCES:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_PREFERENCES), hWnd, Preferences);
			break;
		case ID_TOOLS_TESTCAMERALOCATION:
			{
				int i;
				int images[]={  1860, 1862 };
				for( i=0; i<sizeof(images)/sizeof(int); i++)
				{
					set_current_image(images[i]);
					if( locate_camera(get_current_filename()) < 0 )
					{
						MessageBox(hWnd, L"Not Enough Monument Points for a Solution", L"Error in LocateCamera", MB_ICONERROR);
					}
				}
			}
			break;
		case ID_TOOLS_TESTMATCHINGBLOBS:
			{
				int i;
				int meals[]={  268 };
				for( i=0; i<sizeof(meals)/sizeof(int); i++)
				{
					matchFoodItems(meals[i], BEFORE);
				}
			}
			break;
		case ID_TOOLS_TESTFINDFOOD:
			save_cvImageAsPBM(get_current_filename());
			segment_an_image(get_current_filename(), 0.8f, 250.0f, 9000);
			save_cvSegmentationImageAsPPM((float)0.8f, (float)250, 9000);

			fill_plate_white(get_current_filename());

			FindFood(get_current_filename());
			//pick_food_items();
			break;
		case ID_TOOLS_CALIBRATEIMAGE:
			set_current_image(1609);
			locate_camera(get_current_filename());
			break;
		case ID_TOOLS_TESTCREATERAYS:
			createRays(1);
			free( calculateImagePoint(9292));
			/*free( calculateImagePoint(9290));
			free( calculateImagePoint(9289));
			free( calculateImagePoint(9288));
			free( calculateImagePoint(9283));
			free( calculateImagePoint(9282));
			free( calculateImagePoint(9281));*/
/*
			free( validatePhotoG(9292));
			free( validatePhotoG(9290));
			free( validatePhotoG(9289));
			free( validatePhotoG(9288));
			free( validatePhotoG(9283));
			free( validatePhotoG(9282));
			free( validatePhotoG(9281));
			free( validatePhotoG(9278));
			free( validatePhotoG(9274));
*/
			free( validatePhotoG(14689));
			free( validatePhotoG(14688));
			free( validatePhotoG(14685));
			free( validatePhotoG(14684));
			free( validatePhotoG(14676));
			free( validatePhotoG(14675));
			free( validatePhotoG(14672));
			free( validatePhotoG(14671));
			

			break;
		case ID_TOOLS_TESTUNDISTORTIMAGE:
			{
				char *p;
				set_current_image(1609);
				p = getImageName(1609);
				undistortImage(p);
				free(p);
			}
			break;
		case ID_TOOLS_TESTSEGMENTATION:
			{
				char buff2[MAX_PATH];
				strcpy(buff2, get_current_filename());	
				set_current_filename(buff2);
				save_cvImageAsPBM(NULL);
				segment_an_image(get_current_filename(), 0.55f, 250.0f, 9000);
				save_cvSegmentationImageAsJPG((float)0.55f, (float)250, 9000);
				//MessageBox(NULL,L"Saved a JPG",L"HERE", MB_OK);
				//fill_plate_white(get_current_filename());
				//MessageBox(NULL,L"About to FindFood",L"HERE", MB_OK);
//				FindFood(get_current_filename());


/*				for(int k=8; k<=8; k+=1)
				{
					for( int l=250; l<=250; l+=50 )
					{
						for(int m=9000; m<=9000; m+=2000 )
						{
							set_current_filename(buff2);
							save_cvImageAsPBM(NULL);
							segment_an_image(get_current_filename(), 0.1f*(float)k, (float)l, m);
							save_cvSegmentationImageAsJPG((float)k*0.1f, (float)l, m);
						}
					}
				}*/
			}
			break;
		case ID_TOOLS_TESTOPENCVCANNY:
			{
			  OPENFILENAME ofn={0};
			  wchar_t szFileName[MAX_PATH]={0};
			  ofn.lStructSize=sizeof(OPENFILENAME);
			  ofn.Flags=OFN_ALLOWMULTISELECT|OFN_EXPLORER;
			  ofn.lpstrFilter=L"All Files (*.*)\0*.*\0";
			  ofn.lpstrFile=szFileName;
			  ofn.nMaxFile=MAX_PATH;
			  if(GetOpenFileName(&ofn))
			    {
				  //::MessageBox(hWnd,szFileName, L"Test", MB_OK);
				  ::wcscpy(w_current_file, szFileName);
				  wcstombs(current_file, szFileName, MAX_PATH);
				  ::InvalidateRect(hWnd,NULL, TRUE);
				  //UpdateWindow(hWnd);
				}
			  set_original_filename(current_file);
			  set_current_filename(current_file);
			}
			
			OpenCVcanny(get_current_filename());
			//Negate_Image(get_current_filename());
			save_cvImageAsPBM(get_current_filename());
			break;

		case ID_TOOLS_NEGATEIMAGE:
			Negate_Image(current_file);
			break;
		case ID_ALGORITHM_RUNCANNY3:
			{
//			unsigned char *edge;
			//ImageEdgeMagick(current_file);
//			startpgmcanny3("c:\\temp\\target002.pgm", 1.0, 80.0, 200.0, NULL);
//			startjpgcanny3("c:\\temp\\target001a.jpg", 1.0, 20.0, 80.0, "c:\\temp\\dumpgaussian");
			//canny(current_file);
			}
			::InvalidateRect(hWnd,NULL, TRUE);
			break;
		case ID_TOOLS_TESTRAYINTERSECT:
			{
				POINT_3D one, two, three, four;
				INTERSECT_POINT_3D *midpoint;

				one.x=4.0; one.y=4.0; one.z=4.0;

				two.x=-4.0; two.y=-4.0; two.z=-4.0;

				three.x=1.0; three.y=0.0; three.z=0.0;

				four.x=0.0; four.y=1.0; four.z=0.0;

				midpoint = closest_point_between_2_lines(&one, &two, &three, &four);

				InvalidateRect(hWnd, NULL, TRUE);
			}
			break;
		case ID_TOOLS_TESTDBROUTINES:
			create_person("Josias Dupre");

			create_photoblob(679, "Item 1", 0.5, 127.0, 325.0  ); 
			create_blobpoint(current_photoblob, 314, 563 );
			create_volume(4.5);
//			create_volumepoint(current_volume, 3.412, 45.32, 44.33);
//			create_ray(current_blobpoint, 1.7, 2.6, 3.14, 1);
			
			update_volume(1, 7.8);
//			update_ray(1, 2.89, 76.5, 88.8, 1);

			break;
		case ID_TOOLS_TESTOPENCVSEGMENTATION:
			openCVsegmentation(get_current_filename());
			break;
		case ID_EDGES_RUNCANNY:
			{
//				unsigned char *edge;
			//ImageEdgeMagick(current_file);
			//canny2(1.0, 30, 60, &edge, current_file);
			//canny(current_file);
			}
			::InvalidateRect(hWnd,NULL, TRUE);
			break;
		case ID_EDGES_RUNUPWRITE:
			//input_classes(execpath);
//			recognise("c:\\temp\\newarrangement003.jpg");
			//recognise("c:\\temp\\output.jpg");
			//recognise("c:\\temp\\target001org2_six_to_one.jpg");
			//recognise(current_file, current_file);
			//recognise("c:\\temp\\output4.jpg");
			//recognise("c:\\temp\\output5.jpg");
			//recognise("c:\\temp\\output6.jpg");
			break;
		case ID_FILE_BATCHPROCESS:
			{
				DWORD dwOptions;
				IFileOpenDialog *pfd;
			    
				// CoCreate the dialog object.
				HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, 
											  NULL, 
											  CLSCTX_INPROC_SERVER, 
											  IID_PPV_ARGS(&pfd));

				if (SUCCEEDED(hr))
				{
					// Specify multiselect.
					hr = pfd->GetOptions(&dwOptions);
					hr = pfd->SetFileTypes(2, rgSpec2);

//					if (SUCCEEDED(hr))
//					{
//						hr = pfd->SetOptions(dwOptions | FOS_ALLOWMULTISELECT);
//					}

					if (SUCCEEDED(hr))
					{
						// Show the Open dialog.
						hr = pfd->Show(threadArgs.hWnd);
					
						if (SUCCEEDED(hr))
						{
							// Obtain the result of the user interaction.
							IShellItemArray *psiaResults;
							hr = pfd->GetResults(&psiaResults);
			                
							if (SUCCEEDED(hr))
							{
								int meal_id;
								//meal_id = createMealRecords(1/*person ID*/, BEFORE);
								if (psiaResults) 
								{
									IEnumShellItems *pesi;
									if (SUCCEEDED(hr = psiaResults->EnumItems(&pesi))) 
									{
										char buffer[MAX_PATH];
										IShellItem *psi;
										while (pesi->Next(1, &psi, NULL) == S_OK) 
										{
											LPWSTR pszName;
											if (SUCCEEDED(psi->GetDisplayName(SIGDN_FILESYSPATH,
																  &pszName))) 
											{
												//MessageBox(NULL,pszName, L"FILENAME", MB_OK);
												wcstombs(buffer, pszName, sizeof(buffer));
												doBatchProcess(buffer);
												

												/*set_current_filename(buffer);
												set_filename_base(buffer);
												set_original_filename(buffer);
												threadArgs.mode = WHOLE;
												executeFullProcess(&threadArgs);*/
												CoTaskMemFree(pszName);
											}
											psi->Release();
										}
										pesi->Release();
										hr = S_OK;
										//matchFoodItems(/*meal_id*/ meal_id, /* grouptype */ BEFORE);
									}
								}                    // Do something with the resulting IShellItemArray
								psiaResults->Release();
							}
						}
					}
					pfd->Release();
				}
			}
			break;
		case ID_FILE_OPENSINGLEFILE:
			{
				DWORD dwOptions;
				IFileOpenDialog *pfd;
			    
				// CoCreate the dialog object.
				HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, 
											  NULL, 
											  CLSCTX_INPROC_SERVER, 
											  IID_PPV_ARGS(&pfd));

				if (SUCCEEDED(hr))
				{
					// Specify multiselect.
					hr = pfd->GetOptions(&dwOptions);
					hr = pfd->SetFileTypes(2, rgSpec);

				if (SUCCEEDED(hr))
					{
						hr = pfd->SetOptions(dwOptions | FOS_ALLOWMULTISELECT);
					}

					if (SUCCEEDED(hr))
					{
						// Show the Open dialog.
						hr = pfd->Show(threadArgs.hWnd);
					
						if (SUCCEEDED(hr))
						{
							// Obtain the result of the user interaction.
							IShellItemArray *psiaResults;
							hr = pfd->GetResults(&psiaResults);
			                
							if (SUCCEEDED(hr))
							{
					//			int meal_id;
					//			meal_id = createMealRecords(1/*person ID*/, BEFORE);
								if (psiaResults) 
								{
									IEnumShellItems *pesi;
									if (SUCCEEDED(hr = psiaResults->EnumItems(&pesi))) 
									{
										char buffer[MAX_PATH];
										IShellItem *psi;
										while (pesi->Next(1, &psi, NULL) == S_OK) 
										{
											LPWSTR pszName;
											if (SUCCEEDED(psi->GetDisplayName(SIGDN_FILESYSPATH,
																  &pszName))) 
											{
//												MessageBox(NULL,pszName, L"FILENAME", MB_OK);
												wcstombs(buffer, pszName, sizeof(buffer));
												//doBatchProcess(buffer);

												set_current_filename(buffer);
												set_filename_base(buffer);
												set_original_filename(buffer);
												threadArgs.mode = WHOLE;
												mbstowcs(w_current_file, get_current_filename(), strlen(get_current_filename())+1);
												::InvalidateRect(hWnd,NULL, TRUE);

												//executeFullProcess(&threadArgs);
												CoTaskMemFree(pszName);
											}
											psi->Release();
										}
										pesi->Release();
										hr = S_OK;
										//matchFoodItems(/*meal_id*/ meal_id, /* grouptype */ BEFORE);
									}
								}                    // Do something with the resulting IShellItemArray
								psiaResults->Release();
							}
						}
					}
					pfd->Release();
				}
			}
			break;
		case ID_FILE_OPEN:
			{
				DWORD dwOptions;
				IFileOpenDialog *pfd;
			    
				// CoCreate the dialog object.
				HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, 
											  NULL, 
											  CLSCTX_INPROC_SERVER, 
											  IID_PPV_ARGS(&pfd));
				if (SUCCEEDED(hr))
				{
					// Specify multiselect.
					hr = pfd->GetOptions(&dwOptions);
					hr = pfd->SetFileTypes(2, rgSpec);

					if (SUCCEEDED(hr))
					{
						hr = pfd->SetOptions(dwOptions | FOS_ALLOWMULTISELECT);
					}

					if (SUCCEEDED(hr))
					{
						// Show the Open dialog.
						hr = pfd->Show(threadArgs.hWnd);
					
						if (SUCCEEDED(hr))
						{
							// Obtain the result of the user interaction.
							IShellItemArray *psiaResults;
							hr = pfd->GetResults(&psiaResults);
			                
							if (SUCCEEDED(hr))
							{
								int meal_id;
								int grouptype_meal_id;
								SQL_TIMESTAMP_STRUCT start, end;
								struct tm *newtime;
							 	__int64 ltime;
							    _time64( &ltime );
							    // Obtain coordinated universal time:
							    newtime = _gmtime64( &ltime ); // C4996
								start.hour = end.hour = newtime->tm_hour;
								start.minute = end.minute = newtime->tm_min;
								start.second = end.second = newtime->tm_sec;
								start.day = end.day = newtime->tm_mday;
								start.month = end.month = newtime->tm_mon+1;
								start.year = end.year = newtime->tm_year + 1900;
								start.fraction = end.fraction = 0;
								end.minute += 1;
								while(end.minute >= 60 )
								{
									start.minute -= 2;
									end.minute -= 1;
								}
								
								meal_id = create_meal(1, &start, &end);
								grouptype_meal_id = create_grouptypes_meals(meal_id, BEFORE);
								set_current_grouptype_meal(grouptype_meal_id);

								if (psiaResults) 
								{
									IEnumShellItems *pesi;
									if (SUCCEEDED(hr = psiaResults->EnumItems(&pesi))) 
									{
										char buffer[MAX_PATH];
										IShellItem *psi;
										while (pesi->Next(1, &psi, NULL) == S_OK) 
										{
											LPWSTR pszName;
											if (SUCCEEDED(psi->GetDisplayName(SIGDN_FILESYSPATH,
																  &pszName))) 
											{
												HANDLE hThread;
												//MessageBox(NULL,pszName, L"FILENAME", MB_OK);
												wcstombs(buffer, pszName, sizeof(buffer));
												set_current_filename(buffer);
												set_filename_base(buffer);
												set_original_filename(buffer);
												threadArgs.mode = WHOLE;

												hThread = executeFullProcess(&threadArgs);
												CloseHandle( hThread );
												OutputDebugStringW(L"File: ");
												OutputDebugStringW(pszName);
												OutputDebugStringW(L"\r\n");
												CoTaskMemFree(pszName);
											}
											psi->Release();
										}
										pesi->Release();
										hr = S_OK;
										matchFoodItems(/*meal_id*/ meal_id, /* grouptype */ BEFORE);
									}
								}                    // Do something with the resulting IShellItemArray
								psiaResults->Release();
							}
						}
					}
					pfd->Release();
				}
			}
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

	case WM_PAINT:
		{
			Gdiplus::Image *image;
//			int size_mag;
			hdc = BeginPaint(hWnd, &ps);
			// TODO: Add any drawing code here...
			Gdiplus::Graphics graphics(hdc); //Handle to the device context
			//Load the image from a file
			image = Gdiplus::Image::FromFile(w_current_file);
			//mbstowcs(w_current_file, get_current_filename(), strlen(get_current_filename())+1);
			//image = Gdiplus::Image::FromFile(w_current_file);
				//size_mag = image.GetWidth()/1024;
			if( image != NULL )
				graphics.DrawImage(image, 0, 0, (int)(image->GetWidth()*mag_factor), (int)(image->GetHeight()*mag_factor));
			
			EndPaint(hWnd, &ps);
			delete image;
		}
		break;
	case WM_DESTROY:
        PostQuitMessage(0);  
		shutdown_db();
		DefWindowProc(hWnd, message, wParam, lParam);  

		//DefWindowProc(hWnd, message, wParam, lParam);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for preference box.
INT_PTR CALLBACK Preferences(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}



int save_preferences(HWND hDlg)
{
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}


LRESULT CALLBACK NewWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_SIZE:
			RECT /*rcTool,*/ rcStatus, rcClient;
			int  iStatusHeight, iToolHeight, iEditHeight;
			
			//Get height of toolbar
//			GetClientRect(GetParent(hwnd), &rcClient);
//			GetWindowRect(GetDlgItem(GetParent(hwnd), IDC_MAIN_TOOL), &rcTool);
//			iToolHeight = rcTool.bottom - rcTool.top;
			iToolHeight = 0;

			//Get height of StatusBar
			GetWindowRect(hWndStatus, &rcStatus);
			iStatusHeight = rcStatus.bottom - rcStatus.top;
			
			// Calculate remaining height and size edit
			GetClientRect(GetParent(hwnd), &rcClient);
//			iEditHeight = rcClient.bottom - iToolHeight - iStatusHeight;
			iEditHeight = rcClient.bottom - iStatusHeight;
      //char szText[25];
      //sprintf(szText, "iToolHeight : %d and iEditHeight : %d", iToolHeight, iEditHeight);
      //MessageBox(NULL, szText, "params", 1);
			SetWindowPos(hwnd, NULL, 0, iToolHeight, rcClient.right, iEditHeight, SWP_NOZORDER);
			break;
    default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
			break;
	}
	return 0;
}

