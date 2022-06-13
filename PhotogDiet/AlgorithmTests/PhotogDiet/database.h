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
//extern 'C'
//{




#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include "geometry.h"
// C RunTime Header Files
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#ifdef _PHOTOG
#include <sql.h>
#endif //_PHOTOG

void initialize_db();
void shutdown_db();
int set_current_grouptype_meal(int id);
int get_current_grouptype_meal();
void set_current_filename(char *name);
char *get_current_filename();
void set_filename_base(char *name);
char *get_filename_base();
void set_original_filename(char *name);
char *get_original_filename();
void initialize_db();
int create_image(char *filename);
int create_monumentpoint(int photo_x, int photo_y, double world_x, double world_y, double world_z, int image_id, double shading);
int create_edgepoint(int photo_x, int photo_y, double world_x, double world_y, double world_z);
int get_current_image();
int set_current_image(int);
int get_monument_point_count();
void set_monument_point_filename(char *name);
char *get_monument_point_filename();
int updateClosestMonumentPoint(int pointNumber, int photox, int photoy);
int getClosestMonumentPoint(int pointNumber, int photox, int photoy);
int deleteClosestMonumentPoint(int pointNumber, int photox, int photoy);
int delete_photoblob(int photoblob_id);
int delete_blobpoint_from_photoblob(int photoblob);

int load_preferences(void);

PSIMPLE_POINT get_image_monument_points(int image_id);
int update_3D_coordinates(int monument_point_id, int map_id );

int get_current_image();
int get_monument_point_count();

//int getCameraForRays(int photoBlobID);

int update_3D_coordinates(int monument_point_id, int map_id );
int get_camera_internal_params(CAMERA_INTERNALS *cam_int, int imageid);

int update_camera_location(int id, float r11, float r12, float r13, float r21, float r22, float r23, float r31, float r32, float r33, 
						   float t1, float t2, float t3, float cam_x, float cam_y, float cam_z);
int update_shading_factor(int monument_point_id, int shading_factor );
int update_monumentpoint_diameter(int monument_point_id, int x, int y);
int delete_monument_point(int monument_point_id);

int create_photoblob(int image_id, char *name, double confidence, double center_x, double center_y);
int update_photoblob(int photoblob_id, char *name, double confidence, double entropy, double energy, double inertia);

int create_blobpoint(int photoblob_id, int photo_x, int photo_y);
int update_blobpoint(int photoblob_id, int photo_x, int photo_y, double world_x, 
					 double world_y, double world_z, int blobpoint_id);

int create_volumepoint(int volume_id, double world_x, double world_y, double world_z);
int update_volumepoint(int volumepoint_id, int fooditem_id, double x, double y, double z);

int create_volume(double volume);
int update_volume(int volume_id, double volume);

int create_fooditem(char *name, int grouptype_meal_id, double volume);
int update_fooditem(int fooditem_id, double volume);

#ifdef _PHOTOG
int create_meal(int person_id, SQL_TIMESTAMP_STRUCT * start, SQL_TIMESTAMP_STRUCT * end);
#endif //_PHOTOG
int create_grouptypes_meals(int meal_id, int grouptype_id);

int create_person(char *name);
int update_person(int person_id, char *name);

int get_volume_point_count();
WORLD_POINT * get_volume_points(int fooditem_id);

int get_current_image();
int get_current_photoblob();
int get_meal_blob_count();
int get_current_blobpoint_count();


int createFoodItem(int groupTypeMealID, int group_number);
int updateBlobToFood(int foodItemID, int blob_id);

PHOTOG_PARAMS * getPhotogParams(int monumentPointID);
PHOTOG_PARAMS * getBLOBCenters(int meal_id, int grouptype_id/* BEFORE or AFTER*/);
BLOBPOINT_PARAMS * getBlobPointsForRays(int foodItemID, int blobID);

char * getImageName(int image_id);
int createMealRecords(int personID, int mealType);
int getMealIDfromFoodItem(int foodItemID);
int update_fooditem_volume(int fooditem_id, double volume, int type);


// Database structures
typedef struct images {
	int id;
	char name[256];
	double radial_dist1;
	double radial_dist2;
	double radial_dist3;
	double tangential_dist1;
	double tangential_dist2;
	double fx;
	double fy;
	double cx;
	double cy;
} DB_IMAGE;

typedef struct monumentpoint
{
	int id;
	int photo_x;
	int photo_y;
	double world_x;
	double world_y;
	double world_z;
	int image_id;
	double shading;
} DB_MONUMENTPOINT;

typedef struct photoblobs
{
	int id;
	char texture[1024];
	char name[128];
	double volume;
	double confidence;
	int image_id;
} DB_PHOTOBLOB;

typedef struct blobpoints
{
	int id;
	int photo_x;
	int photo_y;
	int photoblob_id;
} DB_BLOBPOINT;

typedef struct rays
{
	int id;
	double slope;
	double y_intercept;
	int volumepoint_id;
	int blobpoint_id;
} DB_RAY;

typedef struct volumepoints
{
	int id;
	double world_x;
	double world_y;
	double world_z;
	int volume_id;
} DB_VOLUMEPOINT;

typedef struct volumes
{
	int id;
	double volume;
} DB_VOLUME;


#define BEFORE	1
#define AFTER	2


