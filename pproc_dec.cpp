#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "def.h"
#define EXTERN
#include "globals.h"
#include "triangle.h"
#include "prot.h"
#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>



int main(int argc, char **argv)
{
  int int_max_alfa,step;
  int pipe_disp[2], pid;
  FILE *output;

  printf("%s",triangle_4);

  getopt_dec(argc, argv);

  if ((input = fopen(filein, "r")) == NULL)
      fatal("\n Can't open input file");

  printf("%s\n", filein);

  printf("\nReading the header ...\n");
  unpack(-2,input);    /*Initialize unpack */

  N_BITALFA      = (int)unpack(4,input);
  N_BITBETA      = (int)unpack(4,input);
  min_size       = (int)unpack(7,input);
  max_size       = (int)unpack(7,input);
  SHIFT          = (int)unpack(6,input);
  image_width    = (int)unpack(12,input);
  image_height   = (int)unpack(12,input);
  int_max_alfa   = (int)unpack(8,input);
  isColor        = (int)unpack(1,input);

  printf("Minsize: %d\n", min_size);

  printf("header done.\n");

  bits_per_coordinate_w = ceil(log(image_width  / SHIFT ) / log(2.0));
  bits_per_coordinate_h = ceil(log(image_height / SHIFT ) / log(2.0));

  zeroalfa = 0;
  MAX_ALFA = (double) int_max_alfa / (double)(1 << 8) * ( 8.0) ;

  max = image_height;
  min = image_width;
  if(image_width > image_height ) {
    min = image_height;
    max = image_width;
  }
  
  virtual_size = 1 << (int) ceil(log((double) max) / log(2.0));

  trans = &fractal_code; 
  printf("Reading initial coordinates ... \n");
  fflush(stdout);
  
  read_initial_transformations(0,0,virtual_size);
  printf("done.\n");

  // printf("Press [Enter] to start reading MSB from file:\n");
  // getchar();
  // printf("Reading MSB ie bit_depth 1 ...\n");
  // // read_details(0);


  // printf("done.\n");
  // fflush(stdout);

  // printf(" Original image size: %dx%d\n",image_width,image_height);
  
  // image_width  = (int) rint((zoom * image_width));
  // image_height = (int) rint((zoom * image_height));
  // printf(" Original image size: %dx%d\n",image_width,image_height);

  // printf("Zoom: %f\n", zoom);
 
  // Close the files

  fclose(input);
  exit(0);

  if(zoom != 1.0) {
    printf(" Zooming image to   : %dx%d\n",image_width,image_height);
    fflush(stdout);
  } 

  matrix_allocate(image,2+image_width,2+image_height,PIXEL)
  matrix_allocate(image1,2+image_width,2+image_height,PIXEL)
  if(isColor){
    matrix_allocate(image_uch,2+image_width,2+image_height,PIXEL)
    matrix_allocate(image_vch,2+image_width,2+image_height,PIXEL)
  }
  if(piramidal) {
      min *= zoom;
      step = SHIFT * floor(zoom);
      if(step == 0) step = 1;
      lev = 0;
      while(1){
        if(min < 200 || (step & 1))
            break;
        min  >>= 1;
        step >>= 1;
        lev++;
      }
     printf("\n %d level piramid\n",lev);

     iterative_decoding(lev,iterations,zoom);    /* Decode at low resolution */ 
     piramidal_decoding(lev);                    /* Increase resolution      */
     if(quality)
        iterative_decoding(0,2,1.0);    
  } else 
     iterative_decoding(0,iterations,zoom);  
  

  if(postproc) 
     smooth_image(); 

  if( isColor ){ // Conver to RGB
    std::vector<cv::Mat> yuvChannels;
    cv::Mat ych(image_width,image_height, CV_8U);
    cv::Mat uch(image_width,image_height, CV_8U);
    cv::Mat vch(image_width,image_height, CV_8U);
    yuvChannels.push_back(ych);
    yuvChannels.push_back(uch);
    yuvChannels.push_back(vch);

    for (int iii = 0; iii < image_width; iii++) {
      for (int jjj = 0; jjj < image_height; jjj++) {
        yuvChannels[0].at<uchar>(jjj, iii) = image[jjj][iii];
        yuvChannels[1].at<uchar>(jjj, iii) = image_uch[jjj][iii];
        yuvChannels[2].at<uchar>(jjj, iii) = image_vch[jjj][iii];
      }
    }
    cv::merge(yuvChannels, ych);
    cv::cvtColor(ych, ych, CV_YUV2BGR);
    printf("Writing... output.dec.ppm");
    cv::imwrite("output.dec.ppm", ych);
    
  }


  if(display)  {
     if ( pipe(pipe_disp)) 
        fatal("\n Pipe creation error");

     if ((pid=fork()) < 0)
        fatal("\n Fork error");

     if (pid==0) {     /* Child */
         dup2(pipe_disp[0],fileno(stdin));
         close(pipe_disp[0]); 
         close(pipe_disp[1]); 
	 execlp("xv","xv","-",(char *) 0);
         fatal("\n Exec error xv not started");
     }

     printf("\n");
     close(pipe_disp[0]); 
     output = fdopen(pipe_disp[1], "w");
     writeimage_pipe(output, image,image_width,image_height);
  } 
  else 
    if(raw_format)  
       writeimage_raw(fileout, image,image_width,image_height);
    else
      writeimage_pgm(fileout, image,image_width,image_height);
     
  free(image[0]);
  free(image1[0]);

  return 0;
}


