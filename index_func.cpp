/******************************************************************************
 ==============================================================================

             '`
            '--`        Mars -- A quadtree based fractal image
           '`  '`       coder/decoder. Version 1.0 (10/28/1998) 
          '--`'--`      Copyright (C) 1998 Mario Polvere 
         '`      '`     University of Salerno Italy 
        '--`    '--`    
       '`  '`  '`  '`
      '--`'--`'--`'--`

      This program is free software; you can redistribute it and/or modify
      it under the terms of the GNU General Public License as published by
      the Free Software Foundation; either version 2 of the License, or
      (at your option) any later version.

      This program is distributed in the hope that it will be useful,
      but WITHOUT ANY WARRANTY; without even the implied warranty of
      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
      GNU General Public License for more details.

      You should have received a copy of the GNU General Public License
      along with this program; if not, write to the Free Software
      Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 
      02111-1307, USA

      The author can be contacted via e-mail:
                               marpol@iname.com
                               teimars@erifs1.ericsson.se

 ==============================================================================
 ******************************************************************************/



#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
# include <cstdlib>
# include <iostream>
# include <iomanip>
# include <cmath>

#include "def.h"
#include "globals.h"
#include "prot.h"

int class_count[24];
void HV_FisherIndexing(int x_size,int y_size)
{
  int i,j,k,h;
  int count = 0;
  int iso, clas, var_class;
  double sum,sum2;
  double **domi,**flip_domi;
  register double pixel;
  register int x,y;
  struct c *node;


  matrix_allocate(domi,x_size,y_size,double)
  // matrix_allocate(flip_domi,x_size,y_size,double)

  for(i=0;i< image_height - 2 * y_size +1 ;i+= SHIFT) {
    for(j=0;j< image_width - 2 * x_size +1 ;j+= SHIFT ) {
      count ++;
      k=0;
      sum  = 0.0;
      sum2 = 0.0;
   
      for(y=0;y< y_size;y++)
      for(x=0;x< x_size;x++) {
        pixel = contract[y+(i>>1)][x+(j>>1)];
        sum  += pixel;
        sum2 += pixel * pixel;
        domi[y][x] = pixel;
      }
      // printf("x size = %d \t y size = %d \n",x_size,y_size);
      
                                   /* Compute the symmetry operation which */
      adaptiveNewclass_2(x_size,y_size,domi,&iso,&clas); /* brings the domain in the canonical   */
                                      /* orientation                          */
      // flips(size,domi,flip_domi,iso); 
      // printf("class = %d\n\n",clas);
      // var_class = variance_class(size,flip_domi);

      node = (struct c *) malloc(sizeof(struct c));
      node -> ptr_x = i;
      node -> ptr_y = j;
      node -> sum   = sum;
      node -> sum2  = sum2;
      node -> iso  = iso;
      node -> var = variance_3(x_size,y_size,domi,0,0);
      node -> next  = HV_fisher_class[y_size][x_size][clas];
      HV_fisher_class[y_size][x_size][clas] = node;
      class_count[clas]++;
      
    }
    printf(" Classification (Fisher) domain (%dx%d)  %d \r",x_size,y_size,count) ;
    fflush(stdout);

  }

  for(j=0;j<24;j++)
    if(HV_fisher_class[y_size][x_size][j] != NULL)
         goto out_loops;


  out_loops:

  for(h=0;h<24;h++)
    if( HV_fisher_class[y_size][x_size][h] == NULL){
      // printf("Current class is null\n");
      HV_fisher_class[y_size][x_size][h] = HV_fisher_class[y_size][x_size][j];
    }

  printf("\n");
 

  free(domi[0]);
  // free(flip_domi[0]);
}


void ComputeMc(double **block,int size,double *x,double *y,int s)
{
  double row = 0.0;
  double col = 0.0;
  double a = 0.0;
  double b = 0.0;
  double mass  = 0.0;
  register int i,j;
  
  for(i=0;i<size;i++) {
    a = 0.0;
    b = 0.0;
    for(j=0;j<size;j++) {
      a += block[i][j] ;
      b += block[j][i] ;
      mass += block[j][i];
    }
    row += a * (i+1);
    col += b * (i+1); 
  }
  if(mass != 0.0) {
      *y = (size+1)/2.0 - row / mass ; 
      *x = col / mass - (size+1)/2.0 ; 

  }
  else {
   *x = 0.0;
   *y = 0.0;
  }
}


int match(int *iso_1)
{
  int i,j,flag;
  static int count;
  // printf("iso_1[0] = %d\t count = %d\n",iso_1[0],count++);
  for(j=0; j< 24; j++) {
    flag =1;
    for(i=0; i< 4; i++)
      if(iso_1[i] != ordering[j][i]) {
        flag = 0;
        break;
      }
    if (flag == 1) return j;
  }
  fatal("\n Error");
  return(-1);
}


int EnergyCoeff_class(int size,double **block)
{
  int i,j;
  double **n_block;
  matrix_allocate(n_block,size,size,double);
  double sum2 = 0.0;
  double sum = 0.0;
  double mean2 = 0.0, energy_coef = 0.0;
  double mean = 0.0,var;
  int atx = 0, aty = 0;

  for(i=0;i<size;i++){
  for(j=0;j<size;j++) {
     sum2 += block[i][j] * block[i][j];
     sum  += block[i][j];
     // printf("%f\t",block[i][j]);

  }
  // printf("\n");
  }

  mean = sum / (size * size);
  mean2 = sum2 / (size * size);
  var = mean2 - mean * mean;
  // printf("sum= %f\n",sum);
  // printf("size= %d\n",size);
  // printf("\n");
  // printf("\n");


  for(i=0;i<size;i++){
    for(j=0;j<size;j++){
      // double det = sqrt(var * size *size);
      double det = sqrt(var);
      if(det != 0 )
        n_block[i][j] = (block[i][j] - mean) / det; 
      else
        n_block[i][j] = 0;
      // printf("%f\t",n_block[i][j]);
    }
    // printf("\n");
  }
   
  double *vec = new double[size * size];
  for(int x=0; x<size; x++){
    for(int y=0; y<size; y++){
        vec[x+y*size] = n_block[x][y];   
    }
  }
  //haar_2d(size,size,vec);
  double **t_n_block ;
  double *domain_vec = new double[size*size];
  matrix_allocate(t_n_block,size,size,double);
  for(int x=0; x<size; x++){
    for(int y=0; y<size; y++){

        // n_block[x][y] = domain_vec[x+y*size];  
        // energy_coef += (n_block[x][y] * n_block[x][y]) ; 
        energy_coef += abs(n_block[x][y]) ; 

    }
  }
   printf("%f \n",energy_coef);
   printf("\n");
   printf("\n");

  // printf("ecof= %f\n",energy_coef);
  int ecof = (int)floor(energy_coef);
  return ecof;
}

int std_class(int size, double **block)
{
  double std = sqrt(variance_2(size,block,0,0));
  // printf("class = %d\n",(int)rint(std));
  return (int)rint(std);
}

int ent_class(int size, double **block)
{
  double ent = entropy_2(size,block,0,0);
  if(ent < 0) ent = 0.0;
  return ((int)floor(ent));
}

