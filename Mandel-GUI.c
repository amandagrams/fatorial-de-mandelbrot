#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <pthread.h>

#define VAL 255
#define NUMTHREADS 1

typedef struct { unsigned char r, g, b; } rgb_t;
rgb_t **tex = 0;
int gwin;
GLuint texture;
int width = 800, height = 400;
int tex_w, tex_h;
double scale = 1. / 256;
double cx = -.6, cy = 0;
int color_rotate = 0;
int saturation = 1;
int invert = 0;
int max_iter = 256;
pthread_t threads[NUMTHREADS];
int xi = 0, yi = 0, xf = 20, yf = 20;


void render();
void hsv_to_rgb(int hue, int min, int max, rgb_t *p);
void calc_mandel();
void alloc_tex();
void set_texture();
void resize(int w, int h);
void init();
void set_texture();
void *worker(void *arg);

int main(int argc, char **argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
	glutInitWindowSize(width, height);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("MandelBrot");
	init();
	glutDisplayFunc(render);
	glutReshapeFunc(resize);
	glutMainLoop();
	return 0;
}

void render() {
	double	x = (double)width / tex_w, y = (double)height / tex_h;
	glClear(GL_COLOR_BUFFER_BIT);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glBindTexture(GL_TEXTURE_2D, texture);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0); glVertex2i(0, 0);
	glTexCoord2f(x, 0); glVertex2i(width, 0);
	glTexCoord2f(x, y); glVertex2i(width, height);
	glTexCoord2f(0, y); glVertex2i(0, height);
	glEnd();
	glFlush();
	glFinish();
}

void hsv_to_rgb(int hue, int min, int max, rgb_t *p) {
	if (min == max) max = min + 1;
	if (invert) hue = max - (hue - min);
	if (!saturation) {
		p->r = p->g = p->b = 255 * (max - hue) / (max - min);
		return;
	}
	double h = fmod(color_rotate + 1e-4 + 4.0 * (hue - min) / (max - min), 6);
	double c = VAL * saturation;
	double X = c * (1 - fabs(fmod(h, 2) - 1));
	p->r = p->g = p->b = 0;
	switch ((int)h) {
	case 0: p->r = c; p->g = X; return;
	case 1:	p->r = X; p->g = c; return;
	case 2: p->g = c; p->b = X; return;
	case 3: p->g = X; p->b = c; return;
	case 4: p->r = X; p->b = c; return;
	default:p->r = c; p->b = X;
	}
}

void calc_mandel(int wi, int wf, int hi, int hf) {
	int i, j, iter, min, max;
	rgb_t *px;
	double x, y, zx, zy, zx2, zy2;
	min = max_iter; max = 0;
	for (i = hi; i < hf; i++) {
		px = tex[i];
		y = (i - height / 2) * scale + cy;
		for (j = wi; j < wf; j++, px++) {
			x = (j - width / 2) * scale + cx;
			iter = 0;
			zx = hypot(x - .25, y);
			if (x < zx - 2 * zx * zx + .25) iter = max_iter;
			if ((x + 1)*(x + 1) + y * y < 1 / 16) iter = max_iter;
			zx = zy = zx2 = zy2 = 0;
			for (; iter < max_iter && zx2 + zy2 < 4; iter++) {
				zy = 2 * zx * zy + y;
				zx = zx2 - zy2 + x;
				zx2 = zx * zx;
				zy2 = zy * zy;
			}
			if (iter < min) min = iter;
			if (iter > max) max = iter;
			*(unsigned short *)px = iter;
		}
	}
	for (i = hi; i < hf; i++)
		for (j = wf, px = tex[i]; j < wf; j++, px++)
			hsv_to_rgb(*(unsigned short*)px, min, max, px);

}

void alloc_tex() {
	int i, ow = tex_w, oh = tex_h;
	for (tex_w = 1; tex_w < width; tex_w <<= 1);
	for (tex_h = 1; tex_h < height; tex_h <<= 1);
	if (tex_h != oh || tex_w != ow)
		tex = realloc(tex, tex_h * tex_w * 3 + tex_h * sizeof(rgb_t*));
	for (tex[0] = (rgb_t *)(tex + tex_h), i = 1; i < tex_h; i++)
		tex[i] = tex[i - 1] + tex_w;
}

void set_texture() {
	for (int t = 0; t < NUMTHREADS; t++)
		pthread_create(&threads[t], NULL, worker, (void*)t);
	for (int t = 0; t < NUMTHREADS; t++)
		pthread_join(threads[t], NULL);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, tex_w, tex_h,
		0, GL_RGB, GL_UNSIGNED_BYTE, tex[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	render();
}

void resize(int w, int h) {
	printf("resize %d %d\n", w, h);
	width = w;
	height = h;
	glViewport(0, 0, w, h);
	glOrtho(0, w, 0, h, -1, 1);
	set_texture();
}

void init() {
	glGenTextures(1, &texture);
	alloc_tex();
	set_texture();
}

void *worker(void *arg) {
	while (1) {
		calc_mandel(xi, xf, yi, yf);
		if (xf <= width){
			xi += 20;
			xf += 20;
		}
		if (yf <= height) {
			yi += 20;
			yf += 20;
		}
		
		if (xf >= width && yf >= height)
			break;
	}
}