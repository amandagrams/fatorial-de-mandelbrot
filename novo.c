#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include <pthread.h>

#define NUMTHREADS 4
#define INTERACOES 200

FILE * fp;
char *filename = "new1.ppm";
char *comment = "# ";
static unsigned char cor[3];
pthread_t threads[NUMTHREADS];

void mandelbrot(int idThread);
void *worker(void *arg);

int main(int argc, char *argv[]) {

	fp = fopen(filename, "wb");
	fprintf(fp, "P6\n %s\n %d\n %d\n %d\n", comment, 600, 400, 255);

	for (int t = 0; t < NUMTHREADS; t++)
		pthread_create(&threads[t], NULL, worker, (void*)t);
	for (int t = 0; t < NUMTHREADS; t++)
		pthread_join(threads[t], NULL);
	fclose(fp);

	return 0;
}

void mandelbrot(int idThread) {
	int w = 600, h = 400, x, y;
	double pr, pi;
	double newRe, newIm, oldRe, oldIm;
	double zoom = 1, moveX = -0.5, moveY = 0;
	int start = idThread * INTERACOES / NUMTHREADS;
	int end = (idThread + 1) * (INTERACOES / NUMTHREADS) - 1;

	pthread_mutex_lock(&bagLock);

	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			pr = 1.5 * (x - w / 2) / (0.5 * zoom * w) + moveX;
			pi = (y - h / 2) / (0.5 * zoom * h) + moveY;
			newRe = newIm = oldRe = oldIm = 0;
			int i;
			for (i = start; i <= end; i++) {
				oldRe = newRe;
				oldIm = newIm;
				newRe = oldRe * oldRe - oldIm * oldIm + pr;
				newIm = 2 * oldRe * oldIm + pi;
				if ((newRe * newRe + newIm * newIm) > 4)
					break;
			}
			if (i == INTERACOES) {
				cor[0] = 0;
				cor[1] = 0;
				cor[2] = 0;
			}
			else {
				double z = sqrt(newRe * newRe + newIm * newIm);
				int brightness = 256. * log2(1.75 + i - log2(log2(z))) / log2((double)INTERACOES);
				cor[0] = brightness;
				cor[1] = brightness;
				cor[2] = brightness;
			}
			fwrite(cor, 1, 3, fp);
		}
	}
	pthread_mutex_unlock(&bagLock);
}

void *worker(void *arg) {
	int tid = (int)arg;
	mandelbrot(tid);
}