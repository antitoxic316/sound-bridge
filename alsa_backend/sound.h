#ifndef _SOUND_H_
#define _SOUND_H_

#include <stdint.h>
#include <stddef.h>

#include <alsa/asoundlib.h>

#define DEBUG 1

struct alsa_info {
  char* sink_name;
	uint32_t channels_n;
  uint32_t fmt_size;
  snd_pcm_uframes_t period_time; //us
  snd_pcm_uframes_t buffer_time; //us
  snd_pcm_format_t format;
  snd_pcm_access_t access_mode;
  uint32_t rate;
};

extern struct alsa_info alsa_dev;

snd_pcm_t *init_capture_handle();
snd_pcm_t *init_playback_handle();
size_t capture_data(struct alsa_info alsa_dev, snd_pcm_t *handle, uint8_t *buf, size_t frames_to_capture);
size_t playback_data(struct alsa_info alsa_dev, snd_pcm_t *handle, uint8_t *buf, size_t frames_to_playback);


#endif