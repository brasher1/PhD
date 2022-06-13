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
#ifndef GEOMETRY_H
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>


#define PI 3.14159265

typedef struct point_3d {
	double x;
	double y;
	double z;
} POINT_3D;

typedef struct image_2d {
	int x;
	int y;
} IMAGE_2D;

typedef struct intersect_point_3d {
	double x;
	double y;
	double z;
	double distance;
	int ray1_id;
	int ray2_id;
} INTERSECT_POINT_3D;

typedef struct blob_direction {
	int blob1_id;
	int blob2_id;
	int image_id1;
	int image_id2;
	float i_vector;
	float j_vector;
	float new_x1;
	float new_y1;
	float new_z1;
	float new_x2;
	float new_y2;
	float new_z2;
	float angle1to2;
	float angle2to1;
} BLOB_DIRECTION;

typedef struct matching_blobs {
	int blob_id;
	float x;
	float y;
//	float angle1;
//	float angle2;
	int foodItemID;
	int group_number;
} MATCHING_BLOBS;

typedef struct food_blob_group {
	int foodItemID;
	int blob_group_number;
} FOOD_BLOB_GROUP;

typedef struct blobs_in_image {
	int image_id;
	int count_of_blobs;
} BLOBS_IN_IMAGE;

typedef struct simple_point {
	int	x;
	int y;
	int shading;
	int id;
	int	map_id;
} SIMPLE_POINT;

typedef struct world_point {
	double x;
	double y;
	double z;
	int id;
} WORLD_POINT;

typedef struct placemat_point {
	int id;
	int map_id;
	int photox;

	int photoy;

	int shading;
	double worldx;
	double worldy;
	double worldz;
	double slope_forward;
	double slope_backward;
	double distance_forward;
	double distance_backward;
	int monument_point_id;
	int world_true;
	int	x_diameter;
	int y_diameter;
	int triangle_part;
	int quad_part;
} PLACEMAT_POINT;

typedef struct pointprops {
	double 	slope;
	double	distance;
	int		pointfrom;
	int		pointfrom_id;
	int		pointto;
	int		pointto_id;
} POINT_PROPS;

typedef struct simple_point * PPOINT_PROPS;

typedef struct triangle {
	int point1_map_id;
	int point2_map_id;
	int point3_map_id;
	int point1_id;
	int point2_id;
	int point3_id;
	int point1_shading;
	int point2_shading;
	int point3_shading;
	int	pt1x;
	int pt1y;
	int pt2x;
	int pt2y;
	int pt3x;
	int pt3y;
	double slope12;
	double slope23;
	double slope31;
	double distance12;
	double distance23;
	double distance31;
	double angle1;
	double angle2;
	double angle3;
} TRIANGLE;

typedef struct distance_analysis {
	SIMPLE_POINT *from;
	SIMPLE_POINT *to;
	float	distance;
} DISTANCE_ANALYSIS;

typedef struct ordered_edge_points {
	SIMPLE_POINT *point;
	float	distance_from_edge_start;
} ORDERED_EDGE_POINTS;

typedef struct camera_internals {
	float fx;
	float fy;
	float cx;
	float cy;
	float k1;
	float k2;
	float k3;
	float p1;
	float p2;
} CAMERA_INTERNALS;

typedef struct qtyofcolors {
	int count;
	long sum_of_x, sum_of_y;
	unsigned char red, green, blue;
} QtyOfColors;

typedef struct imagepoint {
	int imageX, imageY;
	unsigned char red, green, blue;
	unsigned char connectedred, connectedgreen, connectedblue;
} IMAGEPOINT;

typedef struct simpleimagepoint {
	int imageX, imageY;
} SIMPLEIMAGEPOINT;

typedef struct simple_point * PSIMPLE_POINT;
typedef struct world_point * PWORLD_POINT;
typedef struct placemat_point *PPLACEMAT_POINT;

// ELLIPSE FINDING STRUCTURES
typedef struct ellipseParams {
	float radius1;
	float radius2;
	float offset1;
	float offset2;
	float theta;
} ELLIPSEPARAMS;

