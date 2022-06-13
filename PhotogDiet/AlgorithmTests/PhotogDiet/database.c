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
#include <windows.h>
#include <sql.h>
#include <sqltypes.h>
#include <sqlext.h>
#include <time.h>

#include "geometry.h"
#include "photog.h"

#include "file_utils.h"
int delete_blobpoint_from_photoblob(int photoblob);

int current_image, current_monumentpoint, current_photoblob, current_region, current_regionpoint, monument_point_count, volume_point_count;
int current_photoblob, current_blobpoint, current_blobpoint_count, current_ray, current_volume, current_volumepoint, current_fooditem;
int	current_meal, current_grouptype_meal, current_person;
int meal_blob_count;

int placematmap_id;
int upwrite_radius;
char qhull_path[MAX_PATH];

char current_filename[MAX_PATH];
char original_filename[MAX_PATH];
char filename_base[MAX_PATH];
char monument_point_filename[MAX_PATH];

SQLHENV env;
SQLHDBC dbc;
// SQLBindParameter_Function.cpp
// compile with: ODBC32.lib

#define EMPLOYEE_ID_LEN 10

SQLHENV henv = NULL;
SQLPOINTER rgbValue;
SQLRETURN retcode;
SQLHSTMT hstmt = NULL;
SQLHSTMT stmt = NULL;
SQLSMALLINT sCustID;

SQLCHAR szEmployeeID[EMPLOYEE_ID_LEN];
SQL_DATE_STRUCT dsOrderDate;
SQLINTEGER cbCustID = 0, cbVarType = SQL_NTS;


int load_preferences()
{
	strcpy( qhull_path, "E:\\qhull-2010.1\\project\\Debug");
	placematmap_id = 23;
	upwrite_radius = 7;

	return 0;
}

int set_current_grouptype_meal(int id)
{
	current_grouptype_meal = id;
	return 0;
}
int get_current_grouptype_meal()
{
	return current_grouptype_meal;
}

int get_current_image()
{
	return current_image;
}
int set_current_image(int imageid)
{
	current_image = imageid;
	return imageid;
}
int get_monument_point_count()
{
	return monument_point_count;
}
int get_volume_point_count()
{
	return volume_point_count;
}
int get_meal_blob_count()
{
	return meal_blob_count;
}
int get_current_photoblob()
{
	return current_photoblob;
}
int get_current_blobpoint_count()
{
	return current_blobpoint_count;
}

void extract_error(
    char *fn,
    SQLHANDLE handle,
    SQLSMALLINT type)
{
    SQLSMALLINT	 i = 0;
    SQLINTEGER	 native;
    SQLWCHAR	 state[ 16 ];
    SQLWCHAR	 text[512];
    SQLSMALLINT	 len;
    SQLRETURN	 ret;
			char buffer[1024];
			char buff4[1024];
			wchar_t buff2[1024];

    do
    {
        ret = SQLGetDiagRec(type, handle, ++i, state, &native, text,
                            sizeof(text), &len );

        if (SQL_SUCCEEDED(ret) /*&& wcscmp(state, L"01000") != 0*/ )
		{
			/*fprintf(stderr,
					"\n"
					"The driver reported the following diagnostics whilst running "
					"%s\n\n",
					fn);*/

			mbstowcs(buff2, fn, sizeof(buff2));
			wcstombs(buff4, text, sizeof(buff4));
			sprintf(buffer, "%s: %s:%ld:%ld:%s\n", fn, state, i, native, buff4);
			fwprintf(stderr, TEXT("%S: %S:%ld:%ld:%S\n"), buff2, state, i, native, text);
			mbstowcs(buff2, buffer, sizeof(buff2));
			OutputDebugString(buff2);
			//MessageBox(NULL, buff2, TEXT("ODBC ERROR"), MB_OK);
		}

    }
    while( ret == SQL_SUCCESS );
}



void initialize_db()
{
//  SQLHSTMT stmt;
  SQLRETURN ret; /* ODBC API return status */
  SQLWCHAR outstr[1024];
  SQLSMALLINT outstrlen;
  monument_point_count = 0;

  /* Allocate an environment handle */
  SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
  /* We want ODBC 3 support */
  SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void *) SQL_OV_ODBC3, 0);
  /* Allocate a connection handle */
  SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc);
  /* Connect to the DSN mydsn */
  ret = SQLDriverConnect(dbc, NULL, TEXT("DSN=PhotogDiet;"), SQL_NTS,
			 outstr, sizeof(outstr), &outstrlen,
			 SQL_DRIVER_COMPLETE);
  if (SQL_SUCCEEDED(ret)) {
    printf("Connected\n");
    printf("Returned connection string was:\n\t%s\n", outstr);
    if (ret == SQL_SUCCESS_WITH_INFO) {
      printf("Driver reported the following diagnostics\n");
      extract_error("Driver reported the following diagnostics\\nSQLDriverConnect", dbc, SQL_HANDLE_DBC);
    }
  } 
  else {
    fprintf(stderr, "Failed to connect\n");
	MessageBox(NULL,L"Failed to connect to database",L"Error", MB_ICONHAND);
    extract_error("SQLDriverConnect", dbc, SQL_HANDLE_DBC);
  }	
}

void shutdown_db()
{
  SQLDisconnect(dbc);		/* disconnect from driver */
  /* free up allocated handles */
  SQLFreeHandle(SQL_HANDLE_DBC, dbc);
  SQLFreeHandle(SQL_HANDLE_ENV, env);
}

void set_current_filename(char *name)
{
	strcpy(current_filename, name);
}

void set_filename_base(char *name)
{
	strcpy(filename_base, name);
}

char * get_filename_base()
{
	return &filename_base[0];
}

char *get_current_filename()
{
	return &current_filename[0];
}
void set_original_filename(char *name)
{
	strcpy(original_filename, name);
}

char *get_original_filename()
{
	return &original_filename[0];
}

void set_monument_point_filename(char *name)
{
	strcpy(monument_point_filename, name);
}

char *get_monument_point_filename()
{
	return &monument_point_filename[0];
}


int create_image(char *filename)
{
   __time64_t creation_time;
      struct tm *t;
    char buffer[1024];
	char buffer2[64];
	wchar_t buff[1024];

   SQLWCHAR wfilename_field[MAX_PATH];
   SQLCHAR filename_field[MAX_PATH];
   SQLINTEGER indicator;
           char buf[512];
	SQL_TIMESTAMP_STRUCT create_time;

   int camera;

   // TODO: WARNING:  ONLY ONE CAMERA AT THIS TIME.  This will have to be reworked later.
   //camera = 1;
   camera = 2;

   if( strlen(filename) == 0 )
   {
	   MessageBox(NULL,L"FILENAME is NULL", L"ERROR-CreateImage", MB_ICONHAND);
	   return -1;
   }
   strcpy(filename_field, filename);
   creation_time = get_file_create_time(filename);
   t = _gmtime64(&creation_time);
   create_time.day = t->tm_mday;
   create_time.month = t->tm_mon;
   create_time.year = t->tm_year;
   create_time.hour = t->tm_hour;
   create_time.minute = t->tm_min;
   create_time.second = t->tm_sec;
   create_time.fraction = 0;

   strftime(buffer2,sizeof(buffer), "%Y-%m-%d %H:%M:%S", t);
   
   mbstowcs(wfilename_field, filename, strlen(filename));

   retcode = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &hstmt);
   sprintf(buffer, "INSERT INTO Images(filename, grouptypes_meals_id, cameracalibration_id, created_on) VALUES (\'%s\', %d, %d, \'%s\')", 
	   filename, get_current_grouptype_meal(), camera, buffer2);
   mbstowcs(buff, buffer, sizeof(buff));

   retcode = SQLPrepare(hstmt, buff, SQL_NTS);

   retcode = SQLExecute(hstmt);
///////////////////////////////////////////////////////////////////////
   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("SELECT @@IDENTITY"), SQL_NTS);
   retcode = SQLExecute(hstmt);
   retcode = SQLFetch(hstmt);	
   retcode = SQLGetData(hstmt, 1, SQL_C_CHAR,
                         buf, sizeof(buf), &indicator);
	current_image = atoi(buf);
   SQLFreeHandle(SQL_HANDLE_STMT, &hstmt);

   return 0;
}

int create_monumentpoint(int photo_x, int photo_y, double world_x, double world_y, double world_z, int dummy, double shading)
{
	SQLINTEGER photox, photoy;
	SQLDOUBLE worldx, worldy, worldz;
    SQLINTEGER indicator;
	char buf[256];
	indicator = 0;
	photox = photo_x; photoy = photo_y; 
	worldx = world_x; worldy = world_y; worldz = world_z;
    
   retcode = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &hstmt);
   if( retcode != 0 )
	   MessageBox(NULL,L"Allochandle", L"ERROR", MB_OK);

   retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &photox, 0, &indicator);
   retcode = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &photoy, 0, &indicator);
   retcode = SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &worldx, 0, &indicator);
   retcode = SQLBindParameter(hstmt, 4, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &worldy, 0, &indicator);
   retcode = SQLBindParameter(hstmt, 5, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &worldz, 0, &indicator);
   retcode = SQLBindParameter(hstmt, 6, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &current_image, 0, &indicator);
   retcode = SQLBindParameter(hstmt, 7, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &shading, 0, &indicator);

   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("INSERT INTO MonumentPoints(photo_x, photo_y, world_x, world_y, world_z, image_id, shading) VALUES (?,?,?,?,?,?,?)"), SQL_NTS);
//   if( retcode != 0 )
//	   MessageBox(NULL,L"Prepare", L"ERROR", MB_OK);
   extract_error("Prepare Monument Point", dbc, SQL_HANDLE_DBC);

   retcode = SQLExecute(hstmt);
//   if( retcode != 0 )
//	   MessageBox(NULL,L"Execute", L"ERROR", MB_OK);

   extract_error("Create Monument Point", dbc, SQL_HANDLE_DBC);
	
   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("SELECT @@IDENTITY"), SQL_NTS);
   retcode = SQLExecute(hstmt);
   retcode = SQLFetch(hstmt);	
   retcode = SQLGetData(hstmt, 1, SQL_C_CHAR,
                         buf, sizeof(buf), &indicator);
   current_monumentpoint = atoi(buf);

   SQLFreeHandle(SQL_HANDLE_STMT, &hstmt);
   return current_monumentpoint;
}

int create_edgepoint(int photo_x, int photo_y, double world_x, double world_y, double world_z)
{
	SQLINTEGER photox, photoy;
	SQLDOUBLE worldx, worldy, worldz;
    SQLINTEGER indicator;
	indicator = 0;
	photox = photo_x; photoy = photo_y; 
	worldx = world_x; worldy = world_y; worldz = world_z;

   retcode = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &hstmt);

   retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &photox, 0, &indicator);
   retcode = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &photoy, 0, &indicator);
   retcode = SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &current_monumentpoint, 0, &indicator);

   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("INSERT INTO EdgePoints(photo_x, photo_y, monumentpoint_id) VALUES (?,?,?)"), SQL_NTS);
   retcode = SQLExecute(hstmt);

   SQLFreeHandle(SQL_HANDLE_STMT, &hstmt);

   return 0;
}