int adaptiveVariance_class(int x_size,int y_size,double **block)
{
  int i,j;
  double a[4] = {0.0,0.0,0.0,0.0};
  int order[4];

  a[0] =  variance_3(x_size/2,y_size/2, block, 0, 0);
  a[1] =  variance_3(x_size/2,y_size/2, block, 0, y_size/2);
  a[2] =  variance_3(x_size/2,y_size/2, block, x_size/2, 0);
  a[3] =  variance_3(x_size/2,y_size/2, block,x_size/2, y_size/2);

  for (i=0; i<4; ++i)  order[i] = i ;

  for (i=2; i>=0; --i)
  for (j=0; j<=i; ++j)
     if (a[j]<a[j+1]) {
       swap1(order[j], order[j+1],int)
       swap1(a[j], a[j+1],double)
     }

   int flag;

  for(int j=0; j< 24; j++) {
    flag =1;
    for(int i=0; i< 4; i++)
      if(order[i] != ordering[j][i]) {
        flag = 0;
        break;
      }
    if (flag == 1) return j;
  }
  fatal("\n error occured");
  return (-1);
}


int variance_class(int size,double **block)
{
  int i,j;
  double a[4] = {0.0,0.0,0.0,0.0};
  int order[4];

  a[0] =  variance_2(size/2, block, 0, 0);
  a[1] =  variance_2(size/2, block, 0, size/2);
  a[2] =  variance_2(size/2, block, size/2, 0);
  a[3] =  variance_2(size/2, block, size/2, size/2);

  for (i=0; i<4; ++i)  order[i] = i ;

  for (i=2; i>=0; --i)
  for (j=0; j<=i; ++j)
     if (a[j]<a[j+1]) {
       swap1(order[j], order[j+1],int)
       swap1(a[j], a[j+1],double)
     }

  return  match(order);
}


int hurtgen_class(int size,double **block)
{
  int i,j;
  int clas = 0;
  double quadrant_mean[4] = {0.0,0.0,0.0,0.0};
  double mean;

  for(i=0;i<size/2;i++)
  for(j=0;j<size/2;j++) {
     quadrant_mean[0] += block[i][j];
     quadrant_mean[1] += block[i][j+size/2];
     quadrant_mean[2] += block[i+size/2][j];
     quadrant_mean[3] += block[i+size/2][j+size/2];
  }

  mean = (quadrant_mean[0] + quadrant_mean[1] + 
	           quadrant_mean[2] + quadrant_mean[3]) / (size * size);

  quadrant_mean[0] /= (size/2 * size/2);
  quadrant_mean[1] /= (size/2 * size/2);
  quadrant_mean[2] /= (size/2 * size/2);
  quadrant_mean[3] /= (size/2 * size/2);

  for(i=0;i<4;i++) {
    clas <<=1;
    if(mean < quadrant_mean[i])
       clas |= 1;
  }
  return clas;
}

void adaptiveNewclass_2(int x_size, int y_size, double **block, 
                       int *isom, int *clas)
{
  double a[4] = {0.0,0.0,0.0,0.0};
  int delta[3] = {6,2,1};
  int class1  = 0;

  if(x_size > 0 && y_size >0){
  for(int i=0; i < y_size/2; i++){
    for(int j=0; j < x_size/2; j++){
      a[0] += block[i][j];
      a[1] += block[i][j+x_size/2];
      a[2] += block[i+y_size/2][j];
      a[3] += block[i+y_size/2][j+x_size/2];
     
    }
  }
 }
 for(int i=0; i <= 2; ++i){
  for(int j=i+1; j<=3; ++j){
    if(a[i] < a[j])
      class1 = class1 + delta[i];
  }
 }
  
 if(class1 > N_L1_CLASSES - 1)
  class1 = N_L1_CLASSES - 1;
 
 if(class1 < 0)
  class1 = 0;

 *clas = class1;

}

void adaptiveNewclass(int x_size, int y_size, double **block, 
                       int *isom, int *clas)
{
  double a[4] = {0.0,0.0,0.0,0.0};
  int order[4] = {0,1,2,3};
  
  if(x_size > 0 && y_size >0){
  for(int i=0; i < y_size/2; i++){
    for(int j=0; j < x_size/2; j++){
      a[0] += block[i][j];
      a[1] += block[i][j+x_size/2];
      a[2] += block[i+y_size/2][j];
      a[3] += block[i+y_size/2][j+x_size/2];
     
    }
  }
 }

 for(int i=2; i>=0; i--){
  for(int j=0; j<=i; j++){
    if(a[j] < a[j+1]){
      swap1(order[j],order[j+1],int);
      swap1(a[j],a[j+1],double);
    }
  }
 }
  int index = match(order);
  *isom = ordering[index][4];
  *clas = ordering[index][5];
}

void newclass(int size,double **block,int *isom,int *clas)
{
  int i,j;
  int index;
  double a[4] = {0.0,0.0,0.0,0.0};
  int order[4];


  for(i=0;i<size/2;i++)
  for(j=0;j<size/2;j++) {
     a[0] += block[i][j];
     a[1] += block[i][j+size/2];
     a[2] += block[i+size/2][j];
     a[3] += block[i+size/2][j+size/2];
  }

  for (i=0; i<4; ++i)  order[i] = i
        ;

  for (i=2; i>=0; --i)
  for (j=0; j<=i; ++j)
     if (a[j]<a[j+1]) {
       swap1(order[j], order[j+1],int)
       swap1(a[j], a[j+1],double)
     }

  index = match(order);

  *isom = ordering[index][4];
  *clas = ordering[index][5];
}


void contraction(double **t,PIXEL **fun,int s_x,int s_y)
{
  double tmp;
  int i,j,k,w,s,z;

  for(i=s_x;i< image_height - s_x;i+=2)
  for(j=s_y;j< image_width  - s_y;j+=2) {
     tmp=0.0;
     for(k=0; k < 2; k++) {
       s = 0;
       if((i+ k) >= image_height) 
	  s = 1;
       for(w=0; w < 2; w++) {
         z = 0;
         if((j+ w) >= image_height) 
           z = 1;
         tmp +=(double) fun[i+k-s][j+w-z];
         t[i >> 1][j >> 1] = tmp/4.0;
       }
     }
  }
}


int ShrunkBlock(double **block,int size, int srunk_fact)
{
  double tmp;
  int i,j,k,w;
  int shift;

  shift = (int) rint(log((double) srunk_fact) / log(2.0));

  for(i=0;i< size ;i+=srunk_fact)
  for(j=0;j< size ;j+=srunk_fact) {
     tmp=0;
     for(k=0; k < srunk_fact; k++)
     for(w=0; w < srunk_fact; w++)
        tmp += block[i+k][j+w];
     block[i >> shift][j >> shift] = tmp / (double) (srunk_fact*srunk_fact);
  }
  return(size / srunk_fact);
}


void ComputeSaupeVectors(double **block, int size,int index, float *vector)
{
  double sum = 0.0;
  double var = 0.0;
  double s, v ;
  register int j,i;
  register int k=0;
  int newsize;

  if(average_factor[index] > 1)
     newsize = ShrunkBlock(block,size,average_factor[index]);
  else
     newsize = size;

  for(i=0;i<newsize;i++)
  for(j=0;j<newsize;j++) {
     sum += block[i][j];
     var += block[i][j] * block[i][j];
  }

  s = sum / (double)(newsize*newsize);
  v = sqrt(var - sum*sum / (double)(newsize*newsize));

  for(i=0;i<newsize;i++)
  for(j=0;j<newsize;j++,k++)
    vector[k] = (block[i][j] - s ) / v;

}