void zooming(double scalefactor)
{
  trans = &fractal_code;
  while (trans->next != NULL) {
     trans = trans->next;

     trans->rrx  *= scalefactor;
     trans->rry  *= scalefactor;
     trans->rx   *= scalefactor;
     trans->ry   *= scalefactor;
     trans->dx   *= scalefactor;
     trans->dy   *= scalefactor;
  }
}


void read_initial_transformations(int atx,int aty,int size)
{ 
  long qdy, qdx;
  if(atx >= image_height  || aty >= image_width )
      return;

  if (size > max_size || atx+size > image_height || aty+size > image_width){
      read_initial_transformations(atx,aty,size/2);
      read_initial_transformations(atx+size/2,aty,size/2);
      read_initial_transformations(atx,aty+size/2,size/2);
      read_initial_transformations(atx+size/2,aty+size/2,size/2);
      return;
  }

  if (size > min_size && unpack(1,input)) {
      /* A 1 means we subdivided.. so quadtree */
      // pack(1, (long)1, tempfile);
      printf("1\n");

      read_initial_transformations(atx,aty,size/2);
      read_initial_transformations(atx+size/2,aty,size/2);
      read_initial_transformations(atx,aty+size/2,size/2);
      read_initial_transformations(atx+size/2,aty+size/2,size/2);
  } else {
      // pack(1, (long)0, tempfile);
      printf("0\n");

      // trans->next = (struct t_node *) malloc(sizeof(struct t_node ));
      // trans       = trans->next; 
      // trans->next = NULL;
      // trans->sym_op = (int)unpack(3, input);
      // qdx = unpack(bits_per_coordinate_h,input);
      // qdy = unpack(bits_per_coordinate_w,input);
      // pack(bits_per_coordinate_h, (long)qdx, tempfile);
      // pack(bits_per_coordinate_w, (long)qdy, tempfile);

      // trans->dx = SHIFT * qdx;
      // trans->dy = SHIFT * qdy;

      // printf("%ld %ld\n", qdx, qdy);
  }
}

void read_details(int bit_depth) {
  trans = &fractal_code;
  double alfa, beta;
  int bits = 2;
  if(bit_depth < N_BITBETA)
    bits = 3;
  if(bit_depth < N_BITALFA)
    bits = 4;


  int value = 0;
  while(trans->next){
    value = (int)unpack(bits, input);
    // printf("%d\n", value);
    trans = trans->next;
    if(bit_depth < N_BITALFA){
      if(isColor)
        trans->qalfa |=  value & (1<<3);
      else
        trans->qalfa |=  value & (1<<1);
    }
    if(bit_depth < N_BITBETA){
      if (isColor)
        trans->qbeta |= value & (1<<2);
      else
        trans->qbeta |= value & 1;
    }
    if(isColor){
      if (bit_depth < 8)
      {
        trans->um |= value & (1<<1);        
        trans->vm |= value & 1;
      }
    }
    
    alfa = (double) trans->qalfa / (double)(1 << N_BITALFA) * (MAX_ALFA) ;

    /* Compute beta from the quantized value */
    beta = (double) trans->qbeta/(double)((1 << N_BITBETA)-1)* ((1.0+fabs(alfa)) * 255);
    if (alfa > 0.0) beta  -= alfa * 255;
     
    trans->alfa = alfa;
    trans->beta = beta;

  }
}