PWORLD_POINT get_placemat_map_coordinates(int map_id)
{
	char buffer[2048];
	wchar_t buff[2048];
	SQLSMALLINT columns; /* number of columns in result-set */
	WORLD_POINT *wp = NULL;

	int row = 0;
	
	sprintf(buffer,	"SELECT PlacematMapCoordinates.x,"
                    " PlacematMapCoordinates.y, PlacematMapCoordinates.z,"
					" PlacematMap.id as MapKey, PlacematMapPoints.id as id" 
					" FROM PlacematMap INNER JOIN"
                    " PlacematMapCoordinates ON PlacematMap.id = PlacematMapCoordinates.placematmap_id INNER JOIN"
                    " PlacematMapPoints ON PlacematMapCoordinates.placematmappoint_id = PlacematMapPoints.id"
					" where PlacematMap.id = %d and PlacematMapPoints.id=%d", placematmap_id, map_id);


   mbstowcs(buff, buffer, sizeof(buff));

   retcode = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &hstmt);

   retcode = SQLPrepare(hstmt, (SQLWCHAR*)buff, SQL_NTS);
   extract_error("SQL Prepare", dbc, SQL_HANDLE_DBC);

   retcode = SQLExecute(hstmt);
   extract_error("Execute Monument Point Get", dbc, SQL_HANDLE_DBC);

///////////////////////////////////////////////////////////////////////
	SQLNumResultCols(hstmt, &columns);
    while (SQL_SUCCEEDED(retcode = SQLFetch(hstmt))) 
	{
		SQLUSMALLINT i;
		row++;
		wp = (WORLD_POINT *)realloc(wp, row*sizeof(WORLD_POINT));
		for( i=1; i<=columns; i++)
		{
	        SQLINTEGER indicator;
	        char buf[512];
	        /* retrieve column data as a string */
			retcode = SQLGetData(hstmt, i, SQL_C_CHAR,
                         buf, sizeof(buf), &indicator);
			if( i == 1 )
				wp[row-1].x = atof(buf);
			else if ( i == 2 )
				wp[row-1].y = atof(buf);
			else if (i == 3)
				wp[row-1].z = atof(buf);
			printf("%s", buf);
		}
	}

    extract_error("SQL Fetch", dbc, SQL_HANDLE_DBC);
	
    SQLFreeHandle(SQL_HANDLE_STMT, &hstmt);

   return wp;

}

PSIMPLE_POINT get_image_monument_points(int imageid)
{
	char buffer[2048];
	wchar_t buff[2048];
	SQLSMALLINT columns; /* number of columns in result-set */
	SIMPLE_POINT *sp = NULL;

	int row = 0;
	
	if(imageid==0)
		imageid = current_image;

   sprintf(buffer, "SELECT MonumentPoints.photo_x, MonumentPoints.photo_y, MonumentPoints.shading, MonumentPoints.id, MonumentPoints.map_id FROM Images, MonumentPoints WHERE Images.id = MonumentPoints.image_id AND (MonumentPoints.image_id = %d)", imageid);
   mbstowcs(buff, buffer, sizeof(buff));

   retcode = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &hstmt);

   retcode = SQLPrepare(hstmt, (SQLWCHAR*)buff, SQL_NTS);
   extract_error("SQL Prepare", dbc, SQL_HANDLE_DBC);

   retcode = SQLExecute(hstmt);
   extract_error("Execute Monument Point Get", dbc, SQL_HANDLE_DBC);

///////////////////////////////////////////////////////////////////////
	SQLNumResultCols(hstmt, &columns);
    while (SQL_SUCCEEDED(retcode = SQLFetch(hstmt))) 
	{
		SQLUSMALLINT i;
		row++;
		sp = (SIMPLE_POINT *)realloc(sp, row*sizeof(SIMPLE_POINT));
		memset(&sp[row-1], 0, sizeof(SIMPLE_POINT));
		for( i=1; i<=columns; i++)
		{
	        SQLINTEGER indicator;
	        char buf[512];
	        /* retrieve column data as a string */
			retcode = SQLGetData(hstmt, i, SQL_C_CHAR,
                         buf, sizeof(buf), &indicator);
			if( i == 1 && indicator != -1 )
				sp[row-1].x = atoi(buf);
			else if ( i == 2 && indicator != -1 )
				sp[row-1].y = atoi(buf);
			else if (i == 3 && indicator != -1 )
				sp[row-1].shading = atoi(buf);
			else if(i==4 && indicator != -1 )
				sp[row-1].id = atoi(buf);
			else if(i==5 && indicator != -1 )
				sp[row-1].map_id = atoi(buf);
			printf("%s", buf);

		}
	}
	monument_point_count = row;

	extract_error("SQL Fetch", dbc, SQL_HANDLE_DBC);
	
   SQLFreeHandle(SQL_HANDLE_STMT, &hstmt);

   return sp;
} 
int get_camera_internal_params(CAMERA_INTERNALS *cam_int, int imageid)
{
	char buffer[2048];
	wchar_t buff[2048];
	SQLSMALLINT columns; /* number of columns in result-set */
	SIMPLE_POINT *sp = NULL;

	int row = 0;
	
	if(imageid==0)
		imageid = current_image;

   sprintf(buffer, "SELECT Images.cameracalibration_id, CameraCalibration.id, CameraCalibration.radial_dist1, CameraCalibration.radial_dist2," 
						" CameraCalibration.radial_dist3, CameraCalibration.tangential_dist1, CameraCalibration.tangential_dist2,"
						" CameraCalibration.fx, CameraCalibration.fy, CameraCalibration.cx, CameraCalibration.cy," 
						" Images.id FROM CameraCalibration INNER JOIN Images ON CameraCalibration.id = Images.cameracalibration_id"
						" where Images.id = %d", imageid);
   mbstowcs(buff, buffer, sizeof(buff));

   retcode = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &hstmt);

   retcode = SQLPrepare(hstmt, (SQLWCHAR*)buff, SQL_NTS);
   extract_error("SQL Prepare", dbc, SQL_HANDLE_DBC);

   retcode = SQLExecute(hstmt);
   extract_error("Execute Monument Point Get", dbc, SQL_HANDLE_DBC);

///////////////////////////////////////////////////////////////////////
	SQLNumResultCols(hstmt, &columns);
    while (SQL_SUCCEEDED(retcode = SQLFetch(hstmt))) 
	{
		SQLUSMALLINT i;
		row++;
		for( i=1; i<=columns; i++)
		{
	        SQLINTEGER indicator;
	        char buf[512];
	        /* retrieve column data as a string */
			retcode = SQLGetData(hstmt, i, SQL_C_CHAR,
                         buf, sizeof(buf), &indicator);
			if( i == 3 && indicator != -1 )
				cam_int->k1 = (float)atof(buf);
			else if ( i == 4 && indicator != -1 )
				cam_int->k2 = (float)atof(buf);
			else if (i == 5 && indicator != -1 )
				cam_int->k3 = (float)atof(buf); 
			else if(i==6 && indicator != -1 )
				cam_int->p1 = (float)atof(buf);
			else if(i==7 && indicator != -1 )
				cam_int->p2 = (float)atof(buf);
			else if(i==8 && indicator != -1 )
				cam_int->fx = (float)atof(buf);
			else if(i==9 && indicator != -1 )
				cam_int->fy = (float)atof(buf);
			else if(i==10 && indicator != -1 )
				cam_int->cx = (float)atof(buf);
			else if(i==11 && indicator != -1 )
				cam_int->cy = (float)atof(buf);

			printf("%s", buf);

		}
	}

   extract_error("SQL Fetch", dbc, SQL_HANDLE_DBC);
	
   SQLFreeHandle(SQL_HANDLE_STMT, &hstmt);

   return 0;
} 
PPLACEMAT_POINT get_placemat_points(int imageid)
{
	char buffer[2048];
	wchar_t buff[2048];
	SQLSMALLINT columns; /* number of columns in result-set */
	PLACEMAT_POINT *sp = NULL;

	int row = 0;
	
	if(imageid==0)
		imageid = current_image;

   sprintf(buffer, "SELECT MonumentPoints.photo_x, MonumentPoints.photo_y, MonumentPoints.shading, MonumentPoints.id, "
	   "MonumentPoints.world_x, MonumentPoints.world_y, MonumentPoints.world_z, MonumentPoints.world_true, "
	   "MonumentPoints.x_diameter, MonumentPoints.y_diameter, MonumentPoints.map_id FROM Images, MonumentPoints "
	   "WHERE Images.id = MonumentPoints.image_id AND (MonumentPoints.image_id = %d)", imageid);
   mbstowcs(buff, buffer, sizeof(buff));

   retcode = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &hstmt);

   retcode = SQLPrepare(hstmt, (SQLWCHAR*)buff, SQL_NTS);
   extract_error("SQL Prepare", dbc, SQL_HANDLE_DBC);

   retcode = SQLExecute(hstmt);
   extract_error("Execute Monument Point Get", dbc, SQL_HANDLE_DBC);

///////////////////////////////////////////////////////////////////////
	SQLNumResultCols(hstmt, &columns);
    while (SQL_SUCCEEDED(retcode = SQLFetch(hstmt))) 
	{
		SQLUSMALLINT i;
		row++;
		sp = (PLACEMAT_POINT *)realloc(sp, row*sizeof(PLACEMAT_POINT));
		for( i=1; i<=columns; i++)
		{
	        SQLINTEGER indicator;
	        char buf[512];
	        /* retrieve column data as a string */
			retcode = SQLGetData(hstmt, i, SQL_C_CHAR,
                         buf, sizeof(buf), &indicator);
			switch(i)
			{
			case 1:
				sp[row-1].photox = atoi(buf);
				break;
			case 2:
				sp[row-1].photoy = atoi(buf);
				break;
			case 3:
				sp[row-1].shading = atoi(buf);
				break;
			case 4:
				sp[row-1].id = atoi(buf);
				break;
			case 5:
				sp[row-1].worldx = atof(buf);
				break;
			case 6:
				sp[row-1].worldy = atof(buf);
				break;
			case 7:
				sp[row-1].worldz = atof(buf);
				break;
			case 8:
				sp[row-1].world_true = atoi(buf);
				break;
			case 9:
				sp[row-1].x_diameter = atoi(buf);
				break;
			case 10:
				sp[row-1].y_diameter = atoi(buf);
				break;
			case 11:
				sp[row-1].map_id = atoi(buf);
				if( sp[row-1].world_true == 0 )
					sp[row-1].map_id = 0;
				break;


			}
			printf("%s", buf);

		}
		sp[row-1].triangle_part = 0;
		sp[row-1].quad_part = 0;
	}
	monument_point_count = row;

	extract_error("SQL Fetch", dbc, SQL_HANDLE_DBC);
	
   SQLFreeHandle(SQL_HANDLE_STMT, &hstmt);

   return sp;
} 

