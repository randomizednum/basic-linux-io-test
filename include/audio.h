/*
 * Copyright 2026 Çınar Karaaslan
 * Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted, provided that the above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED “AS IS” AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef AUDIO_H
#define AUDIO_H

#include <stdint.h>

#define DEVFILE_FORMAT_S "/dev/snd/pcmC%uD%up"
#define SAMPLE_RATE 48000

int audio_init(unsigned int card, unsigned int device, int flags);
int audio_render(int fd, int16_t *buf, long nframes);

//this thing is getting messy, TODO
struct audio_context {
	int fd;
	long nframes_left;
	unsigned long max_playback;
	int16_t *return_back;
	int16_t *buf;
};

//Useful when fd is opened in nonblocking mode (i.e., audio_init with flags including O_NONBLOCK)
static inline void audio_play_context(struct audio_context *context) {
	long frames = context->nframes_left;
	unsigned long max_pb = context->max_playback;
	if (frames <= 0) {
		if (context->return_back) {
			context->nframes_left = (context->buf - context->return_back) / 2;
			context->buf = context->return_back;
			audio_play_context(context);
		}
		return;
	}

	long pb_frames = (frames > max_pb) ? max_pb : frames;

	int res = audio_render(context->fd, context->buf, pb_frames);
	if (res == -1) return;

	context->nframes_left -= res;
	context->buf += res * 2;
}

#endif
