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

#include <math.h>

#include "geometry.h"
#include "nrc/nrutil.h"

void dmmult( double **a, int a_rows, int a_cols, 
             double **b, int b_rows, int b_cols, double **y)
/* multiply two matrices a, b, result in y. y must not be same as a or b */
{
   int i, j, k;
   double sum;

   if ( a_cols != b_rows ) {
//      fprintf(stderr,"a_cols <> b_rows (%d,%d): dmmult\n", a_cols, b_rows);
      return(-1);
   }

#ifdef V_CHECK
   if ( !valid_dmatrix_b( a ) ) 
      nrerror("Invalid 1st matrix: dmmult\n");    
   if ( !valid_dmatrix_b( b ) ) 
      nrerror("Invalid 2nd matrix: dmmult\n");    
   if ( !valid_dmatrix_b( y ) ) 
      nrerror("Invalid result matrix: dmmult\n"); 
#endif

   /*  getchar();
       dmdump( stdout, "Matrix a", a, a_rows, a_cols, "%8.2lf");
       dmdump( stdout, "Matrix b", b, b_rows, b_cols, "%8.2lf");
       getchar();
   */
   for ( i=1; i<=a_rows; i++ ) 
      for ( j=1; j<=b_cols; j++ ) {
         sum = 0.0;
         for ( k=1; k<=a_cols; k++ ) sum += a[i][k]*b[k][j];
         y[i][j] = sum;
      }
}


double mean(int m[], int count)
{
	int i;
	double ans;
	ans = 0;
	for( i=0; i<count; i++)
		ans += m[i];
	ans = ans / count;
	return ans;
}

double max1(int m[], int count)
{
	int i;
	int ans;
	ans = m[0];
	for( i=0; i<count; i++)
		if( m[i] > ans )
			ans = m[i];

	return (double)ans;
}

double min2(int m[], int count)
{
	int i;
	int ans;
	ans = m[0];
	for( i=0; i<count; i++)
		if( m[i] < ans )
			ans = m[i];

	return (double)ans;
}

ELLIPSEPARAMS *fitEllipse(int xArray[], int yArray[], int count)
{
	double mx, my;
	double sx, sy;
	double *x, *y;
	int i,j;
	double **D;
	double **Dt;
	double **S;
	double **C;

	S = dmatrix(6,6);
	D = dmatrix(1,6, 1,count);
	x = dvector(1,count);
	y = dvector(1,count);
	C = dmatrix(6,6);

	mx = mean(xArray,count);
	my = mean(yArray,count);
	sx = (max(xArray,count)-min(xArray,count))/2;
	sy = (max(yArray,count)-min(yArray,count))/2;

	for(i=0; i<count; i++)
	{
		x[i] = (xArray[i] - mx)/sx;
		y[i] = (yArray[i] - my)/sy;
	}

	//x = (X-mx)/sx;
	//y = (Y-my)/sy;	

	for( i=0; i<count; i++)
	{
		D[0][i] = x[i]*x[i];
		D[1][i] = x[i]*y[i];
		D[2][i] = y[i]*y[i];
		D[3][i] = x[i];
		D[4][i] = y[i];
		D[5][i] = 1;
	}
	for( i=0; i<6; i++ )
	{
		for(j=0; j<count; j++)
		{
			Dt[j][i] = D[i][j];
		}
	}
	
	dmmult(D, count, 6, Dt, 6, count, S);

	for(i=1; i<=6; i++)
	{
		for(j=1; j<=6; j++)
		{
			C[i][j] = 0;
		}
	}
	C[1][3] = -2;
	C[2][2] = 1;
	C[3][1] = -2;


	free_dmatrix(S,1,6,1,6);
	free_dmatrix(C,1,6,1,6);
	free_dmatrix(D,1,6,1,count);
	free_dvector(x, 1, count);
	free_dvector(y, 1, count);
}