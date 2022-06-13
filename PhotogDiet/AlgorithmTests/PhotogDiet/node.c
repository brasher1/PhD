/*
 * Programmed by:   Robert Ainsley McLaughlin
 * E-mail:          ram@ee.uwa.edu.au
 * Date:            18 August, 1996
 * Last modified:   23 Feb, 1998
 * Organisation:    The Center For Intelligent Information
 *                      Processing Systems,
 *                  Dept. Electrical & Electronic Engineering,
 *                  The University of Western Australia,
 *                  Nedlands W.A. 6907,
 *                  Australia
 */
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <math.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "../include/upwrite.h"
#include "../include/rht.h"






/* Globals
 */
extern int      max_level;


/* Function Prototypes */
NODE            *InsertNode( POINT *p, NODE *tree, double tolerance[],
                                    double quantisation[] );
NODE            *InsertNode_Child( POINT *p, NODE *tree, int level,
                                    double tolerance[], double quantisation[] );
COMPARISON_TYPE CompareNodes( POINT *p1, POINT *p2, int level,
                                    double tolerance[], double quantisation[]);
NODE            *alloc_node(void);
void            free_node(NODE *node);
void            display_tree(NODE *tree, int x_base, int y_base);
int             output_node(NODE *node, FILE *dat_file,
                                    FILE *lines_file, int line_number);
NODE            *find_max_node(NODE *node, NODE *max_node);
int             tree_size(NODE *node, int size);




NODE *
InsertNode( POINT *p, NODE *tree, double tolerance[], double quantisation[] )
{
    NODE    *InsertNode_Child( POINT *p, NODE *tree, int level, double tolerance[], double quantisation[]);


    if (p == (POINT *)NULL)
        {
        return( tree );
        }

    return InsertNode_Child( p, tree, 1, tolerance, quantisation );
}




NODE *
InsertNode_Child( POINT *p, NODE *tree, int level, double tolerance[], double quantisation[] )
{

    NODE    *new_node;
    NODE    *alloc_node(void);
    int     i;


    if( tree==(NODE *)NULL )
        {
        new_node = alloc_node();
        new_node->type  = leaf;
        new_node->data  = p;
        new_node->count = 1;
        new_node->prev  = (NODE *)NULL;
        new_node->next  = (NODE *)NULL;
        new_node->branch= (NODE *)NULL;

        return(new_node);
        }
    
    switch( CompareNodes( p, tree->data, level, tolerance, quantisation ) )
        {

        case lower:
                new_node = alloc_node();
                new_node->type  = leaf;
                new_node->data  = p;
                new_node->count = 1;
                new_node->prev  = tree->prev;
                new_node->next  = tree;
                new_node->branch= (NODE *)NULL;

                tree->prev = new_node;

                return(new_node);

        case within:
                if( tree->type==branch )
                    {
                    tree->branch = InsertNode_Child(p,
                                                tree->branch,
                                                level+1, tolerance,
                                                quantisation );
                    return(tree);
                    }
                else
                    {
                    /* Make a new leaf node, a copy of the
                     * current leaf node
                     */
                    new_node = alloc_node();
                    new_node->type  = leaf;
                    new_node->data = tree->data;
                    new_node->count = tree->count;
                    new_node->prev  = (NODE *)NULL;
                    new_node->next  = (NODE *)NULL;
                    new_node->branch= (NODE *)NULL;

                    /* Change the current leaf node into a branch node */
                    tree->type  = branch;

                    /* Insert p into the new branch */
                    tree->branch = InsertNode_Child( p,
                                        new_node,
                                        level+1, tolerance, quantisation );
                    return(tree);
                    }

        case upper:
                if( tree->next != (NODE *)NULL )
                    {
                    tree->next = InsertNode_Child(p, tree->next,
                                            level, tolerance, quantisation);
                    return(tree);
                    }
                else
                    {
                    new_node = alloc_node();
                    new_node->type  = leaf;
                    new_node->data = p;
                    new_node->count = 1;
                    new_node->prev  = tree;
                    new_node->next  = (NODE *)NULL;
                    new_node->branch= (NODE *)NULL;

                    tree->next = new_node;

                    return(tree);
                    }

        case same:
                    if (tree->type == leaf)
                        {
                        /* Update data co-ordinates
                         * as a weighted average of the old and
                         * new data.
                         */
                        for (i=1; i <= tree->data->dim; i++)
                            tree->data->x[i] = (p->x[i] +
                                                        tree->data->x[i] *
                                                        (double )tree->count) /
                                                (double )(tree->count+1);
                        /* Update count */
                        tree->count++;
                        return(tree);
                        }
                    else                /* branch */
                        {
                        tree->branch = InsertNode_Child( p, tree->branch, level+1, tolerance, quantisation );
                        return(tree);
                        }
                default:                    /* This should never be reached */
                    return((NODE *)NULL);

                }   /* end of 'switch( CompareNodes...' */


}       /* end of InsertNode_Child()*/



