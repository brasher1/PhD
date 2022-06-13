/*
 * Programmed by:   Robert Ainsley McLaughlin
 * E-mail:          ram@ee.uwa.edu.au
 * Date:            18 August, 1996
 * Last modified:   1 March, 1998
 * Organisation:    The Center For Intelligent Information
 *                      Processing Systems,
 *                  Dept. Electrical & Electronic Engineering,
 *                  The University of Western Australia,
 *                  Nedlands W.A. 6907,
 *                  Australia
 */
/* Data Structures.
 */
typedef enum {leaf, branch} NODE_TYPE;

typedef enum {lower, within, upper, same} COMPARISON_TYPE;

typedef struct node_type {
    POINT               *data;          /* contains data */
    int                 count;          /* a count */
    NODE_TYPE           type;           /* leaf or branch */
    struct node_type    *next;          /* doubly linked list */
    struct node_type    *prev;
    struct node_type    *branch;        /* branch of tree coming off this node*/
} NODE;