int update_3D_coordinates(int monument_point_id, int map_id )
{
	WORLD_POINT *wp;
//	SQLINTEGER photox, photoy;
	SQLDOUBLE worldx, worldy, worldz;
    SQLINTEGER indicator;
	char buffer[2048];
	wchar_t buff[2048];

	indicator = 0;
	if( map_id < 0 || map_id > 22 )
		return -1;
	wp = get_placemat_map_coordinates(map_id);
	if( wp == NULL )
	{
		sprintf(buffer, "No MAP Coordinates\nMonPtID=%d, MapID=%d", monument_point_id, map_id);
		mbstowcs(buff, buffer, sizeof(buff));
		MessageBox(NULL, buff, L"ERROR", MB_OK);
	
		return -1;
	}
	worldx = wp->x;
	worldy = wp->y;
	worldz = wp->z;
	current_monumentpoint= monument_point_id;

	sprintf(buffer, "UPDATE MonumentPoints SET world_x=%f, world_y=%f, world_z=%f, world_true=1, map_id=%d where MonumentPoints.id = %d", worldx, worldy, worldz, map_id, monument_point_id);
    mbstowcs(buff, buffer, sizeof(buff));

   retcode = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &hstmt);

   retcode = SQLPrepare(hstmt, (SQLWCHAR*)buff, SQL_NTS);
   extract_error("Prepare Monument Point", dbc, SQL_HANDLE_DBC);

   retcode = SQLExecute(hstmt);
   extract_error("Create Monument Point", dbc, SQL_HANDLE_DBC);
	if( retcode != 0 )
	{
#ifdef VERBOSE_INSTRUMENTATION
		sprintf(buffer, "Update MonumentPoint did not work\nMonPtID=%d, MapID=%d", monument_point_id, map_id);
		mbstowcs(buff, buffer, sizeof(buff));
		MessageBox(NULL, buff, L"ERROR", MB_OK);
#endif //VERBOSE_INSTRUMENTATION
	}
   SQLFreeHandle(SQL_HANDLE_STMT, &hstmt);

   free(wp);

   return 0;

}
int update_shading_factor(int monument_point_id, int shading_factor )
{
    SQLINTEGER indicator;
	char buffer[2048];
	wchar_t buff[2048];

	indicator = 0;

	current_monumentpoint= monument_point_id;

	sprintf(buffer, "UPDATE MonumentPoints SET shading=%d where MonumentPoints.id = %d", shading_factor, monument_point_id);
    mbstowcs(buff, buffer, sizeof(buff));

   retcode = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &hstmt);

   retcode = SQLPrepare(hstmt, (SQLWCHAR*)buff, SQL_NTS);
   extract_error("Prepare Monument Point", dbc, SQL_HANDLE_DBC);

   retcode = SQLExecute(hstmt);
   extract_error("Create Monument Point", dbc, SQL_HANDLE_DBC);
  
   SQLFreeHandle(SQL_HANDLE_STMT, &hstmt);
   return 0;

}
int update_monumentpoint_diameter(int monument_point_id, int x, int y)
{
    SQLINTEGER indicator;
	char buffer[2048];
	wchar_t buff[2048];

	indicator = 0;

	current_monumentpoint= monument_point_id;

	sprintf(buffer, "UPDATE MonumentPoints SET x_diameter=%d, y_diameter=%d where MonumentPoints.id = %d", x, y, monument_point_id);
    mbstowcs(buff, buffer, sizeof(buff));

   retcode = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &hstmt);

   retcode = SQLPrepare(hstmt, (SQLWCHAR*)buff, SQL_NTS);
   extract_error("Prepare Monument Point", dbc, SQL_HANDLE_DBC);

   retcode = SQLExecute(hstmt);
   extract_error("Create Monument Point", dbc, SQL_HANDLE_DBC);

   SQLFreeHandle(SQL_HANDLE_STMT, &hstmt);
   return 0;

}

int delete_monument_point(int monument_point_id)
{
    SQLINTEGER indicator;
	char buffer[2048];
	wchar_t buff[2048];

	indicator = 0;

	current_monumentpoint= monument_point_id;

	sprintf(buffer, "delete from MonumentPoints where MonumentPoints.id = %d", monument_point_id);
    mbstowcs(buff, buffer, sizeof(buff));

   retcode = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &hstmt);

   retcode = SQLPrepare(hstmt, (SQLWCHAR*)buff, SQL_NTS);
   extract_error("Prepare Monument Point", dbc, SQL_HANDLE_DBC);

   retcode = SQLExecute(hstmt);
   extract_error("Create Monument Point", dbc, SQL_HANDLE_DBC);

   SQLFreeHandle(SQL_HANDLE_STMT, &hstmt);
   return 0;

}
int update_camera_location(int id, float r11, float r12, float r13, float r21, float r22, float r23, float r31, float r32, float r33, 
						   float t1, float t2, float t3, float cam_x, float cam_y, float cam_z)
{

	char buffer[2048];
	wchar_t buff[2048];
    
    retcode = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &hstmt);

    sprintf(buffer, "UPDATE Images SET r11=%f, r12=%f, r13=%f, r21=%f, r22=%f, r23=%f, r31=%f, r32=%f, r33=%f, t1=%f, t2=%f, t3=%f, "
					"camera_optical_center_x=%f, camera_optical_center_y=%f, camera_optical_center_z = %f where Images.id = %d", 
		r11, r12, r13, r21, r22, r23, r31, r32, r33, t1, t2, t3, cam_x, cam_y, cam_z, id);

    mbstowcs(buff, buffer, sizeof(buff));

    retcode = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &hstmt);


   retcode = SQLPrepare(hstmt, (SQLWCHAR*)buff, SQL_NTS);
   extract_error("Update Camera Location", dbc, SQL_HANDLE_DBC);

   retcode = SQLExecute(hstmt);
   extract_error("Update Camera Location", dbc, SQL_HANDLE_DBC);
   SQLFreeHandle(SQL_HANDLE_STMT, &hstmt);
   return 0;
	
}

/*
SQLNumResultCols(stmt, &columns);
/* Loop through the rows in the result-set
while (SQL_SUCCEEDED(ret = SQLFetch(stmt))) {
    SQLUSMALLINT i;
    printf("Row %d\n", row++);
    /* Loop through the columns 
    for (i = 1; i <= columns; i++) {
        SQLINTEGER indicator;
        char buf[512];
        /* retrieve column data as a string 
	ret = SQLGetData(stmt, i, SQL_C_CHAR,
                         buf, sizeof(buf), &indicator);
        if (SQL_SUCCEEDED(ret)) {
            /* Handle null columns 
            if (indicator == SQL_NULL_DATA) strcpy(buf, "NULL");
	    printf("  Column %u : %s\n", i, buf);
        }
    }
}
*/

int create_photoblob(int image_id, char *name, double confidence, double center_x, double center_y)
{
	SQLINTEGER len;
//	SQLDOUBLE worldx, worldy, worldz;
    SQLINTEGER indicator;
	char buf[256];
	indicator = 0;
    
	if(name != NULL )
		len = (SQLINTEGER)strlen(name);
	else
		return -1;

   retcode = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &hstmt);

   retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &image_id, 0, &indicator);
   retcode = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, (SQLINTEGER)len, 0, (void *)name, (SQLINTEGER)len, 0);
   retcode = SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &confidence, 0, &indicator);
   retcode = SQLBindParameter(hstmt, 4, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &center_x, 0, &indicator);
   retcode = SQLBindParameter(hstmt, 5, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &center_y, 0, &indicator);

   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("INSERT INTO PhotoBlobs(image_id, name, confidence, center_x, center_y) VALUES (?,?,?,?,?)"), SQL_NTS);
   extract_error("Prepare Photo Blob", dbc, SQL_HANDLE_DBC);

   retcode = SQLExecute(hstmt);
   extract_error("Create Photo Blob", dbc, SQL_HANDLE_DBC);
	
   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("SELECT @@IDENTITY"), SQL_NTS);
   retcode = SQLExecute(hstmt);
   retcode = SQLFetch(hstmt);	
   retcode = SQLGetData(hstmt, 1, SQL_C_CHAR,
                         buf, sizeof(buf), &indicator);
   current_photoblob = atoi(buf);

   SQLFreeHandle(SQL_HANDLE_STMT, &hstmt);
   return current_photoblob;

//   return 0;
}

int create_blobpoint(int photoblob_id, int photo_x, int photo_y)
{
//	SQLINTEGER len;
//	SQLDOUBLE worldx, worldy, worldz;
    SQLINTEGER indicator;
	char buf[256];
	indicator = 0;
    
   retcode = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &hstmt);

   retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &photoblob_id, 0, &indicator);
   retcode = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &photo_x, 0, &indicator);
   retcode = SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &photo_y, 0, &indicator);

   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("INSERT INTO BlobPoints(photoblob_id, photo_x, photo_y) VALUES (?,?,?)"), SQL_NTS);
   extract_error("Prepare Blob Point", dbc, SQL_HANDLE_DBC);

   retcode = SQLExecute(hstmt);
   extract_error("Create Blob Point", dbc, SQL_HANDLE_DBC);
	
   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("SELECT @@IDENTITY"), SQL_NTS);
   retcode = SQLExecute(hstmt);
   retcode = SQLFetch(hstmt);	
   retcode = SQLGetData(hstmt, 1, SQL_C_CHAR,
                         buf, sizeof(buf), &indicator);
   current_blobpoint = atoi(buf);

   SQLFreeHandle(SQL_HANDLE_STMT, &hstmt);
   return current_blobpoint;
}
#ifdef DUMMY
int create_ray(int blobpoint_id, double a, double b, double c, int volumepoint_id)
{
//	SQLINTEGER len;
//	SQLDOUBLE worldx, worldy, worldz;
    SQLINTEGER indicator;
	char buf[256];
	indicator = 0;
 

   retcode = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &hstmt);

   retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &blobpoint_id, 0, &indicator);
   retcode = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &volumepoint_id, 0, &indicator);
   retcode = SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &a, 0, &indicator);
   retcode = SQLBindParameter(hstmt, 4, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &b, 0, &indicator);
   retcode = SQLBindParameter(hstmt, 5, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &c, 0, &indicator);
  
   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("INSERT INTO Rays(blobpoint_id, volumepoint_id, a, b, c) VALUES (?,?,?,?,?)"), SQL_NTS);
   extract_error("Prepare Ray", dbc, SQL_HANDLE_DBC);

   retcode = SQLExecute(hstmt);
   extract_error("Create Ray", dbc, SQL_HANDLE_DBC);
	
   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("SELECT @@IDENTITY"), SQL_NTS);
   retcode = SQLExecute(hstmt);
   retcode = SQLFetch(hstmt);	
   retcode = SQLGetData(hstmt, 1, SQL_C_CHAR,
                         buf, sizeof(buf), &indicator);
   current_ray = atoi(buf);

   SQLFreeHandle(SQL_HANDLE_STMT, &hstmt);
   return current_ray;
}
#endif //DUMMY