void ComputeMcVectors(double **block,double **block_tmp,
                                    int size, int index,double *vector)
{
  double mean;
  double xx,yy;
  double theta;
  double sum = 0.0;
  double **block_1;
  double **block_2;
  register int x,y,i;
  int newsize;

  block_1 = block;
  block_2 = block_tmp;

  if(average_factor[index] > 1)
     newsize = ShrunkBlock(block,size,average_factor[index]);
  else
     newsize = size;
 
  ComputeMc(block_1,newsize, &xx, &yy, 0); /* Compute the MC of the block     */
  theta = atan2(yy, xx);                   /* and put it in polar coordinates */
  if(theta >= 0.0) 
     vector[0] = theta;
  else  
     vector[0] = TWOPI + theta;

  for(x=0;x< newsize;x++)
  for(y=0;y< newsize;y++) {
     sum += block_1[x][y];
  }

  for(i=1;i<2;i++) {                    /* Compute other features by       */
     mean = sum / (newsize * newsize);  /* transforming the original block */
     sum = 0.0;
     for(x=0;x< newsize;x++)
     for(y=0;y< newsize;y++) {
        block_2[x][y] = (block_1[x][y] - mean) * (block_1[x][y] - mean);
        sum += block_2[x][y];
     }
     ComputeMc(block_2,newsize, &xx, &yy,i);
     theta = atan2(yy, xx);     
     if(theta >= 0.0) 
        vector[i] = theta;
     else  
        vector[i] = TWOPI + theta;

     swap1(block_1, block_2, double **)
  }

}


void Saupe_FisherIndexing(int size,int s)
{
  int i,j,k;
  int count = 0;
  int cbook_size;
  int dim_vectors;
  int clas,iso;
  double sum, sum2;
  double **dom_tmp,**domi, **flip_domi;
  register double pixel;
  register int x,y;

  cbook_size = (1+image_width / SHIFT) * (1+image_height / SHIFT);

  dim_vectors = feat_vect_dim[(int) rint(log((double)(size))/log (2.0))];
  matrix_allocate(f_vectors[s],dim_vectors,cbook_size,float)
  codebook[s]=(struct  code_book*) malloc(cbook_size*sizeof(struct code_book));

  matrix_allocate(domi,size,size,double)
  matrix_allocate(flip_domi,size,size,double)
  matrix_allocate(dom_tmp,size,size,double)

  for(i=0;i< image_height - 2 * size +1 ;i+= SHIFT) {
    for(j=0;j< image_width  - 2 * size +1 ;j+= SHIFT,count ++) {
      k=0;
      sum  = 0.0;
      sum2 = 0.0;
      for(x=0;x< size;x++)
      for(y=0;y< size;y++) {
        pixel = contract[x+(i>>1)][y+(j>>1)];
        sum  += pixel;
        sum2 += pixel * pixel;
        domi[x][y] = pixel;
      }
                                      /* Compute the symmetry operation which */
      newclass(size,domi,&iso,&clas); /* brings the domain in the canonical   */
                                      /* orientation                          */
                                      
      flips(size,domi,flip_domi,iso); 

      ComputeSaupeVectors(flip_domi,size,s,f_vectors[s][count]);

      codebook[s][count].sum   = sum;
      codebook[s][count].sum2  = sum2;
      codebook[s][count].ptr_x = i;
      codebook[s][count].ptr_y = j;
      codebook[s][count].isom  = iso;
    }
    printf(" Extracting [%d] features (Saupe)  domain (%dx%d)  %d \r",
	                         	    dim_vectors, size,size,count) ;
    fflush(stdout);

  }
  printf("\n Building Kd-tree... ") ;
  fflush(stdout);
  kd_tree[s] = kdtree_build(f_vectors[s],count-1,dim_vectors);
  printf("Done\n") ;
  fflush(stdout);

  free(domi[0]);
  free(dom_tmp[0]);
  free(flip_domi[0]);
}



void MassCenterIndexing(int size,int s)
{
  int i,j,k;
  int count = 0;
  int cx,cy;
  struct c *node; 
  double sum,sum2;

  double vector[16];

  double **dom_tmp,**domi,**flip_domi;
  register double pixel;
  register int x,y;

  matrix_allocate(domi,size,size,double)
  matrix_allocate(dom_tmp,size,size,double)
  matrix_allocate(flip_domi,size,size,double)

  for(i=0;i< image_height - 2 * size +1 ;i+= SHIFT) {
    for(j=0;j< image_width  - 2 * size +1 ;j+= SHIFT,count ++) {
      k=0;
      sum  = 0.0;
      sum2 = 0.0;
      for(x=0;x< size;x++)
      for(y=0;y< size;y++) {
        pixel = contract[x+(i>>1)][y+(j>>1)];
        sum  += pixel;
        sum2 += pixel * pixel;
        domi[x][y] = pixel;
      }

      ComputeMcVectors(domi,dom_tmp,size,s,vector);

      cx = (int) (vector[0] / TWOPI * n_p_class ); 
      cy = (int) (vector[1] / TWOPI * n_p_class ); 

      node = (struct c *) malloc(sizeof(struct c));
      node -> ptr_x = i;
      node -> ptr_y = j;
      node -> sum   = sum;
      node -> sum2  = sum2;
      node -> next  = class_polar[s][cx][cy];
      class_polar[s][cx][cy] = node;
    }
    printf(" Extracting features (Mass Center) domain (%dx%d)  %d \r",
                                                              size,size,count) ;
    fflush(stdout);

  }

  printf("\n") ;

  free(domi[0]);
  free(dom_tmp[0]);
  free(flip_domi[0]);
}


void Mc_SaupeIndexing(int size,int s)
{
  int i,j,k,h;
  int count = 0;
  int cx,cy;
  struct c *node; 
  int dim_vectors;
  struct c *pointer;
  double sum,sum2;
  double vector[16];
  double **dom_tmp,**domi,**flip_domi;
  register double pixel;
  register int x,y;

  matrix_allocate(domi,size,size,double)
  matrix_allocate(dom_tmp,size,size,double)
  matrix_allocate(flip_domi,size,size,double)

  for(i=0;i< image_height - 2 * size +1 ;i+= SHIFT) {
    for(j=0;j< image_width - 2 * size +1 ;j+= SHIFT,count ++) {
      k=0;
      sum  = 0.0;
      sum2 = 0.0;
      for(x=0;x< size;x++)
      for(y=0;y< size;y++) {
        pixel = contract[x+(i>>1)][y+(j>>1)];
        sum  += pixel;
        sum2 += pixel * pixel;
        domi[x][y] = pixel;
      }

      ComputeMcVectors(domi,dom_tmp,size,s,vector);

      cx = (int) (vector[0] / TWOPI * n_p_class ); 
      cy = (int) (vector[1] / TWOPI * n_p_class ); 

      node = (struct c *) malloc(sizeof(struct c));
      node -> ptr_x = i;
      node -> ptr_y = j;
      node -> sum   = sum;
      node -> sum2  = sum2;
      node -> next  = class_polar[s][cx][cy];
      class_polar[s][cx][cy] = node;

      item_per_class[s][cx][cy] ++;
    }
    printf(" Extracting features (Mass Center) domain (%dx%d)  %d \r",
                                                            size,size,count) ;
    fflush(stdout);

  }
  printf("\n") ;

  dim_vectors = feat_vect_dim[(int) rint(log((double) size)/log (2.0))];

  printf(" Extracting features (Saupe) domain (%dx%d) building Kd-trees",
                                                                size,size) ;
  fflush(stdout);

  for(h=0;h<n_p_class;h++)
  for(i=0;i<n_p_class;i++) {
    c_book[s][h][i] =(struct code_book*) malloc(item_per_class[s][h][i] *
                                                       sizeof(struct code_book));
    matrix_allocate(f_vect[s][h][i],dim_vectors,item_per_class[s][h][i],float)
    pointer = class_polar[s][h][i];
    count = 0;
    while(pointer != NULL) {
      for(x=0;x< size;x++)
      for(y=0;y< size;y++) 
        domi[x][y] = contract[x+(pointer->ptr_x>>1)][y+(pointer->ptr_y>>1)];

      ComputeSaupeVectors(domi,size,s,f_vect[s][h][i][count]);

      c_book[s][h][i][count].sum   = pointer -> sum;
      c_book[s][h][i][count].sum2  = pointer -> sum2;
      c_book[s][h][i][count].ptr_x = pointer -> ptr_x;
      c_book[s][h][i][count].ptr_y = pointer -> ptr_y;

      count ++;
      pointer = pointer -> next;
    }

    if(count > 0)                                             
      class_polar_saupe[s][h][i] = kdtree_build(f_vect[s][h][i],count,dim_vectors);
  }
  printf("\n") ;
  free(domi[0]);
  free(flip_domi[0]);
  free(dom_tmp[0]);
}



