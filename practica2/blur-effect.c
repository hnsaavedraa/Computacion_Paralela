#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <omp.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"

//Comando para Ejecutar
// gcc -Wall -pedantic blur-effect.c -o image -pthread  -lm

// Se utiliza para pasar los parametros a la funcion que ejecuta los hilos
struct parameters
{
	int *scl;
	int *tcl;
	int w;
	int h;
	int r;
	int init_i;
	int fin_i;
};

int max(int num1, int num2)
{
	return (num1 > num2) ? num1 : num2;
}

int min(int num1, int num2)
{
	return (num1 > num2) ? num2 : num1;
}

//Metodo que calcula el radio de las cajas (br) para el algoritmo boxblur
void boxesForGauss(int sigma, int n, double *sizes) 
{

	double wIdeal = sqrt((12 * sigma * sigma / n) + 1); 
	double wl = floor(wIdeal);
	if (fmod(wl, 2.0) == 0.0)
	{
		wl--;
	}
	double wu = wl + 2;

	double mIdeal = (12 * sigma * sigma - n * wl * wl - 4 * n * wl - 3 * n) / (-4 * wl - 4);
	double m = round(mIdeal);
	for (int i = 0; i < n; i++)
	{
		*(sizes + i) = (i < m ? wl : wu);
	}
}

//Metodo que implementa el algoritmo boxblur verticalmente
void boxBlurT(int *scl, int *tcl, int w, int h, int r, int init_i, int fin_i)
{

	for (int i = init_i; i < fin_i; i++)
		for (int j = 0; j < w; j++)
		{
			int val = 0;
			for (int iy = i - r; iy < i + r + 1; iy++)
			{
				int y = min(h - 1, max(0, iy));
				val += *(scl + (y * w + j));
			}

			*(tcl + (i * w + j)) = val / (r + r + 1);
		}
}

//Metodo que implementa el algoritmo boxblur horizontalmente
void boxBlurH(int *scl, int *tcl, int w, int h, int r, int init_i, int fin_i)
{

	for (int i = init_i; i < fin_i; i++)
		for (int j = 0; j < w; j++)
		{
			int val = 0;
			for (int ix = j - r; ix < j + r + 1; ix++)
			{
				int x = min(w - 1, max(0, ix));
				val += *(scl + (i * w + x));
			}

			*(tcl + (i * w + j)) = val / (r + r + 1);
		}
}

//Metodo ejecutado por los hilos
void parallelProcess(struct parameters *dataAux)
{
	boxBlurH(dataAux->tcl, dataAux->scl, dataAux->w, dataAux->h, dataAux->r, dataAux->init_i, dataAux->fin_i);
	boxBlurT(dataAux->scl, dataAux->tcl, dataAux->w, dataAux->h, dataAux->r, dataAux->init_i, dataAux->fin_i);
}

//Metodo donde se crean los hilos y se asigna el trabajo de cada uno
void boxBlur(int *scl, int *tcl, int w, int h, int r, int numThreads)
{

	for (int i = 0; i < (w * h); i++)
	{
		int aux = *(scl + i);
		*(tcl + i) = aux;
	}

	int interval_h = floor((h) / numThreads);
	int init_i;
	int fin_i;
	
	#pragma omp parallel for 
	for (int i = 0; i < numThreads; i++)
	{
		struct parameters data;
		data.scl = scl;
		data.tcl = tcl;
		data.w = w;
		data.h = h;
		data.r = r;
	

		if (i == numThreads - 1)
		{
			fin_i = h;
			init_i = i * interval_h;
			data.init_i = init_i;
			data.fin_i = fin_i;
		}
		else
		{
			init_i = i * interval_h;
			fin_i = (i + 1) * interval_h;
			data.init_i = init_i;
			data.fin_i = fin_i;
		}

		parallelProcess(&data);
	}


}

//Metodo que aplica las iteracion del algoritmo boxblur para aproximar Gaussian blur
void gaussBlur_3(int *scl, int *tcl, int w, int h, int r, int numThreads)
{

	double *bxs = (double *)malloc(sizeof(double) * 3);
	boxesForGauss(r, 3, bxs);

	boxBlur(scl, tcl, w, h, (int)((*(bxs)-1) / 2), numThreads);
	boxBlur(tcl, scl, w, h, (int)((*(bxs + 1) - 1) / 2), numThreads);
	boxBlur(scl, tcl, w, h, (int)((*(bxs + 2) - 1) / 2), numThreads);
}


int main(int argc, char **argv)
{
	// Cargamos la imagen y definimos variables 
	int width, height, channels;
	char img_name[128];
	strcpy(img_name, argv[1]);
	char new_img_name[128];
	int kernel = atoi(argv[3]);
	int numThreads = atoi(argv[4]);
	strcpy(new_img_name, argv[2]);
	unsigned char *img = stbi_load(img_name, &width, &height, &channels, 0);
	if (img == NULL)
	{
		printf("Error in loading the image\n");
		exit(1);
	}
	printf("Loaded image with a width of %dpx, a height of %dpx and %d channels\n", width, height, channels);

	// Almacenamos los valores de cada canal
	int img_size = width * height * channels;
	int *r = (int *)malloc(sizeof(int) * (img_size / 3));
	int *g = (int *)malloc(sizeof(int) * (img_size / 3));
	int *b = (int *)malloc(sizeof(int) * (img_size / 3));
	int i = 0;
	for (unsigned char *p = img; p != img + img_size; p += channels, i++)
	{
		*(r + i) = (uint8_t)*p;
		*(g + i) = (uint8_t) * (p + 1);
		*(b + i) = (uint8_t) * (p + 2);
	}

	// Instanciamos los canales de la imagen de salida
	int *r_target = (int *)malloc(sizeof(int) * (img_size / 3));
	int *g_target = (int *)malloc(sizeof(int) * (img_size / 3));
	int *b_target = (int *)malloc(sizeof(int) * (img_size / 3));
	int j = 0;

	//Aplicamos el algoritmo para cada canal
	gaussBlur_3(r, r_target, width, height, kernel, numThreads);
	gaussBlur_3(g, g_target, width, height, kernel, numThreads);
	gaussBlur_3(b, b_target, width, height, kernel, numThreads);

	// Se reconstruye la imagen a partir de los canales procesados
	for (int i = 0; i < img_size / 3; i++)
	{
		img[j] = *(r_target + i);
		img[j + 1] = 0;
		img[j + 2] = 0;
		j += 3;
	}

	//Se crea la nueva imagen y liberamos memoria 
	stbi_write_jpg(new_img_name, width, height, channels, img, 100);
	free(r_target);
	free(g_target);
	free(b_target);
	free(r);
	free(g);
	free(b);
}
