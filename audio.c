#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sound/asound.h>
#include "audio.h"

//The ALSA driver exposes its interface through the files /dev/snd/pcmCxDyz
////x is the card number, y is the device number, z is "p" for playback and z is "c" for capture (recording)
//In principle, you just open the file and write the samples to it
//But we need to set configuration details like the number of samples
//This is done by setting up two structures:
// - snd_pcm_hw_params,
// - snd_pcm_sw_params
//(See sound/asound.h on your system)

//I failed setting up mono audio playback, so we have to use stereo, i.e. 2 channels
//ALSA takes the input channel-interleaved

#define SET_MASK(maskstruct, bit) ( \
		(maskstruct).bits[0] = 0, \
		(maskstruct).bits[1] = 0, \
		(maskstruct).bits[(bit)>>5] |= (1 << ((bit) & 0x1F)) \
	)

#define MSKIDX(x) ((x) - SNDRV_PCM_HW_PARAM_FIRST_MASK)
#define INTIDX(x) ((x) - SNDRV_PCM_HW_PARAM_FIRST_INTERVAL)

int audio_init(unsigned int card, unsigned int device, int flags) {
	char filenm[128];
	snprintf(filenm, sizeof (filenm), DEVFILE_FORMAT_S, card, device);

	int fd;
	if ((fd = open(filenm, O_RDWR | flags)) == -1) return -1;

	struct snd_pcm_hw_params hparams;
	memset(&hparams, 0, sizeof (hparams));

	//Not sure why we are doing this, TODO - my guess is that setting the first to two all ones is a request for a default
	for (int i = SNDRV_PCM_HW_PARAM_FIRST_MASK; i <= SNDRV_PCM_HW_PARAM_LAST_MASK; ++i)
		hparams.masks[MSKIDX(i)].bits[0] = ~0U,
		hparams.masks[MSKIDX(i)].bits[1] = ~0U;

	//Reset intervals to default
	for (int i = SNDRV_PCM_HW_PARAM_FIRST_INTERVAL; i <= SNDRV_PCM_HW_PARAM_LAST_INTERVAL; ++i)
		hparams.intervals[INTIDX(i)].min = 0U,
		hparams.intervals[INTIDX(i)].max = ~0U;

	hparams.rmask = ~0U; //If a bit is zero, the corresponding parameter is not requested to change. If one, the kernel may "refine" the parameter
	hparams.cmask = 0; //Filled by the kernel to indicate what it has changed as part of its "refinement"
	hparams.info = ~0U;

	SET_MASK(hparams.masks[MSKIDX(SNDRV_PCM_HW_PARAM_ACCESS)], SNDRV_PCM_ACCESS_RW_INTERLEAVED);
	SET_MASK(hparams.masks[MSKIDX(SNDRV_PCM_HW_PARAM_FORMAT)], SNDRV_PCM_FORMAT_S16_LE); //Samples are signed 16 bit little endian

	hparams.intervals[INTIDX(SNDRV_PCM_HW_PARAM_PERIOD_SIZE)] = (struct snd_interval) { .min = 1024, .max = ~0U }; //yet to have understood what this exactly is; TODO (but see comment below)
	hparams.intervals[INTIDX(SNDRV_PCM_HW_PARAM_CHANNELS)] = (struct snd_interval) { .min = 2, .max = 2, .integer = 1 }; //TODO: Linux gives EINVAL when min=max=1; figure out why
	hparams.intervals[INTIDX(SNDRV_PCM_HW_PARAM_PERIODS)] = (struct snd_interval) { .min = 4, .max = 4, .integer = 1 }; //TODO
	hparams.intervals[INTIDX(SNDRV_PCM_HW_PARAM_RATE)] = (struct snd_interval) { .min = SAMPLE_RATE, .max = SAMPLE_RATE, .integer = 1 }; //sampling rate
	//About PERIOD_SIZE and PERIODS, it seems that there is a mechanism for periodic wakeup where the driver causes software interrupts in periods. I'm completely unsure though, so don't count on that.

	if (ioctl(fd, SNDRV_PCM_IOCTL_HW_PARAMS, &hparams) == -1) { close(fd); return -1; }

	struct snd_pcm_sw_params sparams;
	memset(&sparams, 0, sizeof (sparams));

	//TODO
	sparams.tstamp_mode = SNDRV_PCM_TSTAMP_ENABLE;
	sparams.period_step = 1;
	sparams.avail_min = 1024;
	sparams.start_threshold = 1024 * 4;
	sparams.stop_threshold = 1024 * 4;
	//sparams.xfer_align is obsolete
	sparams.silence_threshold = 0;
	sparams.silence_size = 0;

	if (ioctl(fd, SNDRV_PCM_IOCTL_SW_PARAMS, &sparams) == -1) { close(fd); return -1; }
	if (ioctl(fd, SNDRV_PCM_IOCTL_PREPARE) == -1) { close(fd); return -1; }

	return fd;
}

//A frame is essentially a single block of samples per each channel.
//Therefore a frame, in our case, is n_channels*sample_sz = 2*16 = 32 bits.
int audio_render(int fd, int16_t *buf, long nframes) {
	struct snd_xferi sx = (struct snd_xferi) {
		.buf = (void *) buf,
		.frames = nframes,
		.result = 0
	};

	if (ioctl(fd, SNDRV_PCM_IOCTL_WRITEI_FRAMES, &sx) == -1) return -1;

	return (int) sx.result;
}
