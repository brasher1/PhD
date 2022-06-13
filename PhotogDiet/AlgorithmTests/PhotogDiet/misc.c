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
 *
 * Modified by:     David John Brasher
 * E-mail:			brasher1@uab.edu
 * Date:			2010
 * Last Modified:	
 * Organization:	University of Alabama Birmingham
 *					Dept. Electrical and Computer Engineering
 *					
 */
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>
#include <string.h>
#include <direct.h>

#include "upwrite.h"



/* Function Prototypes
 */
double  **alloc_array(int row, int column);
void    free_array(double **A, int row);
double  *copy_column_vector(double **A, int row, int column, int i);
double  *alloc_vector(int dim);
void    output_model(char *label, GAUS_LIST *gaus_list);
void    input_classes(void);
void    output_training_data(GAUS_LIST *gaus_list);
BOOLEAN valid_character(char c, char *valid_characters);
int fprint_gaus(char *name, GAUS *gaus);
int fprint_gaus_list(char *name, GAUS_LIST *gaus);






double **
alloc_array(int row, int column)
{
    double  **rowp;
    int     a;


    if ( (rowp=(double **)malloc((unsigned )(row+1) * sizeof(double *)) ) == NULL)
        return(NULL);
    for(a=1;a<=row;a++)
        {  
        if ((rowp[a]=(double *)malloc((unsigned)(column+1)*sizeof(double))) == NULL)
            return(NULL);
        }  

    return(rowp);

}   /* end of alloc_array() */


void
free_array(double **A, int row)
{
    int i;

    for (i=1; i <= row; i++)
        free(A[i]);
    free(A);

}       /* end of free_array() */



/* Copy column vector i of A[1..row][1..column]
 * Return a pointer to the copy.
 */
double *
copy_column_vector(double **A, int row, int column, int i)
{
    double  *v;
    int     j;

    if ((i < 1) || (i > column))
        {
        fprintf(stderr, "Warning: Tried to access column %d of a %dx%d matrix\n", i, row, column);
        fprintf(stderr, "in copy_column_vector() in misc.c\n");
        exit(1);
        }
    v = (double *)malloc((unsigned )(row+1) * sizeof(double));
    if (v == NULL)
        {
        fprintf(stderr, "malloc failed in copy_column_vector() in misc.c\n");
        exit(1);
        }

    for (j=1; j <= row; j++)
        v[j] = A[j][i];


    return(v);

}   /* end of copy_column_vector() */



double  *alloc_vector(int dim)
{
    return( (double *)malloc(( (unsigned )(dim+1) ) * sizeof(double)) );
}



/* Output the linked list of GAUS in 'gaus_list'
 * in a form that can later be used for recognition.
 */
void
output_model(char *label, GAUS_LIST *gaus_list)
{
    FILE    *outfile;
    char    filename[550];
    GAUS    *gaus;
    void    write_gaus_to_a_file(FILE *out_file, GAUS *gaus);


    if ((gaus_list != NULL) && (gaus_list->head != NULL))
        {
        strncpy(filename, label, 500);
        strcat(filename, ".upwritedata");
        outfile = fopen(filename, "w");
        if (outfile == NULL)
            {
            fprintf(stderr, "Could not open %s for output in output_model() in misc.c\n", label);
            exit(1);
            }

        fprintf(outfile, "%s\n", label);

        for (gaus=gaus_list->head; gaus != NULL; gaus=gaus->next)
            write_gaus_to_a_file(outfile, gaus);

        fclose(outfile);
        }



}   /* end of output_model() */



/* Read in all files with the postfix .upwritedata
 */