int create_volumepoint(int volume_id, double world_x, double world_y, double world_z)
{
    SQLINTEGER indicator;
	char buf[256];
	indicator = 0;

   retcode = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &hstmt);

   retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &volume_id, 0, &indicator);
   retcode = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &world_x, 0, &indicator);
   retcode = SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &world_y, 0, &indicator);
   retcode = SQLBindParameter(hstmt, 4, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &world_z, 0, &indicator);
  
   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("INSERT INTO VolumePoints(volume_id, world_x, world_y, world_z) VALUES (?,?,?,?)"), SQL_NTS);
   extract_error("Prepare Volume", dbc, SQL_HANDLE_DBC);

   retcode = SQLExecute(hstmt);
   extract_error("Create Volume", dbc, SQL_HANDLE_DBC);
	
   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("SELECT @@IDENTITY"), SQL_NTS);
   retcode = SQLExecute(hstmt);
   retcode = SQLFetch(hstmt);	
   retcode = SQLGetData(hstmt, 1, SQL_C_CHAR,
                         buf, sizeof(buf), &indicator);
   current_volumepoint = atoi(buf);

   SQLFreeHandle(SQL_HANDLE_STMT, &hstmt);
   return current_volumepoint;
}

int create_volume(double volume)
{
    SQLINTEGER indicator;
	char buf[256];
	indicator = 0;

   retcode = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &hstmt);

   retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &volume, 0, &indicator);
  
   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("INSERT INTO Volumes(volume) VALUES (?)"), SQL_NTS);
   extract_error("Prepare Volume", dbc, SQL_HANDLE_DBC);

   retcode = SQLExecute(hstmt);
   extract_error("Create Volume", dbc, SQL_HANDLE_DBC);
	
   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("SELECT @@IDENTITY"), SQL_NTS);
   retcode = SQLExecute(hstmt);
   retcode = SQLFetch(hstmt);	
   retcode = SQLGetData(hstmt, 1, SQL_C_CHAR,
                         buf, sizeof(buf), &indicator);
   current_volume = atoi(buf);

   SQLFreeHandle(SQL_HANDLE_STMT, &hstmt);
   return current_volume;
}

int update_volume(int volume_id, double volume)
{
    SQLINTEGER indicator;
	char buf[256];
	indicator = 0;

#define STDDEV				1
#define MEAN				2
#define	POINTSBELOWPLATE	3
#define	TOTALPOINTS			4
#define PLATEHEIGHT			5
#define	MAXZ				6
#define MINZ				7

#define STDMEAN63			8
#define STDMEAN52			9
#define STDMEAN41			10
#define STDMEAN30			11
#define STDMEAN62			12
#define STDMEAN51			13

#define FIVEPERCENT			14
#define	TENPERCENT			15
#define FIFTEENPERCENT		16
#define TWENTYPERCENT		17

   retcode = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &hstmt);

   retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &volume, 0, &indicator);
   retcode = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &volume_id, 0, &indicator);
  
   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("Update Volumes SET volume=? where id=?"), SQL_NTS);
   extract_error("Prepare Volume", dbc, SQL_HANDLE_DBC);

   retcode = SQLExecute(hstmt);
   extract_error("Create Volume", dbc, SQL_HANDLE_DBC);
	
   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("SELECT @@IDENTITY"), SQL_NTS);
   retcode = SQLExecute(hstmt);
   retcode = SQLFetch(hstmt);	
   retcode = SQLGetData(hstmt, 1, SQL_C_CHAR,
                         buf, sizeof(buf), &indicator);
   current_volume = atoi(buf);

   SQLFreeHandle(SQL_HANDLE_STMT, &hstmt);
   return current_volume;
}

int update_volumepoint(int volumepoint_id, int fooditem_id, double x, double y, double z)
{
    SQLINTEGER indicator;
	char buf[256];
	indicator = 0;
 

   retcode = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &hstmt);

   retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &fooditem_id, 0, &indicator);
   retcode = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &x, 0, &indicator);
   retcode = SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &y, 0, &indicator);
   retcode = SQLBindParameter(hstmt, 4, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &z, 0, &indicator);
   retcode = SQLBindParameter(hstmt, 5, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &volumepoint_id, 0, &indicator);
  
   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("Update VolumePoints SET fooditem_id=?, world_z=?, world_y=?, world_z=? where fooditem_id=?"), SQL_NTS);
   extract_error("Prepare VolumePoint", dbc, SQL_HANDLE_DBC);

   retcode = SQLExecute(hstmt);
   extract_error("Update VolumePoint", dbc, SQL_HANDLE_DBC);
	
   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("SELECT @@IDENTITY"), SQL_NTS);
   retcode = SQLExecute(hstmt);
   retcode = SQLFetch(hstmt);	
   retcode = SQLGetData(hstmt, 1, SQL_C_CHAR,
                         buf, sizeof(buf), &indicator);
   current_volumepoint = atoi(buf);

   SQLFreeHandle(SQL_HANDLE_STMT, &hstmt);
   return current_ray;
}

#ifdef DUMMY
int update_ray(int ray_id, double a, double b, double c, int volumepoint_id)
{
    SQLINTEGER indicator;
	char buf[256];
	indicator = 0;
 

   retcode = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &hstmt);

   retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &volumepoint_id, 0, &indicator);
   retcode = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &a, 0, &indicator);
   retcode = SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &b, 0, &indicator);
   retcode = SQLBindParameter(hstmt, 4, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &c, 0, &indicator);
   retcode = SQLBindParameter(hstmt, 5, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &ray_id, 0, &indicator);
  
   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("Update Rays SET volumepoint_id=?, a=?, b=?, c=? where ray_id=?"), SQL_NTS);
   extract_error("Prepare Ray", dbc, SQL_HANDLE_DBC);

   retcode = SQLExecute(hstmt);
   extract_error("Create Ray", dbc, SQL_HANDLE_DBC);
	
   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("SELECT @@IDENTITY"), SQL_NTS);
   retcode = SQLExecute(hstmt);
   retcode = SQLFetch(hstmt);	
   retcode = SQLGetData(hstmt, 1, SQL_C_CHAR,
                         buf, sizeof(buf), &indicator);
   current_ray = atoi(buf);

   SQLFreeHandle(SQL_HANDLE_STMT, &hstmt);
   return current_ray;
}
#endif //DUMMY

int update_photoblob(int photoblob_id, char *name, double confidence, double entropy, double energy, double inertia)
{
	SQLINTEGER len;
//	SQLDOUBLE worldx, worldy, worldz;
    SQLINTEGER indicator;
	char buf[256];
	indicator = 0;
    
	if(name != NULL )
		len = (SQLINTEGER)strlen(name);
	else
		return -1;

   retcode = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &hstmt);

   retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, (SQLINTEGER)len, 0, (void *)name, (SQLINTEGER)len, 0);
   retcode = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &confidence, 0, &indicator);
   retcode = SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &entropy, 0, &indicator);
   retcode = SQLBindParameter(hstmt, 4, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &energy, 0, &indicator);
   retcode = SQLBindParameter(hstmt, 5, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &inertia, 0, &indicator);
   retcode = SQLBindParameter(hstmt, 6, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &photoblob_id, 0, &indicator);

   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("Update PhotoBlobs SET name=?, confidence=?, entropy=?, energy=?, inertia=? where id=?"), SQL_NTS);
   extract_error("Prepare Photo Blob", dbc, SQL_HANDLE_DBC);

   retcode = SQLExecute(hstmt);
   extract_error("Create Photo Blob", dbc, SQL_HANDLE_DBC);
	
   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("SELECT @@IDENTITY"), SQL_NTS);
   retcode = SQLExecute(hstmt);
   retcode = SQLFetch(hstmt);	
   retcode = SQLGetData(hstmt, 1, SQL_C_CHAR,
                         buf, sizeof(buf), &indicator);
   current_photoblob = atoi(buf);

   SQLFreeHandle(SQL_HANDLE_STMT, &hstmt);
   return current_photoblob;
}

int update_blobpoint(int photoblob_id, int photo_x, int photo_y, double world_x, 
					 double world_y, double world_z, int blobpoint_id)
{
    SQLINTEGER indicator;
	char buf[256];
	indicator = 0;
    
   retcode = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &hstmt);

   retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &photo_x, 0, &indicator);
   retcode = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &photo_y, 0, &indicator);

   retcode = SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &world_x, 0, &indicator);
   retcode = SQLBindParameter(hstmt, 4, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &world_y, 0, &indicator);
   retcode = SQLBindParameter(hstmt, 5, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &world_z, 0, &indicator);

   retcode = SQLBindParameter(hstmt, 6, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &photoblob_id, 0, &indicator);
   retcode = SQLBindParameter(hstmt, 7, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &blobpoint_id, 0, &indicator);


   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("Update BlobPoints set photo_x=?, photo_y=?, projected_x=?, projected_y=?, projected_z=?, photoblob_id=? where id=?"), SQL_NTS);
   extract_error("Prepare Blob Point", dbc, SQL_HANDLE_DBC);

   retcode = SQLExecute(hstmt);
   extract_error("Update Blob Point", dbc, SQL_HANDLE_DBC);
	
   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("SELECT @@IDENTITY"), SQL_NTS);
   retcode = SQLExecute(hstmt);
   retcode = SQLFetch(hstmt);	
   retcode = SQLGetData(hstmt, 1, SQL_C_CHAR,
                         buf, sizeof(buf), &indicator);
   current_blobpoint = atoi(buf);

   SQLFreeHandle(SQL_HANDLE_STMT, &hstmt);
   return current_blobpoint;   
   
}

int create_fooditem(char *name, int grouptype_meal_id, double volume)
{

	SQLINTEGER indicator,len;
	char buf[256];
	indicator = 0;
	if(name != NULL )
		len = (SQLINTEGER)strlen(name);
	else
		return -1;

   retcode = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &hstmt);

   retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, (SQLINTEGER)len, 0, (void *)name, 0, &indicator);
   retcode = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &grouptype_meal_id, 0, &indicator);
   retcode = SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &volume, 0, &indicator);
  
   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("INSERT INTO fooditems(name, grouptype_meal_id, volume) VALUES (?,?,?)"), SQL_NTS);
   extract_error("Prepare FoodItem", dbc, SQL_HANDLE_DBC);

   retcode = SQLExecute(hstmt);
   extract_error("Create Fooditem", dbc, SQL_HANDLE_DBC);
	
   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("SELECT @@IDENTITY"), SQL_NTS);
   retcode = SQLExecute(hstmt);
   retcode = SQLFetch(hstmt);	
   retcode = SQLGetData(hstmt, 1, SQL_C_CHAR,
                         buf, sizeof(buf), &indicator);
   current_fooditem = atoi(buf);

   SQLFreeHandle(SQL_HANDLE_STMT, &hstmt);
   return current_fooditem;

}

