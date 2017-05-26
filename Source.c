#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <pthread.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>

#define iXmax 800
#define iYmax 800
#define CxMin  -2.5
#define CxMax  1.5
#define CyMin  -2.0
#define CyMax  2.0
#define IterationMax 200
#define EscapeRadius 2
#define MaxColorComponentValue 255
#define NUMTHREADS 1

int iX, iY;
double Cx, Cy;
double PixelWidth = (CxMax - CxMin) / iXmax;
double PixelHeight = (CyMax - CyMin) / iYmax;
FILE * fp;
char *filename = "new1.ppm";
char *comment = "# ";
static unsigned char color[3];
double Zx, Zy, Zx2, Zy2;
int Iteration;
double ER2 = EscapeRadius*EscapeRadius;
pthread_mutex_t bagLock;
pthread_t threads[NUMTHREADS];

void *processa(void *);

int main(int argc, char *argv[]) {

	fp = fopen(filename, "wb"); 
	fprintf(fp, "P6\n %s\n %d\n %d\n %d\n", comment, iXmax, iYmax, MaxColorComponentValue);
	for (int t = 0; t < NUMTHREADS; t++)
		pthread_create(&threads[t], NULL, processa, (void*)t);
	for (int t = 0; t < NUMTHREADS; t++)
		pthread_join(threads[t], NULL);
	fclose(fp);
	return 0;
}

void *processa(void* arg) {
	int x = 0;
	long myid = (long)arg;
	while (true){
		for (iY = 0; iY<iYmax; iY++) {
			Cy = CyMin + iY*PixelHeight;
			if (fabs(Cy)< PixelHeight / 2)
				Cy = 0.0;
			for (iX = 0; iX<iXmax; iX++) {
				Cx = CxMin + iX*PixelWidth;
				Zx = 0.0;
				Zy = 0.0;
				Zx2 = Zx*Zx;
				Zy2 = Zy*Zy;
				for (Iteration = 0; Iteration<IterationMax && ((Zx2 + Zy2)<ER2); Iteration++) {
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
				};
				fwrite(color, 1, 3, fp);
				if (x >= IterationMax){
					break;
				}
				x++;
			}
		}

	}

}