void iterative_decoding(int level,int n_iter,double zoo)
{
  int rx,ry,rrx,rry,dx,dy;
  register int i,j,ii,jj,s;
  register PIXEL **imag,**imag1;
  double pixel;
  double z_factor;
  int width,height;
  static int first_time = 0;


  printf("\n");
  z_factor = zoo / (double) (1 << level);
  zooming(z_factor);
  width  = (int) rint(image_width  * z_factor / zoo); 
  height = (int) rint(image_height * z_factor / zoo); 

  if(first_time++ == 0)
     for(i=0;i< height;i++)
     for(j=0;j< width ;j++) 
        image[i][j] = 128;

  for(s=0; s < n_iter ; s++) {
     imag = image;
     imag1 = image1;

     if(level > 0)
         printf(" Decoding at low resolution (%dx%d) %d\r",width,height,s);
     else
         printf(" Iterative decoding... %d\r",s);

     fflush(stdout);
     trans = &fractal_code;
     while(trans->next != NULL)  {
        trans = trans->next;
        rx=(int)rint(trans->rx);
        ry=(int)rint(trans->ry);
        dx=(int)rint(trans->dx);
        dy=(int)rint(trans->dy);
        rrx=(int)rint(trans->rrx);
        rry=(int)rint(trans->rry);

        switch(trans->sym_op) {     
         case IDENTITY   : 
            for(i=rx,ii=dx;i< rrx;i++,ii+=2)
            for(j=ry,jj=dy;j< rry;j++,jj+=2) {
               pixel = (double)(imag[ii][jj]+imag[ii+1][jj] +
                                         imag[ii][jj+1]+imag[ii+1][jj+1])/4.0;
               imag1[i][j] = bound(0.5 + pixel * trans->alfa + trans->beta);
               if (isColor) {
                 image_uch[i][j] = trans->um;
                 image_vch[i][j] = trans->vm;
               }
            }
            break;
         case R_ROTATE90 :  
            for(j=rry-1,ii=dx;j>= ry;j--,ii+=2)
            for(i=rx,jj=dy;i< rrx;i++,jj+=2) {
               pixel = (double)(imag[ii][jj]+imag[ii+1][jj] +
                                         imag[ii][jj+1]+imag[ii+1][jj+1])/4.0;
               imag1[i][j] = bound(0.5 + pixel * trans->alfa + trans->beta);
               if (isColor) {
                 image_uch[i][j] = trans->um;
                 image_vch[i][j] = trans->vm;
               }

            }
            break;
         case L_ROTATE90 :  
	    for(j=ry,ii=dx;j< rry;j++,ii+=2) 
            for(i=rrx-1,jj=dy;i>= rx;i--,jj+=2) {
               pixel = (double)(imag[ii][jj]+imag[ii+1][jj] +
                                         imag[ii][jj+1]+imag[ii+1][jj+1])/4.0;
               imag1[i][j] = bound(0.5 + pixel * trans->alfa + trans->beta);
               if (isColor) {
                 image_uch[i][j] = trans->um;
                 image_vch[i][j] = trans->vm;
               }

            }
            break;
         case ROTATE180 :  
	    for(i=rrx-1,ii=dx;i>= rx;i--,ii+=2) 
            for(j=rry-1,jj=dy;j>= ry;j--,jj+=2) {
               pixel = (double)(imag[ii][jj]+imag[ii+1][jj] +
                                         imag[ii][jj+1]+imag[ii+1][jj+1])/4.0;
               imag1[i][j] = bound(0.5 + pixel * trans->alfa + trans->beta);
               if (isColor) {
                 image_uch[i][j] = trans->um;
                 image_vch[i][j] = trans->vm;
               }
            }
            break;
         case R_VERTICAL :  
	    for(i=rx,ii=dx;i< rrx;i++,ii+=2) 
            for(j=rry-1,jj=dy;j>= ry;j--,jj+=2) {
               pixel = (double)(imag[ii][jj]+imag[ii+1][jj] +
                                         imag[ii][jj+1]+imag[ii+1][jj+1])/4.0;
               imag1[i][j] = bound(0.5 + pixel * trans->alfa + trans->beta);
               if (isColor) {
                 image_uch[i][j] = trans->um;
                 image_vch[i][j] = trans->vm;
               }
            }
            break;
         case R_HORIZONTAL: 
	    for(i=rrx-1,ii=dx;i>= rx;i--,ii+=2)
            for(j=ry,jj=dy;j< rry;j++,jj+=2) {
               pixel = (double)(imag[ii][jj]+imag[ii+1][jj] +
                                         imag[ii][jj+1]+imag[ii+1][jj+1])/4.0;
               imag1[i][j] = bound(0.5 + pixel * trans->alfa + trans->beta);
               if (isColor) {
                 image_uch[i][j] = trans->um;
                 image_vch[i][j] = trans->vm;
               }
            }
            break;
         case F_DIAGONAL :  
	    for(j=ry,ii=dx;j< rry;j++,ii+=2)
            for(i=rx,jj=dy;i< rrx;i++,jj+=2) {
               pixel = (double)(imag[ii][jj]+imag[ii+1][jj] +
                                         imag[ii][jj+1]+imag[ii+1][jj+1])/4.0;
               imag1[i][j] = bound(0.5 + pixel * trans->alfa + trans->beta);
               if (isColor) {
                 image_uch[i][j] = trans->um;
                 image_vch[i][j] = trans->vm;
               }
            }
            break;
         case S_DIAGONAL:   
	    for(j=rry-1,ii=dx;j>= ry;j--,ii+=2) 
	    for(i=rrx-1,jj=dy;i>= rx;i--,jj+=2){
               pixel = (double)(imag[ii][jj]+imag[ii+1][jj] +
                                         imag[ii][jj+1]+imag[ii+1][jj+1])/4.0;
               imag1[i][j] = bound(0.5 + pixel * trans->alfa + trans->beta);
               if (isColor) {
                 image_uch[i][j] = trans->um;
                 image_vch[i][j] = trans->vm;
               }
            }
            break;

       }
     } 

   swap1(image1, image, PIXEL **)

 }
  printf("\n");
}