void SaupeIndexing(int size,int s)
{
  int i,j,k;
  int count = 0;
  int cbook_size;
  int dim_vectors;
  double sum,sum2;
  double **dom_tmp,**domi;
  register double pixel;
  register int x,y;

  cbook_size = (1+image_width / SHIFT) * (1+image_height / SHIFT);
  dim_vectors = feat_vect_dim[(int) rint(log((double) size)/log (2.0))];

  matrix_allocate(f_vectors[s],dim_vectors,cbook_size,float)
  codebook[s]=(struct  code_book*) malloc(cbook_size*sizeof(struct code_book));
  matrix_allocate(domi,size,size,double)
  matrix_allocate(dom_tmp,size,size,double)

  for(i=0;i< image_height - 2 * size +1 ;i+= SHIFT) {
    for(j=0;j< image_width - 2 * size +1 ;j+= SHIFT,count ++) {
      k=0;
      sum  = 0.0;
      sum2 = 0.0;
      for(x=0;x< size;x++)
      for(y=0;y< size;y++) {
        pixel = contract[x+(i>>1)][y+(j>>1)];
        sum  += pixel;
        sum2 += pixel * pixel;
        domi[x][y] = pixel;
      }

      ComputeSaupeVectors(domi,size,s,f_vectors[s][count]);

      codebook[s][count].sum  = sum;
      codebook[s][count].sum2 = sum2;
      codebook[s][count].ptr_x = i;
      codebook[s][count].ptr_y = j;
    }
    printf(" Extracting [%d] features (Saupe) domain (%dx%d)  %d \r",
		                               dim_vectors, size,size,count) ;
    fflush(stdout);

  }
  printf("\n Building Kd-tree... ") ;
  fflush(stdout);
  kd_tree[s]= kdtree_build(f_vectors[s],count-1,dim_vectors);
  printf("Done\n") ;
  fflush(stdout);

  free(domi[0]);
  free(dom_tmp[0]);

}


void HurtgenIndexing(int size,int s)
{
  int i,j,k,h;
  int count = 0;
  int m_class_1, m_class_2;
  double sum,sum2;
  double **domi;
  register double pixel;
  register int x,y;
  struct c *node;

  matrix_allocate(domi,size,size,double)

  for(i=0;i< image_height - 2 * size +1 ;i+= SHIFT) {
    for(j=0;j< image_width - 2 * size +1 ;j+= SHIFT) {
      count ++;
      k=0;
      sum  = 0.0;
      sum2 = 0.0;
      for(x=0;x< size;x++)
      for(y=0;y< size;y++) {
        pixel = contract[x+(i>>1)][y+(j>>1)];
        sum  += pixel;
        sum2 += pixel * pixel;
        domi[x][y] = pixel;
      }

      m_class_1 = hurtgen_class(size,domi);
      m_class_2 = variance_class(size,domi);

      node = (struct c *) malloc(sizeof(struct c));
      node -> ptr_x = i;
      node -> ptr_y = j;
      node -> sum   = sum;
      node -> sum2  = sum2;
      node -> next  = class_hurtgen[s][m_class_1][m_class_2];
      class_hurtgen[s][m_class_1][m_class_2] = node;
    }
    printf(" Classification (Hurtgen) domain (%dx%d)  %d \r",size,size,count) ;
    fflush(stdout);
  }

  for(i=0;i<16;i++)     /* Find a not empty class */
  for(j=0;j<24;j++)
     if(class_hurtgen[s][i][j] != NULL)
          goto out_loops;


  out_loops:

  for(k=0;k<16;k++)   /* Make sure no class is empty */
  for(h=0;h<24;h++)
     if(class_hurtgen[s][k][h] == NULL)
       class_hurtgen[s][k][h] = class_hurtgen[s][i][j];


  printf("\n");
  free(domi[0]);

}

void haar_2d ( int m, int n, double u[] )

/******************************************************************************/
/*
  Purpose:

    HAAR_2D computes the Haar transform of an array.

  Discussion:

    For the classical Haar transform, M and N should be a power of 2.
    However, this is not required here.

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    06 March 2014

  Author:

    John Burkardt

  Parameters:

    Input, int M, N, the dimensions of the array.

    Input/output, double U[M*N], the array to be transformed.
*/
{
  int i;
  int j;
  int k;
  double s;
  double *v;

  s = sqrt ( 2.0 );

  v = ( double * ) malloc ( m * n * sizeof ( double ) );

  for ( j = 0; j < n; j++ )
  {
    for ( i = 0; i < m; i++ )
    {
      v[i+j*m] = u[i+j*m];
    }
  }
/*
  Determine K, the largest power of 2 such that K <= M.
*/
  k = 1;
  while ( k * 2 <= m )
  {
    k = k * 2;
  }
/*
  Transform all columns.
*/
  while ( 1 < k )
  {
    k = k / 2;

    for ( j = 0; j < n; j++ )
    {
      for ( i = 0; i < k; i++ )
      {
        v[i  +j*m] = ( u[2*i+j*m] + u[2*i+1+j*m] ) / s;
        v[k+i+j*m] = ( u[2*i+j*m] - u[2*i+1+j*m] ) / s;
      }
    }
    for ( j = 0; j < n; j++ )
    {
      for ( i = 0; i < 2 * k; i++ )
      {
        u[i+j*m] = v[i+j*m];
      }
    }
  }
/*
  Determine K, the largest power of 2 such that K <= N.
*/
  k = 1;
  while ( k * 2 <= n )
  {
    k = k * 2;
  }
/*
  Transform all rows.
*/
  while ( 1 < k )
  { 
    k = k / 2;

    for ( j = 0; j < k; j++ )
    {
      for ( i = 0; i < m; i++ )
      {
        v[i+(  j)*m] = ( u[i+2*j*m] + u[i+(2*j+1)*m] ) / s;
        v[i+(k+j)*m] = ( u[i+2*j*m] - u[i+(2*j+1)*m] ) / s;
      }
    }

    for ( j = 0; j < 2 * k; j++ )
    {
      for ( i = 0; i < m; i++ )
      {
        u[i+j*m] = v[i+j*m];
      }
    }
  }
  free ( v );

  return;
}

void findMaxEnt(int size, int s)
{
  double pixel = 0.0, block_ent = 0.0, max_ent = 0.0;
  double **domi2;
  matrix_allocate(domi2,size,size,double);
  
    for(int i=0;i< image_height - 2 * size +1 ;i+= SHIFT) {
    for(int j=0;j< image_width - 2 * size +1 ;j+= SHIFT ) {
      for(int x=0;x< size;x++)
      for(int y=0;y< size;y++) {
        pixel = contract[x+(i>>1)][y+(j>>1)];
        domi2[x][y] = pixel;
      }
      block_ent = entropy_2(size,domi2,0,0);
      // printf("Block Entropy = %f\n",block_ent);
      if(max_ent < block_ent) max_ent = block_ent;

    }
  }
  
  max_ent_arr[s] = max_ent;
  // printf("Max std = %f\n",max_std);
}

