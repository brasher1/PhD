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

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include <sys/types.h>
#include <magick/api.h>
#ifdef DUMMY_CODE
#define TRUE	1
#define FALSE	0
#define PI	3.1415926535897932384626433832795

int edgeEnd;



void findEdge(int rowShift, int colShift, int row, int col, int dir, int lowerThreshold, Image *image, int **edgeDir, double **gradient, int H, int W, PixelPacket *pp);
void suppressNonMax(int rowShift, int colShift, int row, int col, int dir, int lowerThresholdImage, Image *image, int **edgeDir, double **gradient, int H, int W, PixelPacket *pp, double **);

void ImageEdgeMagick(char *filename)
{
  Image *image;
	Image *image2;
  ExceptionInfo
    exception;
	ImageInfo *image_info;
	ImageInfo *image_info2;

	InitializeMagick(filename);
	GetExceptionInfo(&exception);
	image_info=CloneImageInfo((ImageInfo *) NULL);
	strcpy(image_info->filename, filename);

	image=ReadImage(image_info,&exception);
  	
	image2 = EdgeImage(image, 0, &exception);

	image_info2=CloneImageInfo((ImageInfo *) NULL);
	strcpy(image_info2->filename, "c:\\temp\\dumpster.jpg");

	WriteImage(image_info2, image2);
}


