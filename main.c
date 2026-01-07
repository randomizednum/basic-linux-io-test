/*
 * Copyright 2026 Çınar Karaaslan
 * Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted, provided that the above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED “AS IS” AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include "mouse.h"
#include "audio.h"
#include "piece.h"
#include "framebuf.h"

static inline void fail(const char *msg) {
	fprintf(stderr, "%s: %d\n", msg, errno);
	exit(1);
}

int main(void) {
	int mice = open("/dev/input/mice", O_RDONLY);
	if (mice == -1) fail("couldn't open mouse device");

	int mice_oldf = fcntl(mice, F_GETFL);
	if (mice_oldf == -1) fail("couldn't get file flags of mouse device");

	if (fcntl(mice, F_SETFL, mice_oldf | O_NONBLOCK) == -1) fail("couldn't set file flags of mouse device");


	int audfd = audio_init(0, 0, O_NONBLOCK);
	if (audfd == -1) fail("couldn't open audio device");

	double notes1[] = { G, REST, D  , Eb, F, REST, Eb, D  , C  , REST, C  , Eb, G , REST, F, Eb, D  , REST, D  , Eb, F, REST, G   , REST, Eb, REST, C  , REST, C   };
	double notes2[] = { D, REST, B/2, C , D, REST, C , B/2, G/2, REST, G/2, C , Eb, REST, D, C , B/2, G/2 , B/2, C , D, G/2 , Eb  , G/2 , C , G/2 , G/2, REST, G/2 };

	int sz = sizeof (notes1) / sizeof (notes1[0]);
	if (sizeof (notes1) != sizeof (notes2)) fail("programmer skill issue");

	int spn = SAMPLE_RATE/4; //SAMPLE_RATE/4 samples per note (i.e., 0.25 seconds per note)
	int16_t *pbbuf = calloc(GET_PIECE_BUF_SIZE(sz, spn), sizeof (int16_t)); //Buffer to accumulate samples
	compile(notes1, sz, pbbuf, spn);
	compile(notes2, sz, pbbuf, spn);

	struct audio_context pbctx = (struct audio_context) {
		.fd = audfd,
		.nframes_left = GET_PIECE_BUF_FRAMES(sz, spn),
		.max_playback = 16384, //at most 16384 frames at a time
		.buf = pbbuf
	};


	int fbfd = open("/dev/fb0", O_RDWR);
	if (fbfd == -1) fail("couldn't open fb0");

	struct fb_fix_screeninfo fixscinfo;
	struct fb_var_screeninfo varscinfo;
	ioctl(fbfd, FBIOGET_FSCREENINFO, &fixscinfo);
	ioctl(fbfd, FBIOGET_VSCREENINFO, &varscinfo);

	struct fbdata fbdata = get_fbdata(&varscinfo, &fixscinfo);

	printf("type:\t%d\nvisual:\t%d\n", fixscinfo.type, fixscinfo.visual);
	printf("xres:\t%d\nyres:\t%d\n", varscinfo.xres, varscinfo.yres);
	printf("vxres:\t%d\n", varscinfo.xres_virtual);
	printf("vyres:\t%d\n", varscinfo.yres_virtual);
	printf("xoff:\t%d\n", varscinfo.xoffset);
	printf("yoff:\t%d\n", varscinfo.yoffset);
	printf("line:\t%d\n", fixscinfo.line_length);
	printf("memlen:\t%d\n", fixscinfo.smem_len);

	unsigned char *fbmem = mmap(NULL, fixscinfo.smem_len, PROT_WRITE, MAP_SHARED, fbfd, 0);
	unsigned char *tmpbuf = malloc(fixscinfo.smem_len);


	struct mouse_data prev_mdata;
	while (1) {
		audio_play_context(&pbctx);

		empty(fbdata, tmpbuf, (struct color) {
			.r = 0,
			.g = 0,
			.b = 0,
			.a = 0
		});

		for (double angle_c = 0; angle_c < 1-0.01; angle_c += 1.0/16) {
			double angle = 2 * M_PI * angle_c;
			double s = sin(angle) * 200, c = cos(angle) * 200;
			draw_line(
				fbdata,
				tmpbuf,
				(struct fb_pos) { .x = 400, .y = 400 },
				(struct fb_pos) { .x = 400 + c, .y = 400 + s },
				(struct color) { .r = 255, .g = 255, .b = 255, .a = 0 }
			);

		}

		struct mouse_data mdata = read_mouse(mice);
		if (!(mdata.dx || mdata.dy || mdata.buttons)) mdata = prev_mdata;
		else prev_mdata = mdata;

		struct color delta_color = (struct color) {
			.r = mdata.buttons & 1 ? 255 : 128,
			.g = mdata.buttons & 2 ? 255 : 128,
			.b = mdata.buttons & 4 ? 255 : 128
		};

		draw_line(
			fbdata,
			tmpbuf,
			(struct fb_pos) {
				.x = fbdata.w/2,
				.y = fbdata.h/2
			},
			(struct fb_pos) {
				.x = fbdata.w/2 + mdata.dx * 10,
				.y = fbdata.h/2 - mdata.dy * 10
			},
			delta_color
		);

		memcpy(fbmem, tmpbuf, fixscinfo.smem_len);
	}
}

