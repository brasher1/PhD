// UpWrite.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <process.h>

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include <commctrl.h>
#include <crtdbg.h>

#include <Shobjidl.h>



int recognise(char *filename, char *orgfilename);

void initialize_db();
int load_preferences(void);
int shutdown_db();
void input_classes(char *);
void input_ellipse_classes(char *);
int set_current_image(int imageid);

int _tmain(int argc, _TCHAR * argv[])
{
	char execpath[MAX_PATH];
	int i;
	char currentfile[MAX_PATH];
	char originalfile[MAX_PATH];
	char temp[128];

	initialize_db();


	printf("ARGC = %d\n", argc);

	if( argc == 5 )
	{
		wcstombs(execpath,argv[1], sizeof(execpath));
		wcstombs(currentfile,argv[2], sizeof(currentfile));
		wcstombs(originalfile,argv[3], sizeof(originalfile));
		wcstombs(temp,argv[4], sizeof(temp));
		set_current_image(atoi(temp));
			
		load_preferences();
		input_ellipse_classes(execpath);
		
		recognise(currentfile, originalfile);
	}
	else
	{
		for(i=0; i<argc; i++)
		{
			wprintf(L"ARGV[%d] = %S\n", i, (wchar_t *)argv[i]);
		}
	}
	shutdown_db();	
	return 0;
}