//int main(int argc,char **argv)
void canny(char *filename)
{
	unsigned int	W, H;			// Width and Height of current frame [pixels]
	unsigned int    row, col;		// Pixel's row and col positions
	unsigned int   i,j;				// Dummy variable for row-column vector
	int	    upperThreshold = (0.4)*MaxRGB;	// Gradient strength nessicary to start edge
	int		lowerThreshold = (0.1)*MaxRGB;	// Minimum gradient strength to continue edge
	unsigned long iOffset;			// Variable to offset row-column vector during sobel mask
	int rowOffset;					// Row offset from the current pixel
	int colOffset;					// Col offset from the current pixel
	int rowTotal = 0;				// Row position of offset pixel
	int colTotal = 0;				// Col position of offset pixel
	int Gx;							// Sum of Sobel mask products values in the x direction
	int Gy;							// Sum of Sobel mask products values in the y direction
	double thisAngle;				// Gradient direction based on Gx and Gy
	int newAngle;					// Approximation of the gradient direction
//	int edgeEnd;					// Stores whether or not the edge is at the edge of the possible image
	int GxMask[3][3];				// Sobel mask in the x direction
	int GyMask[3][3];				// Sobel mask in the y direction
	int newPixel;					// Sum pixel values for gaussian
	int gaussianMask[5][5];			// Gaussian mask
	double **nonMax;			// Temporarily stores gradients and positions of pixels in parallel edges
	int offset;
	int x, y;
	FILE *output;

  ExceptionInfo
    exception;

    int **edgeDir;
	double **gradient;

  Image
    *image,

    *resize_image;

  ImageInfo
    *image_info;
	FILE *f;


	PixelPacket *pp;
typedef struct _PixelPacket5x5
{
	PixelPacket pp[5][5];
} PixelPacket5x5;

typedef struct _PixelPacket3x3
{
	PixelPacket pp[5][5];
} PixelPacket3x3;

  PixelPacket5x5 *pixel;
  PixelPacket3x3 *pixel3x3;
  PixelPacket *pixels;

#define GAUSS_SIZE	5
#define SIGMA		1.0

//double gaussianMask[GAUSS_SIZE][GAUSS_SIZE];

	/* Declare Sobel masks */
	GxMask[0][0] = -1; GxMask[0][1] = 0; GxMask[0][2] = 1;
	GxMask[1][0] = -2; GxMask[1][1] = 0; GxMask[1][2] = 2;
	GxMask[2][0] = -1; GxMask[2][1] = 0; GxMask[2][2] = 1;
	
	GyMask[0][0] =  1; GyMask[0][1] =  2; GyMask[0][2] =  1;
	GyMask[1][0] =  0; GyMask[1][1] =  0; GyMask[1][2] =  0;
	GyMask[2][0] = -1; GyMask[2][1] = -2; GyMask[2][2] = -1;

/*	offset = (int)(GAUSS_SIZE/2);  

	for( x=-offset; x<GAUSS_SIZE-offset; x++)
	{
		for(y=-offset; y<GAUSS_SIZE-offset; y++)
		{
			gaussianMask[x+offset][y+offset] = (1.0/(2.0*PI*SIGMA*SIGMA))*exp(-(x*x+y*y)/(2.0*SIGMA*SIGMA));
		}
	}
*/
	/* Declare Gaussian mask */
	gaussianMask[0][0] = 2;		gaussianMask[0][1] = 4;		gaussianMask[0][2] = 5;		gaussianMask[0][3] = 4;		gaussianMask[0][4] = 2;	
	gaussianMask[1][0] = 4;		gaussianMask[1][1] = 9;		gaussianMask[1][2] = 12;	gaussianMask[1][3] = 9;		gaussianMask[1][4] = 4;	
	gaussianMask[2][0] = 5;		gaussianMask[2][1] = 12;	gaussianMask[2][2] = 15;	gaussianMask[2][3] = 12;	gaussianMask[2][4] = 5;	
	gaussianMask[3][0] = 4;		gaussianMask[3][1] = 9;		gaussianMask[3][2] = 12;	gaussianMask[3][3] = 9;		gaussianMask[3][4] = 4;	
	gaussianMask[4][0] = 2;		gaussianMask[4][1] = 4;		gaussianMask[4][2] = 5;		gaussianMask[4][3] = 4;		gaussianMask[4][4] = 2;	
/*
	gaussianMask[0][0] = 2;	gaussianMask[0][1] = 4;	 gaussianMask[0][2] = 4;  gaussianMask[0][3] =  4;  gaussianMask[0][4] = 2;
	gaussianMask[1][0] = 4;	gaussianMask[1][1] = 8;	 gaussianMask[1][2] = 16; gaussianMask[1][3] = 	8;  gaussianMask[1][4] = 4;
	gaussianMask[2][0] = 4;	gaussianMask[2][1] = 16; gaussianMask[2][2] = 32; gaussianMask[2][3] = 	16; gaussianMask[2][4] = 4;
	gaussianMask[3][0] = 4;	gaussianMask[3][1] = 8;	 gaussianMask[3][2] = 16; gaussianMask[3][3] = 	8;  gaussianMask[3][4] = 4;
	gaussianMask[4][0] = 2;	gaussianMask[4][1] = 4;	 gaussianMask[4][2] = 4;  gaussianMask[4][3] = 	4;  gaussianMask[4][4] = 2;
*/
	/*
    Initialize the image info structure and read the list of files
    provided by the user as a image sequence
  */


  InitializeMagick(filename);
  GetExceptionInfo(&exception);
  image_info=CloneImageInfo((ImageInfo *) NULL);
  strcpy(image_info->filename, filename);

  image=ReadImage(image_info,&exception);
  GetImageInfo(image_info);

  if (exception.severity != UndefinedException)
      CatchException(&exception);

  H = image->rows;  W = image->columns;

  nonMax= (double**)malloc(H * sizeof(double*));
	for(i = 0; i < H; i++) {
		nonMax[i] = (double*)malloc(3 * sizeof(double));
	}

  edgeDir = (int**)malloc(H * sizeof(int*));
  gradient = (double**)malloc(H * sizeof(double*));
  for(i = 0; i < H; i++) {
	  edgeDir[i] = (int*)malloc(W * sizeof(int));
	  gradient[i] = (double*)malloc(W * sizeof(double));
  }
	for(i=0; i<H; i++)
		for(j=0; j<W; j++)
		{
			edgeDir[i][j] = 0;
			gradient[i][j] = 0.0;
		}
	
	
	pixels = GetImagePixels(image, 0,0, W,H);
	f = fopen("c:\\temp\\outputarrays.txt","w+");
	if( f != NULL )
	{
		fprintf(f, "\nActual Pixels\n");
		for( row=0; row<H; row++)
		{
			if(row>0 )
				fprintf(f, "\n");
			for( col=0; col<W; col++)
				fprintf(f, "\t%u", pixels[row*W + col].blue); 
			fprintf(f, "->\n");
		}
		fprintf(f, "\n");
	}


	/* Gaussian Blur */
	offset =  (int)(GAUSS_SIZE/2);  
   for (row = 2; row < H-2; row++) {
		for (col = 2; col < W-2; col++) {
			newPixel = 0;
			for( rowOffset=-2; rowOffset<=2; rowOffset++)
			{
				for( colOffset=-2; colOffset<=2; colOffset++)
				{
					rowTotal = row + rowOffset;
					colTotal = col + colOffset;
					iOffset = (unsigned long)(rowTotal*W + colTotal);
					newPixel += pixels[iOffset].blue * gaussianMask[2+rowOffset][2+colOffset];
				}
			}
			i = (unsigned long)(row*W + col);
			pixels[i].blue = 
			pixels[i].red = 
			pixels[i].green = (Quantum)(newPixel/159.0);
		} 
	}
/*	SyncImagePixels(image);  
	strcpy(image_info->filename, "c:\\temp\\target001new.jpg");

    WriteImage(image_info,image);
	exit(0);
  */  
	/* Non-maximum Suppression */
   output = fopen("c:\\temp\\dirandcoord.txt", "a+");
	/* Determine edge directions and gradient strengths */
	for (row = 1; row < H-1; row++) {
		for (col = 1; col < W-1; col++) {
			i = (unsigned long)(row*W + col);
			Gx = 0;
			Gy = 0;
			for(rowOffset=-1; rowOffset<=1; rowOffset++)
			{
				for(colOffset=-1; colOffset<=1; colOffset++)
				{
					rowTotal = row+rowOffset;
					colTotal = col+colOffset;
					iOffset = (unsigned long)(rowTotal*W + colTotal);
					Gx = Gx + pixels[iOffset].blue * GxMask[rowOffset+1][colOffset+1];
					Gy = Gy + pixels[iOffset].blue * GyMask[rowOffset+1][colOffset+1];
				}
			}

			gradient[row][col] = sqrt(pow(Gx,2.0) + pow(Gy,2.0));	// Calculate gradient strength			
			thisAngle = (atan2(Gx,Gy)/PI) * 180.0;		// Calculate actual direction of edge
			
			/* Convert actual edge direction to approximate value */
			if ( ( (thisAngle < 22.5) && (thisAngle > -22.5) ) || (thisAngle > 157.5) || (thisAngle < -157.5) )
				newAngle = 0;
			if ( ( (thisAngle > 22.5) && (thisAngle < 67.5) ) || ( (thisAngle < -112.5) && (thisAngle > -157.5) ) )
				newAngle = 45;
			if ( ( (thisAngle > 67.5) && (thisAngle < 112.5) ) || ( (thisAngle < -67.5) && (thisAngle > -112.5) ) )
				newAngle = 90;
			if ( ( (thisAngle > 112.5) && (thisAngle < 157.5) ) || ( (thisAngle < -22.5) && (thisAngle > -67.5) ) )
				newAngle = 135;
			fprintf(output, "R,C=%d,%d\tgrad=%f\tnewAngle=%d\tthisAngle=%f\t\n", row, col, 
				gradient[row][col], newAngle, thisAngle);

			edgeDir[row][col] = newAngle;		// Store the approximate edge direction of each pixel in one array
		}
	}
	fclose(output);

	if( f != NULL )
	{
		fprintf(f, "GRADIENTS\n");
		for( row=0; row<H; row++)
		{
			if(row>0 )
				fprintf(f, "\n");
			for( col=0; col<W; col++)
				fprintf(f, "\t%f", gradient[row][col]); 
			fprintf(f, "->\n");
		}
		fprintf(f, "\nEdge Directions\n");
		for( row=0; row<H; row++)
		{
			if(row>0 )
				fprintf(f, "\n");
			for( col=0; col<W; col++)
				fprintf(f, "\t%u", edgeDir[row][col]); 
			fprintf(f,"->\n");
		}
		fprintf(f, "\nGaussian Pixels\n");
		for( row=0; row<H; row++)
		{
			if(row>0 )
				fprintf(f, "\n");
			for( col=0; col<W; col++)
				fprintf(f, "\t%u", pixels[row*W + col].blue); 
			fprintf(f, "->\n");
		}
		fprintf(f, "\n");
	}
//***************************************
	for (row = 1; row < H - 1; row++) {
		for (col = 1; col < W - 1; col++) {
			i = (unsigned long)(row*W + col);
			if (pixels[i].blue == MaxRGB) {		// Check to see if current pixel is an edge
				/* Switch based on current pixel's edge direction */
				switch (edgeDir[row][col]) {		
					case 0:
						suppressNonMax( 1, 0, row, col, 0, lowerThreshold, image, edgeDir, gradient, H, W, pixels, nonMax );
						break;
					case 45:
						suppressNonMax( 1, -1, row, col, 45, lowerThreshold, image, edgeDir, gradient, H, W, pixels, nonMax);
						break;
					case 90:
						suppressNonMax( 0, 1, row, col, 90, lowerThreshold, image, edgeDir, gradient, H, W, pixels, nonMax);
						break;
					case 135:
						suppressNonMax( 1, 1, row, col, 135, lowerThreshold, image, edgeDir, gradient, H, W, pixels, nonMax);
						break;
					default :
						break;
				}
			}	
		}
	}
//*************************************************


	for (row = 1; row < H - 1; row++) {
		for (col = 1; col < W - 1; col++) {
			edgeEnd = FALSE;
			if (gradient[row][col] > upperThreshold) {		// Check to see if current pixel has a high enough gradient strength to be part of an edge
				/* Switch based on current pixel's edge direction */
				switch (edgeDir[row][col]){		
					case 0:
						findEdge(0, 1, row, col, 0, lowerThreshold, image, edgeDir, gradient, H, W, pixels);
						break;
					case 45:
						findEdge(1, 1, row, col, 45, lowerThreshold, image, edgeDir, gradient, H, W, pixels);
						break;
					case 90:
						findEdge(1, 0, row, col, 90, lowerThreshold, image, edgeDir, gradient, H, W, pixels);
						break;
					case 135:
						findEdge(1, -1, row, col, 135, lowerThreshold, image, edgeDir, gradient, H, W, pixels);
						break;
					default :
						i = (unsigned long)(row*W + col);
						pixels[i].blue = 
						pixels[i].red = 
						pixels[i].green = 0;
						
						break;
					}
				}
			else {
				i = (unsigned long)(row*W + col);
						pixels[i].blue = 
						pixels[i].red = 
						pixels[i].green = 0;
			}	
		}
	}

	if(f != NULL )
	{
		fprintf(f, "AFTER EDGES ARE FOUND\n");
		for( row=0; row<H; row++)
		{
			if(row>0 )
				fprintf(f, "\n");
			for( col=0; col<W; col++)
				fprintf(f, "\t%u", pixels[row*W + col].blue);
			fprintf(f, "->\n");
		}
		fprintf(f,"\n");
	}

/*	SyncImagePixels(image);  
	strcpy(image_info->filename, "c:\\temp\\target001new.jpg");

    WriteImage(image_info,image);
	exit(0);
  */
	/* Suppress any pixels not changed by the edge tracing */
	for (row = 0; row < H; row++) {
		for (col = 0; col < W; col++) {	
			// Recall each pixel is composed of 3 bytes
			i = (unsigned long)(row*W + col);
			// If a pixel's grayValue is not black or white make it black
			if( pixels[i].blue != MaxRGB && pixels[i].blue != 0) 
				pixels[i].blue = 
				pixels[i].red =
				pixels[i].green = 0; // Make pixel black
		}
	}
	if(f != NULL )
	{
		fprintf(f, "AFTER SUPPRESSION OF NON-EDGE PIXELS\n");
		for( row=0; row<H; row++)
		{
			if(row>0 )
				fprintf(f, "\n");
			for( col=0; col<W; col++)
				fprintf(f, "\t%u", pixels[row*W + col].blue); 
			fprintf(f, "->\n");
		}
		fprintf(f,"\n");
	}


	if(f != NULL )
	{
		fprintf(f, "AFTER SUPPRESSION OF NON-MAX PIXELS\n");
		for( row=0; row<H; row++)
		{
			if(row>0 )
				fprintf(f, "\n");
			for( col=0; col<W; col++)
				fprintf(f, "\t%u", pixels[row*W + col].blue); 
			fprintf(f,"->\n");
		}
		fprintf(f,"\n");
		fclose(f);
	}

	SyncImagePixels(image);  
	strcpy(image_info->filename, "c:\\temp\\target001new.jpg");

    WriteImage(image_info,image);
    

  /*
    Release resources
  */

  for(i = 0; i < H; i++)  
  {
	  if( gradient[i] )
		  free(gradient[i]);
	  if(edgeDir[i])
		free(edgeDir[i]);
  }
  if( gradient )
	  free(gradient);
  if( edgeDir)
	free(edgeDir);

    for(i = 0; i < H; i++)  
	{
		if( nonMax[i] )
			free(nonMax[i]);
	}
	if( nonMax )
	  free(nonMax);
  

  DestroyImageInfo(image_info);
  DestroyExceptionInfo(&exception);
  DestroyMagick();
  return;
}

