/*
 * Programmed by:   Daivd John Brasher
 * E-mail:          brasher1@uab.edu
 * Date:            2009
 * 
 * Organization:    Dept. Electrical & Computer Engineering,
 *                  The University of Alabama at Birmingham,
 *                  Birmingham, Alabama,
 *                  USA
 */
#include <stdafx.h>

#ifdef _CH_
#pragma package <opencv>
#endif
#define CV_NO_BACKWARD_COMPATIBILITY

/*
int main(int argc, char **argv) {
  if (argc != 6) {
    fprintf(stderr, "usage: %s sigma k min input(ppm) output(ppm)\n", argv[0]);
    return 1;
  }
  
  float sigma = atof(argv[1]);
  float k = atof(argv[2]);
  int min_size = atoi(argv[3]);
	
  printf("loading input image.\n");
  image<rgb> *input = loadPPM(argv[4]);
	
  printf("processing\n");
  int num_ccs; 
  image<rgb> *seg = segment_image(input, sigma, k, min_size, &num_ccs); 
  savePPM(seg, argv[5]);

  printf("got %d components\n", num_ccs);
  printf("done! uff...thats hard work.\n");

  return 0;
}

*/
/*
static image<rgb> *load_cvImage(char *name)
{
	int x, y;

	IplImage *img = cvLoadImage(name);

	image<rgb> *im = new image<rgb>(img->width, img->height);

	for( y=0; y<img->height; y++)
	{
		unsigned char *ptr = (unsigned char *)(img->imageData + y * img->widthStep);
		for(x=0; x<img->width; x++)
		{	
//			(unsigned char *)imPtr(im, 0, 0) = ptr[3*x+1];
//			ptr[3*x+2];
//			ptr[3*x+3];
		}
	}
/*
  char buf[BUF_SIZE], doc[BUF_SIZE];
  
//    /* read header 
  std::ifstream file(name, std::ios::in | std::ios::binary);
  pnm_read(file, buf);
  if (strncmp(buf, "P6", 2))
    throw pnm_error();

  pnm_read(file, buf);
  int width = atoi(buf);
  pnm_read(file, buf);
  int height = atoi(buf);

  pnm_read(file, buf);
  if (atoi(buf) > UCHAR_MAX)
    throw pnm_error();

 //    /* read data 
  image<rgb> *im = new image<rgb>(width, height);
  file.read((char *)imPtr(im, 0, 0), width * height * sizeof(rgb));
*/
/*  return im;
}*/