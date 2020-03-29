#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <pthread.h>


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"


struct parameters
{
	int* scl;
	int* tcl;
	int w;
	int h;
	int r;
	int init_i;
	int fin_i;

};

void parallelProcessH(void* data){
	struct parameters* dataAux;
	dataAux = (struct parameters*) data; 
	printf(" scl en parallel proccessH %i \n", dataAux->scl);


	boxBlurH_3(((struct parameters*) data)->tcl,((struct parameters*) data)->scl,dataAux->w,dataAux->h,dataAux->r,dataAux->init_i,dataAux->fin_i);

}

void parallelProcessT(void* data){
	struct parameters* dataAux;
	dataAux = (struct parameters*) data; 
	printf(" scl en parallel proccessT %i \n", dataAux->scl);


    boxBlurT_3(((struct parameters*) data)->scl,((struct parameters*) data)->tcl,dataAux->w,dataAux->h,dataAux->r,dataAux->init_i,dataAux->fin_i);

}


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

void boxBlurT_3 (int* scl,int* tcl,int w,int h,int r,int init_i,int fin_i) {
	printf("direccion en T %i \n",tcl);
	// Reemplazar i, j y h,w por los valores de inicio y fin del intervalo
    for(int i=init_i; i<fin_i; i++)
        for(int j=0; j<w; j++) {
            int val = 0;
            for(int iy=i-r; iy<i+r+1; iy++) {
                int y = min(h-1, max(0, iy));
                val += *(scl+(y*w+j));
            }
            
            *(tcl+(i*w+j)) = val/(r+r+1);
           
        }
}

void boxBlurH_3 (int* scl,int* tcl,int w,int h,int r,int init_i,int fin_i) {
	printf("direccion en H %i \n",scl);
	//printf("Entro a boxBlurH_3 \n");
	// Reemplazar i, j y h,w por los valores de inicio y fin del intervalo
    for(int i=init_i; i<fin_i; i++)
        for(int j=0; j<w; j++)  {
            int val = 0;
            for(int ix=j-r; ix<j+r+1; ix++) {
                int x = min(w-1, max(0, ix));
                val += *(scl+(i*w+x));
            }
           
            *(tcl+(i*w+j)) = val/(r+r+1);
            
        }
} 

void boxBlur_3 (int* scl,int* tcl,int w,int h,int r,int numThreads) {
	int interval_h = floor((h)/ numThreads);
	printf("tcl Box Blur %i \n",tcl );
	//printf("interval h %i \n",interval_h );

	int threadId[numThreads], i, *retval;
	pthread_t thread[numThreads];

	int init_i;

	int fin_i;



	struct parameters data;
	data.scl = scl;
	data.tcl = tcl;
	data.w = w;
	data.h = h;
	data.r = r;



	for(i = 0; i < numThreads; i++){
        threadId[i] = i;
        if( i == numThreads-1 ){
        	fin_i = h;
        	init_i = i*interval_h;
        }
        else{     	
        	init_i = i*interval_h;
        	fin_i = (i+1)*interval_h;
        }

    	data.init_i = init_i;
		data.fin_i = fin_i;
		//printf("init i %i \n", init_i);


        pthread_create(&thread[i], NULL, (void *)parallelProcessH, &data);
    
    }

    for(i = 0; i < numThreads; i++){
        pthread_join(thread[i], (void **)&retval);
        printf("Termino hilo %i \n", i);
    }

    	for(i = 0; i < numThreads; i++){
        threadId[i] = i;
        if( i == numThreads-1 ){
        	fin_i = h;
        	init_i = i*interval_h;
        }
        else{     	
        	init_i = i*interval_h;
        	fin_i = (i+1)*interval_h;
        }

    	data.init_i = init_i;
		data.fin_i = fin_i;
		


        pthread_create(&thread[i], NULL, (void *)parallelProcessT, &data);
    
    }

    for(i = 0; i < numThreads; i++){
        pthread_join(thread[i], (void **)&retval);
            int j =0;


        unsigned char new_image[w*h];
		for(int b = 0; b <w*h; b++ ){
			new_image[j] =  *(tcl+b);

		 	j++;

		}

		
		char name[] = "imagen .jpg";
		name[6] = (char)i;
		stbi_write_jpg(name, w, h, 1, new_image, 100);
        printf("Termino hilo %i \n", i);
    }





    


}


void  gaussBlur_3 (int* scl,int* tcl,int w,int h,int r, int numThreads) {
	printf("tcl gauss Blur %i \n",tcl );

	
	double  *bxs = (double*)malloc(sizeof(double)*3);
	boxesForGauss(r,3,bxs);

	for(int i=0; i<(w*h); i++) {
		int aux =*(scl+i);
		*(tcl+i) = aux;
    }
    boxBlur_3 (scl, tcl, w, h, (int)((*(bxs)-1)/2),numThreads);

    /*
   
    for(int i=0; i<(w*h); i++) {
		int aux =*(scl+i);
		*(tcl+i) = aux;
    }
   boxBlur_3 (tcl, scl, w, h, (int)((*(bxs+1)-1)/2),numThreads);
     for(int i=0; i<(w*h); i++) {
		int aux =*(scl+i);
		*(tcl+i) = aux;
    }
    boxBlur_3 (scl, tcl, w, h, (int)((*(bxs+2)-1)/2),numThreads);
*/
}




int main(void) {

	int width, height, channels;
	unsigned char *img = stbi_load("mini.jpg", &width, &height, &channels, 0); //// cero para cargar todos los canales
	if(img == NULL) {
	 printf("Error in loading the image\n");
	 exit(1);
	}
	printf("Loaded image with a width of %dpx, a height of %dpx and %d channels\n", width, height, channels);

	int img_size  = width * height * channels;
	printf("%i \n",img_size);
	

	int  *r = (int*)malloc(sizeof(int)*(img_size/3));

	int  *g = (int*)malloc(sizeof(int)*(img_size/3));
	int  *b = (int*)malloc(sizeof(int)*(img_size/3));
	int i = 0;
	for(unsigned char *p = img; p != img + img_size; p += channels,i++) {
		*(r+i) = (uint8_t) *p ;
		*(g+i) = (uint8_t) *(p + 1);
		*(b+i) = (uint8_t) *(p + 2);

	}



	 int  *r_target = (int*)malloc(sizeof(int)*(img_size/3));
	 int  *g_target = (int*)malloc(sizeof(int)*(img_size/3));
	 int  *b_target = (int*)malloc(sizeof(int)*(img_size/3));
	 int kernel = 15;
	 int numThreads = 4;
	 int j = 0;
		printf("Direccion r %i \n",r );
	 //char[] = *r
	 gaussBlur_3(r,r_target,width,height,kernel,numThreads);
	 gaussBlur_3(g,g_target,width,height,kernel,numThreads);
	 gaussBlur_3(b,b_target,width,height,kernel,numThreads);


	unsigned char new_image[img_size];
	for(int i = 0; i <img_size/3; i++ ){
		new_image[j] =  *(r_target+i);
		new_image[j+1] =  *(g_target+i);
	 	new_image[j+2] =  *(b_target+i);
	 	j+=3;

	}
	


	stbi_write_jpg("NoSeasAlemania.jpg", width, height, channels, new_image, 100);


   
 }