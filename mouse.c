/*
 * Copyright 2026 Çınar Karaaslan
 * Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted, provided that the above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED “AS IS” AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

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