void
input_ellipse_classes(char *exepath)
{
   char pathbuffer[_MAX_PATH];
   char searchfile[] = "ellipse.upwritedata";

   char envvar[] = "PATH";
//   wchar_t *exedir;
	int k;
//	struct dirent   *dp;
//    DIR         *dirp;
    FILE        *infile;
    GAUS        *gaus;
    GAUS_LIST   *gaus_list;
//    int         length;
//    char        tmp_str[13], 
	char label[500];
    int         return_val;
    GAUS        *read_gaus_from_file(FILE *in_file, int *return_val);
    GAUS_LIST   *put_in_linked_list_of_gaus(GAUS_LIST *gaus_list, GAUS *g);
    GAUS_LIST   *put_in_linked_list_of_linked_list_of_gaus(GAUS_LIST *head_list, GAUS_LIST *list);


    gClassList = NULL;


    /* Find all files in directory with the postfix .upwritedata
     * Read these in as data files.
     */
/*    if ((dirp=opendir(".")) == NULL)
        {
        fprintf(stderr, "Could not check current directory for files with .upwritedata postfix\n");
        fprintf(stderr, "in input_classes() in misc.c\n");
        exit(1);
        }
*/
	_getcwd(pathbuffer, _MAX_PATH);

	for( k=0; k<1; k++)
	{
	   // Search for file in PATH environment variable:
	   _searchenv( searchfile, envvar, pathbuffer ); // C4996
	   // Note: _searchenv is deprecated; consider using _searchenv_s
	   if( *pathbuffer != '\0' )
	   {
		  printf( "Path for %s:\n%s\n", searchfile/*searchfile*/, pathbuffer );
		  infile = fopen(pathbuffer, "r");
	   }
	   else
	   {

		   strcpy(pathbuffer, exepath);
		   strcat(pathbuffer, "\\");
		   strcat(pathbuffer, searchfile);
		  infile = fopen(pathbuffer, "r");
		  //printf( "%s not found\n", searchfile );
	   }

		   strcpy(pathbuffer, exepath);
		   strcat(pathbuffer, "\\");
		   strcat(pathbuffer, searchfile);
		  infile = fopen(pathbuffer, "r");
	   if (infile == NULL)
						{
						fprintf(stderr, "Could not open %s for input in input_classes() in misc.c\n", pathbuffer);
						exit(11);
						}

					/* Read in the label for this class of objects.
					 */
					if (fscanf(infile, "%s", label) != 1)
						{
						fprintf(stderr, "Could not read %s for input. Continuing.\n", pathbuffer);
	//                    continue;
						}

					/* Read in the linked list of GAUS
					 * modelling this class of objects.
					 */
					gaus_list = NULL;
					do
						{
						gaus = read_gaus_from_file(infile, &return_val);

						if (gaus != NULL)
							{
							gaus->label = (char *)malloc((1+strlen(label)) * sizeof(char) );
							if (gaus->label == NULL)
								{
								fprintf(stderr, "malloc failed in input_classes() in misc.c\n");
								exit(1);
								}
							strcpy(gaus->label, label);
							gaus_list = put_in_linked_list_of_gaus(gaus_list, gaus);
							}
						}
					while (return_val == 0);

					gClassList = put_in_linked_list_of_linked_list_of_gaus(gClassList, gaus_list);

	}
}

/* Read in all files with the postfix .upwritedata
 */
