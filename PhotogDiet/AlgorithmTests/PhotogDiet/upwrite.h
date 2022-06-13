
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
 *
 * Modified by:     David John Brasher
 * E-mail:			brasher1@uab.edu
 * Date:			2010
 * Last Modified:	
 * Organization:	University of Alabama Birmingham
 *					Dept. Electrical and Computer Engineering
 *					
 */
/* MACROS.
 */
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>


extern double   sqrarg;
#define SQR(a)  ( (sqrarg=(double)(a)) == 0.0 ? 0.0 : sqrarg*sqrarg)

extern double   cubarg;
#define CUBE(a)  ((cubarg=(double )(a)) == 0.0 ? 0.0 : cubarg*cubarg*cubarg)

extern double   absarg;
#define ABS(a)  ((absarg=(a)) > 0 ? absarg : -absarg)

#define SIGN(a,b)   ((b) >= 0.0 ? fabs(a) : -fabs(a))

#define alloc_vector_MACRO(dim) ((double *)malloc(( (unsigned int)(dim+1) ) * sizeof(double)) )



/* PARAMETERS.
 */
#define NUM_RANDOM_COLORS   40
#define NEIGHBOURHOOD       (2*gResolutionRadius)
#define DISPLAY_FONT_NAME   "fixed"
#define CHUNK_NEIGHBOURHOOD 15.0
#define CHUNK_TOLERANCE     20.0
#define MAX_NUM_SHT_AND_PHT_LINES   200
#define	WINDOW_SIZE_X       700     /* Dimension of window */
#define	WINDOW_SIZE_Y       700     /* Dimension of window */



/* DEFINITIONS.
 */
#define NORMAL              0
#define HIGHLIGHT           1

#define WHITE   63
#define BLACK   0


#define A_VERY_SMALL_POSITIVE_VALUE 10e-5
#define A_FAIRLY_SMALL_POSITIVE_VALUE   1

#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE
#define FALSE   0
#endif

#ifndef M_PI
#define M_PI        3.14159265358979323846
#endif
#ifndef M_PI_2
#define M_PI_2      1.57079632679489661923
#endif

/* sqrt(2*M_PI) : used in calculating likelihoods.
 */
#define SQRT_2_PI   2.50662827463100050241




/* TYPE DECLARATIONS
 */
typedef int BOOLEAN;


typedef int (*func_ptr)();  /* a pointer to a function. */

#ifdef DUMMY
typedef struct {
    int     x, y;       /* Position of top left corner of menu. */
    int     num_options;
    char    **label;    /* The labels which appear in each
                         * menu option.
                         */
//    Window  bkgd_win;   /* Window associated with the menu. */
//    GC      gc;         /* GC associated with the menu. */
   // Window  *pane;      /* Window associated with each option. */
//    XFontStruct *font_info; /* info on the font used. */
    int     width;      /* in pixels of the menu (not incl. border width). */
    int     height;     /* in pixels of the menu  (not incl. border width)*/
    int     pane_height;    /* in pixels of each individual pane */
    int     border_width;   /* in pixels, for the border around
                             * the outside of the menu.
                             * Each pane has a border width of 1 pixel.
                             */
    func_ptr    *callback;      /* Pointer to an array of functions.
                                 * The first function in the array
                                 * is called when the first menu option
                                 * is selected,
                                 * The second function in the array
                                 * is called when the second menu option
                                 * is selected,
                                 * etc.
                                 */
} MENU;
#endif //DUMMY

#ifdef DUMMY
typedef struct {
    int     x, y;       /* Position of top left corner of parent menu. */
    int     num_menus;  /* number of children menus. */
    char    **label;    /* The label which appears in
                         * parent menu option.
                         */
    Window  bkgd_win;   /* Window associated with the parent menu. */
    GC      gc;         /* GC associated with the menu. */
    Window  *pane;      /* Window associated with each option. */
    XFontStruct *font_info; /* info on the font used. */
    int     width;      /* in pixels of the menu (not incl. border width). */
    int     height;     /* in pixels of the menu  (not incl. border width)*/
    int     pane_height;    /* in pixels of each individual pane */
    int     border_width;   /* in pixels, for the border around
                             * the outside of the menu.
                             * Each pane has a border width of 1 pixel.
                             */
    MENU    **menus;        /* Pointer to an array of menus.
                         * One menu for each option in the arent menu.
                         */

} PARENT_MENU;
#endif //DUMMY

