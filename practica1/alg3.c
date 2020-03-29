#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"

int max(int num1, int num2)
{
    return (num1 > num2 ) ? num1 : num2;
}


int min(int num1, int num2) 
{
    return (num1 > num2 ) ? num2 : num1;
}


void boxesForGauss(int sigma, int n, double* sizes)  // standard deviation, number of boxes
{
	
    double wIdeal = sqrt((12*sigma*sigma/n)+1);  // Ideal averaging filter width 
    double wl = floor(wIdeal);  
    if(fmod(wl,2.0) == 0.0){
    	wl--;	
    } 
    double wu = wl+2;
				
    double mIdeal = (12*sigma*sigma - n*wl*wl - 4*n*wl - 3*n)/(-4*wl - 4);
    double m = round(mIdeal);
    // var sigmaActual = Math.sqrt( (m*wl*wl + (n-m)*wu*wu - n)/12 );
				
   
    for(int i=0; i<n; i++){
    	*(sizes+i) = (i<m?wl:wu);
    	
    	
    } 
}

void boxBlurT_3 (int* scl,int* tcl,int w,int h,int r) {
    for(int i=0; i<h; i++)
        for(int j=0; j<w; j++) {
            int val = 0;
            for(int iy=i-r; iy<i+r+1; iy++) {
                int y = min(h-1, max(0, iy));
                val += *(scl+(y*w+j));
            }
            *(tcl+(i*w+j)) = val/(r+r+1);
        }
}

void boxBlurH_3 (int* scl,int* tcl,int w,int h,int r) {
    for(int i=0; i<h; i++)
        for(int j=0; j<w; j++)  {
            int val = 0;
            for(int ix=j-r; ix<j+r+1; ix++) {
                int x = min(w-1, max(0, ix));
                val += *(scl+(i*w+x));
            }
            *(tcl+(i*w+j)) = val/(r+r+1);
        }
} 

void boxBlur_3 (int* scl,int* tcl,int w,int h,int r) {
    for(int i=0; i<(w*h); i++) {
		int aux =*(scl+i);
		*(tcl+i) = aux;
    }
    boxBlurH_3(tcl, scl, w, h, r);
    boxBlurT_3(scl, tcl, w, h, r);
}


void  gaussBlur_3 (int* scl,int* tcl,int w,int h,int r) {
	double  *bxs = (double*)malloc(sizeof(double)*3);
	boxesForGauss(r,3,bxs);

    boxBlur_3 (scl, tcl, w, h, (int)((*(bxs)-1)/2));
    boxBlur_3 (tcl, scl, w, h, (int)((*(bxs+1)-1)/2));
    boxBlur_3 (scl, tcl, w, h, (int)((*(bxs+2)-1)/2));
}




int main(void) {

	int width, height, channels;
	unsigned char *img = stbi_load("cballs.jpg", &width, &height, &channels, 0); //// cero para cargar todos los canales
	if(img == NULL) {
	 printf("Error in loading the image\n");
	 exit(1);
	}
	printf("Loaded image with a width of %dpx, a height of %dpx and %d channels\n", width, height, channels);

	size_t img_size  = width * height * channels;
	
	int r[img_size/3];
	int g[img_size/3];
	int b[img_size/3];
	int i = 0;
	for(unsigned char *p = img; p != img + img_size; p += channels,i++) {
		r[i] = (uint8_t) *p ;
		g[i] = (uint8_t) *(p + 1);
		b[i] = (uint8_t) *(p + 2);

	}



	 int  *r_target = (int*)malloc(sizeof(int)*(img_size/3));
	 int  *g_target = (int*)malloc(sizeof(int)*(img_size/3));
	 int  *b_target = (int*)malloc(sizeof(int)*(img_size/3));
	 int kernel = 5;
	 gaussBlur_3(r,r_target,width,height,kernel);
	 gaussBlur_3(g,g_target,width,height,kernel);
	 gaussBlur_3(b,b_target,width,height,kernel);


	unsigned char new_image[img_size];
	int j =0;
	for(int i = 0; i <img_size/3; i++ ){
		new_image[j] =  *(r_target+i);
		new_image[j+1] =  *(g_target+i);
	 	new_image[j+2] =  *(b_target+i);
	 	j+=3;

	}
	


	stbi_write_jpg("NoSeasAlemania.jpg", width, height, channels, new_image, 100);


   
 }