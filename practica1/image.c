#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"


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
    	//printf("%f \n", *(sizes+i));
    	
    } 
}

void boxBlurT_4 (int* scl,int* tcl,int w,int h,int r) {
    double iarr = 1 / (r+r+1);
    for(int i = 0; i<w; i++) {
        int ti = i;
        int li = ti;
        int ri = ti+r*w;
        int fv = *(scl+ti); 
        int lv = *(scl+(ti+w*(h-1)));
        int val = (r+1)*fv;
        for(int j=0; j<r; j++) val += *(scl+(ti+j*w));
        for(int j=0  ; j<=r ; j++) {
			val += *(scl+ri) - fv ;
			*(tcl+ti) = round(val*iarr);
			ri+=w; 
			ti+=w; 
			//printf("%f %i\n",*(tcl+ti),*(tcl+ti));
         }
        for(int j=r+1; j<h-r; j++) {
         val += *(scl+ri) - *(scl+li);
         *(tcl+ti) = round(val*iarr);
	     li+=w; 
	     ri+=w; 
	     ti+=w; 
	      //printf("%f %i\n",*(tcl+ti),*(tcl+ti));
       }
        for(int j=h-r; j<h  ; j++) { 
        	val += lv- *(scl+li);  
        	*(tcl+ti) = round(val*iarr);  
        	li+=w;
        	ti+=w; 
        	 // qprintf("%f %i\n",*(tcl+ti),*(tcl+ti));
        }

    }
    printf("dentro de la funcion %i \n",*(tcl+2));
}

void boxBlurH_4 (int* scl,int* tcl,int w,int h,int r) {
    double iarr = 1 / (r+r+1);
    for(int i=0; i<h; i++) {
        int ti = i*w, li = ti, ri = ti+r;
        int fv = *(scl+ti), lv = *(scl+(ti+w-1));
        int val = (r+1)*fv;
        for(int j=0; j<r; j++) val += *(scl+(ti+j));

        for(int j=0  ; j<=r ; j++) {
         val += *(scl+(ri++)) - fv;   
         *(tcl+(ti++)) = round(val*iarr); 
     	}

        for(int j=r+1; j<w-r; j++) { 
        	val += *(scl+(ri++)) - *(scl+(li++));   
        	*(tcl+(ti++)) = round(val*iarr); 
        }

        for(int j=w-r; j<w  ; j++) {
	         val += lv - *(scl+(li++));  
	         *(tcl+(ti++)) = round(val*iarr); 
     	}
    }
}

void boxBlur_4 (int* scl,int* tcl,int w,int h,int r) {
// revisar este for 
    for(int i=0; i<(w*h); i++){
    	int aux =*(scl+i);
    	*(tcl+i) = aux;
    	//printf("esta es %i \n", aux);


    } 
    boxBlurH_4(tcl, scl, w, h, r);
    boxBlurT_4(scl, tcl, w, h, r);
}

void gaussBlur_4 (int* scl,int* tcl,int w,int h,int r) {
	double  *bxs = (double*)malloc(sizeof(double)*3);
	boxesForGauss(r,3,bxs);
		for (int i = 0; i < 3; ++i)
	{
		//printf("lala %i \n",(int)(*(bxs+i)));
	}
    boxBlur_4 (scl, tcl, w, h, (int)((*(bxs)-1)/2));
    //boxBlur_4 (tcl, scl, w, h, (int)((*(bxs+1)-1)/2));
    //boxBlur_4 (scl, tcl, w, h, (int)((*(bxs+2)-1)/2));
}








int main(void) {

	int width, height, channels;
	unsigned char *img = stbi_load("captura.jpg", &width, &height, &channels, 0); //// cero para cargar todos los canales
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



	printf("Original: \n");




	 int  *r_target = (int*)malloc(sizeof(int)*(img_size/3));
	 int  *g_target = (int*)malloc(sizeof(int)*(img_size/3));
	 int  *b_target = (int*)malloc(sizeof(int)*(img_size/3));
	 gaussBlur_4(r,r_target,width,height,3);
	 printf("R  %i \n",*(r_target+2));
	 gaussBlur_4(g,g_target,width,height,3);
	 printf("G  %i \n",*(g_target+2));
	 gaussBlur_4(b,b_target,width,height,3);
	 printf("B %i \n",*(b_target+2));


	 
	unsigned char new_image2[img_size];
	int y =0;
	for(int i = 0; i <img_size/3; i++ ){
		new_image2[y] = (char) (r[i]);
		new_image2[y+1] = (char) (g[i]);
	 	new_image2[y+2] = (char) (b[i]);
	 	y+=3;
	 	//printf("%i \n",(r[i]));
	}
	 	printf("\n \n \n \n \n \n \n \n \n");	




	unsigned char new_image[img_size];
	int j =0;
	printf("Transformada : \n");
	for(int i = 0; i <img_size/3; i++ ){
		new_image[j] =  *(r_target+i);
		new_image[j+1] =  *(g_target+i);
	 	new_image[j+2] =  *(b_target+i);
	 	j+=3;
	 	//printf("%i \n",*(r_target+i));
	}
	




	stbi_write_jpg("EsUnManzano.jpg", width, height, channels, new_image, 100);

	stbi_write_jpg("COPIASEBASTYANESUNHPTA.jpg", width, height, channels, new_image2, 100);

   
 }