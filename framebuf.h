#ifndef FRAMEBUF_H
#define FRAMEBUF_H
//Header-only library for frame buffer processing
//Assumes a lot of stuff

#include <stdint.h>
#include <linux/fb.h>

struct fb_pos {
	uint16_t x;
	uint16_t y;
};

struct color {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
};

struct fbdata {
	unsigned int line_len;
	unsigned int w;
	unsigned int h;
};

static inline struct fbdata get_fbdata(struct fb_var_screeninfo *vsc, struct fb_fix_screeninfo *fsc) {
	return (struct fbdata) {
		.line_len = fsc->line_length / 4,
		.w = vsc->xres,
		.h = vsc->yres
	};
}

#define OUT_RANGE(data, pos) ((pos).y >= (data).h || (pos).x >= (data).w)

static inline void draw_point(struct fbdata data, unsigned char *buf, struct fb_pos pos, struct color c) {
	if (OUT_RANGE(data, pos)) return;
	unsigned char *p = buf + (data.line_len * pos.y + pos.x) * 4;
	p[0] = c.b;
	p[1] = c.g;
	p[2] = c.r;
	p[3] = c.a;
}

static inline void empty(struct fbdata data, unsigned char *buf, struct color c) {
	struct fb_pos pos;
	for (pos.y = 0; pos.y < data.h; ++pos.y)
		for (pos.x = 0; pos.x < data.w; ++pos.x)
			draw_point(data, buf, pos, c);
}

//Naive line drawing
static inline void draw_steep_line(struct fbdata data, unsigned char *buf, struct fb_pos p1, struct fb_pos p2, struct color c) {
	double slope = (double) (p2.x - p1.x) / (p2.y - p1.y);
	for (int y = p1.y; y <= p2.y; ++y) {
		int x = p1.x + (int) ((y - p1.y) * slope + 0.5);
		draw_point(data, buf, (struct fb_pos) { .x = x, .y = y }, c);
	}
}

static inline void draw_flat_line(struct fbdata data, unsigned char *buf, struct fb_pos p1, struct fb_pos p2, struct color c) {
	double slope = (double) (p2.y - p1.y) / (p2.x - p1.x);
	for (int x = p1.x; x <= p2.x; ++x) {
		int y = p1.y + (int) ((x - p1.x) * slope + 0.5);
		draw_point(data, buf, (struct fb_pos) { .x = x, .y = y }, c);
	}

}

#define ABS(x) ((x) > 0 ? (x) : -(x))

static inline void draw_line(struct fbdata data, unsigned char *buf, struct fb_pos p1, struct fb_pos p2, struct color c) {
	if (OUT_RANGE(data, p1) || OUT_RANGE(data, p2)) return;
	if (ABS(p1.x - p2.x) > ABS(p1.y - p2.y)) {
		if (p1.x < p2.x) return draw_flat_line(data, buf, p1, p2, c);
		return draw_flat_line(data, buf, p2, p1, c);
	} else {
		if (p1.y < p2.y) return draw_steep_line(data, buf, p1, p2, c);
		return draw_steep_line(data, buf, p2, p1, c);
	}
}

#endif