typedef struct ellipseParams * PELLIPSEPARAMS;

typedef struct photog_params {
	float fx;
	float fy;
	float cx;
	float cy;
	float k1;
	float k2;
	float k3;
	float p1;
	float p2;
	float r11;
	float r12;
	float r13;
	float r21;
	float r22;
	float r23;
	float r31;
	float r32;
	float r33;
	float t1;
	float t2;
	float t3;
	int image_x;  //in pixels
	int image_y;  //in pixels
	float world_x;  // a 3D point going from camera through image pixels
	float world_y;
	float world_z;
	float camera_x;  //optical center of the camera
	float camera_y;
	float camera_z;
	int meal_id;
	int blob_id;
	int image_id;
	int grouptype_meal_id;
	float i; //vector diretion toward the place mat
	float j;
	float k;
} PHOTOG_PARAMS;

typedef struct camera_params {
	float fx;
	float fy;
	float cx;
	float cy;
	float k1;
	float k2;
	float k3;
	float p1;
	float p2;
	float camera_x;  //optical center of the camera
	float camera_y;
	float camera_z;
} CAMERA_PARAMS;

typedef struct blobpoint_params
{
	int imageX;
	int imageY;
	int blobPointID;
	int photoBlobID;
	int foodItemID;
	float cameraX;
	float cameraY;
	float cameraZ;
	float worldX;
	float worldY;
	float worldZ;
} BLOBPOINT_PARAMS;

typedef struct rayintersect_params
{
	int foodItemID;
	float intersectX;
	float intersectY;
	float intersectZ;
	float intersectDistance;
	float distanceFromCamera;
	float cameraX;
	float cameraY;
	float cameraZ;
	float worldX;
	float worldY;
	float worldZ;
	int blobpoint1ID;
	int blobpoint2ID;
	int blob1ID;
	int blob2ID;
} RAYINTERSECT_PARAMS;

typedef struct food_rays {
	BLOBPOINT_PARAMS *blobpoints;
	CAMERA_PARAMS *camera;
	int blobPointCount;
} FOOD_RAYS;

typedef struct meal {
	int *image_ids;
	int count;
} MEAL;

typedef struct blob_center {
	int blob_id;
	int center_x;
	int center_y;
} BLOB_CENTER;

typedef struct ray_data {
	double x;
	double y;
	double z;
	double distance_apart;
} RAY_DATA;

typedef struct blobpoint_raydata {
	int	image_x;
	int image_y;
	float projected_x;
	float projected_y;
	float projected_z;
} BLOBPOINT_RAYDATA;

int draw_monument_point(char *filename, int x, int y, int shading_factor, SIMPLE_POINT *pts, int count);
PPLACEMAT_POINT get_placemat_points(int imageid);
INTERSECT_POINT_3D * closest_point_between_2_lines(POINT_3D *p1, POINT_3D *p2,
								  POINT_3D *p3, POINT_3D *p4);
int fill_plate_white(char *filename);
int FindFood(char *filename);
int FindFood2(char *filename);
int pick_food_items();
int createRays(int photoblob_id);
IMAGE_2D *calculateImagePoint(int monumentPointID);
IMAGE_2D *validatePhotoG(int monumentPointID);
int findFoodbyRays(int meal_id);
POINT_3D *createRayPoint(PHOTOG_PARAMS *pp);
int matchFoodItems(int meal_id, int grouptype_id);
int createRay(PHOTOG_PARAMS *pp);
int undistortImage(char *filename);

int fixSegmentation(void);
float calculateVolume(char *inputfile);


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

#define STDMEAN22			18
#define STDMEAN21			19
#define STDMEAN31			20

IMAGEPOINT * create_imagepoint_item(void);
PHOTOG_PARAMS * getCameraForRays(int photoBlobID);

int wrightGeomViewPointFile(char *inputfile, char *outputfile);
int histogramPointCloud(char *name, int foodItemID);
ELLIPSEPARAMS *fitEllipse(int xArray[], int yArray[], int count);


#define GEOMETRY_H
#endif //GEOMETRY_H