#ifdef DUMMY
typedef struct {

    int     x, y;       /* Positin of top left corner of bkgd_win. */
    char    *message;   /* string displayed in message_win. */
    Window  bkgd_win;   /* Main Window. */
    Window  message_win, input_win;     /* sub-windows. */
    GC      gc;
    XFontStruct *font_info; /* info about the font used. */
    int     width;      /* in pixels of the windows (not incl. border width).*/
    int     height;     /* in pixels of bckg_win (not incl. border width). */
    int     pane_height;    /* inpixels if sub-windows */
    int     border_width;   /* in pixels. */

} INPUT_PROMPT;
#endif //DUMMY

#ifdef DUMMY
typedef struct {
    int     x, y;
    int     num_strings;    /* number of strings to be displayed */
    char    **message;      /* array of the strings displayed in message_win.*/
    Window  win;    /* Main Window. */
    GC      gc;
    XFontStruct *font_info; /* info about the font used. */
    int     width;      /* in pixels of the windows (not incl. border width).*/
    int     height;     /* in pixels of bckg_win (not incl. border width). */
    int     line_height;    /* in pixels of each line of text in the window */
    int     border_width;   /* in pixels. */

} MESSAGE_WINDOW;
#endif //DUMMY


typedef struct image_type {
    unsigned int    x, y;   /* dimensions of image
                             * Allowable values for x are [0, x)
                             *                      y are [0, y)
                             */
    unsigned char   **data; /* The pixel values for the image.
                             * All values are in the range [0,63].
                             */
} IMAGE;



typedef struct point_type {
    int dim;
    double  *x;         /* indexed [1..dim] */
    BOOLEAN flag;       /* generic flag - to be used for whatever
                         * seems important at the time.
                         */
    struct point_type   *next;
    struct point_type   *next_list;
} POINT;



typedef struct point_list_type {
    POINT   *head;
    POINT   *tail;
    int     num_elements;   /* number of elements in linked list. */
    struct  point_list_type *next_list;

} POINT_LIST;




typedef struct gaus_type {
    int dim;
    double  num_points;     /* number of data points that gaussian
                             */
    double  *mean;          /* mean of gaussian.  indexed [1..dim] */
    double  **covar;        /* covariance matrix of gaussian
                             * indexed [1..dim][1..dim]
                             * and [row][column]
                             */
    double  **inverse_covar;    /* Inverse of covariance matrix.
                                 */
    double  *eig_val;       /* eigenvalues of covariance matrix
                             * in descending order.
                             * indexed [1..dim]
                             */
    double  **eig_vec;      /* normalised eigenvectors (i.e. length of 1)
                             * eig_vec[][1] corresponds to eig_val[1] etc.
                             * indexed [1..dim][1..dim]
                             * and [row][column]
                             */
    double  sqrt_det;       /* determinant of covariance matrix.
                             * Used when calculating likelihoods.
                             */
    double  weight;     /* A weight attatched to the gaussian.
                         * Usually set to 1.0.
                         * However, when using a gaussian mixture model
                         * may be less than 1.0.
                         * such that all weights in the gaussian
                         * mixture model sum to 1.0
                         */
    BOOLEAN flag;       /* generic flag - to be used for whatever
                         * seems important at the time.
                         */
    double  value;      /* generis value - to be used however seems
                         * appropriate.
                         */
    int color;          /* a color assigned to the GAUS.
                         */
    char    *label;     /* generic label. Assumed to be NULL terminated
                         * To be used however seems
                         * appropriate.
                         */
    struct gaus_type *original;     /* Sometimes, a GAUS is a copy of
                                     * some original GAUS.
                                     * 'original' is a pointer to the original
                                     * gaus.
                                     */
    POINT_LIST  *point_list;/* An array of the points which formed the GAUS.
                             * This is set in the spot algorithm.
                             */
    struct gaus_type    *next;      /* Used in linked lists of gaus */
    struct gaus_type    *prev;      /* Used in doubly linked lists of gaus */

} GAUS;



typedef struct gaus_list_type {
    GAUS    *head;
    GAUS    *tail;
    int     num_elements;   /* number of elements in linked list. */
	
	int	max_x, max_y, min_x, min_y;
	int mean_x, mean_y;
	double shading;

    struct  gaus_list_type  *next_list;

} GAUS_LIST;



/* A 2-D array of pointers to GAUS.
 */
typedef struct {
    int     x, y;       /* dimensions of the array. */

    GAUS    ***gaus;    /* gaus[x][y] is a pointer to a GAUS.
                         * x must be in the range [0, x).
                         * y must be in the range [0, y).
                         */

} GAUS_ARRAY;


