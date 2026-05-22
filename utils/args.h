#ifndef _ARGS_H_
#define _ARGS_H_

#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>

#include "sound.h" //for format definitions

struct server_config {
  char *hostname;
  char *port;
  char *alsa_sink;
  uint32_t alsa_channels_n;
  uint32_t alsa_rate;
  snd_pcm_uframes_t alsa_period_time;
  snd_pcm_format_t alsa_fmt;
  uint32_t alsa_fmtsize;
};

extern struct server_config server_conf;

#define SERVER_LONG_OPTS_NUM 7
struct option server_long_opts[SERVER_LONG_OPTS_NUM + 1] = {
  {
    .name = "hostname",
    .has_arg = required_argument,
    .flag = 0,
    .val = 0
  },
  {
    .name = "port",
    .has_arg = required_argument,
    .flag = 0,
    .val = 0
  },
  //alsa args
  {
    .name = "alsa-sink",
    .has_arg = optional_argument,
    .flag = 0,
    .val = 0
  },
  {
    .name = "alsa-rate",
    .has_arg = optional_argument,
    .flag = 0,
    .val = 0
  },
  {
    .name = "alsa-period-time",
    .has_arg = optional_argument,
    .flag = 0,
    .val = 0
  },
  {
    .name = "alsa-channels",
    .has_arg = optional_argument,
    .flag = 0,
    .val = 0
  },
  {
    .name = "alsa-format",
    .has_arg = optional_argument,
    .flag = 0,
    .val = 0
  },
  {
    0,0,0,0  
  }
};
const char *server_optstring = "h0";



struct client_config {
  char *port;
  char *alsa_sink;
  uint32_t alsa_channels_n;
  uint32_t alsa_rate;
  snd_pcm_uframes_t alsa_period_time;
  snd_pcm_format_t alsa_fmt;
  uint32_t alsa_fmtsize;
};
extern struct client_config client_conf;

#define CLIENT_LONG_OPTS_NUM 6
struct option client_long_opts[CLIENT_LONG_OPTS_NUM + 1] = {
  {
    .name = "port",
    .has_arg = required_argument,
    .flag = 0,
    .val = 0
  },
  //alsa args
  {
    .name = "alsa-sink",
    .has_arg = optional_argument,
    .flag = 0,
    .val = 0
  },
  {
    .name = "alsa-rate",
    .has_arg = optional_argument,
    .flag = 0,
    .val = 0
  },
  {
    .name = "alsa-period-time",
    .has_arg = optional_argument,
    .flag = 0,
    .val = 0
  },
  {
    .name = "alsa-channels",
    .has_arg = optional_argument,
    .flag = 0,
    .val = 0
  },
  {
    .name = "alsa-format",
    .has_arg = optional_argument,
    .flag = 0,
    .val = 0
  },
  {
    0,0,0,0  
  }
};
const char *client_optstring = "h0";


//non zero on error
int initialize_server_from_main(int argc, char *argv[]);
//non zero on error
int initialize_client_from_main(int argc, char *argv[]);

#endif