/******************************************************************
LINK_LST.C
 
Taken from Advanced C: Techniques & Applications
 
*******************************************************************/
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "linklist.h"
#include "geometry.h"


////////////////////////////////////////////////////////////////////
header_t *create_header(void)
{
    header_t *new;
 
    if((new = (header_t *)GlobalAlloc(GMEM_FIXED,sizeof(header_t))) != NULL )
    {
        new->count = 0;
		new->pixel_count = 0;
		new->isPlate = 0;
		new->isFood = 0;
        new->first = new->last = NULL;
		new->bbox_containedObjects = 0;
    }
    return(new);
}
////////////////////////////////////////////////////////////////////
IMAGEPOINT * create_imagepoint_item(void)
{
    IMAGEPOINT *item;
 
    if((item = (IMAGEPOINT *)GlobalAlloc(GMEM_FIXED,sizeof(IMAGEPOINT))) != NULL )
    {
        item->blue=0;
		item->connectedblue = 0;
		item->connectedgreen = 0;
		item->connectedred = 0;
		item->green = 0;
		item->imageX = 0;
		item->imageY = 0;
		item->red = 0;
    }
    return(item);
}
 
////////////////////////////////////////////////////////////////////
node_t *instance_node( void *data, node_t *prev, node_t *next)
{
    node_t *new;
 
    if( (new = (node_t *)GlobalAlloc(GMEM_FIXED,sizeof(node_t)))!= NULL )
    {
        new->item = data;
        new->prev = prev;
        new->next = next;
    }
    return(new);
}
////////////////////////////////////////////////////////////////////
 
////////////////////////////////////////////////////////////////////
int insert_node(void *data, header_t *list )
{
    node_t *new;
 
    if( (new = instance_node( data, NULL, list->first )) != NULL )
    {
        if( list->count )
            list->first->prev = new;
        else
            list->last = new;
        list->first = new;
        list->count++;
        new->header = list;
        return(TRUE);
    }
    else
        return(FALSE);
}
////////////////////////////////////////////////////////////////////
 
////////////////////////////////////////////////////////////////////
int append_node(void *data, header_t *list )
{
    node_t *new;
 
    if( (new = instance_node(data,list->last, NULL )) != NULL)
    {
        if( list->count )
            list->last->next = new;
        else
            list->first = new;
        list->last = new;
        list->count++;
        new->header = list;
        return(TRUE);
    }
    else
        return(FALSE);
}
////////////////////////////////////////////////////////////////////
 
////////////////////////////////////////////////////////////////////
void *del_first_node(header_t *list)
{
    node_t *temp;
    void *data;
 
    temp = list->first;
    data = temp->item;
    list->first = temp->next;
    list->count--;
    if( list->count )
        list->first->prev = NULL;
    else
        list->last = NULL;
    GlobalFree((char *)temp);
    return(data);
}
////////////////////////////////////////////////////////////////////
 
////////////////////////////////////////////////////////////////////
void *del_last_node( header_t *list)
{
    node_t *temp;
    void *data;
 
    temp = list->last;
    data = temp->item;
    list->last = temp->prev;
    list->count--;
    if( list->count )
        list->last->next = NULL;
    else
        list->first = NULL;
    GlobalFree((char *)temp);
    return(data);
}
////////////////////////////////////////////////////////////////////
 
////////////////////////////////////////////////////////////////////
void *del_any_node( node_t *node)
{
    void *data;
 
    if( node->prev == NULL )
        return( del_first_node(node->header) );
    if( node->next == NULL )
        return( del_last_node(node->header) );
    node->prev->next = node->next;
    node->next->prev = node->prev;
    node->header->count -= 1;
    data = node->item;
    GlobalFree(node);
    return(data);
}
////////////////////////////////////////////////////////////////////
 
////////////////////////////////////////////////////////////////////
void destroy_list( header_t *head_list )
{
    if( head_list != NULL )
    {
        while( head_list->count > 0 )
		{
			if( head_list->pixel_count > 0 )
				GlobalFree( (IMAGEPOINT *)del_first_node(head_list));
			else
				del_first_node(head_list);
		}
        GlobalFree( head_list );
    }
}
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
node_t *get_node_at(header_t *hdr, int i)
{
	node_t *node;
	int j;
	node = hdr->first;
	if( i < hdr->count )
	{
		for( j = 0; j<i; j++ )
			node = node->next;
	}
	else
		return NULL;
	return node;
}
////////////////////////////////////////////////////////////////////