typedef struct chunk_type {
    GAUS_LIST   *gaus_list;
    POINT       *point;
    BOOLEAN     flag;           /* Generic flag - to be used for whatever
                                 * seems important at the time.
                                 */


    struct chunk_type   *next;
    struct chunk_type   *prev;

} CHUNK;



typedef struct chunk_list_type {
    CHUNK   *head;
    CHUNK   *tail;
    int     num_elements;   /* number of elements in linked list. */
    struct  chunk_list_type *next_list;

} CHUNK_LIST;



typedef struct basis_type {
    int dim;                /* dimension of basis */
    int dim_std_basis;      /* dimension of standard basis
                             * since this data structure will often
                             * describe a subspace, basis->dim_std_basis
                             * may be larger than basis->dim.
                             */
    double  *origin;        /* position of origin relative to standard basis
                             * indexed [1..dim]
                             */
    double  **vector;       /* basis vectors
                             * indexed [1..dim][1..dim]
                             * and [row][column]
                             */
    struct basis_type   *next;      /* Used in linked lists */
    struct basis_type   *next_list; /* Used in linked lists of linked lists */

} BASIS;




typedef struct black_pixel_type {
    int     x, y;
    double  angle;          /* of tangent */
    double  slope;          /* tan(angle) */
    struct black_pixel_type *next;
    struct black_pixel_type *prev;
} BLACK_PIXEL;


typedef struct black_pixel_list_type {
    BLACK_PIXEL     *head;
    BLACK_PIXEL     *tail;
    int             num_elements;   /* number of elements in linked list. */

} BLACK_PIXEL_LIST;




/* A 2-D array, of the same size as the image.
 * There is an entry corresponding to each black pixel in the image.
 * The entry is a pointer to the corresponding entry in the linked list
 * of BLACK_PIXEL.
 */
typedef struct {
    int         x, y;
    BLACK_PIXEL ***p;

} MAP_FROM_POSITION_TO_BLACK_PIXEL;






/* Data Structures used for displaying
 * recognised objects.
 */
typedef enum {upwrite, hough_line, hough_circle, hough_ellipse} OBJECT_TYPE;

struct upwrite_type {
    int     x, y;
    char    *label;
//    XFontStruct *font_info; /* info about the font used. */
    short   width;          /* width of label (in pixels) */
    int     ascent;         /* ascent of label (in pixels) */
    int     descent;        /* descent of label (in pixels) */
    int     color_index;
};


struct hough_line_type {
    int     x_start, y_start;
    int     x_end, y_end;
    int     width;
    int     color_index;
};

struct hough_circle_type {
    int     x, y;       /* Upper-left corner of bounding rectangle */
    unsigned int    diameter;
    int width;      /* line thickness */
    int     color_index;
};

struct hough_ellipse_type {
    double  x, y;       /* centre */
    double  a, b;       /* length of major/minor axes */
    double  theta;      /* angle of major axis */
    int width;      /* line thickness */
    int     color_index;
};



typedef struct display_object_type {
    OBJECT_TYPE             type;

    union {
        struct upwrite_type         upwrite;
        struct hough_line_type      hough_line;
        struct hough_circle_type    hough_circle;
        struct hough_ellipse_type   hough_ellipse;
    } obj;          /* specific to the type of object being drawn */

    struct display_object_type  *next;

} DISPLAY_OBJECT;



typedef struct display_object_list_type {
    DISPLAY_OBJECT  *head;
    DISPLAY_OBJECT  *tail;
    int     num_elements;   /* number of elements in linked list. */

} DISPLAY_OBJECT_LIST;




/* GLOBAL DECLARATIONS.
 */
extern double           gResolutionRadius;
//extern Display          *gDisplay;
//extern Colormap         gColormap;
extern int              gLineThickness;
//extern XColor           *gGreyValues;
//extern XColor           *gRandomColors;
extern int              gNumGreyLevels; /* The number of grey levels
                                         * that will be displayed
                                         * in the image.
                                         */
extern long int         gTime;      /* Used in coloring algorithm */
extern BOOLEAN          gGraphics;
extern POINT_LIST       *gTrainingPointList;
extern int              gOldCountTrainingPoint;
extern GAUS_LIST        *gClassList;
extern DISPLAY_OBJECT_LIST  *gDisplayObjectList;
extern int              gVisualDepth;
extern double           drand48(void);