void piramidal_decoding(int level)
{
  int rx,ry,rrx,rry,dx,dy;
  register int i,j,ii,jj;
  register PIXEL **imag,**imag1;
  double pixel;

  if(level < 1)
     return;

  zooming(2.0);   /* Increase resolution */

  imag = image;
  imag1 = image1;

  printf(" Increasing resolution... \r");
  fflush(stdout);
  trans = &fractal_code;

  while(trans->next != NULL)  {
     trans = trans->next;

     rx=(int)rint(trans->rx);
     ry=(int)rint(trans->ry);
     dx=(int)rint(trans->dx);
     dy=(int)rint(trans->dy);
     rrx=(int)rint(trans->rrx);
     rry=(int)rint(trans->rry);

     switch(trans->sym_op) {
        case IDENTITY   :  
	   for(i=rx,ii=dx >> 1;i< rrx;i++,ii++)
           for(j=ry,jj=dy >> 1;j< rry;j++,jj++) {
              pixel = (double)imag[ii][jj];
              imag1[i][j] = bound(0.5 + pixel * trans->alfa + trans->beta);
              if (isColor) {
                image_uch[i][j] = trans->um;
                image_vch[i][j] = trans->vm;
              }
           }
           break;
        case R_ROTATE90 :  
	   for(j=rry-1,ii=dx >> 1;j>= ry;j--,ii++)
           for(i=rx,jj=dy >> 1;i< rrx;i++,jj++) {
              pixel = (double)imag[ii][jj];
              imag1[i][j] = bound(0.5 + pixel * trans->alfa + trans->beta);
              if (isColor) {
                image_uch[i][j] = trans->um;
                image_vch[i][j] = trans->vm;
              }
           }
           break;
        case L_ROTATE90 :  
	   for(j=ry,ii=dx >> 1;j< rry;j++,ii++)
           for(i=rrx-1,jj=dy >> 1;i>= rx;i--,jj++) {
              pixel = (double)imag[ii][jj];
              imag1[i][j] = bound(0.5 + pixel * trans->alfa + trans->beta);
              if (isColor) {
                image_uch[i][j] = trans->um;
                image_vch[i][j] = trans->vm;
              }
           }
           break;
        case ROTATE180 :  
	   for(i=rrx-1,ii=dx >> 1;i>= rx;i--,ii++)
           for(j=rry-1,jj=dy >> 1;j>= ry;j--,jj++) {
              pixel = (double)imag[ii][jj];
              imag1[i][j] = bound(0.5 + pixel * trans->alfa + trans->beta);
              if (isColor) {
                image_uch[i][j] = trans->um;
                image_vch[i][j] = trans->vm;
              }
           }
           break;
        case R_VERTICAL :  
	   for(i=rx,ii=dx >> 1;i< rrx;i++,ii++)
           for(j=rry-1,jj=dy >> 1;j>= ry;j--,jj++) {
              pixel = (double)imag[ii][jj];
              imag1[i][j] = bound(0.5 + pixel * trans->alfa + trans->beta);
              if (isColor) {
                image_uch[i][j] = trans->um;
                image_vch[i][j] = trans->vm;
              }
           }
           break;
        case R_HORIZONTAL: 
	   for(i=rrx-1,ii=dx >> 1;i>= rx;i--,ii++)
           for(j=ry,jj=dy >> 1;j< rry;j++,jj++) {
              pixel = (double)imag[ii][jj];
              imag1[i][j] = bound(0.5 + pixel * trans->alfa + trans->beta);
              if (isColor) {
                image_uch[i][j] = trans->um;
                image_vch[i][j] = trans->vm;
              }
           }
           break;
        case F_DIAGONAL :  
	   for(j=ry,ii=dx >> 1;j< rry;j++,ii++)
           for(i=rx,jj=dy >> 1;i< rrx;i++,jj++) {
              pixel = (double)imag[ii][jj];
              imag1[i][j] = bound(0.5 + pixel * trans->alfa + trans->beta);
              if (isColor) {
                image_uch[i][j] = trans->um;
                image_vch[i][j] = trans->vm;
              }
           }
           break;
        case S_DIAGONAL:   
	   for(j=rry-1,ii=dx >> 1;j>= ry;j--,ii++)
           for(i=rrx-1,jj=dy >> 1;i>= rx;i--,jj++){
              pixel = (double)imag[ii][jj];
              imag1[i][j] = bound(0.5 + pixel * trans->alfa + trans->beta);
              if (isColor) {
                image_uch[i][j] = trans->um;
                image_vch[i][j] = trans->vm;
              }
           }
           break;
     }

  } 
  swap1(image1, image, PIXEL **)

  if(level > 1) {
    if(quality)
       iterative_decoding(0,2,1.0); 
    piramidal_decoding(level-1);
  }

}