void findMaxStd(int size, int s)
{
  double pixel = 0.0, block_var = 0.0, max_var = 0.0;
  double **domi2;
  matrix_allocate(domi2,size,size,double);
  
    for(int i=0;i< image_height - 2 * size +1 ;i+= SHIFT) {
    for(int j=0;j< image_width - 2 * size +1 ;j+= SHIFT ) {
        for(int x=0;x< size;x++)
        for(int y=0;y< size;y++) {
        pixel = contract[x+(i>>1)][y+(j>>1)];
        domi2[x][y] = pixel;
        block_var = variance_2(size,domi2,0,0);
      }
      if(max_var < block_var) max_var = block_var;

    }
  }
  // printf("Max std = %f\n",max_var);
  max_std_arr[s] = sqrt(max_var);
}

void BasicFIC_Indexing(int size,int s)
{
  int i,j,k,h;
  int count = 0;
  int iso, clas;
  double sum,sum2;
  double **domi,**flip_domi;
  register double pixel;
  register int x,y;
  struct c *node;

  matrix_allocate(domi,size,size,double)
  matrix_allocate(flip_domi,size,size,double)

  for(i=0;i< image_height - 2 * size +1 ;i+= SHIFT) {
    for(j=0;j< image_width - 2 * size +1 ;j+= SHIFT ) {
      count ++;
      k=0;
      sum  = 0.0;
      sum2 = 0.0;
      for(x=0;x< size;x++)
      for(y=0;y< size;y++) {
        pixel = contract[x+(i>>1)][y+(j>>1)];
        sum  += pixel;
        sum2 += pixel * pixel;
        domi[x][y] = pixel;
      }
      
                                   /* Compute the symmetry operation which */
      newclass(size,domi,&iso,&clas); /* brings the domain in the canonical   */
                                      /* orientation                          */
      flips(size,domi,flip_domi,iso); 


      node = (struct c *) malloc(sizeof(struct c));
      node -> ptr_x = i;
      node -> ptr_y = j;
      node -> sum   = sum;
      node -> sum2  = sum2;
      node -> iso  = iso;
      node -> next  = class_basicFIC[s][clas];
      class_basicFIC[s][clas] = node;
      
    }
    printf(" Classification (Basic FIC) domain (%dx%d)  %d \r",size,size,count) ;
    fflush(stdout);

  }

  printf("\n");
  free(domi[0]);
  free(flip_domi[0]);
}


void EntropyIndexing(int size,int s)
{
  int i,j,k,h;
  int count = 0;
  int iso, clas, entr_class;
  double sum,sum2,sum3,sum4,max_var = 0.0,block_var = 0.0;
  double **domi,**flip_domi;
  register double pixel;
  register int x,y;
  struct c *node;

  matrix_allocate(domi,size,size,double);
  
  matrix_allocate(flip_domi,size,size,double);

 
  for(i=0;i< image_height - 2 * size +1 ;i+= SHIFT) {
    for(j=0;j< image_width - 2 * size +1 ;j+= SHIFT ) {
      count ++;
      k=0;
      sum  = 0.0;
      sum2 = 0.0;
       for(x=0;x< size;x++)
        for(y=0;y< size;y++) {
        pixel = contract[x+(i>>1)][y+(j>>1)];
        sum  += pixel;
        sum2 += pixel * pixel;
        domi[x][y] = pixel;
      }
                                   /* Compute the symmetry operation which */
      newclass(size,domi,&iso,&clas); /* brings the domain in the canonical   */
                                      /* orientation                          */
      flips(size,domi,flip_domi,iso); 

      entr_class = ent_class(size,flip_domi);
      // printf("std class= %d\n",stdd_class);
      node = (struct c *) malloc(sizeof(struct c));
      node -> ptr_x = i;
      node -> ptr_y = j;
      node -> sum   = sum;
      node -> sum2  = sum2;
      node -> iso  = iso;
      node -> next  = class_entropy[s][clas][entr_class];
      class_entropy[s][clas][entr_class] = node;
      
    }
    printf(" Classification (Entropy Based) domain (%dx%d)  %d \r",size,size,count) ;
    fflush(stdout);

  }

  for(i=0;i<3;i++)    /* Find a not empty class */
  for(j=0;j<final_max_ent_q;j++)
    if(class_entropy[s][i][j] != NULL)
         goto out_loops;


  out_loops:

  for(k=0;k<3;k++)  /* Make sure no class is empty */
  for(h=0;h<final_max_ent_q;h++)
    if(class_entropy[s][k][h] == NULL)
      class_entropy[s][k][h] = class_entropy[s][i][j];


  printf("\n");
  // for(int i = 0; i < 24; i++){
  //   printf("Class %d count = %d\n",i,class_count[i]);
  // }

  free(domi[0]);
  free(flip_domi[0]);
}

void COVIndexing(int size,int s)
{
  int i,j,k,h;
  int count = 0;
  int iso, clas, stdd_class;
  double sum,sum2,sum3,sum4,max_var = 0.0,block_var = 0.0;
  double **domi,**flip_domi;
  register double pixel;
  register int x,y;
  struct c *node;

  matrix_allocate(domi,size,size,double);
  
  matrix_allocate(flip_domi,size,size,double);

 
  for(i=0;i< image_height - 2 * size +1 ;i+= SHIFT) {
    for(j=0;j< image_width - 2 * size +1 ;j+= SHIFT ) {
      count ++;
      k=0;
      sum  = 0.0;
      sum2 = 0.0;
       for(x=0;x< size;x++)
        for(y=0;y< size;y++) {
        pixel = contract[x+(i>>1)][y+(j>>1)];
        sum  += pixel;
        sum2 += pixel * pixel;
        domi[x][y] = pixel;
      }
                                      /* Compute the symmetry operation which */
      newclass(size,domi,&iso,&clas); /* brings the domain in the canonical   */
                                      /* orientation                          */
      flips(size,domi,flip_domi,iso); 

      stdd_class = std_class(size,flip_domi);
      if(stdd_class > final_max_std) stdd_class = final_max_std;
      // printf("std class= %d\n",stdd_class);
      node = (struct c *) malloc(sizeof(struct c));
      node -> ptr_x = i;
      node -> ptr_y = j;
      node -> sum   = sum;
      node -> sum2  = sum2;
      node -> iso  = iso;
      node -> var = variance_2(size,domi,0,0);
      node -> next  = class_std[s][clas][stdd_class];
      class_std[s][clas][stdd_class] = node;
      
    }
    printf(" Classification (COV) domain (%dx%d)  %d \r",size,size,count) ;
    fflush(stdout);

  }
  // printf("Max STD= %d\n",final_max_std);
  for(i=0;i<3;i++)    /* Find a not empty class */
  for(j=0;j<final_max_std;j++)
    if(class_std[s][i][j] != NULL)
         goto out_loops;


  out_loops:

  for(k=0;k<3;k++)  /* Make sure no class is empty */
  for(h=0;h<final_max_std;h++)
    if(class_std[s][k][h] == NULL)
      class_std[s][k][h] = class_std[s][i][j];


  printf("\n");
  // for(int i = 0; i < 24; i++){
  //   printf("Class %d count = %d\n",i,class_count[i]);
  // }

  free(domi[0]);
  free(flip_domi[0]);
}

