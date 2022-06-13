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

#include <windows.h>
#include <commctrl.h>

//extern HWND hWndStatus;

int send_statusbar_message(int index, wchar_t *str )
{
//	SendMessage(hWndStatus, SB_SETTEXT, index, str);
	return 0;
}
