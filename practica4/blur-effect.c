#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <mpi.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"
#define MAXTASKS 32

//Comando para Ejecutar
// gcc -Wall -pedantic blur-effect.c -o image -pthread  -lm

// Se utiliza para pasar los parametros a la funcion que ejecuta los hilos

int max(int num1, int num2)
{
	return (num1 > num2) ? num1 : num2;
}

int min(int num1, int num2)
{
	return (num1 > num2) ? num2 : num1;
}

int *intdup(int const *src, int len)
{
	int *p = malloc(len * sizeof(int));
	memcpy(p, src, len * sizeof(int));
	return p;
}

void buildTarget(int target[], int *recive[], int lastrecive[], int n, int lastn, int slaves)
{
	int x = 0;
	for (int i = 0; i < slaves; i++)
	{
		if (i == (slaves - 1))
		{
			for (int y = 0; y < lastn; y++)
			{
				target[x] = lastrecive[y];
				x++;
			}
		}
		else
		{
			for (int y = 0; y < n; y++)
			{
				target[x] = recive[i][y];
				x++;
			}
		}
	}
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
void boxBlurT(int *scl, int *tcl, int w, int h, int r, int init, int fin)
{
printf("box blur T w: %i  h: %i  init: %i  fin: %i \n ",w,h,init,fin);


	for (int i = init; i < fin; i++)
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
void boxBlurH(int *scl, int *tcl, int w, int h, int r, int init, int fin)
{

printf("box blur H w: %i  h: %i  init: %i  fin: %i \n ",w,h,init,fin);
	for (int i = init; i < fin; i++)
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
void boxBlur(int *scl, int *tcl, int w, int h, int r, int init, int fin)
{
	for (int i = 0; i < (w * h); i++)
		tcl[i] = scl[i];
	boxBlurH(tcl, scl, w, h, r, init, fin);
	boxBlurT(scl, tcl, w, h, r, init, fin);
}

//Metodo donde se crean los hilos y se asigna el trabajo de cada uno

//Metodo que aplica las iteracion del algoritmo boxblur para aproximar Gaussian blur
void gaussBlur_3(int *scl, int *tcl, int w, int h, int r, int init, int fin)
{

	double *bxs = (double *)malloc(sizeof(double) * 3);
	boxesForGauss(r, 3, bxs);

	boxBlur(scl, tcl, w, h, (int)((*(bxs)-1) / 2), init, fin);
	boxBlur(tcl, scl, w, h, (int)((*(bxs + 1) - 1) / 2), init, fin);
	boxBlur(scl, tcl, w, h, (int)((*(bxs + 2) - 1) / 2), init, fin);
}

int main(int argc, char **argv)
{
	int width, height, channels;
	char img_name[128];
	strcpy(img_name, argv[1]);
	char new_img_name[128];
	int kernel = atoi(argv[3]);
	strcpy(new_img_name, argv[2]);

	int tag = 0, tasks, iam;

	MPI_Status status;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &tasks);
	MPI_Comm_rank(MPI_COMM_WORLD, &iam);
	int chunckSize;

	if (iam == 0)
	{
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

		// Instanciamos los canales de la imagen de salida
		int *r_target = (int *)malloc(sizeof(int) * (img_size / 3));
		int *g_target = (int *)malloc(sizeof(int) * (img_size / 3));
		int *b_target = (int *)malloc(sizeof(int) * (img_size / 3));
		int j = 0;
		int i = 0;

		for (unsigned char *p = img; p != img + img_size; p += channels, i++)
		{
			*(r + i) = (uint8_t)*p;
			*(g + i) = (uint8_t) * (p + 1);
			*(b + i) = (uint8_t) * (p + 2);
		}

		chunckSize = (height / (tasks - 1)) * width;
		int *toSend = (int *)malloc(sizeof(int) * (chunckSize));
		int z = tasks - 1;
		int lastChuckSize = (width * height) - ((tasks - 2) * chunckSize);
		int *lasttoSend = (int *)malloc(sizeof(int) * (lastChuckSize));

		//Canal R
		for (int x = 1; x < (tasks); x++)
		{
			// memcpy(toSend, &r[(x - 1) * chunckSize], chunckSize * sizeof(*r));
			MPI_Send(&width, 1, MPI_INT, x, tag, MPI_COMM_WORLD);
			MPI_Send(&height, 1, MPI_INT, x, tag, MPI_COMM_WORLD);
			if (x == (tasks - 1))
			{
				MPI_Send(&lastChuckSize, 1, MPI_INT, x, tag, MPI_COMM_WORLD);
			}
			else
			{
				MPI_Send(&chunckSize, 1, MPI_INT, x, tag, MPI_COMM_WORLD);
			}

			MPI_Send(r, (width * height), MPI_INT, x, tag, MPI_COMM_WORLD);
		}

		

		int *result[tasks - 2];
		int *resultLast;
		for (int g = 1; g < tasks; g++)
		{
			if (g == tasks - 1)
			{

				int *r_recv = (int *)malloc(sizeof(int) * (lastChuckSize));
				MPI_Recv(r_recv, lastChuckSize, MPI_INT, g, tag, MPI_COMM_WORLD, &status);
				resultLast = intdup(r_recv, lastChuckSize);
				//printf("position despues master : %i iam %i \n", r_recv[lastChuckSize - 1], g);
				free(r_recv);
			}
			else
			{
				int *r_recv = (int *)malloc(sizeof(int) * (chunckSize));

				MPI_Recv(r_recv, chunckSize, MPI_INT, g, tag, MPI_COMM_WORLD, &status);
				result[g - 1] = intdup(r_recv, chunckSize);
				//printf("position despues master : %i iam %i \n", r_recv[chunckSize - 1], g);
				free(r_recv);
			}

			
		}

		buildTarget(r_target, result, resultLast, chunckSize, lastChuckSize, tasks - 1);
		//printf("position en array target : %i iam %i \n", r_target[(3*chunckSize) - 1], 3);

		// //Canal G
		// for (int x = 1; x < (tasks - 1); x++)
		// {
		// 	memcpy(toSend, &g[(x - 1) * chunckSize], chunckSize * sizeof(*g));
		// 	MPI_Send(&chunckSize, 1, MPI_INT, x, tag, MPI_COMM_WORLD);
		// 	MPI_Send(toSend, chunckSize, MPI_INT, x, tag, MPI_COMM_WORLD);
		// 	MPI_Send(&width, 1, MPI_INT, x, tag, MPI_COMM_WORLD);
		// }

		// memcpy(lasttoSend, &g[(z - 1) * chunckSize], lastChuckSize * sizeof(*g));
		// MPI_Send(&lastChuckSize, 1, MPI_INT, z, tag, MPI_COMM_WORLD);
		// MPI_Send(lasttoSend, lastChuckSize, MPI_INT, z, tag, MPI_COMM_WORLD);
		// MPI_Send(&width, 1, MPI_INT, z, tag, MPI_COMM_WORLD);

		// for (int h = 1; h < tasks - 1; h++)
		// {
		// 	int *g_recv = (int *)malloc(sizeof(int) * (chunckSize));
		// 	MPI_Recv(g_recv, chunckSize, MPI_INT, h, tag, MPI_COMM_WORLD, &status);
		// 	printf("position despues master : %i iam %i \n", g_recv[chunckSize - 1], h);
		// 	result[h - 1] = intdup(g_recv, chunckSize);
		// 	printf("position en array target : %i iam %i \n", result[h - 1][(chunckSize - 1)], h);

		// 	free(g_recv);
		// }

		// int *g_recv = (int *)malloc(sizeof(int) * (lastChuckSize));
		// MPI_Recv(g_recv, lastChuckSize, MPI_INT, tasks - 1, tag, MPI_COMM_WORLD, &status);
		// resultLast = g_recv;
		// printf("position despues master : %i iam %i \n", g_recv[lastChuckSize - 1], tasks - 1);
		// free(g_recv);

		// buildTarget(g_target, result, resultLast, chunckSize, lastChuckSize, tasks - 1);

		// //Canal B
		// for (int x = 1; x < (tasks - 1); x++)
		// {
		// 	memcpy(toSend, &b[(x - 1) * chunckSize], chunckSize * sizeof(*b));
		// 	MPI_Send(&chunckSize, 1, MPI_INT, x, tag, MPI_COMM_WORLD);
		// 	MPI_Send(toSend, chunckSize, MPI_INT, x, tag, MPI_COMM_WORLD);
		// 	MPI_Send(&width, 1, MPI_INT, x, tag, MPI_COMM_WORLD);
		// }

		// memcpy(lasttoSend, &b[(z - 1) * chunckSize], lastChuckSize * sizeof(*b));
		// MPI_Send(&lastChuckSize, 1, MPI_INT, z, tag, MPI_COMM_WORLD);
		// MPI_Send(lasttoSend, lastChuckSize, MPI_INT, z, tag, MPI_COMM_WORLD);
		// MPI_Send(&width, 1, MPI_INT, z, tag, MPI_COMM_WORLD);

		// for (int h = 1; h < tasks - 1; h++)
		// {
		// 	int *b_recv = (int *)malloc(sizeof(int) * (chunckSize));
		// 	MPI_Recv(b_recv, chunckSize, MPI_INT, h, tag, MPI_COMM_WORLD, &status);
		// 	printf("position despues master : %i iam %i \n", b_recv[chunckSize - 1], h);
		// 	result[h - 1] = intdup(b_recv, chunckSize);
		// 	printf("position en array target : %i iam %i \n", result[h - 1][(chunckSize - 1)], h);

		// 	free(b_recv);
		// }

		// int *b_recv = (int *)malloc(sizeof(int) * (lastChuckSize));
		// MPI_Recv(b_recv, lastChuckSize, MPI_INT, tasks - 1, tag, MPI_COMM_WORLD, &status);
		// resultLast = b_recv;
		// printf("position despues master : %i iam %i \n", b_recv[lastChuckSize - 1], tasks - 1);
		// free(b_recv);

		buildTarget(b_target, result, resultLast, chunckSize, lastChuckSize, tasks - 1);

		// Se reconstruye la imagen a partir de los canales procesados
		for (int i = 0; i < img_size / 3; i++)
		{
			img[j] = *(r_target + i);
			img[j + 1] = 0;
			img[j + 2] = 0;
			j += 3;
		}

		stbi_write_jpg(new_img_name, width, height, channels, img, 100);

		free(toSend);
		free(lasttoSend);
		free(r_target);
		// free(g_target);
		// free(b_target);
		free(r);
		// free(g);
		// free(b);
	}
	else
	{
		MPI_Recv(&width, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
		MPI_Recv(&height, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
		MPI_Recv(&chunckSize, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
		int *r_scl = (int *)malloc(sizeof(int *) * (width * height));
		MPI_Recv(r_scl, (width * height), MPI_INT, 0, tag, MPI_COMM_WORLD, &status);

		//Aplicamos el algoritmo para cada canal
		int *r_tcl = (int *)malloc(sizeof(int *) * (width * height));
		
		gaussBlur_3(r_scl, r_tcl, width, height, kernel, (height / (tasks-1)) * (iam - 1), (height / (tasks-1)) * iam);

		

		int *r_send = (int *)malloc(sizeof(int *) * (chunckSize));
		memcpy(r_send, &r_tcl[(iam - 1) * chunckSize], chunckSize * sizeof(*r_tcl));

		//printf("position despues master : %i iam %i \n", r_send[chunckSize - 1], iam);
		MPI_Send(r_send, chunckSize, MPI_INT, 0, tag, MPI_COMM_WORLD);

		free(r_scl);
		free(r_tcl);

		// MPI_Recv(&chunckSize, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
		// int *g_scl = (int *)malloc(sizeof(int *) * (chunckSize));
		// MPI_Recv(g_scl, chunckSize, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
		// MPI_Recv(&width, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
		// //Aplicamos el algoritmo para cada canal
		// int *g_tcl = (int *)malloc(sizeof(int *) * (chunckSize));
		// gaussBlur_3(g_scl, g_tcl, width, chunckSize / width, kernel);
		// //printf("position despues : %i iam %i \n", tcl[chunckSize - 1], iam);
		// MPI_Send(g_tcl, chunckSize, MPI_INT, 0, tag, MPI_COMM_WORLD);

		// free(g_scl);
		// free(g_tcl);

		// MPI_Recv(&chunckSize, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
		// int *b_scl = (int *)malloc(sizeof(int *) * (chunckSize));
		// MPI_Recv(b_scl, chunckSize, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
		// MPI_Recv(&width, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
		// //Aplicamos el algoritmo para cada canal
		// int *b_tcl = (int *)malloc(sizeof(int *) * (chunckSize));
		// gaussBlur_3(b_scl, b_tcl, width, chunckSize / width, kernel);
		// //printf("position despues : %i iam %i \n", tcl[chunckSize - 1], iam);
		// MPI_Send(b_tcl, chunckSize, MPI_INT, 0, tag, MPI_COMM_WORLD);

		// free(b_scl);
		// free(b_tcl);
	}

	MPI_Finalize();

	// // Cargamos la imagen y definimos variables

	// //Aplicamos el algoritmo para cada canal
	// gaussBlur_3(r, r_target, width, height, kernel);
	// gaussBlur_3(b, b_target, width, height, kernel);
	// gaussBlur_3(g, g_target, width, height, kernel);

	// // Se reconstruye la imagen a partir de los canales procesados
	// for (int i = 0; i < img_size / 3; i++)
	// {
	// 	img[j] = *(r_target + i);
	// 	img[j + 1] = *(g_target + i);
	// 	img[j + 2] = *(b_target + i);
	// 	j += 3;
	// }

	// //Se crea la nueva imagen y liberamos memoria
	// stbi_write_jpg(new_img_name, width, height, channels, img, 100);
	// free(r_target);
	// free(g_target);
	// free(b_target);
	// free(r);
	// free(g);
	// free(b);
}
