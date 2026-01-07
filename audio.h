#ifndef AUDIO_H
#define AUDIO_H

#include <stdint.h>

#define DEVFILE_FORMAT_S "/dev/snd/pcmC%uD%up"
#define SAMPLE_RATE 48000

int audio_init(unsigned int card, unsigned int device, int flags);
int audio_render(int fd, int16_t *buf, long nframes);

struct audio_context {
	int fd;
	long nframes_left;
	unsigned long max_playback;
	int16_t *buf;
};

//Useful when fd is opened in nonblocking mode (i.e., audio_init with flags including O_NONBLOCK)
static inline void audio_play_context(struct audio_context *context) {
	long frames = context->nframes_left;
	unsigned long max_pb = context->max_playback;
	if (frames <= 0) return;

	long pb_frames = (frames > max_pb) ? max_pb : frames;

	int res = audio_render(context->fd, context->buf, pb_frames);
	if (res == -1) return;

	context->nframes_left -= res;
	context->buf += res * 2;
}

#endif
