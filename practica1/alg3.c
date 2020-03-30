#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <pthread.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"

void imprimir(int *scl, int id)
{
	printf("Inicio impresion Hilo %i \n", id);
	for (int z = 0; z < 64; z++)
	{
		if (fmod(z, 8) == 0)
		{
			printf("\n");
		}

		printf("%i ", *(scl + z));
	}
	printf("Final impresion Hilo %i \n \n", id);
}

struct parameters
{
	int *scl;
	int *tcl;
	int w;
	int h;
	int r;
	int init_i;
	int fin_i;
	int id;
};

int max(int num1, int num2)
{
	return (num1 > num2) ? num1 : num2;
}

int min(int num1, int num2)
{
	return (num1 > num2) ? num2 : num1;
}

void boxesForGauss(int sigma, int n, double *sizes) // standard deviation, number of boxes
{

	double wIdeal = sqrt((12 * sigma * sigma / n) + 1); // Ideal averaging filter width
	double wl = floor(wIdeal);
	if (fmod(wl, 2.0) == 0.0)
	{
		wl--;
	}
	double wu = wl + 2;

	double mIdeal = (12 * sigma * sigma - n * wl * wl - 4 * n * wl - 3 * n) / (-4 * wl - 4);
	double m = round(mIdeal);
	// var sigmaActual = Math.sqrt( (m*wl*wl + (n-m)*wu*wu - n)/12 );

	for (int i = 0; i < n; i++)
	{
		*(sizes + i) = (i < m ? wl : wu);
	}
}

void boxBlurT_3(int *scl, int *tcl, int w, int h, int r, int init_i, int fin_i)
{

	// Reemplazar i, j y h,w por los valores de inicio y fin del intervalo
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

void boxBlurH_3(int *scl, int *tcl, int w, int h, int r, int init_i, int fin_i)
{

	//printf("Entro a boxBlurH_3 \n");
	// Reemplazar i, j y h,w por los valores de inicio y fin del intervalo
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

void parallelProcess(void *data)
{

	struct parameters *dataAux;
	dataAux = (struct parameters *)data;

	int init_aux = (dataAux->init_i);
	int fin_aux = (((struct parameters *)data)->fin_i);
	printf("init_ i en parallel %i en hilo %i \n", dataAux->init_i, dataAux->id);

	boxBlurH_3(((struct parameters *)data)->tcl, ((struct parameters *)data)->scl, dataAux->w, dataAux->h, dataAux->r, init_aux, fin_aux);
	boxBlurT_3(((struct parameters *)data)->scl, ((struct parameters *)data)->tcl, dataAux->w, dataAux->h, dataAux->r, init_aux, fin_aux);
}

void boxBlur_3(int *scl, int *tcl, int w, int h, int r, int numThreads)
{

	for (int i = 0; i < (w * h); i++)
	{
		int aux = *(scl + i);
		*(tcl + i) = aux;
	}
	int interval_h = floor((h) / numThreads);
	int threadId[numThreads], i, *retval;
	pthread_t thread[numThreads];
	int init_i;
	int fin_i;

	struct parameters data_array[numThreads];

	for (i = 0; i < numThreads; i++)
	{
		struct parameters data;
		data.scl = scl;
		data.tcl = tcl;
		data.w = w;
		data.h = h;
		data.r = r;
		threadId[i] = i;

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

		data.id = i;
		data_array[i] = data;
		//printf("init i %i \n", init_i);
		printf("init_ i en create  %i en Hilo %i \n", (data.init_i), i);
		printf("Direccion de memoria %i en el hilo %i \n", &data_array[i], i);

		pthread_create(&thread[i], NULL, (void *)parallelProcess, &data_array[i]);
	}

	for (i = 0; i < numThreads; i++)
	{
		//pthread_join(thread[i], (void **)&retval);
		int retor = pthread_join(thread[i], NULL);
		printf("scl del hilo %i \n", i);
		imprimir(scl, i);
		printf("tcl del hilo %i \n", i);
		imprimir(tcl, i);
		printf("RETORNO %i \n", retor);
		printf("Termino hilo %i \n", i);
	}
}

void gaussBlur_3(int *scl, int *tcl, int w, int h, int r, int numThreads)
{

	double *bxs = (double *)malloc(sizeof(double) * 3);
	boxesForGauss(r, 3, bxs);

	boxBlur_3(scl, tcl, w, h, (int)((*(bxs)-1) / 2), numThreads);
	boxBlur_3(tcl, scl, w, h, (int)((*(bxs + 1) - 1) / 2), numThreads);
	boxBlur_3(scl, tcl, w, h, (int)((*(bxs + 2) - 1) / 2), numThreads);
}

int main(void)
{

	int width, height, channels;
	unsigned char *img = stbi_load("1080.jpg", &width, &height, &channels, 0); //// cero para cargar todos los canales
	if (img == NULL)
	{
		printf("Error in loading the image\n");
		exit(1);
	}
	printf("Loaded image with a width of %dpx, a height of %dpx and %d channels\n", width, height, channels);

	int img_size = width * height * channels;
	printf("%i \n", img_size);

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
	imprimir(r, 1000);

	int *r_target = (int *)malloc(sizeof(int) * (img_size / 3));
	int *g_target = (int *)malloc(sizeof(int) * (img_size / 3));
	int *b_target = (int *)malloc(sizeof(int) * (img_size / 3));
	int kernel = 15;
	int numThreads = 8;
	int j = 0;
	printf("Direccion r %i \n", r);
	//char[] = *r
	gaussBlur_3(r, r_target, width, height, kernel, numThreads);
	gaussBlur_3(g, g_target, width, height, kernel, numThreads);
	gaussBlur_3(b, b_target, width, height, kernel, numThreads);

	unsigned char new_image[img_size];
	for (int i = 0; i < img_size / 3; i++)
	{
		new_image[j] = *(r_target + i);
		new_image[j + 1] = *(g_target + i);
		new_image[j + 2] = *(b_target + i);
		j += 3;
	}

	stbi_write_jpg("NoSeasAlemania.jpg", width, height, channels, new_image, 100);
}