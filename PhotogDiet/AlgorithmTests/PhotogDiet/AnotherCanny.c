Main Page | File List
canny_model_changed.c

00001 /****************************************************************************/
00002 /*                        Edge Detection Program                            */ 
00003 /*  A suggested user interface is as follows:                               */   
00004 /*  edge {-roberts,-prewitt,-sobel,-frei} [-skipNMS] [-t thresh1 thresh2] img > edgem*/
00005 /*                    ECE 532 : Digital Image Analysis                      */
00006 /*                              HW Assignment 1                             */
00007 /*  Input: PGM file
00008     Output:PGM file + Image map.
00009     Author: Nikhil Shirahatti
00010     Date: 09/13/2003
00011 
00012 *****************************************************************************/
00013 #include <stdio.h>
00014 #include <stdlib.h>
00015 #include <string.h>
00016 #include <math.h>
00017 
00018 extern int read_pgm_hdr(FILE *, int *, int *);
00019 extern void **matrix(int, int, int, int, int);
00020 extern void error(const char *);
00021 int skipNMS=0;
00022 float ** gauss_smooth;
00023 
00024 
00025 /*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
00026 
00027 /* READ PGM HEADER */
00028 
00029 /* This function reads the header of a PGM image. */
00030 /* The dimensions are returned as arguments. */
00031 /* This function ensures that there's no more than 8 bpp. */
00032 /* The return value is negative if there's an error. */
00033 
00034 int read_pgm_hdr(FILE *fp, int *nrows, int *ncols)
00035 {
00036   char filetype[3];
00037   int maxval;
00038 
00039   if(skipcomment(fp) == EOF
00040      || fscanf(fp, "%2s", filetype) != 1
00041      || strcmp(filetype, "P5")
00042      || skipcomment(fp) == EOF
00043      || fscanf(fp, "%d", ncols) != 1
00044      || skipcomment(fp) == EOF
00045      || fscanf(fp, "%d", nrows) != 1
00046      || skipcomment(fp) == EOF
00047      || fscanf(fp, "%d%*c", &maxval) != 1
00048      || maxval > 255)
00049     return(-1);
00050   else return(0);
00051 }
00052 
00053 /*-----------------------------------------------------------------------------------*/
00054 /* ERROR HANDLER */
00055 
00056 void error(const char *msg)
00057 {
00058   fprintf(stderr, "%s\n", msg);
00059   exit(1);
00060 }
00061 
00062 /*------------------------------------------------------------------------------------*/
00063 
00064 /* DYNAMICALLY ALLOCATE A PSEUDO 2-D ARRAY */
00065 
00066 /* This function allocates a pseudo 2-D array of size nrows x ncols. */
00067 /* The coordinates of the first pixel will be first_row_coord and */
00068 /* first_col_coord. The data structure consists of one contiguous */
00069 /* chunk of memory, consisting of a list of row pointers, followed */
00070 /* by the array element values. */
00071 /* Assumption:  nrows*ncols*element_size, rounded up to a multiple  */
00072 /* of sizeof(long double), must fit in a long type.  If not, then */
00073 /* the "i += ..." step could overflow.                              */
00074 
00075 void **matrix(int nrows, int ncols, int first_row_coord,
00076               int first_col_coord, int element_size)
00077 {
00078   void **p;
00079   int alignment;
00080   long i;
00081 
00082   if(nrows < 1 || ncols < 1) return(NULL);
00083   i = nrows*sizeof(void *);
00084   /* align the addr of the data to be a multiple of sizeof(long double) */
00085   alignment = i % sizeof(long double);
00086   if(alignment != 0) alignment = sizeof(long double) - alignment;
00087   i += nrows*ncols*element_size+alignment;
00088   if((p = (void **)malloc((size_t)i)) != NULL)
00089   {
00090     /* compute the address of matrix[first_row_coord][0] */
00091     p[0] = (char *)(p+nrows)+alignment-first_col_coord*element_size;
00092     for(i = 1; i < nrows; i++)
00093       /* compute the address of matrix[first_row_coord+i][0] */
00094       p[i] = (char *)(p[i-1])+ncols*element_size;
00095     /* compute the address of matrix[0][0] */
00096     p -= first_row_coord;
00097   }
00098   return(p);
00099 }
00100 
00101 /*----------------------------------------------------------------------------------------------------------------------*/
00102 /* SKIP COMMENT */
00103 
00104 /* This function skips past a comment in a file. The comment */
00105 /* begins with a '#' character and ends with a newline character. */
00106 /* The function returns EOF if there's an error. */
00107 
00108 int skipcomment(FILE *fp)
00109 {
00110   int i;
00111 
00112   if((i = getc(fp)) == '#')
00113     while((i = getc(fp)) != '\n' && i != EOF);
00114   return(ungetc(i, fp));
00115 }
00116 
00117 
00118 /*---------------------------------------------------------------------------------------------------------------------*/
00119 
00120 /* REFLECT AN IMAGE ACROSS ITS BORDERS */
00121 
00122 /* The parameter "amount" tells the number of rows or columns to be */
00123 /* reflected across each of the borders. */
00124 /* It is assumed that the data type is unsigned char. */
00125 /* It is assumed that the array was allocated to be of size at least */
00126 /* (nrows+2*amount) by (ncols+2*amount), and that the image was loaded */
00127 /* into the middle portion of the array, with coordinates, */
00128 /*      0 <= row < nrows, 0 <= col < ncols */
00129 /* thereby leaving empty elements along the borders outside the image */
00130 /* The "reflect" function will then fill in those empty */
00131 /* elements along the borders with the reflected image pixel values. */
00132 /* For example, x[0][-1] will be assigned the value of x[0][0], */
00133 /* and x[0][-2] will be assigned the value of x[0][1], if amount=2. */
00134 
00135 void reflect(unsigned char **xc, int nrows, int ncols, int amount)
00136 {
00137   int i, j;
00138 
00139   if(matrix == NULL || nrows < 1 || ncols < 1 || amount < 1
00140     || amount > (nrows+1)/2 || amount > (ncols+1)/2)
00141     error("reflect: bad args");
00142 
00143   for(i = -amount; i < 0; i++)
00144   {
00145     for(j = -amount; j < 0; j++)
00146       xc[i][j] = xc[-i-1][-j-1];
00147     for(j = 0; j < ncols; j++)
00148       xc[i][j] = xc[-i-1][j];
00149     for(j = ncols; j < ncols+amount; j++)
00150       xc[i][j] = xc[-i-1][ncols+ncols-j-1];
00151   }
00152   for(i = 0; i < nrows; i++)
00153   {
00154     for(j = -amount; j < 0; j++)
00155       xc[i][j] = xc[i][-j-1];
00156     for(j = ncols; j < ncols+amount; j++)
00157       xc[i][j] = xc[i][ncols+ncols-j-1];
00158   }
00159   for(i = nrows; i < nrows+amount; i++)
00160   {
00161     for(j = -amount; j < 0; j++)
00162       xc[i][j] = xc[nrows+nrows-i-1][-j-1];
00163     for(j = 0; j < ncols; j++)
00164       xc[i][j] = xc[nrows+nrows-i-1][j];
00165     for(j = ncols; j < ncols+amount; j++)
00166       xc[i][j] = xc[nrows+nrows-i-1][ncols+ncols-j-1];
00167   }
00168 }
00169 /*---------------------------------------------------------------------------------------------*/
00170 
00171 /* REFLECTING FLOAT */
00172 
00173 void reflectf(float **xc, int nrows, int ncols, int amount)
00174 {
00175  int i, j;
00176 
00177   if(matrix == NULL || nrows < 1 || ncols < 1 || amount < 1
00178     || amount > (nrows+1)/2 || amount > (ncols+1)/2)
00179     error("reflect: bad args");
00180 
00181   
00182   for(i = -amount; i < 0; i++)
00183   {
00184     for(j = -amount; j < 0; j++)
00185       xc[i][j] = xc[-i-1][-j-1];
00186     for(j = 0; j < ncols; j++)
00187       xc[i][j] = xc[-i-1][j];
00188     for(j = ncols; j < ncols+amount; j++)
00189       xc[i][j] = xc[-i-1][ncols+ncols-j-1];
00190   }
00191   
00192   for(i = 0; i < nrows; i++)
00193   {
00194     for(j = -amount; j < 0; j++)
00195       xc[i][j] = xc[i][-j-1];
00196     for(j = ncols; j < ncols+amount; j++)
00197       xc[i][j] = xc[i][ncols+ncols-j-1];
00198   }
00199   
00200   for(i = nrows; i < nrows+amount; i++)
00201   {
00202     for(j = -amount; j < 0; j++)
00203       xc[i][j] = xc[nrows+nrows-i-1][-j-1];
00204     for(j = 0; j < ncols; j++)
00205       xc[i][j] = xc[nrows+nrows-i-1][j];
00206     for(j = ncols; j < ncols+amount; j++)
00207       xc[i][j] = xc[nrows+nrows-i-1][ncols+ncols-j-1];
00208   }
00209 }
00210 
00211 /*-------------------------------------------------------------------------------------------------------------------*/
00212 
00213 /* A method for Linear Interpolation */
00214 
00215 float LinearInterpolate(
00216    float y1,float y2,
00217    float mu)
00218 {
00219    return(y1*(1-mu)+y2*mu);
00220 }
00221 
00222 /*-------------------------------------------------------------------------------------------*/
00223 void nms(float **anglec,float **magc,float **dd_image,int nrows ,int ncols,int thresh,int thresh2)
00224 {
00225 
00226   
00227   /* PERFORMS NON-MAXIMUM SUPRESSION TO DETECT EDGES */
00228 
00229   int i,j,r,c,edgepoints1=0,edgepoints2=0,last=0,nochange,row,col,count,run;
00230   FILE *fpy;
00231   float angle,maxm;
00232   int ax_pos,ay_pos,r1,c1,r2,c2,edgecount,r3,c3,r4,c4,count_points=0;
00233   unsigned char** edgemap;
00234   float mu =0.5 , anglecal, ivalue1,ivalue2;
00235   printf("============================================================\n");
00236   printf(" Performing Non_maximum-Supression\n");
00237   
00238 
00239   edgemap = (unsigned char **) matrix(nrows+2,ncols+2,-1,-1,sizeof(char));
00240   /* SO FIRST REFLECT THE MAGC BY 1 PIXEL */ 
00241   printf("-------------------------------------------------\n");
00242   printf("Finished processing: Reflected magnitude image by 1 pixel\n");
00243   printf("Finished processing: Reflected angle image by 1 pixel\n");
00244 
00245  /* HYSTERESIS THRESHODING */ 
00246 
00247       for(i=0;i<nrows;i++)
00248         {
00249           for(j=0;j<ncols;j++)
00250             {
00251               edgemap[i][j] =0;
00252             }
00253         }
00254 
00255       /* ACTUAL PROCESSING */
00256       if(thresh != thresh2)
00257         {
00258           while( nochange)
00259             {
00260                printf( "Run %d \n",run);
00261               count_points =0;
00262               edgepoints2 =0;
00263              
00264               for(i=0;i<ncols;i++)
00265                 {
00266                   for(j=0;j<nrows;j++)
00267                     {
00268               
00269                       count =0;
00270              
00271                       for(row = i-1; row<= i+1; row++)
00272                         {
00273                           for(col = j-1; col <= j+1; col++)
00274                             {
00275                                       
00276                               if((dd_image[i][j] * dd_image[row][col] < 0  && (abs(dd_image[i][j] <= abs(dd_image[row][col])))))
00277                                 count ++;
00278                             }
00279                         }
00280                       if(count >= 1)
00281                         {
00282                           
00283                           if(magc[i][j] >= thresh2 *thresh2)
00284                             {
00285                               edgemap[i][j] = 255;
00286                               edgepoints2++;
00287                             }
00288                           else if(magc[i][j] <= thresh*thresh)
00289                             {
00290                               edgemap[i][j] = 0;
00291                             }
00292                           else
00293                             {
00294                               edgecount =0;
00295                               for(r=i-1;r<= i+1; r++)
00296                                 {
00297                                   for(c=j-1;c<=j+1;c++)
00298                                     {
00299                                       if(edgemap[r][c] == 255 )
00300                                         edgecount++;
00301                                     }
00302                                 }
00303                               if(edgecount >= 1)
00304                                 {
00305                                   edgemap[i][j] = 255;
00306                                   edgepoints2++;
00307                                 }
00308                               else
00309                                 edgemap[i][j] =0;
00310                             }
00311                         }
00312                     }
00313                 }
00314             
00315               /* CONDITION FOR CONVERGENCE */
00316               if(abs(last-edgepoints2) <= 100)
00317                 nochange=0; /* means there is nochange stop */
00318               else
00319                 nochange =1; /* means there is change -> iterate */
00320               last = edgepoints2;
00321               count_points = count_points + edgepoints2;
00322               edgepoints2 =0;
00323               run +=1;
00324               
00325             }
00326           run +=1;
00327         }
00328 
00329         
00330 
00331 printf(" Number of Edgepoints after hysterisis thresholding is %d \n",last );
00332   if(last < 13000)
00333     {
00334       printf("===========================================================\n");
00335       printf(" Reduce lower threshold or Upper threshold and try again \n");
00336       exit(0);
00337     }
00338   printf(" Finsihed calculating the edges using thresholding and NMS\n");
00339 
00340  /* WRITE THE IMAGE AFTER HYSTERISIS THRESHOLDING*/
00341 
00342   if((fpy =fopen("ecanny_map.pgm","w"))== 0)
00343          error(" Error writing file\n");
00344   fprintf(fpy, "P5\n%d %d\n255\n", ncols, nrows);
00345   for(i = 0; i < nrows; i++)
00346     if(fwrite(&edgemap[i][0], sizeof(char), ncols, fpy) != ncols)
00347       error("can't write the image");
00348           
00349           
00350  
00351   if((last > 12000 && last < 15000))
00352     {
00353       printf(" # Edgepoints within the range specified\n");
00354     }
00355   else if(last < 12000 )
00356     {
00357       thresh = thresh -5;
00358       thresh2 = thresh2 -5;
00359       nms(anglec,magc,dd_image,nrows,ncols,thresh,thresh2);
00360     }
00361   else
00362     {
00363       thresh = thresh + 5;
00364       thresh2 = thresh2 +5;
00365       nms(anglec,magc,dd_image,nrows,ncols,thresh,thresh2);
00366     }
00367 
00368   printf("The lower threshold used = %d \n",thresh);
00369    printf("The Upper threshold used = %d \n",thresh2);
00370    printf("==========================================\n");
00371    count_points =0;
00372 }
00373 
00374 
00375   
00376 
00377 /*-----------------------------------------------------------------------------------------------/
00378 /* GAUSSIAN SMOOTHING */
00379 
00380 /* Smooth thr image by a gaussian kernel which depends on the sigma */
00381 /* User driven sigma */
00382 /* store in a global array gauss_smooth_x */
00383 
00384 int gauss( unsigned char **xc, int nrows, int ncols, float sigma,int scale)
00385 {
00386   FILE *fpy;
00387   float  **gauss_filter,sum;
00388   unsigned char ** write_smooth_image;
00389   int grows,gcols,set,i,j,r,c;
00390   printf("----------------------------------------------------------------\n");
00391   printf(" Performing Gaussian Smoothing using sigma of %f \n", sigma);
00392 
00393   
00394   /* OPEN FILE FOR IMAGE WRITE */
00395   if((fpy =fopen("smoothed_image.pgm","w"))== 0)
00396     error(" Error writing file\n");
00397   printf("-------------------------------------------------------------------\n");
00398 
00399 
00400   /* DEFINE A GAUSSIAN FILTER KERNEL */
00401 
00402   printf(" DEFINE A GAUSSIAN KERNEL\n");
00403 
00404   /* ALLOCATE MEMORY */
00405  
00406   grows =4*scale +1;
00407   gcols =4*scale +1;
00408   printf(" ALLOCATE MEMORY\n");
00409   gauss_filter = (float **)matrix(grows, gcols, -2*scale, -2*scale, sizeof(float)); 
00410   printf("ALLOCATED MEMORY\n");
00411      
00412   set = -scale;
00413   /*  ALLOCATE MEMORY FOR GAUSS SMOOTH */
00414   printf(" ALLOCATE MEMORY FOR GAUSS SMOOTH\n");
00415   gauss_smooth = (float **)matrix(nrows-4*set, ncols-4*set, 2*set, 2*set, sizeof(float));
00416   write_smooth_image = (unsigned char **)matrix(nrows, ncols, 0, 0, sizeof(char));
00417   printf("ALLOCATED MEMORY FOR GAUSS SMOOTH\n");
00418   
00419   sum =0;
00420   
00421   reflectf(gauss_smooth,nrows,ncols,2*scale);
00422  
00423   /* FILL IN THE VALUES FOR THE GAUSSIAN FILTER */
00424   printf("------------------------------------------------------\n");
00425   printf(" FILLING UP GAUSS FILTER VALUES\n");
00426   for( i=-(gcols/2); i<= gcols/2; i++)
00427     {
00428       for(j=-(grows/2);j<= gcols/2; j++)
00429         {
00430           gauss_filter[i][j] = (1/(2*M_PI*sigma*sigma))*exp(-((i*i)+ (j*j))/(2*sigma*sigma));
00431           sum = sum + gauss_filter[i][j];
00432         }
00433     }
00434   /* MAKING DC-BIAS ZERO */
00435    for( i=-(gcols/2); i<= gcols/2; i++)
00436     {
00437       for(j=-(grows/2);j<= gcols/2; j++)
00438         {
00439           gauss_filter[i][j] = gauss_filter[i][j];
00440         }
00441     }
00442   printf(" FILLED UP GAUSS FILTER VALUES\n");
00443 
00444  /* INITIALIZE GAUSS SMOOTH*/
00445   printf("---------------------------------------------------------\n");
00446   printf("Initializing Gauss smooth matrix\n");
00447   for(i=set;i<nrows; i++)
00448     {
00449       for(j=set;j< ncols; j++)
00450         {
00451           gauss_smooth[i][j] =0;
00452          
00453         }
00454     }
00455   
00456   /* CONVOLVE WITH GAUSSIAN FILTER AND STORE IN GAUSS_SMOOTH */
00457   printf(" ------------------------------------------------------------\n");
00458   printf(" CONVOLUTION WITH GAUSS FILTER\n");  
00459   for(i=0;i<nrows; i++)
00460     {
00461       for(j=0;j< ncols; j++)
00462         {
00463           /* CONVOLUTION*/
00464            
00465           for( c=set; c<= -set; c++)
00466             {
00467               for(r=set;r<= -set; r++)
00468                 {
00469                   gauss_smooth[i][j] = gauss_smooth[i][j] + (float) xc[i+c][j+r] * gauss_filter[c][r];
00470                                   
00471                 }
00472             }
00473         }
00474     }
00475 
00476   /* CHAR THE IMAGE FOR WRITING */
00477    printf(" CONVOLUTION WITH GAUSS FILTER\n");  
00478   for(i=0;i<nrows; i++)
00479     {
00480       for(j=0;j< ncols; j++)
00481         {
00482           
00483           write_smooth_image[i][j] = (char) (gauss_smooth[i][j]);
00484          /*  gauss_smooth[i][j] =  gauss_smooth[i][j] * dc_gain[i][j]; */
00485         }
00486     }
00487   
00488   /* WRITE THE IMAGE */
00489   fprintf(fpy, "P5\n%d %d\n255\n", ncols, nrows);
00490   for(i = 0; i < nrows; i++)
00491     if(fwrite(&write_smooth_image[i][0], sizeof(char), ncols, fpy) != ncols)
00492       error("can't write the image");
00493 
00494   /* CLOSE FILE & QUIT */
00495   fclose(fpy); 
00496   return 1;
00497   
00498 
00499 }
00500   
00501 
00502 
00503 
00504 
00505 /*---------------------------------------------------------------------------*/
00506 
00507 /* EDGE DETECTION BY CANNY OPERATOR */
00508 
00509 /* The yc array is where the magnitude of the resultant correlation is stored*/
00510 /* The zc array is where the gradient of the resultant correlation is stored*/
00511 /* The edgec array is where edge/ not an edge info is stored in 1's or 0's */
00512 
00513 void canny(unsigned char **xc, int nrows, int ncols, int thresh,int thresh2,float sigma,int scale)
00514 {  
00515  
00516   int r,c,i,j,set,edgepoints2,last,nochange=1,edgecount,check,count,row,col,run,t1,t2;
00517   int count_points =0;
00518   float **fx,**fy,**fxy,**fxx,**fyy,**gradient, **dd_image,**anglec;
00519   unsigned char **edgemap;
00520   FILE *fpy;
00521   printf(" Edge detection will be performed by Canny method\n");
00522   t1 = thresh;
00523   t2 = thresh2;
00524  
00525   printf("-------------------------------------------------------------------\n");
00526 
00527 
00528   /* STEP: 1 --- GAUSSIAN SMOOTHING */
00529   
00530   if(check = gauss( xc, nrows,ncols, sigma,scale) == 0)
00531     error(" Error in gauss smoothing function\n");
00532 
00533   /* ALLOCATING MEM */
00534   
00535   set = scale;
00536   fx = (float **)matrix(nrows+4*scale, ncols+4*scale, -2*scale,-2*scale, sizeof(float)); 
00537   fy = (float **)matrix(nrows+4*scale, ncols+4*scale, -2*scale, -2*scale, sizeof(float)); 
00538   fxy = (float **)matrix(nrows+4*set, ncols+4*set, -2*set, -2*set, sizeof(float)); 
00539   fxx = (float **)matrix(nrows+4*set, ncols+4*set, -2*set, -2*set, sizeof(float)); 
00540   fyy = (float **)matrix(nrows+4*set, ncols+4*set, -2*set, -2*set, sizeof(float)); 
00541   gradient =(float **)matrix(nrows+2*set, ncols+2*set, -2*set, -2*set, sizeof(float));
00542   dd_image=(float **)matrix(nrows+4*set, ncols+4*set, -2*set, -2*set, sizeof(float));
00543   anglec = (float **)matrix(nrows+4*set, ncols+4*set, -2*set, -2*set, sizeof(float));
00544    
00545   reflectf(fx,nrows,ncols,scale);
00546   reflectf(fy,nrows,ncols,scale);
00547   reflectf(fxy,nrows,ncols,scale);
00548   reflectf(dd_image,nrows,ncols,scale);
00549   
00550   edgemap = (unsigned char **)matrix(nrows, ncols, 0, 0, sizeof(char));
00551   printf("-------------------------------------------\n"); 
00552   printf(" Allocated memory for edgemap\n");
00553          
00554   /* FIND THE LoG OF THE SMOOTHED IMAGE */
00555   
00556   for(i=0;i<ncols;i++)
00557     {
00558       for(j=0;j<nrows;j++)
00559         {
00560           
00561           fx[i][j] = gauss_smooth[i][j+1] - gauss_smooth[i][j-1];
00562           fy[i][j] = gauss_smooth[i+1][j] - gauss_smooth[i-1][j];
00563           fxy[i][j] = gauss_smooth[i-1][j-1]+gauss_smooth[i+1][j-1] - gauss_smooth[i+1][j+1] - gauss_smooth[i-1][j-1];
00564           fxx[i][j] = gauss_smooth[i][j+2]- 2*gauss_smooth[i][j] + gauss_smooth[i][j-2];
00565           fyy[i][j] = gauss_smooth[i+2][j] - 2*gauss_smooth[i][j] + gauss_smooth[i-2][j];
00566           gradient[i][j] = fx[i][j] * fx[i][j] + fy[i][j]*fy[i][j];
00567           dd_image[i][j] =(fx[i][j] * fx[i][j] * fxx[i][j] + 2*fx[i][j]*fy[i][j]*fxy[i][j] + fy[i][j]*fy[i][j]* fyy[i][j])/gradient[i][j];
00568           
00569          
00570         }
00571     }
00572 
00573   /* Non-Maximum Supression */
00574   nms(anglec,gradient,dd_image,nrows,ncols,thresh,thresh2);
00575 
00576  
00577 }
00578  
00579 
00580 
00581 
00582 /* ====================================================================================*/
00583 
00584 int main(int argc, char **argv)
00585 {
00586   FILE *fpx, *fpy;
00587   int set,nrows, ncols, i, j,thresh,thresh2,scale,c=0;
00588   float sigma;
00589   unsigned char **x;
00590   char *str;
00591   int r=0,p=0,s=0,f=0,histh=0;
00592 
00593   
00594   /* PARSE THE COMMAND LINE */
00595   while(--argc > 0 && **(argv+1) == '-')
00596     switch((*++argv)[1])
00597     {
00598    
00599     case 's':
00600       
00601           skipNMS =1;
00602           printf("skipNMS\n");
00603 
00604   
00605     case 't':
00606       printf("Setting threshold \n");
00607       printf(" %s is the threshold\n", *++argv);
00608       str = *(argv);
00609       thresh = atoi(str);
00610       str = *(++argv);
00611       printf(" The str is %s\n",str);
00612       
00613       if(str[0] <= '9' && str[0] >= '0')
00614         {
00615            histh =1;
00616            printf("Performing Hysteresis Thresholding\n");      
00617            thresh2 = atoi(str);
00618         }
00619         else
00620         {
00621            histh =0;
00622            printf("Using only one global threshold\n");
00623            thresh2 = thresh;
00624            printf(" Mention the upper threshold also\n");
00625            error("Usage:edge {-roberts,-prewitt,-sobel,-frei} [-skipNMS] [-t thresh_low thresh_high] img ");
00626            exit(0); 
00627         }      
00628      
00629       break;
00630       
00631     case 'c':
00632       printf(" Proceeding with Canny Edge-detection algorithm\n");
00633       c =1;
00634       break;
00635          
00636     default:
00637       error("Usage:edge {-roberts,-prewitt,-sobel,-frei} [-skipNMS] [-t thresh_low thresh_high] img ");
00638       break;
00639 
00640 
00641     }   /* end of case statement */
00642 
00643 
00644   /* IF CANNY ASK USER FOR SIGMA VALUE */
00645   if(c)
00646     {
00647       printf(" Choose a sigma value ( Caorseness parameter) :");
00648       scanf("%f",&sigma);
00649       printf(" Choose a scale for the sigma: ");
00650       scanf("%d",&scale);
00651     }
00652 
00653   /* OPEN FILES */
00654   printf("------------------------------------------------\n");
00655   printf("Opening image file\n"); 
00656   if(argc == 0) fpx = stdin;
00657   else 
00658    {
00659      printf(" Making decesion\n");
00660      if(histh)
00661         {       
00662           if((fpx = fopen(*(++argv), "r")) == NULL)
00663           {             
00664                   printf("%s1\n",(*argv));
00665                   error("can't open file");
00666           }
00667         }               
00668         else
00669         {
00670           if((fpx = fopen((str), "r")) == NULL)
00671           {  
00672              printf("%s2\n",(*argv));
00673              error("can't open file");
00674           }
00675         }
00676    }    
00677   printf("-----------------------------------------\n");
00678   printf(" Opened file --image file %s \n", *argv);
00679   fpy = stdout;
00680 
00681   /* READ HEADER */
00682   if(read_pgm_hdr(fpx, &nrows, &ncols) < 0)
00683     error("not a PGM image or bpp > 8");
00684   printf("------------------------------------------\n");
00685   printf(" Read Header \n");
00686 
00687 
00688   /* ALLOCATE ARRAYS */
00689   /* The x array is extended by one pixel on all four sides. */
00690   /* The first element of the array is x[-1][-1]. */
00691   /* The first actual pixel of the image will be stored at x[0][0]. */
00692   /* y is the output image -- magnitude */
00693   /* z is the output image -- gradient  */
00694   /* edge_map is the matrix containbing whether a given pixel is an edge or not */
00695 
00696   x = (unsigned char **)matrix(nrows+2*scale, ncols+2*scale, -scale, -scale, sizeof(char));  
00697     
00698   printf("--------------------------------------------------------\n");
00699   printf("Allocated arrays for output image:gradient and magnitude and edge map\n");
00700   if(x == NULL) error("can't allocate memory");
00701 
00702 
00703   /* READ THE IMAGE */
00704   for(i = 0; i < nrows; i++)
00705     if(fread(&x[i][0], sizeof(char), ncols, fpx) != ncols)
00706       error("can't read the image");
00707   printf("----------------------------------------------\n");
00708   printf(" Read image: %s\n",*argv);
00709 
00710   /* REFLECT THE IMAGE ACROSS ITS BORDERS BY ONE PIXEL */
00711   reflect(x, nrows, ncols, scale);
00712   printf("-------------------------------------------------\n");
00713   printf("Finished processing: Reflected image by 1 pixel\n");
00714   printf("TEST %d \n",x[-1][-1]);
00715 
00716   /* DECIDE ON NON-MAXIMUM SUPRESSION */
00717 
00718   if(skipNMS)
00719     {
00720       printf("----------------------------------------------\n");
00721       printf(" Skip Non-Maximum-Supression\n");
00722     }
00723   else
00724     {
00725       printf("-----------------------------------------------\n");
00726       printf(" Performing NMS \n");
00727     }
00728   /* DECIDE ON EDGE DETECTION METHOD */
00729   
00730   if(c ==1)
00731     {
00732       printf("-----------------------------\n");
00733       printf(" Apply Canny Edge Detector\n");
00734       canny(x,nrows,ncols,thresh,thresh2,sigma,scale);
00735     }
00736   
00737   fclose(fpx);
00738   return 1;
00739 }
00740 
00741 /*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
Generated on Tue May 3 18:52:22 2005 for CANNY by   1.3.6