void
input_classes(char *exepath)
{
   char pathbuffer[_MAX_PATH];
   char searchfile[] = "ellipse.upwritedata";
   char *fnames[] = {"ellipse.upwritedata",
					"line.upwritedata",
					"circle.upwritedata"};

   char envvar[] = "PATH";
//   wchar_t *exedir;
	int k;
//	struct dirent   *dp;
//    DIR         *dirp;
    FILE        *infile;
    GAUS        *gaus;
    GAUS_LIST   *gaus_list;
//    int         length;
//    char        tmp_str[13], 
	char label[500];
    int         return_val;
    GAUS        *read_gaus_from_file(FILE *in_file, int *return_val);
    GAUS_LIST   *put_in_linked_list_of_gaus(GAUS_LIST *gaus_list, GAUS *g);
    GAUS_LIST   *put_in_linked_list_of_linked_list_of_gaus(GAUS_LIST *head_list, GAUS_LIST *list);


    gClassList = NULL;


    /* Find all files in directory with the postfix .upwritedata
     * Read these in as data files.
     */
/*    if ((dirp=opendir(".")) == NULL)
        {
        fprintf(stderr, "Could not check current directory for files with .upwritedata postfix\n");
        fprintf(stderr, "in input_classes() in misc.c\n");
        exit(1);
        }
*/
	_getcwd(pathbuffer, _MAX_PATH);

	for( k=0; k<3; k++)
	{
	   // Search for file in PATH environment variable:
	   _searchenv( fnames[k], envvar, pathbuffer ); // C4996
	   // Note: _searchenv is deprecated; consider using _searchenv_s
	   if( *pathbuffer != '\0' )
	   {
		  printf( "Path for %s:\n%s\n", fnames[k]/*searchfile*/, pathbuffer );
		  infile = fopen(pathbuffer, "r");
	   }
	   else
	   {

		   strcpy(pathbuffer, exepath);
		   strcat(pathbuffer, "\\");
		   strcat(pathbuffer, fnames[k]/*searchfile*/);
		  infile = fopen(pathbuffer, "r");
		  //printf( "%s not found\n", searchfile );
	   }

		   strcpy(pathbuffer, exepath);
		   strcat(pathbuffer, "\\");
		   strcat(pathbuffer, fnames[k]/*searchfile*/);
		  infile = fopen(pathbuffer, "r");
	   if (infile == NULL)
						{
						fprintf(stderr, "Could not open %s for input in input_classes() in misc.c\n", pathbuffer);
						exit(11);
						}

					/* Read in the label for this class of objects.
					 */
					if (fscanf(infile, "%s", label) != 1)
						{
						fprintf(stderr, "Could not read %s for input. Continuing.\n", pathbuffer);
	//                    continue;
						}

					/* Read in the linked list of GAUS
					 * modelling this class of objects.
					 */
					gaus_list = NULL;
					do
						{
						gaus = read_gaus_from_file(infile, &return_val);

						if (gaus != NULL)
							{
							gaus->label = (char *)malloc((1+strlen(label)) * sizeof(char) );
							if (gaus->label == NULL)
								{
								fprintf(stderr, "malloc failed in input_classes() in misc.c\n");
								exit(1);
								}
							strcpy(gaus->label, label);
							gaus_list = put_in_linked_list_of_gaus(gaus_list, gaus);
							}
						}
					while (return_val == 0);

					gClassList = put_in_linked_list_of_linked_list_of_gaus(gClassList, gaus_list);

	}
#ifdef DEMMY
    while ( (dp=readdir(dirp)) != NULL)
        {
        if (dp->d_ino == 0)     /* empty entry */
            {
            continue;
            }
        /* Does this file have the postfix .upwritedata
         */
        length = strlen(dp->d_name);
        if (length > 12)        /* Is file name longer than 12 */
            {
            strncpy(tmp_str, &(dp->d_name[length-12]), 13); /* 13 incl. NULL */
            if (strcmp(tmp_str, ".upwritedata") == 0)
                {
                infile = fopen(dp->d_name, "r");
                if (infile == NULL)
                    {
                    fprintf(stderr, "Could not open %s for input in input_classes() in misc.c\n", dp->d_name);
                    exit(1);
                    }

                /* Read in the label for this class of objects.
                 */
                if (fscanf(infile, "%s", label) != 1)
                    {
                    fprintf(stderr, "Could not read %s for input. Continuing.\n", dp->d_name);
                    continue;
                    }

                /* Read in the linked list of GAUS
                 * modelling this class of objects.
                 */
                gaus_list = NULL;
                do
                    {
                    gaus = read_gaus_from_file(infile, &return_val);

                    if (gaus != NULL)
                        {
                        gaus->label = (char *)malloc((1+strlen(label)) * sizeof(char) );
                        if (gaus->label == NULL)
                            {
                            fprintf(stderr, "malloc failed in input_classes() in misc.c\n");
                            exit(1);
                            }
                        strcpy(gaus->label, label);
                        gaus_list = put_in_linked_list_of_gaus(gaus_list, gaus);
                        }
                    }
                while (return_val == 0);

                gClassList = put_in_linked_list_of_linked_list_of_gaus(gClassList, gaus_list);


                }   /* end of 'if (strcmp(tmp_str,...`  */
            }   /* end of 'if (length > 12)' */
        }   /* end of ' while ( (dp=readdir(dirp))...' */
#endif //DUMMY

//    if (closedir(dirp) == -1)
  //      perror("closedir() failed in input_classes() in misc.c\n");



}   /* end of input_classes() */




/* Output the Training data in a form that the xgobi program
 * can read. Training points will be drawn in white.
 *
 * Also output the gaussians modelling this data.
 * We draw a line representing each eigenvector of the
 * gaussian in red. These are drawn in red.
 * The line extends in the direction of both the
 * positive and the negative of the eigenvector.
 * Each line is scaled by the eigenvalue of the corresponding eigenvector.
 *
 * In other words, each gaussian is drawn as a star of red
 * lines which intersect at the gaussian's mean.
 * The length of each line tells you something about
 * the covariance of the gaussian in that direction.
 *
 * The data is output to the files:
 *      tmp.dat
 *      tmp.colors
 *      tmp.glyphs
 *      tmp.lines
 *      tmp.linecolors
 */
