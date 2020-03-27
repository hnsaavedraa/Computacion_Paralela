#include <stdio.h>
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"

int main(void) {
     int width, height, channels;
      unsigned char *img = stbi_load("captura.jpg", &width, &height, &channels, 0); //// cero para cargar todos los canales
     if(img == NULL) {
         printf("Error in loading the image\n");
         exit(1);
     }
     printf("Loaded image with a width of %dpx, a height of %dpx and %d channels\n", width, height, channels);
   
   
     size_t img_size  = width * height * channels;
     int s = 0;



int r[img_size/3];
int g[img_size/3];
int b[img_size/3];
int i = 0;
for(unsigned char *p = img; p != img + img_size; p += channels,i++) {
	r[i] = (uint8_t) *p ;
	g[i] = (uint8_t) *(p + 1);
	b[i] = (uint8_t) *(p + 2);

}
printf("%i ",i);
int count = 0;
for(int x = 0; x< img_size/3; x++,count++){
	if(count > width){
		printf("\n");
		count = 0;
	}

	printf("%i ", r[x]);

}

unsigned char new_image[img_size];
int j =0;
for(int i = 0; i <img_size/3; i++ ){
	new_image[j] = (char) r[i];
	new_image[j+1] = (char) r[i];
 	new_image[j+2] = (char) r[i];
 	j+=3;
}

stbi_write_jpg("sky_sepia.jpg", width, height, channels, new_image, 100);



     // stbi_write_jpg("sky2.jpg", width, height, channels, sepia_img, 100);
 
     
 }