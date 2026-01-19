/*
 * Copyright 2026 Çınar Karaaslan
 * Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted, provided that the above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED “AS IS” AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "kb.h"

//At most 32 at a time
#define MAX_EVENTS 32

struct input_event {
	struct timeval time;
	unsigned short type;
	unsigned short code;
	int value;
};

struct kbevent *process_kbevents(int fd) {
	struct input_event buf[MAX_EVENTS];
	int nread = read(fd, buf, sizeof (buf));
	if (nread == -1 || !nread) return NULL;

	size_t inpsz = sizeof (struct input_event);
	while ((nread % inpsz) != 0) {
		size_t left = inpsz - nread % inpsz;
		nread += read(fd, ((char *) buf) + nread, left);
	}

	int n_events = nread / inpsz;
	struct kbevent *event = NULL;
	for (int i = n_events - 1; i >= 0; --i) {
		struct input_event *ievent = buf + i;
		if (ievent->value == 2) continue; //ignore key repeat events

		struct kbevent *newevent = malloc(sizeof (struct kbevent));
		newevent->key = ievent->code;
		newevent->type = ievent->value; //1 is press, 0 is release
		newevent->next = event;
		event = newevent;
	}

	return event;
}