void findEdge(int rowShift, int colShift, int row, int col, int dir, int lowerThreshold, Image *image, int **edgeDir, double **gradient, int H, int W, PixelPacket *pixels)
{

	int newRow;
	int newCol;
	unsigned long i;
//	int edgeEnd;
	FILE *output;
//	edgeEnd = FALSE;


	output = fopen("c:\\temp\\findEdge.txt", "a+");
	fprintf(output, "NEXT CALL\n");
	/* Find the row and column values for the next possible pixel on the edge */
	if (colShift < 0) {
		if (col > 0)
			newCol = col + colShift;
		else
			edgeEnd = TRUE;
	} else if (col < W - 1) {
		newCol = col + colShift;
	} else
		edgeEnd = TRUE;		// If the next pixel would be off image, don't do the while loop
	if (rowShift < 0) {
		if (row > 0)
			newRow = row + rowShift;
		else
			edgeEnd = TRUE;
	} else if (row < H - 1) {
		newRow = row + rowShift;
	} else
		edgeEnd = TRUE;	
		
	/* Determine edge directions and gradient strengths */
	while ( (edgeDir[newRow][newCol]==dir) && !edgeEnd && (gradient[newRow][newCol] > lowerThreshold) ) {
		/* Set the new pixel as white to show it is an edge */
		i = (unsigned long)(newRow*W + newCol);
		fprintf(output, "In While: %d,%d,\tedgeDir=%d,\tGradient=%f\n", newRow, newCol, edgeDir[newRow][newCol], gradient[newRow][newCol]); 
		pixels[i].blue =
	    pixels[i].red = 
		pixels[i].green = MaxRGB;

		if (colShift < 0) {
			if (newCol > 0)
				newCol = newCol + colShift;
			else
				edgeEnd = TRUE;	
		} else if (newCol < W - 1) {
			newCol = newCol + colShift;
		} else
			edgeEnd = TRUE;	
		if (rowShift < 0) {
			if (newRow > 0)
				newRow = newRow + rowShift;
			else
				edgeEnd = TRUE;
		} else if (newRow < H - 1) {
			newRow = newRow + rowShift;
		} else
			edgeEnd = TRUE;	
	}	
	fclose(output);
}