int update_fooditem_volume(int fooditem_id, double volume, int type)
{

	SQLINTEGER indicator;//, len;
	char buf[256];
    FILE *output;
	indicator = 0;


	output = fopen("database.txt", "a+");
	fprintf(output,"ENTERING update_fooditem_volume\n");

   retcode = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &hstmt);

   retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &volume, 0, &indicator);
   retcode = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &fooditem_id, 0, &indicator);

   switch(type)
   {
   case STDDEV:
	   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("Update Fooditems set stddev=? where id=?"), SQL_NTS);
	   break;
   case MEAN:
	   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("Update Fooditems set mean=? where id=?"), SQL_NTS);
	   break;
   case POINTSBELOWPLATE:
	   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("Update Fooditems set pointsbelowplate=? where id=?"), SQL_NTS);
	   break;
   case TOTALPOINTS:		
	   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("Update Fooditems set totalpoints=? where id=?"), SQL_NTS);
	   break;
   case PLATEHEIGHT:			
	   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("Update Fooditems set plateheight=? where id=?"), SQL_NTS);
	   break;
   case MAXZ:		
	   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("Update Fooditems set maxz=? where id=?"), SQL_NTS);
	   break;
   case MINZ:				
	   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("Update Fooditems set minz=? where id=?"), SQL_NTS);
	   break;
   case STDMEAN63:
	   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("Update Fooditems set stdmean63_volume=? where id=?"), SQL_NTS);
	   break;
   case STDMEAN52:			
	   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("Update Fooditems set stdmean52_volume=? where id=?"), SQL_NTS);
	   break;
   case STDMEAN41:			
	   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("Update Fooditems set stdmean41_volume=? where id=?"), SQL_NTS);
	   break;
   case STDMEAN30:			
	   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("Update Fooditems set stdmean30_volume=? where id=?"), SQL_NTS);
	   break;
   case STDMEAN31:			
	   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("Update Fooditems set stdmean31_volume=? where id=?"), SQL_NTS);
	   break;
   case STDMEAN22:			
	   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("Update Fooditems set stdmean22_volume=? where id=?"), SQL_NTS);
	   break;
   case STDMEAN21:			
	   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("Update Fooditems set stdmean21_volume=? where id=?"), SQL_NTS);
	   break;
   case STDMEAN62:			
	   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("Update Fooditems set stdmean62_volume=? where id=?"), SQL_NTS);
	   break;
   case STDMEAN51:			
	   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("Update Fooditems set stdmean51_volume=? where id=?"), SQL_NTS);
	   break;
   case FIVEPERCENT:			
	   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("Update Fooditems set fivepercent_volume=? where id=?"), SQL_NTS);
	   break;
   case TENPERCENT:		
	   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("Update Fooditems set tenpercent_volume=? where id=?"), SQL_NTS);
	   break;
   case FIFTEENPERCENT:
	   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("Update Fooditems set fifteenpercent_volume=? where id=?"), SQL_NTS);
	   break;
   case TWENTYPERCENT:		
	   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("Update Fooditems set twentypercent_volume=? where id=?"), SQL_NTS);
	   break;
   }
   fprintf(output,"After Case Stmt: Type = %d\n", type);
/*
	[volume] [real] NULL,
	[unfiltered_volume] [real] NULL,
	[stdmeanaverage_volume] [real] NULL,

*/
//   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("Update Fooditems set volume=? where id=?"), SQL_NTS);
   extract_error("Prepare FoodItem", dbc, SQL_HANDLE_DBC);

   retcode = SQLExecute(hstmt);
   extract_error("Update FoodItem", dbc, SQL_HANDLE_DBC);
   fprintf(output,"After SQLExecute: retcode = %d", retcode);

   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("SELECT @@IDENTITY"), SQL_NTS);
   retcode = SQLExecute(hstmt);
   retcode = SQLFetch(hstmt);	
   retcode = SQLGetData(hstmt, 1, SQL_C_CHAR,
                         buf, sizeof(buf), &indicator);
   current_fooditem = atoi(buf);

   SQLFreeHandle(SQL_HANDLE_STMT, &hstmt);
   fclose(output);
   return current_fooditem;   
}

int create_person(char *name)
{
	SQLINTEGER indicator,len;
	char buf[256];
	indicator = 0;
	if(name != NULL )
		len = (SQLINTEGER)strlen(name);
	else
		return -1;

   retcode = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &hstmt);

   retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, (SQLINTEGER)len, 0, (void *)name, 0, &indicator);
  
   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("INSERT INTO people(name) VALUES (?)"), SQL_NTS);
   extract_error("Prepare Person", dbc, SQL_HANDLE_DBC);

   retcode = SQLExecute(hstmt);
   extract_error("Create Person", dbc, SQL_HANDLE_DBC);
	
   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("SELECT @@IDENTITY"), SQL_NTS);
   retcode = SQLExecute(hstmt);
   retcode = SQLFetch(hstmt);	
   retcode = SQLGetData(hstmt, 1, SQL_C_CHAR,
                         buf, sizeof(buf), &indicator);
   current_person = atoi(buf);

   SQLFreeHandle(SQL_HANDLE_STMT, &hstmt);
   return current_person;
}

int update_person(int person_id, char *name)
{

	SQLINTEGER indicator, len;
	char buf[256];
	indicator = 0;

	if(name != NULL )
		len = (SQLINTEGER)strlen(name);
	else
		return -1;
    
   retcode = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &hstmt);

   retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, (SQLINTEGER)len, 0, (void *)name, 0, &indicator);
   retcode = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &person_id, 0, &indicator);

   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("Update people set name=? where id=?"), SQL_NTS);
   extract_error("Prepare person", dbc, SQL_HANDLE_DBC);

   retcode = SQLExecute(hstmt);
   extract_error("Update person", dbc, SQL_HANDLE_DBC);
	
   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("SELECT @@IDENTITY"), SQL_NTS);
   retcode = SQLExecute(hstmt);
   retcode = SQLFetch(hstmt);	
   retcode = SQLGetData(hstmt, 1, SQL_C_CHAR,
                         buf, sizeof(buf), &indicator);
   current_person = atoi(buf);

   SQLFreeHandle(SQL_HANDLE_STMT, &hstmt);
   return current_person;   
}

void ReportSQLError(HSTMT hstmt)
{
   SQLCHAR msg[1024];
   SQLSMALLINT smInt;

   while(SQLGetDiagField(SQL_HANDLE_STMT,hstmt,SQL_DIAG_MESSAGE_TEXT,msg,1024,NULL,&smInt)==SQL_SUCCESS)
     fprintf(stderr, "%s\n", msg);

}

int create_meal(int person_id, SQL_TIMESTAMP_STRUCT * start, SQL_TIMESTAMP_STRUCT * end)
{
	SQLINTEGER indicator;//,len;
	char buf[256];
//	int number;
//	float remainder;
	indicator = 0;

   retcode = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &hstmt);

   retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &person_id, 0, &indicator);
   retcode = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_TIMESTAMP, SQL_TIMESTAMP, SQL_TIMESTAMP_LEN, 0, (SQLPOINTER)start, 
	   0, &indicator);
   retcode = SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_TIMESTAMP, SQL_TIMESTAMP, SQL_TIMESTAMP_LEN, 0, (SQLPOINTER)end, 
	  0, &indicator);
	  
   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("INSERT INTO meals(person_id, start_time, end_time) VALUES (?,?,?)"), SQL_NTS);
   if( retcode != 0)
   {
	   MessageBox(NULL,L"Meal not created",L"ERROR-ceate_meal-Prepare", MB_ICONHAND);
   }
   extract_error("Prepare Meal", dbc, SQL_HANDLE_DBC);

   retcode = SQLExecute(hstmt);
   if( retcode != 0)
   {
	   MessageBox(NULL,L"Meal not created",L"ERROR-ceate_meal-Execute", MB_ICONHAND);
   }
   extract_error("Create Meal", dbc, SQL_HANDLE_DBC);
	
   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("SELECT @@IDENTITY"), SQL_NTS);
   retcode = SQLExecute(hstmt);
   retcode = SQLFetch(hstmt);	
   retcode = SQLGetData(hstmt, 1, SQL_C_CHAR,
                         buf, sizeof(buf), &indicator);
   current_meal = atoi(buf);

   SQLFreeHandle(SQL_HANDLE_STMT, &hstmt);
   return current_meal;
}

int update_meal(int person_id, char *name)
{

	SQLINTEGER indicator, len;
	char buf[256];
	indicator = 0;

	if(name != NULL )
		len = (SQLINTEGER)strlen(name);
	else
		return -1;
    
   retcode = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &hstmt);

   retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, (SQLINTEGER)len, 0, (void *)name, 0, &indicator);
   retcode = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &person_id, 0, &indicator);

   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("Update people set name=? where id=?"), SQL_NTS);
   extract_error("Prepare person", dbc, SQL_HANDLE_DBC);

   retcode = SQLExecute(hstmt);
   extract_error("Update person", dbc, SQL_HANDLE_DBC);
	
   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("SELECT @@IDENTITY"), SQL_NTS);
   retcode = SQLExecute(hstmt);
   retcode = SQLFetch(hstmt);	
   retcode = SQLGetData(hstmt, 1, SQL_C_CHAR,
                         buf, sizeof(buf), &indicator);
   current_person = atoi(buf);

   SQLFreeHandle(SQL_HANDLE_STMT, &hstmt);
   return current_person;   
}

int create_grouptypes_meals(int meal_id, int grouptype_id)
{
	SQLINTEGER indicator,len;
	char buf[256];
	indicator = 0;
	
   retcode = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &hstmt);

   retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &meal_id, 0, &indicator);
   retcode = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &grouptype_id, 0, &indicator);
	  
   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("INSERT INTO grouptypes_meals(meal_id, grouptype_id) VALUES (?,?)"), SQL_NTS);
   extract_error("Prepare GroupType_Meals", dbc, SQL_HANDLE_DBC);

   retcode = SQLExecute(hstmt);
   extract_error("Create GroupType_Meals", dbc, SQL_HANDLE_DBC);
	
   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("SELECT @@IDENTITY"), SQL_NTS);
   retcode = SQLExecute(hstmt);
   retcode = SQLFetch(hstmt);	
   retcode = SQLGetData(hstmt, 1, SQL_C_CHAR,
                         buf, sizeof(buf), &indicator);
   current_grouptype_meal = atoi(buf);

   SQLFreeHandle(SQL_HANDLE_STMT, &hstmt);
   return current_grouptype_meal;
}


WORLD_POINT * get_volume_points(int fooditem_id)
{
	char buffer[2048];
	wchar_t buff[2048];
	SQLSMALLINT columns; /* number of columns in result-set */
	WORLD_POINT *wp = NULL;

	int row = 0;
	
	if(fooditem_id==0)
		fooditem_id = current_fooditem;

   sprintf(buffer, "SELECT VolumePoints.world_x, VolumePoints.world_y, VolumePoints.world_z, FROM VolumePoints WHERE VolumePoints.FoodItem_id = %d)", fooditem_id);
   mbstowcs(buff, buffer, sizeof(buff));

   retcode = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &hstmt);

   retcode = SQLPrepare(hstmt, (SQLWCHAR*)buff, SQL_NTS);
   extract_error("SQL Prepare", dbc, SQL_HANDLE_DBC);

   retcode = SQLExecute(hstmt);
   extract_error("Execute Volume Point Get", dbc, SQL_HANDLE_DBC);

