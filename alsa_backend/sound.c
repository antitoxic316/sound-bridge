#include <stdio.h>
#include <signal.h>
#include <stdbool.h>
#include <alloca.h>
#include <stdint.h>

#include <alsa/asoundlib.h>

#include "sound.h"

struct alsa_info alsa_dev = {
  .sink_name = "default",
  .channels_n = 1,
  .fmt_size = 2,
  .period_time = 10000,
  .buffer_time = 4194304,
  .format = SND_PCM_FORMAT_S16_LE,
	.access_mode = SND_PCM_ACCESS_RW_INTERLEAVED,
  .rate = 44100
};

static int xrun_recovery(snd_pcm_t *handle, int err)
{
    if (err == -EPIPE) {    /* under-run */
        err = snd_pcm_prepare(handle);
        if (err < 0)
            printf("Can't recovery from underrun, prepare failed: %s\n", snd_strerror(err));
        return 0;
    } else if (err == -ESTRPIPE) {
        while ((err = snd_pcm_resume(handle)) == -EAGAIN)
            sleep(1);   /* wait until the suspend flag is released */
        if (err < 0) {
            err = snd_pcm_prepare(handle);
            if (err < 0)
                printf("Can't recovery from suspend, prepare failed: %s\n", snd_strerror(err));
        }
        return 0;
    }
    return err;
}

static snd_pcm_t *init_handle_generic(snd_pcm_t *handle){
	snd_pcm_sw_params_t *sw_params = NULL;
	snd_pcm_hw_params_t *hw_params = NULL;

	snd_pcm_sw_params_malloc(&sw_params);
	snd_pcm_sw_params_current(handle, sw_params);

	snd_pcm_hw_params_alloca(&hw_params);

	if (snd_pcm_hw_params_any(handle, hw_params) < 0) {
		printf("Failed to retrieve HW params\n");
		goto handle_init_err_cleanup;
	}


	int error = 0;
	if ((error = snd_pcm_hw_params_set_access(handle, hw_params, alsa_dev.access_mode)) < 0) {
		printf("ERROR: Can't set interleaved mode. %s\n", snd_strerror(error));
		goto handle_init_err_cleanup;
	}

	if ((error = snd_pcm_hw_params_set_format(handle, hw_params, alsa_dev.format)) < 0) {
		printf("ERROR: Can't set format. %s\n", snd_strerror(error));
		goto handle_init_err_cleanup;
	}

	if ((error = snd_pcm_hw_params_set_channels(handle, hw_params, alsa_dev.channels_n)) < 0) {
		printf("ERROR: Can't set channels number. %s\n", snd_strerror(error));
		goto handle_init_err_cleanup;
	}
	if ((error = snd_pcm_hw_params_set_rate_near(handle, hw_params, &alsa_dev.rate, 0)) < 0) {
		printf("ERROR: Can't set rate. %s\n", snd_strerror(error));
		goto handle_init_err_cleanup;
	}
	if ((error = snd_pcm_hw_params_set_period_size_near(handle, hw_params, &alsa_dev.period_time, 0)) < 0) {
		printf("Error: Can't set period size\n");
	  goto handle_init_err_cleanup;
	}

	if ((error = snd_pcm_hw_params(handle, hw_params)) < 0) {
		printf("Failed to set HW params\n");
		goto handle_init_err_cleanup;
	}

	return handle;

handle_init_err_cleanup:
	snd_pcm_close(handle);
	exit(error);
}

snd_pcm_t *init_capture_handle(){
	snd_pcm_t *handle = NULL;

	int error = 0;
	error = snd_pcm_open(&handle, alsa_dev.sink_name, SND_PCM_STREAM_CAPTURE, 0);
	if (error < 0) {
		printf("Failed to open PCM device %s\n", snd_strerror(error));
		snd_pcm_close(handle);
		exit(error);
	}

	handle = init_handle_generic(handle);
	
	printf("Capture setup succesful\n");
  return handle;
}

snd_pcm_t *init_playback_handle(){
	snd_pcm_t *handle = NULL;

	int error = 0;
	error = snd_pcm_open(&handle, alsa_dev.sink_name, SND_PCM_STREAM_PLAYBACK, 0);
	if(error < 0){		
		printf("Failed to open PCM device %s\n", snd_strerror(error));
		snd_pcm_close(handle);
		exit(error);
	}

	handle = init_handle_generic(handle);

	printf("playback handle inited\n");
	return handle;
}

//returns numbers of bytes read
size_t capture_data(struct alsa_info *alsa_dev, snd_pcm_t *handle, uint8_t *buf, size_t frames_to_read){
	/* Note: ALSA Reads/Writes in number of samples! */
	int frames_read = snd_pcm_readi(handle, buf, frames_to_read);
	if (frames_read < 0) {
		/* Recover the ALSA internal state if an error occurse */
		int recover = xrun_recovery(handle, frames_read);
		if (recover < 0) {
			printf("Failed to recover ALSA state\n");
			exit(2);
		}
		printf("ALSA State recoverd\n");
	}

	return frames_read * alsa_dev->fmt_size * alsa_dev->channels_n;
}

size_t playback_data(struct alsa_info *alsa_dev, snd_pcm_t *handle, uint8_t *buf, size_t frames_to_playback){
	int written = snd_pcm_writei(handle, (void *)buf, frames_to_playback); // 16bit signet format

	if(written < 0){ 
		int recover = xrun_recovery(handle, written);
		if (recover < 0) {
			printf("Failed to recover ALSA state\n");
			exit(2);
		}
		printf("ALSA State recoverd\n");
		written = 0;
	}

	return written * alsa_dev->fmt_size * alsa_dev->channels_n;
}
