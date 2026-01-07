#include <unistd.h>
#include "mouse.h"

struct mouse_data read_mouse(int fd) {
	struct mouse_data data = (struct mouse_data) {
		.dx = 0,
		.dy = 0,
		.buttons = 0
	};

	unsigned char buf[3];
	int nread = read(fd, buf, 3);
	if (!nread || nread == -1) return data;

	while (nread < 3) {
		int retval = read(fd, buf + nread, 3 - nread);
		if (retval != -1) nread += retval;
	}

	data.buttons = buf[0] & 0x07;
	data.dx = buf[1];
	data.dy = buf[2];

	if (buf[0] & 0x10) data.dx -= 0x100;
	if (buf[0] & 0x20) data.dy -= 0x100;

	return data;
}
