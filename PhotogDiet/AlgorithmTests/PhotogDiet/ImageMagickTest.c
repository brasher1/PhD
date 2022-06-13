#include <stdio.h>
#include <stdio.h>
#include <wand/MagickWand.h>
#include <windows.h>
#include <crtdbg.h>


void readjpg(char *filename)
{
	MagickWand *wand;
    MagickWand *mw,*mwl,*mwb;
    PixelIterator *imw,*imwl,*imwb;
    PixelWand **pmw,**pmwl,**pmwb;

	unsigned int	W, H;			// Width and Height of current frame [pixels]
	unsigned int    row, col;		// Pixel's row and col positions
	unsigned long   i;				// Dummy variable for row-column vector
	int	    upperThreshold = 60;	// Gradient strength nessicary to start edge
	int		lowerThreshold = 30;	// Minimum gradient strength to continue edge	
	int GxMask[3][3];				// Sobel mask in the x direction
	int GyMask[3][3];				// Sobel mask in the y direction
	int gaussianMask[5][5];			// Gaussian mask
	unsigned short newPixel;
	unsigned short current_pixel;
	unsigned long iOffset;			// Variable to offset row-column vector during sobel mask
	int rowOffset;					// Row offset from the current pixel
	int colOffset;					// Col offset from the current pixel
	int rowTotal = 0;				// Row position of offset pixel
	int colTotal = 0;				// Col position of offset pixel

	unsigned long y;
    register long x;
    unsigned long width,height;
    Quantum qr,qg,qb;
    Quantum qrb,qgb,qbb;
    Quantum qrl,qgl,qbl;
    double lh,ls,ll;
    double bh,bs,bb;

	/* Declare Sobel masks */
	GxMask[0][0] = -1; GxMask[0][1] = 0; GxMask[0][2] = 1;
	GxMask[1][0] = -2; GxMask[1][1] = 0; GxMask[1][2] = 2;
	GxMask[2][0] = -1; GxMask[2][1] = 0; GxMask[2][2] = 1;
	
	GyMask[0][0] =  1; GyMask[0][1] =  2; GyMask[0][2] =  1;
	GyMask[1][0] =  0; GyMask[1][1] =  0; GyMask[1][2] =  0;
	GyMask[2][0] = -1; GyMask[2][1] = -2; GyMask[2][2] = -1;

	/* Declare Gaussian mask */
	gaussianMask[0][0] = 2;		gaussianMask[0][1] = 4;		gaussianMask[0][2] = 5;		gaussianMask[0][3] = 4;		gaussianMask[0][4] = 2;	
	gaussianMask[1][0] = 4;		gaussianMask[1][1] = 9;		gaussianMask[1][2] = 12;	gaussianMask[1][3] = 9;		gaussianMask[1][4] = 4;	
	gaussianMask[2][0] = 5;		gaussianMask[2][1] = 12;	gaussianMask[2][2] = 15;	gaussianMask[2][3] = 12;	gaussianMask[2][4] = 2;	
	gaussianMask[3][0] = 4;		gaussianMask[3][1] = 9;		gaussianMask[3][2] = 12;	gaussianMask[3][3] = 9;		gaussianMask[3][4] = 4;	
	gaussianMask[4][0] = 2;		gaussianMask[4][1] = 4;		gaussianMask[4][2] = 5;		gaussianMask[4][3] = 4;		gaussianMask[4][4] = 2;	

	MagickWandGenesis();
	mw = NewMagickWand();


	if (MagickReadImage(mw, "c:\\temp\\target001.jpg"))
	{
		_RPT2( _CRT_ERROR, "Height: %ld,  Width: %ld", MagickGetImageHeight(mw), MagickGetImageWidth(mw) );
	}

	MagickWriteImage(mw, "c:\\temp\\target001_Magick.jpg");

    width = MagickGetImageWidth(mw);
    height = MagickGetImageHeight(mw);
    W = MagickGetImageWidth(mw);
    H = MagickGetImageHeight(mw);
	
	MagickSetImageColorspace(mw, GRAYColorspace);

    mwl = NewMagickWand();
//    mwb = NewMagickWand();

	MagickSetImageColorspace(mwl, GRAYColorspace);
//	MagickSetImageColorspace(mwb, GRAYColorspace);

    // Set the hsl and hsb images to the same size as the input image
    MagickSetSize(mwl,width,height);
//    MagickSetSize(mwb,width,height);
    // Even though we won't be reading these images they must be initialized
    // to something
//    MagickReadImage(mwb,"xc:none");
	MagickReadImage(mwl,"xc:none");

    // Create iterators for each image
    imw = NewPixelIterator(mw);
    imwl = NewPixelIterator(mwl);
//    imwb = NewPixelIterator(mwb);
	/* Gaussian Blur */
	for (row = 2; row < height-2; row++) {
        pmw = PixelGetNextIteratorRow(imw, &width);
        pmwl = PixelGetNextIteratorRow(imwl, &width);
		for (col = 2; col < width-2; col++) {
            qr = PixelGetRedQuantum(pmw[col]);
            qg = PixelGetGreenQuantum(pmw[col]);
            qb = PixelGetBlueQuantum(pmw[col]);
			current_pixel =  ( qr + qb + qg )/3;
			newPixel = 0;
			for (rowOffset=-2; rowOffset<=2; rowOffset++) {
				for (colOffset=-2; colOffset<=2; colOffset++) {
					rowTotal = row + rowOffset;
					colTotal = col + colOffset;
					iOffset = (unsigned long)(rowTotal*3*W + colTotal*3);
					qr = PixelGetRedQuantum(pmw[col+iOffset]);
					qg = PixelGetGreenQuantum(pmw[col+iOffset]);
					qb = PixelGetBlueQuantum(pmw[col+iOffset]);

					newPixel += (*(m_destinationBmp + iOffset)) * gaussianMask[2 + rowOffset][2 + colOffset];
				}
			}
			i = (unsigned long)(row*3*W + col*3);
            PixelSetRedQuantum(pmwl[col+i],newPixel);
            PixelSetGreenQuantum(pmwl[col+i],newPixel);
            PixelSetBlueQuantum(pmwl[col+i],newPixel);
		}
	}
	SyncAuthenticPixels(pmwl,NULL);

#ifdef TRUER
    for (y=0; y < height; y++) {
        // Get the next row from each image
        pmw = PixelGetNextIteratorRow(imw, &width);
        pmwl = PixelGetNextIteratorRow(imwl, &width);
//       pmwb = PixelGetNextIteratorRow(imwb, &width);
        for (x=0; x < (long) width; x++) {
            // Get the RGB quanta from the source image
            qr = PixelGetRedQuantum(pmw[x]);
            qg = PixelGetGreenQuantum(pmw[x]);
            qb = PixelGetBlueQuantum(pmw[x]);


            // Convert the source quanta to HSB
//            ConvertRGBToHSB(qr,qg,qb,&bh,&bs,&bb);
//            bb *= 0.8;
//            ConvertHSBToRGB(bh,bs,bb,&qrb,&qgb,&qbb);
//          // Set the pixel in the HSB output image
//            PixelSetRedQuantum(pmwb[x],qrb);
//            PixelSetGreenQuantum(pmwb[x],qgb);
//            PixelSetBlueQuantum(pmwb[x],qbb);

            // Convert the source quanta to HSL
            ConvertRGBToHSL(qr,qg,qb,&lh,&ls,&ll);
            ll *= 0.8;
            ConvertHSLToRGB(lh,ls,ll,&qrl,&qgl,&qbl);
            // Set the pixel in the HSL output image
            PixelSetRedQuantum(pmwl[x],qrl);
            PixelSetGreenQuantum(pmwl[x],qgl);
            PixelSetBlueQuantum(pmwl[x],qbl);
        }
        // Sync writes the pixels back to the magick wands
        PixelSyncIterator(imwl);
        PixelSyncIterator(imwb);
    }
#endif
    // write the results
//	MagickWriteImage(mwb,"c:\\temp\\logo_hsb.jpg");
    MagickWriteImage(mwl,"c:\\temp\\logo_hsl.jpg");

    // Clean up the iterators and magick wands
    imw = DestroyPixelIterator(imw);
    imwl = DestroyPixelIterator(imwl);
    imwb = DestroyPixelIterator(imwb);
    mw = DestroyMagickWand(mw);
    mwl = DestroyMagickWand(mwl);
    mwb = DestroyMagickWand(mwb);
    MagickWandTerminus();
}