void suppressNonMax(int rowShift, int colShift, int row, int col, int dir, int lowerThresholdImage, Image *image, int **edgeDir, double **gradient, int H, int W, PixelPacket *pixels, double **nonMax)
{
	int newRow = 0;
	int newCol = 0;
	unsigned long i;
	int edgeEnd = FALSE;
//	double **nonMax;			// Temporarily stores gradients and positions of pixels in parallel edges
	int pixelCount = 0;					// Stores the number of pixels in parallel edges
	int count;						// A for loop counter
	int max[3];						// Maximum point in a wide edge
	
	if (colShift < 0) {
		if (col > 0)
			newCol = col + colShift;
		else
			edgeEnd = TRUE;
	} else if (col < W - 1) {
		newCol = col + colShift;
	} else
		edgeEnd = TRUE;		// If the next pixel would be off image, don't do the while loop
	if (rowShift < 0) {
		if (row > 0)
			newRow = row + rowShift;
		else
			edgeEnd = TRUE;
	} else if (row < H - 1) {
		newRow = row + rowShift;
	} else
		edgeEnd = TRUE;	
	i = (unsigned long)(newRow*W + newCol);
	/* Find non-maximum parallel edges tracing up */
	while ((edgeDir[newRow][newCol] == dir) && !edgeEnd && (pixels[i].blue == MaxRGB)) {
		if (colShift < 0) {
			if (newCol > 0)
				newCol = newCol + colShift;
			else
				edgeEnd = TRUE;	
		} else if (newCol < W - 1) {
			newCol = newCol + colShift;
		} else
			edgeEnd = TRUE;	
		if (rowShift < 0) {
			if (newRow > 0)
				newRow = newRow + rowShift;
			else
				edgeEnd = TRUE;
		} else if (newRow < H - 1) {
			newRow = newRow + rowShift;
		} else
			edgeEnd = TRUE;	
		nonMax[pixelCount][0] = newRow;
		nonMax[pixelCount][1] = newCol;
		nonMax[pixelCount][2] = gradient[newRow][newCol];
		pixelCount++;
		i = (unsigned long)(newRow*W + newCol);
	}

	/* Find non-maximum parallel edges tracing down */
	edgeEnd = FALSE;
	colShift *= -1;
	rowShift *= -1;
	if (colShift < 0) {
		if (col > 0)
			newCol = col + colShift;
		else
			edgeEnd = TRUE;
	} else if (col < W - 1) {
		newCol = col + colShift;
	} else
		edgeEnd = TRUE;	
	if (rowShift < 0) {
		if (row > 0)
			newRow = row + rowShift;
		else
			edgeEnd = TRUE;
	} else if (row < H - 1) {
		newRow = row + rowShift;
	} else
		edgeEnd = TRUE;	
	i = (unsigned long)(newRow*W + newCol);
	while ((edgeDir[newRow][newCol] == dir) && !edgeEnd && (pixels[i].blue == MaxRGB)) {
		if (colShift < 0) {
			if (newCol > 0)
				newCol = newCol + colShift;
			else
				edgeEnd = TRUE;	
		} else if (newCol < W - 1) {
			newCol = newCol + colShift;
		} else
			edgeEnd = TRUE;	
		if (rowShift < 0) {
			if (newRow > 0)
				newRow = newRow + rowShift;
			else
				edgeEnd = TRUE;
		} else if (newRow < H - 1) {
			newRow = newRow + rowShift;
		} else
			edgeEnd = TRUE;	
		nonMax[pixelCount][0] = newRow;
		nonMax[pixelCount][1] = newCol;
		nonMax[pixelCount][2] = gradient[newRow][newCol];
		pixelCount++;
		i = (unsigned long)(newRow*W + newCol);
	}

	/* Suppress non-maximum edges */
	max[0] = 0;
	max[1] = 0;
	max[2] = 0;
	for (count = 0; count < pixelCount; count++) {
		if (nonMax[count][2] > max[2]) {
			max[0] = (int)nonMax[count][0];
			max[1] = (int)nonMax[count][1];
			max[2] = (int)nonMax[count][2];
		}
	}
	for (count = 0; count < pixelCount; count++) {
		i = (unsigned long)(nonMax[count][0]*W + nonMax[count][1]);
			pixels[i].blue = 
			pixels[i].red =
			pixels[i].green = 0;
	}


}
#endif //DUMMY_CODE