///////////////////////////////////////////////////////////////////////
	SQLNumResultCols(hstmt, &columns);
    while (SQL_SUCCEEDED(retcode = SQLFetch(hstmt))) 
	{
		SQLUSMALLINT i;
		row++;
		wp = (WORLD_POINT *)realloc(wp, row*sizeof(WORLD_POINT));
		for( i=1; i<=columns; i++)
		{
	        SQLINTEGER indicator;
	        char buf[512];
	        /* retrieve column data as a string */
			retcode = SQLGetData(hstmt, i, SQL_C_CHAR,
                         buf, sizeof(buf), &indicator);
			if( i == 1 )
				wp[row-1].x = atof(buf);
			else if ( i == 2 )
				wp[row-1].y = atof(buf);
			else if (i == 3)
				wp[row-1].z = atof(buf);
			printf("%s", buf);

		}
	}
	volume_point_count = row;

	extract_error("SQL Fetch", dbc, SQL_HANDLE_DBC);
	
   SQLFreeHandle(SQL_HANDLE_STMT, &hstmt);

   return wp;
} 

PHOTOG_PARAMS * getPhotogParams(int monumentPointID)
{
	char buffer[2048];
	wchar_t buff[2048];
	SQLSMALLINT columns; /* number of columns in result-set */
//		SQLUSMALLINT i;
	PHOTOG_PARAMS *pp;

	int row = 0;


	pp = malloc(sizeof(PHOTOG_PARAMS));

   sprintf(buffer, "SELECT CameraCalibration.radial_dist1, CameraCalibration.radial_dist2, CameraCalibration.radial_dist3, CameraCalibration.tangential_dist1," 
                    " CameraCalibration.tangential_dist2, CameraCalibration.fx, CameraCalibration.fy, CameraCalibration.cx, CameraCalibration.cy, Images.t1, Images.t2,"
                    " Images.t3, Images.r11, Images.r12, Images.r13, Images.r21, Images.r22, Images.r23, Images.r31, Images.r32, Images.r33, MonumentPoints.world_x,"
                    " MonumentPoints.world_y, MonumentPoints.world_z, MonumentPoints.photo_x, MonumentPoints.photo_y, MonumentPoints.world_true, MonumentPoints.id"
					" FROM CameraCalibration INNER JOIN"
                    " Images ON CameraCalibration.id = Images.cameracalibration_id INNER JOIN"
                    " MonumentPoints ON Images.id = MonumentPoints.image_id"
					" WHERE (MonumentPoints.world_true = 1 and MonumentPoints.id = %d)", monumentPointID);
   mbstowcs(buff, buffer, sizeof(buff));

   retcode = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &hstmt);

   retcode = SQLPrepare(hstmt, (SQLWCHAR*)buff, SQL_NTS);
   extract_error("SQL Prepare", dbc, SQL_HANDLE_DBC);

   retcode = SQLExecute(hstmt);
   extract_error("Execute Volume Point Get", dbc, SQL_HANDLE_DBC);

///////////////////////////////////////////////////////////////////////
	SQLNumResultCols(hstmt, &columns);
    while (SQL_SUCCEEDED(retcode = SQLFetch(hstmt))) 
	{
		SQLUSMALLINT i;
		row++;
		pp = (PHOTOG_PARAMS *)realloc(pp, row*sizeof(PHOTOG_PARAMS));
		for( i=1; i<=columns; i++)
		{
			SQLINTEGER indicator;
			char buf[512];
			/* retrieve column data as a string */
			retcode = SQLGetData(hstmt, i, SQL_C_CHAR,
						 buf, sizeof(buf), &indicator);
			switch(i)
			{
			case 1:
				pp[0].k1 = atof(buf);
				break;
			case 2:
				pp[0].k2 = atof(buf);
				break;
			case 3:
				pp[0].k3 = atof(buf);
				break;
			case 4:
				pp[0].p1 = atof(buf);
				break;
			case 5:
				pp[0].p2 = atof(buf);
				break;
			case 6:
				pp[0].fx = atof(buf);
				break;
			case 7:
				pp[0].fy = atof(buf);
				break;
			case 8:
				pp[0].cx = atof(buf);
				break;
			case 9:
				pp[0].cy = atof(buf);
				break;
			case 10:
				pp[0].t1 = atof(buf);
				break;
			case 11:
				pp[0].t2 = atof(buf);
				break;
			case 12:
				pp[0].t3 = atof(buf);
				break;
			case 13:
				pp[0].r11 = atof(buf);
				break;
			case 14:
				pp[0].r12 = atof(buf);
				break;
			case 15:
				pp[0].r13 = atof(buf);
				break;
			case 16:
				pp[0].r21 = atof(buf);
				break;
			case 17:
				pp[0].r22 = atof(buf);
				break;
			case 18:
				pp[0].r23 = atof(buf);
				break;
			case 19:
				pp[0].r31 = atof(buf);
				break;
			case 20:
				pp[0].r32 = atof(buf);
				break;
			case 21:
				pp[0].r33 = atof(buf);
				break;
			case 22:
				pp[0].world_x = atof(buf);
				break;
			case 23:
				pp[0].world_y = atof(buf);
				break;
			case 24:
				pp[0].world_z = atof(buf);
				break;
			case 25:
				pp[0].image_x = atoi(buf);
				break;
			case 26:
				pp[0].image_y = atoi(buf);
				break;
			}
		}
	}
	extract_error("SQL Fetch", dbc, SQL_HANDLE_DBC);
	
   SQLFreeHandle(SQL_HANDLE_STMT, &hstmt);

   return pp;
}

PHOTOG_PARAMS * getBLOBCenters(int meal_id, int grouptype_id)
{
	char buffer[2048];
	wchar_t buff[2048];
	SQLSMALLINT columns; /* number of columns in result-set */
		SQLUSMALLINT i;
	PHOTOG_PARAMS *pp;

	int row = 0;


	pp = NULL;

	sprintf(buffer, "SELECT Images.t1, Images.t2, Images.t3, Images.r11, Images.r12, Images.r13, "
		" Images.r21, Images.r22, Images.r23,"
		" Images.r31, Images.r32, Images.r33,"
		" CameraCalibration.fx, CameraCalibration.fy, CameraCalibration.cx, CameraCalibration.cy, "
		" PhotoBlobs.center_x, PhotoBlobs.center_y, "
		" PhotoBlobs.id, Images.id AS ImagesID, Images.filename, "
		" GroupTypes.name, Meals.id AS MealsID, "
		" Images.camera_optical_center_x, Images.camera_optical_center_y, "
		" Images.camera_optical_center_z, "
		" CameraCalibration.radial_dist1, CameraCalibration.radial_dist2, "
		" CameraCalibration.radial_dist3, "
		" CameraCalibration.tangential_dist1, CameraCalibration.tangential_dist2, "
		" GroupTypes_Meals.id as GTMealID " 
		" FROM Meals INNER JOIN "
		" GroupTypes_Meals ON Meals.id = GroupTypes_Meals.meal_id INNER JOIN"
		" Images ON GroupTypes_Meals.id = Images.grouptypes_meals_id INNER JOIN"
		" GroupTypes ON GroupTypes_Meals.grouptype_id = GroupTypes.id INNER JOIN"
		" PhotoBlobs ON Images.id = PhotoBlobs.image_id INNER JOIN"
		" CameraCalibration ON Images.cameracalibration_id = CameraCalibration.id"
		" WHERE     (Meals.id = %d) AND (GroupTypes.id = %d)", meal_id, grouptype_id);

   mbstowcs(buff, buffer, sizeof(buff));

   retcode = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &hstmt);

   retcode = SQLPrepare(hstmt, (SQLWCHAR*)buff, SQL_NTS);
   extract_error("SQL Prepare", dbc, SQL_HANDLE_DBC);

   retcode = SQLExecute(hstmt);
   extract_error("Execute Volume Point Get", dbc, SQL_HANDLE_DBC);

///////////////////////////////////////////////////////////////////////
	SQLNumResultCols(hstmt, &columns);
    while (SQL_SUCCEEDED(retcode = SQLFetch(hstmt))) 
	{
		SQLUSMALLINT i;
		row++;
		pp = (PHOTOG_PARAMS *)realloc(pp, row*sizeof(PHOTOG_PARAMS));
		for( i=1; i<=columns; i++)
		{
			SQLINTEGER indicator;
			char buf[512];
			/* retrieve column data as a string */
			retcode = SQLGetData(hstmt, i, SQL_C_CHAR,
						 buf, sizeof(buf), &indicator);
			switch(i)
			{
			case 1:
				pp[row-1].t1 = atof(buf);
				break;
			case 2:
				pp[row-1].t2 = atof(buf);
				break;
			case 3:
				pp[row-1].t3 = atof(buf);
				break;
			case 4:
				pp[row-1].r11 = atof(buf);
				break;
			case 5:
				pp[row-1].r12 = atof(buf);
				break;
			case 6:
				pp[row-1].r13 = atof(buf);
				break;
			case 7:
				pp[row-1].r21 = atof(buf);
				break;
			case 8:
				pp[row-1].r22 = atof(buf);
				break;
			case 9:
				pp[row-1].r23 = atof(buf);
				break;
			case 10:
				pp[row-1].r31 = atof(buf);
				break;
			case 11:
				pp[row-1].r32 = atof(buf);
				break;
			case 12:
				pp[row-1].r33 = atof(buf);
				break; 
			case 13:
				pp[row-1].fx = atof(buf);
				break;
			case 14:
				pp[row-1].fy = atof(buf);
				break;
			case 15:
				pp[row-1].cx = atof(buf);
				break;
			case 16:
				pp[row-1].cy = atof(buf);
				break;
			case 17:
				pp[row-1].image_x = atof(buf);
				break;
			case 18:
				pp[row-1].image_y = atof(buf);
				break;
			case 19:  ////////////////////////
				pp[row-1].blob_id = atoi(buf);
				break;
			case 20:
				pp[row-1].image_id = atoi(buf);
				break;
			case 21:  //filename
				//pp[row-1]. = atof(buf);
				break;
			case 22: //Grouptype_name
				//pp[row-1]. = atof(buf);
				break;
			case 23:
				pp[row-1].meal_id = atoi(buf);
				break;
			case 24:
				pp[row-1].camera_x = atof(buf);
				break;
			case 25:
				pp[row-1].camera_y = atof(buf);
				break;
			case 26:
				pp[row-1].camera_z = atof(buf);
				break;
			case 27:
				pp[row-1].k1 = atof(buf);
				break;
			case 28:
				pp[row-1].k2 = atof(buf);
				break;
			case 29:
				pp[row-1].k3 = atof(buf);
				break;
			case 30:
				pp[row-1].p1 = atof(buf);
				break;
			case 31:
				pp[row-1].p2 = atof(buf);
				break;
			case 32:
				pp[row-1].grouptype_meal_id = atoi(buf);
				break;
			}
		}
	}
	extract_error("SQL Fetch", dbc, SQL_HANDLE_DBC);
	meal_blob_count = row;

   SQLFreeHandle(SQL_HANDLE_STMT, &hstmt);

   return pp;
}