void STDIndexing(int size,int s)
{
  int i,j,k,h;
  int count = 0;
  int iso, clas, stdd_class;
  double sum,sum2,sum3,sum4,max_var = 0.0,block_var = 0.0;
  double **domi,**flip_domi;
  register double pixel;
  register int x,y;
  struct c *node;

  matrix_allocate(domi,size,size,double);
  
  matrix_allocate(flip_domi,size,size,double);

 
  for(i=0;i< image_height - 2 * size +1 ;i+= SHIFT) {
    for(j=0;j< image_width - 2 * size +1 ;j+= SHIFT ) {
      count ++;
      k=0;
      sum  = 0.0;
      sum2 = 0.0;
       for(x=0;x< size;x++)
        for(y=0;y< size;y++) {
        pixel = contract[x+(i>>1)][y+(j>>1)];
        sum  += pixel;
        sum2 += pixel * pixel;
        domi[x][y] = pixel;
      }
                                      /* Compute the symmetry operation which */
      newclass(size,domi,&iso,&clas); /* brings the domain in the canonical   */
                                      /* orientation                          */
      flips(size,domi,flip_domi,iso); 

      stdd_class = std_class(size,flip_domi);
      // if(stdd_class > final_max_std) stdd_class = final_max_std;
      // printf("std class= %d\n",stdd_class);
      node = (struct c *) malloc(sizeof(struct c));
      node -> ptr_x = i;
      node -> ptr_y = j;
      node -> sum   = sum;
      node -> sum2  = sum2;
      node -> iso  = iso;
      node -> next  = class_std[s][clas][stdd_class];
      class_std[s][clas][stdd_class] = node;
      
    }
    printf(" Classification (Fisher) domain (%dx%d)  %d \r",size,size,count) ;
    fflush(stdout);

  }
  // printf("Max STD= %d\n",final_max_std);
  for(i=0;i<3;i++)    /* Find a not empty class */
  for(j=0;j<final_max_std;j++)
    if(class_std[s][i][j] != NULL)
         goto out_loops;


  out_loops:

  for(k=0;k<3;k++)  /* Make sure no class is empty */
  for(h=0;h<final_max_std;h++)
    if(class_std[s][k][h] == NULL)
      class_std[s][k][h] = class_std[s][i][j];


  printf("\n");
  // for(int i = 0; i < 24; i++){
  //   printf("Class %d count = %d\n",i,class_count[i]);
  // }

  free(domi[0]);
  free(flip_domi[0]);
}


void EnergyCoeff_FisherIndexing(int size,int s)
{
  int i,j,k,h;
  int count = 0;
  int iso, clas, var_class;
  double sum,sum2,t_sum, energy_coeff = 0.0, vari;
  double max_energy_coeff = 0.0, min_energy_coeff = 1000000.000000;
  double **domi,**flip_domi;
  register double pixel;
  register int x,y;
  struct c *node;


  
  matrix_allocate(domi,size,size,double);
  matrix_allocate(flip_domi,size,size,double);
  
  
  for(i=0;i< image_height - 2 * size +1 ;i+= SHIFT) {
    for(j=0;j< image_width - 2 * size +1 ;j+= SHIFT ) {
      energy_coeff = 0.0;
      count ++;
      k=0;
      t_sum = 0.0;
      sum  = 0.0;
      sum2 = 0.0;
      for(x=0;x< size;x++)
      for(y=0;y< size;y++) {
        pixel = contract[x+(i>>1)][y+(j>>1)];
        sum  += pixel;
        sum2 += pixel * pixel;
        domi[x][y] = pixel;
      }

                                   /* Compute the symmetry operation which */
      newclass(size,domi,&iso,&clas); /* brings the domain in the canonical   */
                                      /* orientation                          */
      flips(size,domi,flip_domi,iso); 
      // printf("here\n");
       var_class =  EnergyCoeff_class(size,flip_domi);
      // var_class = EnergyCoeff_class(size,flip_domi);
       // printf("var class=%d\n",var_class);
      
      node = (struct c *) malloc(sizeof(struct c));
      node -> ptr_x = i;
      node -> ptr_y = j;
      node -> sum   = sum;
      node -> sum2  = sum2;
      node -> iso  = iso;
      node -> var = vari;
      node -> next  = class_fisher[s][clas][var_class];
      class_fisher[s][clas][var_class] = node;
      class_count[var_class]++;
      
    }
    printf(" Classification (Fisher) domain (%dx%d)  %d \r",size,size,count) ;
    fflush(stdout);
  }
  // printf("Max Energy Coeff = %f\n",max_energy_coeff);

  for(i=0;i<3;i++)    /* Find a not empty class */
  for(j=0;j<24;j++)
    if(class_fisher[s][i][j] != NULL)
         goto out_loops;


  out_loops:

  for(k=0;k<3;k++)  /* Make sure no class is empty */
  for(h=0;h<24;h++)
    if(class_fisher[s][k][h] == NULL)
      class_fisher[s][k][h] = class_fisher[s][i][j];


  printf("\n");
  // for(int i = 0; i < 24; i++){
  //   printf("Class %d count = %d\n",i,class_count[i]);
  // }

  free(domi[0]);
  free(flip_domi[0]);
}

void adaptiveFisherIndexing_2(int x_size,int y_size)
{
 // y_size = 6; x_size = 4;
  
  int i,j,k,h;
  int count = 0;
  int iso, clas, var_class;
  double sum,sum2;
  double **domi,**flip_domi;
  register double pixel;
  register int x,y;
  struct c *node;
  int x_s = x_size;
  int y_s = y_size;
  //todo : check images dimensions in mars_enc
 // DOMAIN_X_BITS = (int)log2(x_size);
  //DOMAIN_Y_BITS = (int)log2(y_size);
  // printf("allocating memory\n");
  matrix_allocate(L1_var_limits,MAX_ADAP_D_BITS + 1,2,int);
  L1_class_width = new int[MAX_ADAP_D_BITS + 1];
  matrix_allocate(domi,x_size,y_size,double)
  // matrix_allocate(flip_domi,x_size,y_size,double)
  
  // printf("memory allocated\n");
  for(i=0;i< image_height - 2 * y_size +1 ;i+= SHIFT) {
    for(j=0;j< image_width - 2 * x_size +1 ;j+= SHIFT ) {
      count ++;
      k=0;
      sum  = 0.0;
      sum2 = 0.0;
      for(y=0;y< y_size;y++)
      for(x=0;x< x_size;x++) {
        pixel = contract[y+(i>>1)][x+(j>>1)];
        sum  += pixel;
        sum2 += pixel * pixel;
        domi[y][x] = pixel;
      }

          
      adaptiveNewclass_2(x_size,y_size,domi,&iso,&clas); 
      // printf("x size = %d\t y size = %d\r",x_size,y_size);
      // printf("Domain Class = %d\r",clas);
      // exit(0);

     // adaptiveFlips(x_size,y_size,domi,flip_domi,iso); 

      //var_class = adaptiveVariance_class(x_size,y_size,flip_domi);
      iso = 0 ;//not considering rotations yet, to be implemented in the future
      node = (struct c *) malloc(sizeof(struct c));
      node -> ptr_x = i;
      node -> ptr_y = j;
      node -> sum   = sum;
      node -> sum2  = sum2;
      node -> iso  = iso;
      //node -> var = variance_3(x_size,y_size,domi,0,0);
      node -> next  = adaptive_fisher_class[y_s][x_s][clas];
      adaptive_fisher_class[y_s][x_s][clas] = node;
      class_count[clas]++;
    }
    printf(" Classification (Fisher) domain (%dx%d)  %d \r",y_size,x_size,count) ;
    // printf("\n");
  
    fflush(stdout);
  }
  for(i=0;i<24;i++)    /* Find a not empty class */
    if(adaptive_fisher_class[y_s][x_s][i] != NULL)
         goto out_loops;


  out_loops:

  for(k=0;k<24;k++)  /* Make sure no class is empty */
    if(adaptive_fisher_class[y_s][x_s][k] == NULL)
      adaptive_fisher_class[y_s][x_s][k] = adaptive_fisher_class[y_s][x_s][i];


  printf("\n");
  // for(int i = 0; i < 24; i++){
  //   printf("Class %d count = %d\n",i,class_count[i]);
  // }

  free(domi[0]);
  // free(flip_domi[0]);
}

