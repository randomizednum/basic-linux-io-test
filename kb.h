#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

struct kbevent {
	uint8_t type;
	uint8_t key;
	struct kbevent *next;
};

struct kbevent *process_kbevents(int kbfd);

#endif
