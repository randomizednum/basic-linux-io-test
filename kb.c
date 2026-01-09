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