void adaptiveFisherIndexing(int x_size,int y_size,int x_s, int y_s)
{

  int i,j,k,h;
  int count = 0;
  int iso, clas, var_class;
  double sum,sum2,sum3,sum4;
  double **domi,**flip_domi;
  register double pixel;
  register int x,y;
  struct c *node;
  // FisherIndexing_2(x_size,(int)log2(x_size));
  //todo : check images dimensions in mars_enc
 // DOMAIN_X_BITS = (int)log2(x_size);
  //DOMAIN_Y_BITS = (int)log2(y_size);
  // printf("allocating memory\n");
  matrix_allocate(domi,x_size,y_size,double)
  matrix_allocate(flip_domi,x_size,y_size,double)
  
  // printf("memory allocated\n");
  for(i=0;i< image_height - 2 * y_size +1 ;i+= SHIFT) {
    for(j=0;j< image_width - 2 * x_size +1 ;j+= SHIFT ) {
      count ++;
      k=0;
      sum  = 0.0;
      sum2 = 0.0;
      
      for(y=0;y< y_size;y++)
      for(x=0;x< x_size;x++) {
        pixel = contract[y+(i>>1)][x+(j>>1)];
        sum  += pixel;
        sum2 += pixel * pixel;
        domi[y][x] = pixel;
      }
      
                                   /* Compute the symmetry operation which */
      adaptiveNewclass(x_size,y_size,domi,&iso,&clas); /* brings the domain in the canonical   */
      
     // adaptiveFlips(x_size,y_size,domi,flip_domi,iso); 
       
      //var_class = adaptiveVariance_class(x_size,y_size,flip_domi);
      iso = 0 ;//not considering rotations yet, to be implemented in the future
      node = (struct c *) malloc(sizeof(struct c));
      node -> ptr_x = i;
      node -> ptr_y = j;
      node -> sum   = sum;
      node -> sum2  = sum2;
      node -> sum3  = sum3;
      node -> sum4  = sum4;
      node -> iso  = iso;
      //node -> var = variance_3(x_size,y_size,domi,0,0);
      node -> next  = adaptive_fisher_class[y_s][x_s][clas];
      adaptive_fisher_class[y_s][x_s][clas] = node;
      
    }
    printf(" Classification (Fisher) domain (%dx%d)  %d \r",y_size,x_size,count) ;
    fflush(stdout);

  }

  for(i=0;i<24;i++)    /* Find a not empty class */
    if(adaptive_fisher_class[y_s][x_s][i] != NULL)
         goto out_loops;


  out_loops:

  for(k=0;k<24;k++)  /* Make sure no class is empty */
    if(adaptive_fisher_class[y_s][x_s][k] == NULL)
      adaptive_fisher_class[y_s][x_s][k] = adaptive_fisher_class[y_s][x_s][i];


  printf("\n");
  // for(int i = 0; i < 24; i++){
  //   printf("Class %d count = %d\n",i,class_count[i]);
  // }

  free(domi[0]);
  free(flip_domi[0]);
}

void FisherIndexing_2(int size,int s)
{
  int i,j,k,h;
  int count = 0;
  int iso, clas, var_class;
  double sum,sum2,sum3,sum4;
  double **domi,**flip_domi;
  register double pixel;
  register int x,y;
  struct c *node;
  partition_type = 1;
  matrix_allocate(domi,size,size,double)
  matrix_allocate(flip_domi,size,size,double)
  

  for(i=0;i< image_height - 2 * size +1 ;i+= SHIFT) {
    for(j=0;j< image_width - 2 * size +1 ;j+= SHIFT ) {
      count ++;
      k=0;
      sum  = 0.0;
      sum2 = 0.0;
      sum3 = 0.0;
      sum4 = 0.0;
      for(x=0;x< size;x++)
      for(y=0;y< size;y++) {
        pixel = contract[x+(i>>1)][y+(j>>1)];
        sum  += pixel;
        sum2 += pixel * pixel;
        sum3 += pixel * pixel * pixel;
        sum4 += pixel * pixel * pixel * pixel;
        domi[x][y] = pixel;
      }
      
                                   /* Compute the symmetry operation which */
      newclass(size,domi,&iso,&clas); /* brings the domain in the canonical   */
                                      /* orientation                          */
      flips(size,domi,flip_domi,iso); 

      var_class = variance_class(size,flip_domi);

      node = (struct c *) malloc(sizeof(struct c));
      node -> ptr_x = i;
      node -> ptr_y = j;
      node -> sum   = sum;
      node -> sum2  = sum2;
      node -> sum3  = sum3;
      node -> sum4  = sum4;
      node -> iso  = iso;
      node -> var = variance_2(size,domi,0,0);
      node -> next  = class_fisher[s][clas][var_class];
      class_fisher[s][clas][var_class] = node;
      class_count[var_class]++;
      
    }
    // printf(" Classification (Fisher) domain (%dx%d)  %d \r",size,size,count) ;
    fflush(stdout);

  }

  for(i=0;i<3;i++)    /* Find a not empty class */
  for(j=0;j<24;j++)
    if(class_fisher[s][i][j] != NULL)
         goto out_loops;


  out_loops:

  for(k=0;k<3;k++)  /* Make sure no class is empty */
  for(h=0;h<24;h++)
    if(class_fisher[s][k][h] == NULL){
      // printf("Current class is null\n");
      class_fisher[s][k][h] = class_fisher[s][i][j];
    }

  // printf("\n");
  // for(int i = 0; i < 24; i++){
  //   printf("Class %d count = %d\n",i,class_count[i]);
  // }

  free(domi[0]);
  free(flip_domi[0]);
}

void push(struct c** head_ref, int new_data)
{
    /* allocate node */
    struct c* new_node = new struct c;
 
    /* put in the data  */
    new_node->var = new_data;
 
    /* link the old list off the new node */
    new_node->next = (*head_ref);
 
    /* move the head to point to the new node */
    (*head_ref)    = new_node;
}
 
/* A utility function to print linked list */
void printList(struct c *node)
{
    while (node != NULL)
    {
        printf("%d  ", node->var);
        node = node->next;
    }
    printf("\n");
}
 
// Returns the last node of the list
struct c *getTail(struct c *cur)
{
    while (cur != NULL && cur->next != NULL)
        cur = cur->next;
    return cur;
}
 
// Partitions the list taking the last element as the pivot
struct c *partition(struct c *head, struct c *end,
                       struct c **newHead, struct c **newEnd)
{
    struct c *pivot = end;
    struct c *prev = NULL, *cur = head, *tail = pivot;
 
    // During partition, both the head and end of the list might change
    // which is updated in the newHead and newEnd variables
    while (cur != pivot)
    {
        if (cur->var < pivot->var)
        {
            // First node that has a value less than the pivot - becomes
            // the new head
            if ((*newHead) == NULL)
                (*newHead) = cur;
 
            prev = cur;  
            cur = cur->next;
        }
        else // If cur node is greater than pivot
        {
            // Move cur node to next of tail, and change tail
            if (prev)
                prev->next = cur->next;
            struct c *tmp = cur->next;
            cur->next = NULL;
            tail->next = cur;
            tail = cur;
            cur = tmp;
        }
    }
 
