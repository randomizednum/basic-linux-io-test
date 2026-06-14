#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <complex.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include "mouse.h"
#include "audio.h"
#include "piece.h"
#include "framebuf.h"
#include "kb.h"

#define FLOAT double
#define COLORFULNESS 0

static inline void fail(const char *msg) {
	fprintf(stderr, "%s: %d\n", msg, errno);
	exit(1);
}

//r -> rg
//rg -> g
//g -> gb
//gb -> b
//b -> rb
//rb -> r
static inline struct color degree_to_color(double d_init) {
#if COLORFULNESS > 0
	int r, g, b;

	double d = fmod(d_init * COLORFULNESS, 1);

	if (d * 6 < 1) r = 255, g = (d*6) * 256, b = 0;
	else if (d * 6 < 2) r = 255 - (int) ((d*6-1) * 256), g = 255, b = 0;
	else if (d * 6 < 3) r = 0, g = 255, b = (d*6-2) * 256;
	else if (d * 6 < 4) r = 0, g = 255 - (int) ((d*6-3) * 256), b = 255;
	else if (d * 6 < 5) r = (d*6-4) * 256, g = 0, b = 255;
	else r = 255, g = 0, b = 255 - (int) ((d*6-5) * 256);

	return (struct color) { .a = 0, .r = r, .g = g, .b = b };
#else
	int c = d_init * 256;
	return (struct color) { .a = 0, .r = c, .g = c, .b = c};
#endif
}

static inline struct color get_color(FLOAT _Complex point, int const it_count) {
	FLOAT _Complex z = 0;
	for (int i = 0; i < it_count; ++i) {
		FLOAT re = creal(z), im = cimag(z);

		if (re*re + im*im > 4) {
			return degree_to_color(4 * (1.0/4 - 1.0/(4 + i)));
		}

		z = z*z + point;
	}

	return (struct color) { .a = 0, .r = 0, .g = 0, .b = 0 };
}

int main(int argc, char **argv) {
	int fbfd = open("/dev/fb0", O_RDWR);
	if (fbfd == -1) fail("couldn't open fb0");

	int it_count = 100;
	FLOAT upp = 0.002; //units per pixel
	FLOAT c_re = 0, c_im = 0;

	for (int i = 1; i < argc - 1; ++i) {
		char *arg = argv[i];
		if (strcmp(arg, "--iterations") == 0) it_count = atoi(argv[++i]);
		else if (strcmp(arg, "--units-per-pixel") == 0) upp = atof(argv[++i]);
		else if (strcmp(arg, "--center-re") == 0) c_re = atof(argv[++i]);
		else if (strcmp(arg, "--center-im") == 0) c_im = atof(argv[++i]);
	}

	struct fb_fix_screeninfo fixscinfo;
	struct fb_var_screeninfo varscinfo;
	ioctl(fbfd, FBIOGET_FSCREENINFO, &fixscinfo);
	ioctl(fbfd, FBIOGET_VSCREENINFO, &varscinfo);

	struct fbdata fbdata = get_fbdata(&varscinfo, &fixscinfo);

	unsigned char *fbmem = mmap(NULL, fixscinfo.smem_len, PROT_WRITE, MAP_SHARED, fbfd, 0);

	for (int sy = 0; sy < fbdata.h; ++sy) {
		for (int sx = 0; sx < fbdata.w; ++sx) {
			FLOAT x = (sx - (int) fbdata.w / 2) * upp + c_re;
			FLOAT y = (sy - (int) fbdata.h / 2) * upp + c_im;

			struct color c = get_color(CMPLX(x, y), it_count);

			draw_point(fbdata, fbmem, (struct fb_pos) { .x = sx, .y = sy }, c);
		}
	}

	while (1) {
		pause();
	}
}