int createFoodItem(int groupTypeMealID, int group_number)
{
//	SQLINTEGER photox, photoy;
//	SQLDOUBLE worldx, worldy, worldz;
    SQLINTEGER indicator, len;
	char buf[256];
	indicator = 0;
    
	sprintf(buf, "%d", group_number);
	if(buf != NULL )
		len = (SQLINTEGER)strlen(buf);
	else
		return -1;
    
  
   retcode = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &hstmt);

   retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, (SQLINTEGER)len, 0, (void *)buf, 0, &indicator);
   retcode = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &groupTypeMealID, 0, &indicator);

   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("INSERT INTO FoodItems (name, grouptype_meals_id) VALUES (?,?)"), SQL_NTS);
   extract_error("Prepare Food Item", dbc, SQL_HANDLE_DBC);

   retcode = SQLExecute(hstmt);
   extract_error("Create Food Item", dbc, SQL_HANDLE_DBC);
	
   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("SELECT @@IDENTITY"), SQL_NTS);
   retcode = SQLExecute(hstmt);
   retcode = SQLFetch(hstmt);	
   retcode = SQLGetData(hstmt, 1, SQL_C_CHAR,
                         buf, sizeof(buf), &indicator);
   current_fooditem = atoi(buf);

   SQLFreeHandle(SQL_HANDLE_STMT, &hstmt);
   return current_fooditem;
}

int updateBlobToFood(int foodItemID, int blob_id)
{
    SQLINTEGER indicator;
	char buf[256];
	indicator = 0;

   retcode = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &hstmt);

   retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &foodItemID, 0, &indicator);
   retcode = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &blob_id, 0, &indicator);

   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("Update PhotoBlobs SET FoodItems_id=? where id=?"), SQL_NTS);
   extract_error("Prepare Photo Blob", dbc, SQL_HANDLE_DBC);

   retcode = SQLExecute(hstmt);
   extract_error("Update Photo Blob", dbc, SQL_HANDLE_DBC);
	
   retcode = SQLPrepare(hstmt, (SQLWCHAR*)TEXT("SELECT @@IDENTITY"), SQL_NTS);
   retcode = SQLExecute(hstmt);
   retcode = SQLFetch(hstmt);	
   retcode = SQLGetData(hstmt, 1, SQL_C_CHAR,
                         buf, sizeof(buf), &indicator);
   current_photoblob = atoi(buf);

   SQLFreeHandle(SQL_HANDLE_STMT, &hstmt);
   return current_photoblob;
	
}

BLOBPOINT_PARAMS * getBlobPointsForRays(int foodItemID, int blobID)
{
	char buffer[2048];
	wchar_t buff[2048];
	SQLSMALLINT columns; /* number of columns in result-set */
		SQLUSMALLINT i;
	BLOBPOINT_PARAMS *bp;

	int row = 0;


	bp = NULL;

	sprintf(buffer, "SELECT BlobPoints.photo_x, BlobPoints.photo_y, "
					" BlobPoints.id, PhotoBlobs.id AS Expr1, "
					" PhotoBlobs.FoodItems_id "
					" FROM PhotoBlobs INNER JOIN "
                    " BlobPoints ON PhotoBlobs.id = "
					" BlobPoints.photoblob_id INNER JOIN "
                    " FoodItems ON PhotoBlobs.FoodItems_id = FoodItems.id "
					" WHERE (FoodItems.id = %d and photoblobs.id = %d)", foodItemID, blobID);

   mbstowcs(buff, buffer, sizeof(buff));

   retcode = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &hstmt);

   retcode = SQLPrepare(hstmt, (SQLWCHAR*)buff, SQL_NTS);
   extract_error("SQL Prepare", dbc, SQL_HANDLE_DBC);

   retcode = SQLExecute(hstmt);
   extract_error("Execute Volume Point Get", dbc, SQL_HANDLE_DBC);

///////////////////////////////////////////////////////////////////////
	SQLNumResultCols(hstmt, &columns);
    while (SQL_SUCCEEDED(retcode = SQLFetch(hstmt))) 
	{
		SQLUSMALLINT i;
		row++;
		bp = (BLOBPOINT_PARAMS *)realloc(bp, row*sizeof(BLOBPOINT_PARAMS));
		if( bp == NULL )
		{
			MessageBox(NULL,L"BP Array could not be realloced",L"ERROR-GetBlobPointsForRays", MB_ICONHAND);
			return -1;
		}
		for( i=1; i<=columns; i++)
		{
			SQLINTEGER indicator;
			char buf[512];
			/* retrieve column data as a string */
			retcode = SQLGetData(hstmt, i, SQL_C_CHAR,
						 buf, sizeof(buf), &indicator);
			switch(i)
			{
			case 1:
				bp[row-1].imageX = atoi(buf);
				break;
			case 2:
				bp[row-1].imageY = atoi(buf);
				break;
			case 3:
				bp[row-1].blobPointID = atoi(buf);
				break;
			case 4:
				bp[row-1].photoBlobID = atoi(buf);
				break;
			case 5:
				bp[row-1].foodItemID = atoi(buf);
				break;
			}
		}
	}
	extract_error("SQL Fetch", dbc, SQL_HANDLE_DBC);
	current_blobpoint_count = row;

   SQLFreeHandle(SQL_HANDLE_STMT, &hstmt);

   return bp;
}

PHOTOG_PARAMS * getCameraForRays(int photoBlobID)
{
	char buffer[2048];
	wchar_t buff[2048];
	SQLSMALLINT columns; /* number of columns in result-set */
		SQLUSMALLINT i;
	PHOTOG_PARAMS *pp;

	int row = 0;


	pp = NULL;

   sprintf(buffer, "SELECT Images.t1, Images.t2, Images.t3, Images.r11, Images.r12, Images.r13, "
					" Images.r21, Images.r22, Images.r23, Images.r31, Images.r32, Images.r33, "
                    " CameraCalibration.fx, CameraCalibration.fy, CameraCalibration.cx, CameraCalibration.cy, "
					" Images.camera_optical_center_x, Images.camera_optical_center_y, "
                    " Images.camera_optical_center_z, PhotoBlobs.id "
					" FROM         PhotoBlobs INNER JOIN "
                    " Images ON PhotoBlobs.image_id = Images.id INNER JOIN "
                    " CameraCalibration ON Images.cameracalibration_id = CameraCalibration.id "
					" WHERE (PhotoBlobs.id = %d)" , photoBlobID);
   mbstowcs(buff, buffer, sizeof(buff));

   retcode = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &hstmt);

   retcode = SQLPrepare(hstmt, (SQLWCHAR*)buff, SQL_NTS);
   extract_error("SQL Prepare", dbc, SQL_HANDLE_DBC);

   retcode = SQLExecute(hstmt);
   extract_error("Execute Get", dbc, SQL_HANDLE_DBC);

///////////////////////////////////////////////////////////////////////
	SQLNumResultCols(hstmt, &columns);
    while (SQL_SUCCEEDED(retcode = SQLFetch(hstmt))) 
	{
		SQLUSMALLINT i;
		row++;
		pp = (PHOTOG_PARAMS *)realloc(pp, row*sizeof(PHOTOG_PARAMS));
		for( i=1; i<=columns; i++)
		{
			SQLINTEGER indicator;
			char buf[512];
			/* retrieve column data as a string */
			retcode = SQLGetData(hstmt, i, SQL_C_CHAR,
						 buf, sizeof(buf), &indicator);
			switch(i)
			{
			case 1:
				pp[row-1].t1 = atof(buf);
				break;
			case 2:
				pp[row-1].t2 = atof(buf);
				break;
			case 3:
				pp[row-1].t3 = atof(buf);
				break;
			case 4:
				pp[row-1].r11 = atof(buf);
				break;
			case 5:
				pp[row-1].r12 = atof(buf);
				break;
			case 6:
				pp[row-1].r13 = atof(buf);
				break;
			case 7:
				pp[row-1].r21 = atof(buf);
				break;
			case 8:
				pp[row-1].r22 = atof(buf);
				break;
			case 9:
				pp[row-1].r23 = atof(buf);
				break;
			case 10:
				pp[row-1].r31 = atof(buf);
				break;
			case 11:
				pp[row-1].r32 = atof(buf);
				break;
			case 12:
				pp[row-1].r33 = atof(buf);
				break;
			case 13:
				pp[row-1].fx = atof(buf);
				break;
			case 14:
				pp[row-1].fy = atof(buf);
				break;
			case 15:
				pp[row-1].cx = atof(buf);
				break;
			case 16:
				pp[row-1].cy = atof(buf);
				break;
			case 17:
				pp[row-1].camera_x = atof(buf);
				break;
			case 18:
				pp[row-1].camera_y = atof(buf);
				break;
			case 19:
				pp[row-1].camera_z = atof(buf);
				break;
			}
		}
	}
	extract_error("SQL Fetch", dbc, SQL_HANDLE_DBC);
//	MessageBox(NULL,L"Done with Camera Params",L"OK", MB_OK);
   SQLFreeHandle(SQL_HANDLE_STMT, &hstmt);

   return pp;
}

int getMealIDfromFoodItem(int foodItemID)
{
	char buffer[2048];
	wchar_t buff[2048];
	SQLSMALLINT columns; /* number of columns in result-set */
	int mealID;
	int flag = FALSE;
	int row = 0;
	
	mealID = 0;
   sprintf(buffer, "SELECT Meals.id as MealID "
					"FROM FoodItems INNER JOIN GroupTypes_Meals ON "
					"FoodItems.GroupType_Meals_id = GroupTypes_Meals.id INNER JOIN "
					"Meals ON GroupTypes_Meals.meal_id = Meals.id "
					"where FoodItems.id = %d", foodItemID);
   mbstowcs(buff, buffer, sizeof(buff));

   retcode = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &hstmt);

   retcode = SQLPrepare(hstmt, (SQLWCHAR*)buff, SQL_NTS);
   extract_error("SQL Prepare", dbc, SQL_HANDLE_DBC);

   retcode = SQLExecute(hstmt);
   extract_error("Execute Monument Point Get", dbc, SQL_HANDLE_DBC);

///////////////////////////////////////////////////////////////////////
	SQLNumResultCols(hstmt, &columns);
    while (SQL_SUCCEEDED(retcode = SQLFetch(hstmt))) 
	{
		SQLUSMALLINT i;
		row++;
		for( i=1; i<=columns; i++)
		{
	        SQLINTEGER indicator;
	        char buf[512];
	        /* retrieve column data as a string */
			retcode = SQLGetData(hstmt, i, SQL_C_CHAR,
                         buf, sizeof(buf), &indicator);
			if( i == 1 && indicator != -1 )
			{
				mealID = atoi(buf);
			}
		}
	}

   extract_error("SQL Fetch", dbc, SQL_HANDLE_DBC);
	
   SQLFreeHandle(SQL_HANDLE_STMT, &hstmt);
   return mealID;
}

