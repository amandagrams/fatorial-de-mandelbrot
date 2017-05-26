#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <pthread.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>

#define w 800
#define h 800
#define wMin -2.5
#define wMax 1.5
#define hMin  -2.0
#define hMax  2.0
#define IterationMax 200
#define EscapeRadius 2
#define MaxColorComponentValue 255
#define NUMTHREADS 2

double Cx, Cy;
double PixelWidth = (wMax - wMin) / w;
double PixelHeight = (hMax - hMin) / h;
FILE * fp;
char *filename = "new1.ppm";
char *comment = "# ";
static unsigned char color[3];
double ER2 = EscapeRadius*EscapeRadius;
pthread_t threads[NUMTHREADS];

void mandelBrot(int id);
void *worker(void* arg);

int main(int argc, char *argv[]) {

	fp = fopen(filename, "wb"); 
	fprintf(fp, "P6\n %s\n %d\n %d\n %d\n", comment, w, h, MaxColorComponentValue);
	for (int t = 0; t < NUMTHREADS; t++)
		pthread_create(&threads[t], NULL, worker, (void*)t);
	for (int t = 0; t < NUMTHREADS; t++)
		pthread_join(threads[t], NULL);
	fclose(fp);
	return 0;
}

void *worker(void* arg) {
	int id = (int)arg;
	mandelBrot(id);
}

void mandelBrot(int id) {
	int x, y;
	int Iteration;
	double Zx, Zy, Zx2, Zy2;
	int start = id * IterationMax / NUMTHREADS;
	int end = (id + 1) * (IterationMax / NUMTHREADS);
	
	for (y = 0; y < w; y++) {
		Cy = hMin + y*PixelHeight;
		if (fabs(Cy)< PixelHeight / 2)
			Cy = 0.0;
		for (x = 0; x < h; x++) {
			Cx = wMin + x*PixelWidth;
			Zx = 0.0;
			Zy = 0.0;
			Zx2 = Zx*Zx;
			Zy2 = Zy*Zy;
			for (Iteration = start; Iteration < end && ((Zx2 + Zy2) < ER2); Iteration++) {
				Zy = 2 * Zx*Zy + Cy;
				Zx = Zx2 - Zy2 + Cx;
				Zx2 = Zx*Zx;
				Zy2 = Zy*Zy;
			}
			if (Iteration == IterationMax) {
				color[0] = 0;
				color[1] = 0;
				color[2] = 0;
			}
			else {
				color[0] = 255;
				color[1] = 255;
				color[2] = 255;
			}
			fwrite(color, 1, 3, fp);
		}
	}
}