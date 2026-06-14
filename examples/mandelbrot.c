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

static inline void fail(const char *msg) {
	fprintf(stderr, "%s: %d\n", msg, errno);
	exit(1);
}

static inline int get_color(FLOAT _Complex point, int const it_count) {
	FLOAT _Complex z = 0;
	for (int i = 0; i < it_count; ++i) {
		FLOAT re = creal(z), im = cimag(z);

		if (re*re + im*im > 4) {
			return 4 * (1.0/4 - 1.0/(4 + i)) * 256;
		}

		z = z*z + point;
	}

	return 0;
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

			int color_n = get_color(CMPLX(x, y), it_count);

			struct color c = {
				.a = 0,
				.r = color_n,
				.g = color_n,
				.b = color_n
			};
			draw_point(fbdata, fbmem, (struct fb_pos) { .x = sx, .y = sy }, c);
		}
	}

	while (1) {
		pause();
	}
}