void
output_training_data(GAUS_LIST *gaus_list)
{
    int     i, j, line_count;
    POINT   *point;
    GAUS    *gaus;
    FILE    *outfile_dat;
    FILE    *outfile_glyphs, *outfile_colors, *outfile_lines, *outfile_linecolors;

    /* Write the training points to the file tmp.dat
     */
    line_count = 0;
    outfile_dat = fopen("tmp.dat", "w");
    outfile_colors = fopen("tmp.colors", "w");
    outfile_glyphs = fopen("tmp.glyphs", "w");
    outfile_lines = fopen("tmp.lines", "w");
    outfile_linecolors = fopen("tmp.linecolors", "w");
    if ((outfile_dat == NULL) || (outfile_colors == NULL) || (outfile_glyphs == NULL) || (outfile_lines == NULL) || (outfile_linecolors == NULL))
        {
        fprintf(stderr, "Could not open the file tmp.dat for output.\n");
        fprintf(stderr, "Have not written training data points to file.\n");
        }
    else
        for (point=gTrainingPointList->head; point != NULL; point=point->next)
            {
            line_count++;
            for (i=1; i <= point->dim; i++)
                fprintf(outfile_dat, "%.10f ", (float )point->x[i]);
            fprintf(outfile_colors, "white\n");
            fprintf(outfile_glyphs, "1\n");
            fprintf(outfile_dat, "\n");
            }

    if ((gaus_list != NULL) && (gaus_list ->head != NULL))
        for (gaus=gaus_list->head; gaus != NULL; gaus=gaus->next)
            {
            for (i=1; i <= gaus->dim; i++)      /* For each eigenvector */
                {
                line_count++;
                for (j=1; j <= gaus->dim; j++)  /* for each co-ord */
                    fprintf(outfile_dat, "%f ", (float )(gaus->mean[j]-sqrt(gaus->eig_val[i])*gaus->eig_vec[j][i]));
                fprintf(outfile_dat, "\n");
                fprintf(outfile_glyphs, "31\n");
                fprintf(outfile_colors, "red\n");

                line_count++;
                for (j=1; j <= gaus->dim; j++)  /* for each co-ord */
                    fprintf(outfile_dat, "%f ", (float )(gaus->mean[j]+sqrt(gaus->eig_val[i])*gaus->eig_vec[j][i]));
                fprintf(outfile_dat, "\n");
                fprintf(outfile_glyphs, "31\n");
                fprintf(outfile_colors, "red\n");
                fprintf(outfile_lines, "%d %d\n", line_count-1,line_count);
                fprintf(outfile_linecolors, "red\n");
                }
            }



    fclose(outfile_dat);
    fclose(outfile_glyphs);
    fclose(outfile_colors);
    fclose(outfile_lines);
    fclose(outfile_linecolors);

}       /* output_training_data_for_xgobi() */



/* Return True if (char )c occurs in
 * the string (char *)valid_characters.
 * Else return False.
 */
BOOLEAN
valid_character(char c, char *valid_characters)
{
    while (*valid_characters != (char )NULL)
        {
        if (c == *valid_characters)
            return(TRUE);
        valid_characters++;
        }

    return(FALSE);

}           /* end of valid_character() */


int fprint_gaus_list(char *name, GAUS_LIST *gaus_list)
{
	char filename[256];
	int i;
//	FILE *output;
	GAUS *gaus;

//
    strcpy(filename, name);
	strcat(filename, ".txt");

	
//	output = fopen(filename, "a+");
	
    for (i=0, gaus=gaus_list->head; gaus != NULL; gaus=gaus->next, i++)
	{
		sprintf(filename, "%s%d", name, i );
		fprint_gaus(filename, gaus);
	}

//	fprintf(output, "%s\n", filename);
//	fprintf(output, "head.eigval=%f, tail.eigval=%f\n", *gaus_list->head->eig_val, *gaus_list->tail->eig_val);
//	fprintf(output, "head.eigvec=%f, tail.eigvec=%f\n", *gaus_list->head->eig_vec, *gaus_list->tail->eig_vec);
//	fprintf(output, "gaus_list.num_elements=%d\n", gaus_list->num_elements);

//	fclose(output);
return 0;
}

int fprint_gaus(char *name, GAUS *gaus)
{
    int     i, j;
	char filename[256];

	FILE *output;

    strcpy(filename, name);
	strcat(filename, ".txt");
	output = fopen(filename, "a+");

    fprintf(output,"num_points: %lf\n", gaus->num_points);
    fprintf(output,"mean: ");
    for (i=1; i <= gaus->dim; i++)
        fprintf(output,"%lf ", gaus->mean[i]);
    fprintf(output,"\n");
    fprintf(output, "eig_val: ");
    for (i=1; i <= gaus->dim; i++)
        fprintf(output,"%lf ", gaus->eig_val[i]);
    fprintf(output,"\n");
    fprintf(output,"---\n");
    fprintf(output,"covar: ");
    for (i=1; i <= gaus->dim; i++)
        {
        for (j=1; j <= gaus->dim; j++)
            fprintf(output,"%lf ", gaus->covar[i][j]);
        fprintf(output,"\n");
        }
    fprintf(output,"---\n");

    fprintf(output,"---\n\n\n\n");
    fclose(output);

	return 0;
}