void smooth_image()
{
    double pixel1, pixel2;
    int i,j;
    int w1,w2;
    int rx,ry,rrx,rry;

    printf("\n Postprocessing ...");
    fflush(stdout);

    trans = &fractal_code;
    while (trans->next != NULL) {
       trans = trans->next;

       rx=(int)rint(trans->rx);
       ry=(int)rint(trans->ry);
       rrx=(int)rint(trans->rrx);
       rry=(int)rint(trans->rry);

       if (rx == 0 || ry == 0 || (int)trans->size == 1)
           continue;

       if (trans->size == min_size) { 
          w1 = 5;
          w2 = 1;
       } 
       else {
          w1 = 2;
          w2 = 1;
       }

       for (i=rx; i<rrx; ++i) {
          pixel1 = image[i][ry];
          pixel2 = image[i][ry-1];
          image[i][ry] = (w1*pixel1 + w2*pixel2)/(w1+w2);
          image[i][ry-1] = (w2*pixel1 + w1*pixel2)/(w1+w2);
       }

       for (j=ry; j<rry; ++j) {
          pixel1 = image[rx][j];
          pixel2 = image[rx-1][j];
          image[rx][j] = (w1*pixel1 + w2*pixel2)/(w1+w2);
          image[rx-1][j] = (w2*pixel1 + w1*pixel2)/(w1+w2);
       }
   }
   printf("done \n");
   fflush(stdout);
}



