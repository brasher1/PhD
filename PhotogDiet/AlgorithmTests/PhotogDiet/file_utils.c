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

#include <io.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <share.h>

__time64_t get_file_create_time( char *filename)
{
   struct _stat buf;
   int fd, result;
   char buffer[] = "A line to output";
   char timebuf[26];
   char buffer2[1024];
   errno_t err;

   fd = _sopen( filename,
			_O_RDONLY,
             _SH_DENYNO,0);
   if( fd != -1 )
   {}//   _write( fd, buffer, strlen( buffer ) );
   else
   {
	   _get_errno(&err);
	switch( err)
	{
	case EACCES:
		MessageBox(NULL,L"Given path is a directory, or file is read-only, but an open-for-writing operation was attempted.",L"ERROR", MB_ICONHAND);
		break;
	case EEXIST:
	    MessageBox(NULL, L"_O_CREAT and _O_EXCL flags were specified, but filename already exists.",L"ERROR", MB_ICONHAND);
		break;
	case EINVAL:
	    MessageBox(NULL, L"Invalid oflag or shflag argument.",L"ERROR", MB_ICONHAND);
		break;
	case EMFILE:
		MessageBox(NULL, L"No more file descriptors available.",L"ERROR", MB_ICONHAND);
		break;
	case ENOENT:
		MessageBox(NULL, L"File or Path not Found",L"ERROR", MB_ICONHAND);
		break;
	default:
		MessageBox(NULL, L"Unknown Error", L"ERROR", MB_ICONHAND);
		break;
	}
   }
   // Get data associated with "fd": 
   result = _fstat( fd, &buf );

   // Check if statistics are valid: 
   if( result != 0 )
   {
      if (errno == EBADF)
        printf( "Bad file descriptor.\n" );
      else if (errno == EINVAL)
        printf( "Invalid argument to _fstat.\n" );
   }
   else
   {
      printf( "File size     : %ld\n", buf.st_size );
      err = ctime_s(timebuf, 26, &buf.st_ctime);
      if (err)
      {
         printf("Invalid argument to ctime_s.");
         exit(1);
      }
      printf( "Time created : %s", timebuf );
      sprintf( buffer2, "Time created : %s", timebuf );
   }
   _close( fd );
   return buf.st_ctime;
}
