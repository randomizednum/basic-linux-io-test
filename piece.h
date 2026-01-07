#ifndef PIECE_H
#define PIECE_H

#include <math.h>
#include <stdint.h>

#ifndef SAMPLE_RATE
#error "SAMPLE_RATE needs to be defined before including piece.h"
#endif

#define C 523.2511306012
#define CII 554.36526195374
#define D 587.32953583482
#define DII 622.25396744416
#define E 659.25511382574
#define F 698.45646286601
#define FII 739.98884542327
#define G 783.9908719635
#define GII 830.60939515989
#define A 880
#define AII 932.32752303618
#define B 987.76660251225

#define Db CII
#define Eb DII
#define Gb FII
#define Ab GII
#define Bb AII

#define REST 0.0

#define GET_PIECE_BUF_FRAMES(n_notes, samples_per_note) ((n_notes) * (samples_per_note))
#define GET_PIECE_BUF_SIZE(n_notes, samples_per_note) (GET_PIECE_BUF_FRAMES(n_notes, samples_per_note) * 2)

static inline void compile(double *notes, int n_notes, int16_t *buf, int samples_per_note) {
	for (int i = 0; i < n_notes; ++i) {
		double note = notes[i];
		if (note == REST) { buf += samples_per_note * 2; continue; }

		for (int j = 0; j < samples_per_note; ++j) {
			double v = sin((2 * M_PI) * j / SAMPLE_RATE * note);
			*buf++ += v * 1024;
			*buf++ += v * 1024;
		}
	}
}

#endif
