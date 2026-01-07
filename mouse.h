#ifndef MOUSE_H
#define MOUSE_H

#define BUTTON_MIDLE 0x04
#define BUTTON_RIGHT 0x02
#define BUTTON_LEFT  0x01

struct mouse_data {
	int dx;
	int dy;
	unsigned char buttons;
};

struct mouse_data read_mouse(int fd);

#endif