    // If the pivot data is the smallest element in the current list,
    // pivot becomes the head
    if ((*newHead) == NULL)
        (*newHead) = pivot;
 
    // Update newEnd to the current last node
    (*newEnd) = tail;
 
    // Return the pivot node
    return pivot;
}
 
 
//here the sorting happens exclusive of the end node
struct c *quickSortRecur(struct c *head, struct c *end)
{
    // base condition
    if (!head || head == end)
        return head;
 
    struct c *newHead = NULL, *newEnd = NULL;
 
    // Partition the list, newHead and newEnd will be updated
    // by the partition function
    struct c *pivot = partition(head, end, &newHead, &newEnd);
 
    // If pivot is the smallest element - no need to recur for
    // the left part.
    if (newHead != pivot)
    {
        // Set the node before the pivot node as NULL
        struct c *tmp = newHead;
        while (tmp->next != pivot)
            tmp = tmp->next;
        tmp->next = NULL;
 
        // Recur for the list before pivot
        newHead = quickSortRecur(newHead, tmp);
 
        // Change next of last node of the left half to pivot
        tmp = getTail(newHead);
        tmp->next =  pivot;
    }
 
    // Recur for the list after the pivot element
    pivot->next = quickSortRecur(pivot->next, newEnd);
 
    return newHead;
}
 
// The main function for quick sort. This is a wrapper over recursive
// function quickSortRecur()
void quickSort(struct c **headRef)
{
    *headRef = quickSortRecur(*headRef, getTail(*headRef));
    return;
}

void FisherIndexing_domainSort(int size,int s)
{
  int i,j,k,h;
  int count = 0;
  int iso, clas, var_class;
  double sum,sum2;
  double **domi,**flip_domi;
  register double pixel;
  register int x,y;
  struct c *node;

  matrix_allocate(domi,size,size,double)
  matrix_allocate(flip_domi,size,size,double)
  

  for(i=0;i< image_height - 2 * size +1 ;i+= SHIFT) {
    for(j=0;j< image_width - 2 * size +1 ;j+= SHIFT ) {
      count ++;
      k=0;
      sum  = 0.0;
      sum2 = 0.0;
   
      for(x=0;x< size;x++)
      for(y=0;y< size;y++) {
        pixel = contract[x+(i>>1)][y+(j>>1)];
        sum  += pixel;
        sum2 += pixel * pixel;
        domi[x][y] = pixel;
      }

      
                                   /* Compute the symmetry operation which */
      newclass(size,domi,&iso,&clas); /* brings the domain in the canonical   */
                                      /* orientation                          */
      flips(size,domi,flip_domi,iso); 

      var_class = variance_class(size,flip_domi);

      node = (struct c *) malloc(sizeof(struct c));
      node -> ptr_x = i;
      node -> ptr_y = j;
      node -> sum   = sum;
      node -> sum2  = sum2;
      node -> iso  = iso;
      node -> var = variance_2(size,flip_domi,0,0);
      node -> next  = class_fisher[s][clas][var_class];
      class_fisher[s][clas][var_class] = node;
      class_count[var_class]++;
      
    }
    printf(" Classification (Fisher) domain (%dx%d)  %d \r",size,size,count) ;
    fflush(stdout);

  }

  for(i=0;i<3;i++)    /* Find a not empty class */
  for(j=0;j<24;j++)
    if(class_fisher[s][i][j] != NULL)
         goto out_loops;


  out_loops:

  for(k=0;k<3;k++)  /* Make sure no class is empty */
  for(h=0;h<24;h++)
    if(class_fisher[s][k][h] == NULL){
      // printf("Current class is null\n");
      class_fisher[s][k][h] = class_fisher[s][i][j];
    }

  printf("\n");
  // for(int i = 0; i < 24; i++){
  //   printf("Class %d count = %d\n",i,class_count[i]);
  // }
  // printf(" Sorting Domain pool... \t");  
  for(int i=0; i < 3; i++){
    for(int j=0; j< 24 ;j ++){

      quickSort(&class_fisher[s][i][j]);
    }
  }
  // printf(" Done \n");
  free(domi[0]);
  free(flip_domi[0]);
}

void FisherIndexing(int size,int s)
{
  int i,j,k,h;
  int count = 0;
  int iso, clas, var_class;
  double sum,sum2;
  double **domi,**flip_domi;
  register double pixel;
  register int x,y;
  struct c *node;

  matrix_allocate(domi,size,size,double)
  matrix_allocate(flip_domi,size,size,double)

  for(i=0;i< image_height - 2 * size +1 ;i+= SHIFT) {
    for(j=0;j< image_width - 2 * size +1 ;j+= SHIFT ) {
      count ++;
      k=0;
      sum  = 0.0;
      sum2 = 0.0;
   
      for(x=0;x< size;x++)
      for(y=0;y< size;y++) {
        pixel = contract[x+(i>>1)][y+(j>>1)];
        sum  += pixel;
        sum2 += pixel * pixel;
        domi[x][y] = pixel;
      }

      
                                   /* Compute the symmetry operation which */
      newclass(size,domi,&iso,&clas); /* brings the domain in the canonical   */
                                      /* orientation                          */
      flips(size,domi,flip_domi,iso); 

      var_class = variance_class(size,flip_domi);

      node = (struct c *) malloc(sizeof(struct c));
      node -> ptr_x = i;
      node -> ptr_y = j;
      node -> sum   = sum;
      node -> sum2  = sum2;
      node -> iso  = iso;
      node -> var = variance_2(size,flip_domi,0,0);
      node -> next  = class_fisher[s][clas][var_class];
      class_fisher[s][clas][var_class] = node;
      
    }
    printf(" Classification (Fisher) domain (%dx%d)  %d \r",size,size,count) ;
    fflush(stdout);

  }

  for(i=0;i<3;i++)    /* Find a not empty class */
  for(j=0;j<24;j++)
    if(class_fisher[s][i][j] != NULL)
         goto out_loops;


  out_loops:

  for(k=0;k<3;k++)  /* Make sure no class is empty */
  for(h=0;h<24;h++)
    if(class_fisher[s][k][h] == NULL){
      class_fisher[s][k][h] = class_fisher[s][i][j];
    }

  printf("\n");
  
  free(domi[0]);
  free(flip_domi[0]);
}

void ComputeFeatVectDimSaupe()
{
  int i;
  int size;

  for(i=1;i<=(int) rint(log((double) max_size)/ log(2.0)); i++) {
    size = (int) rint(pow(2.0,(double)i));
    if(shrunk_factor_saupe) {
      n_features = (size*size) / (shrunk_factor_saupe * shrunk_factor_saupe);
      if(n_features < 4)
        n_features = 4;
    }
    if((size*size)  > n_features ) { 
       average_factor[i] = size / sqrt(n_features);
       feat_vect_dim[i] = n_features;
    }
    else {
       average_factor[i] = 1;
       feat_vect_dim[i] = size*size;
    }
  }
}


void ComputeAverageFactorMc()   
{                             
  int i;
  int size;

  for(i=1;i<=(int) rint(log((double) max_size)/ log(2.0)); i++) {
      size = (int) rint(pow(2.0,(double)i));
      if(shrunk_factor_mc) {
          average_factor[i] = shrunk_factor_mc;
          if((size*size)/(shrunk_factor_mc*shrunk_factor_mc) < 16) 
          average_factor[i] = 1;
       }
       else {
          average_factor[i] = 1;
       }
   }
}



