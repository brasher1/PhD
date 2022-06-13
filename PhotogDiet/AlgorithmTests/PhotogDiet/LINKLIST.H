#ifndef __LINKLIST_H__
#define __LINKLIST_H__

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include <windows.h>
/*** LINK LIST STRUCTS *******************************/
/*#defines for linked lists*/
struct node
{
   void *item;
   struct header *header;
   struct node *next, *prev;
};
typedef struct node node_t;

struct header
{
   int   count;
   node_t *first, *last;
   unsigned char red;
   unsigned char green;
   unsigned char blue;
   int average_x;
   int average_y;
   int min_x;
   int min_y;
   int max_x;
   int max_y;
   int pixel_count;
   float Cx;
   float Cy;
   float R1;
   float R2;
   float Theta;
   float sdCx;
   float sdCy;
   float sdR1;
   float sdR2;
   float sdTheta;
   float area;
   float percentOfImage;
   int   isPlate;
   int   isFood;
   float bbox_sizeX;
   float bbox_sizeY;
   float bbox_centerX;
   float bbox_centerY;
   float bbox_angle;
   float bbox_area;
   int  bbox_containedObjects;
   int photoblob_id;
};
typedef struct header header_t;
/*****************************************************/

header_t *create_header( void);
node_t *instance_node( void *data, node_t *prev, node_t *next);
int insert_node( void *data, header_t *list);
int append_node(void *data, header_t *list);
void *del_first_node(header_t *list);
void *del_last_node(header_t *list);
void *del_any_node(node_t *node);
node_t *get_node_at(header_t *hdr, int i);
void destroy_list( header_t *head_list );



#endif //__LINKLIST_H__