/* tolerance[] is used to decide if two leaves are the same
 * (to within some tolerance).
 * quantisation[] is used to decide where to put a new leaf into
 * the tree.
 */
COMPARISON_TYPE
CompareNodes( POINT *p1, POINT *p2, int level, double tolerance[], double quantisation[])
{
    BOOLEAN same_flag;
    int     i;

    if (p1->dim != p2->dim)
        {
        fprintf(stderr, "comparing data of differing dimension in CompareNodes() in cell.c\n");
        exit(1);
        }


    /* Are they the same in all values (to within some tolerance) */
    same_flag = TRUE;
    for (i=1; i <= p1->dim; i++)
        {
        if (i != 5)
            if (ABS(p1->x[i] - p2->x[i]) > tolerance[i])
                {
                same_flag = FALSE;
                break;
                }
        else                /* Comparing theta */
            {
            if ((ABS(p1->x[i] - p2->x[i]) > tolerance[i]) &&
                    (ABS(p1->x[i] - p2->x[i]+M_PI) > tolerance[i]) &&
                    (ABS(p1->x[i] - p2->x[i]-M_PI) > tolerance[i]))
                {
                same_flag = FALSE;
                break;
                }
            }
        }
    if (same_flag == TRUE)
        return(same);

    /* Not the same values, give an order based on a particular value */
    if (level != p1->dim)
        {
        if ((int )(p1->x[level]/quantisation[i]) == (int )(p2->x[level]/quantisation[i]))
            return(within);
        else if (p1->x[level] < p2->x[level])
            return(lower);
        else
            return(upper);
        }
    else                        /* level == p1->dim, don't allow WITHIN */
        {
        if (p1->x[level] <= p2->x[level])
            return(lower);
        else
            return(upper);
        }

}   /* end of CompareNodes() */






NODE *
alloc_node(void)
{
    NODE    *c;


    c = (NODE *)malloc(sizeof(NODE));
    if (c == (NODE *)NULL)
        {
        fprintf(stderr, "malloc failed in alloc_tree_node() in tree_node.c\n");
        exit(1);
        }
    c->count = 0;
    c->data = (POINT *)NULL;
    c->next = (NODE *)NULL;
    c->prev = (NODE *)NULL;
    c->branch = (NODE *)NULL;

    return(c);

}   /* end of alloc_node() */



void
free_node(NODE *node)
{
    void    free_node(NODE *node);
    void    free_point(POINT *p);


    if (node == (NODE *)NULL)
        return;

    if (node->type == leaf)
        {
        free_point(node->data);
        }
    else if (node->type == branch)
        {
        free_node(node->branch);
        }

    free_node(node->next);

}   /* end of free_node() */



int
output_node(NODE *node, FILE *dat_file, FILE *lines_file, int line_number)
{
    int     i;
    int     output_node(NODE *node, FILE *dat_file,
                                    FILE *lines_file, int line_number);

    if (node == (NODE *)NULL)
        return(line_number);

    if (node->type == leaf)
        {
        for (i=1; i <= node->data->dim; i++)
            fprintf(dat_file, "%f ", (float )node->data->x[i]);
        fprintf(dat_file, "0\n");
        fflush(dat_file);

        for (i=1; i <= node->data->dim; i++)
            fprintf(dat_file, "%f ", (float )node->data->x[i]);
        fprintf(dat_file, "%d\n", node->count);
        fflush(dat_file);

        fprintf(lines_file, "%d %d\n", line_number+1, line_number+2);
        line_number+=2;
        fflush(lines_file);
        }
    else if (node->type == branch)
        {
        line_number = output_node(node->branch, dat_file,
                                    lines_file, line_number);
        }

    line_number = output_node(node->next, dat_file, lines_file, line_number);


    return(line_number);

}       /* end of output_node() */


NODE *
find_max_node(NODE *node, NODE *max_node)
{
    NODE    *find_max_node(NODE *node, NODE *max_node);



    if (node == (NODE *)NULL)
        return( (NODE *)max_node );

    if (node->type == leaf)
        {
        if (max_node == (NODE *)NULL)
            max_node = node;
        else if (node->count > max_node->count)
            max_node = node;
        }
    else if (node->type == branch)
        {
        max_node = find_max_node(node->branch, max_node);
        }

    max_node = find_max_node(node->next, max_node);


    return(max_node);

}       /* end of find_max_node() */


int
tree_size(NODE *node, int size)
{
    if (node == (NODE *)NULL)
        return(size);


    if (node->type == leaf)
        size++;
    else if (node->type == branch)
        size = tree_size(node->branch, size);


    size = tree_size(node->next, size);

    return(size);

}       /* end of tree_size() */