char * getImageName(int image_id)
{
	char buffer[2048];
	wchar_t buff[2048];
	SQLSMALLINT columns; /* number of columns in result-set */
	char *p;
	int flag = FALSE;
	int row = 0;
	
	if(image_id==0)
		image_id = current_image;

   sprintf(buffer, "SELECT Images.filename from Images"
						" where Images.id = %d", image_id);
   mbstowcs(buff, buffer, sizeof(buff));

   retcode = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &hstmt);

   retcode = SQLPrepare(hstmt, (SQLWCHAR*)buff, SQL_NTS);
   extract_error("SQL Prepare", dbc, SQL_HANDLE_DBC);

   retcode = SQLExecute(hstmt);
   extract_error("Execute Monument Point Get", dbc, SQL_HANDLE_DBC);

///////////////////////////////////////////////////////////////////////
	SQLNumResultCols(hstmt, &columns);
    while (SQL_SUCCEEDED(retcode = SQLFetch(hstmt))) 
	{
		SQLUSMALLINT i;
		row++;
		for( i=1; i<=columns; i++)
		{
	        SQLINTEGER indicator;
	        char buf[512];
	        /* retrieve column data as a string */
			retcode = SQLGetData(hstmt, i, SQL_C_CHAR,
                         buf, sizeof(buf), &indicator);
			if( i == 1 && indicator != -1 )
			{
				flag = TRUE;
				p = malloc(strlen(buf)+1);
				strcpy(p, buf);
				//p = &buf[0];
			}
		}
	}

   extract_error("SQL Fetch", dbc, SQL_HANDLE_DBC);
	
   SQLFreeHandle(SQL_HANDLE_STMT, &hstmt);
   if( flag == TRUE )
		return p;
   else
	   return NULL;
}

int createMealRecords(int personID, int mealType)
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

	meal_id = create_meal(personID, &start, &end);
	grouptype_meal_id = create_grouptypes_meals(meal_id, mealType);
	set_current_grouptype_meal(grouptype_meal_id);
	return meal_id;
}

int deleteClosestMonumentPoint(pointNumber, photox, photoy)
{
    SQLINTEGER indicator;
	char buffer[2048];
	wchar_t buff[2048];
	int id;
	SQLSMALLINT columns;
	
	indicator = 0;

    sprintf(buffer, "delete from MonumentPoints "
					"where (SQRT(SQUARE((photo_x-%d) + SQUARE(photo_y-%d)))) = "
					"(select min( "
					"(SQRT(SQUARE((photo_x-%d) + SQUARE(photo_y-%d))))) "
					"from MonumentPoints where image_id = %d) and image_id = %d",
					photox, photoy, photox, photoy, 
					get_current_image(), get_current_image());

   mbstowcs(buff, buffer, sizeof(buff));
   //MessageBox(NULL, buff, L"SQL String", MB_OK);

   retcode = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &hstmt);
	if( retcode != 0 )
		MessageBox(NULL,L"ALLOCHANDLE FAILED",L"ERROR", MB_ICONHAND);

   retcode = SQLPrepare(hstmt, (SQLWCHAR*)buff, SQL_NTS);
	if( retcode != 0 )
		MessageBox(NULL,L"PREPARE FAILED",L"ERROR", MB_ICONHAND);
   extract_error("Prepare Monument Point", dbc, SQL_HANDLE_DBC);

   retcode = SQLExecute(hstmt);
	if( retcode != 0 )
		MessageBox(NULL,L"EXECUTE FAILED",L"ERROR", MB_ICONHAND);
   extract_error("Create Monument Point", dbc, SQL_HANDLE_DBC);

   if( retcode )
	   MessageBox(NULL, L"retcode has error", L"ERROR",MB_ICONHAND);

   SQLFreeHandle(SQL_HANDLE_STMT, &hstmt);
   return 0;	
}

int getClosestMonumentPoint(int pointNumber, int photox, int photoy)
{
    SQLINTEGER indicator;
	char buffer[2048];
	wchar_t buff[2048];
	int id;
	SQLSMALLINT columns;
	
	indicator = 0;

    sprintf(buffer, "select id from MonumentPoints "
					"where (SQRT(SQUARE((photo_x-%d) + SQUARE(photo_y-%d)))) = "
					"(select min( "
					"(SQRT(SQUARE((photo_x-%d) + SQUARE(photo_y-%d))))) "
					"from MonumentPoints where image_id = %d) and image_id = %d",
					photox, photoy, photox, photoy, 
					get_current_image(), get_current_image());

   mbstowcs(buff, buffer, sizeof(buff));
   //MessageBox(NULL, buff, L"SQL String", MB_OK);

   retcode = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &hstmt);
	if( retcode != 0 )
		MessageBox(NULL,L"ALLOCHANDLE FAILED",L"ERROR", MB_ICONHAND);

   retcode = SQLPrepare(hstmt, (SQLWCHAR*)buff, SQL_NTS);
	if( retcode != 0 )
		MessageBox(NULL,L"PREPARE FAILED",L"ERROR", MB_ICONHAND);
   extract_error("Prepare Monument Point", dbc, SQL_HANDLE_DBC);

   retcode = SQLExecute(hstmt);
	if( retcode != 0 )
		MessageBox(NULL,L"EXECUTE FAILED",L"ERROR", MB_ICONHAND);
   extract_error("Create Monument Point", dbc, SQL_HANDLE_DBC);

   if( retcode )
	   MessageBox(NULL, L"retcode has error", L"ERROR",MB_ICONHAND);

   id = -1;

	retcode = SQLNumResultCols(hstmt, &columns);
	if( columns <= 0 )
	{
		sprintf(buffer, "COLUMNS RETCODE: %X,  COLUMNS: %d", retcode, columns);
		mbstowcs(buff, buffer, sizeof(buff));
		MessageBox(NULL, buff, L"ERROR", MB_ICONHAND);
	}
    while (SQL_SUCCEEDED(retcode = SQLFetch(hstmt))) 
	{
		SQLUSMALLINT i;
	//	MessageBox(NULL,L"Inside While",L"TRACK", MB_OK);
		
		for( i=1; i<=columns; i++)
		{
			SQLINTEGER indicator;
			char buf[512];
	//		MessageBox(NULL,L"Inside For",L"TRACK", MB_OK);
	        

	        /* retrieve column data as a string */
			retcode = SQLGetData(hstmt, i, SQL_C_CHAR,
                         buf, sizeof(buf), &indicator);
			if( i == 1 && indicator != -1 )
			{
	//			MessageBox(NULL,L"Inside If",L"TRACK", MB_OK);
				id = atoi(buf);
			}
		}
	}

   SQLFreeHandle(SQL_HANDLE_STMT, &hstmt);
   return id;	

}

int updateClosestMonumentPoint(int pointNumber, int photox, int photoy)
{
    SQLINTEGER indicator;
	char buffer[2048];
	wchar_t buff[2048];
	int id;
	int columns;
	
	indicator = 0;

	
	sprintf(buffer, "UPDATE MonumentPoints "
					"set map_id = %d " 
					"where (SQRT(SQUARE((photo_x-%d) + SQUARE(photo_y-%d)))) = "
					"(select min( "
					"(SQRT(SQUARE((photo_x-%d) + SQUARE(photo_y-%d))))) "
					"from MonumentPoints where image_id = %d) and image_id = %d",
					pointNumber, photox, photoy, photox, photoy, 
					get_current_image(), get_current_image());

    mbstowcs(buff, buffer, sizeof(buff));
	//MessageBox(NULL,buff,L"SQL",MB_OK);

   retcode = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &hstmt);

   retcode = SQLPrepare(hstmt, (SQLWCHAR*)buff, SQL_NTS);
   extract_error("Prepare Monument Point", dbc, SQL_HANDLE_DBC);

   retcode = SQLExecute(hstmt);
   extract_error("Create Monument Point", dbc, SQL_HANDLE_DBC);

   sprintf(buffer, "select id from MonumentPoints "
					"where (SQRT(SQUARE((photo_x-%d) + SQUARE(photo_y-%d)))) = "
					"(select min( "
					"(SQRT(SQUARE((photo_x-%d) + SQUARE(photo_y-%d))))) "
					"from MonumentPoints where image_id = %d) and image_id = %d",
					photox, photoy, photox, photoy, 
					get_current_image(), get_current_image());
    mbstowcs(buff, buffer, sizeof(buff));
   retcode = SQLPrepare(hstmt, (SQLWCHAR*)buff, SQL_NTS);
   extract_error("Prepare Monument Point", dbc, SQL_HANDLE_DBC);

   retcode = SQLExecute(hstmt);
   extract_error("Create Monument Point", dbc, SQL_HANDLE_DBC);
	
   id = -1;

	SQLNumResultCols(hstmt, &columns);
    while (SQL_SUCCEEDED(retcode = SQLFetch(hstmt))) 
	{
		SQLUSMALLINT i;
		for( i=1; i<=columns; i++)
		{
	        SQLINTEGER indicator;
	        char buf[512];
	        /* retrieve column data as a string */
			retcode = SQLGetData(hstmt, i, SQL_C_CHAR,
                         buf, sizeof(buf), &indicator);
			if( i == 1 && indicator != -1 )
			{
				id = atoi(buf);
			}
		}
	}

   SQLFreeHandle(SQL_HANDLE_STMT, &hstmt);
   return id;	
}

int delete_photoblob(int photoblob_id)
{
    SQLINTEGER indicator;
	char buffer[2048];
	wchar_t buff[2048];

	delete_blobpoint_from_photoblob(photoblob_id);

	indicator = 0;

	sprintf(buffer, "delete from Photoblobs where id = %d", photoblob_id);
    mbstowcs(buff, buffer, sizeof(buff));

   retcode = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &hstmt);

   retcode = SQLPrepare(hstmt, (SQLWCHAR*)buff, SQL_NTS);
   extract_error("Prepare photoblob", dbc, SQL_HANDLE_DBC);

   retcode = SQLExecute(hstmt);
   extract_error("delete photoblob", dbc, SQL_HANDLE_DBC);

   SQLFreeHandle(SQL_HANDLE_STMT, &hstmt);
   return 0;

}

int delete_blobpoint_from_photoblob(int photoblob)
{
    SQLINTEGER indicator;
	char buffer[2048];
	wchar_t buff[2048];

	indicator = 0;

	sprintf(buffer, "delete from blobpoints where photoblob_id = %d", photoblob);
    mbstowcs(buff, buffer, sizeof(buff));

   retcode = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &hstmt);

   retcode = SQLPrepare(hstmt, (SQLWCHAR*)buff, SQL_NTS);
   extract_error("Prepare Monument Point", dbc, SQL_HANDLE_DBC);

   retcode = SQLExecute(hstmt);
   extract_error("Create Monument Point", dbc, SQL_HANDLE_DBC);

   SQLFreeHandle(SQL_HANDLE_STMT, &hstmt);
   return 0;
}

int check_photoblob_inside_placemat(int photoblobid)
{

	return 0;
}
