#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>

#include "args.h"

struct server_config server_conf = {
  .hostname = "localhost",
  .port = "55555",
  .alsa_channels_n = 1,
  .alsa_fmt = SND_PCM_FORMAT_S16,
  .alsa_fmtsize = 2,
  .alsa_sink = "default",
  .alsa_rate = 44100,
  .alsa_channels_n = 1,
  .alsa_period_time = 256,
};


struct client_config client_conf = {
  .port = "55555",
  .alsa_channels_n = 1,
  .alsa_fmt = SND_PCM_FORMAT_S16,
  .alsa_fmtsize = 2,
  .alsa_sink = "default",
  .alsa_rate = 44100,
  .alsa_channels_n = 1,
  .alsa_period_time = 256,
};


static int update_server_conf(const char *opt_name, const char *arg){
  if(!strcmp(opt_name, "cli-hostname")) {
    server_conf.hostname = calloc(1, strlen(arg)+1);
    strcpy(server_conf.hostname, arg);
  } else if (!strcmp(opt_name, "port")) {
    server_conf.port = calloc(1, strlen(arg)+1);
    strcpy(server_conf.port, arg);
  } else if (!strcmp(opt_name, "alsa-sink")) {
    server_conf.alsa_sink = calloc(1, strlen(arg)+1);
    strcpy(server_conf.alsa_sink, arg);
  } else if (!strcmp(opt_name, "alsa-rate")) {
    char *end;
    long r = strtol(arg, &end, 10);
    if(*end != '\0' || r < 0 || r > UINT32_MAX){
      printf("rate: %s not supported\n", arg);
      return 1;
    }
    server_conf.alsa_rate = r;
  } else if (!strcmp(opt_name, "alsa-period-time")) {
    char *end;
    uint64_t r = strtoul(arg, &end, 10);
    if(*end != '\0'){
      printf("period time: %s not supported\n", arg);
      return 1;
    }
    server_conf.alsa_period_time = r;
  } else if (!strcmp(opt_name, "alsa-channels")) {
    char *end;
    long r = strtol(arg, &end, 10);
    if(*end != '\0' || r < 0 || r > UINT32_MAX){
      printf("channel num: %s not supported\n", arg);
      return 1;
    }
    server_conf.alsa_channels_n = r;
  } else if (!strcmp(opt_name, "alsa-format")) {
    if(!strcmp(arg, "s16_le")){
      server_conf.alsa_fmt = SND_PCM_FORMAT_S16_LE;
      server_conf.alsa_fmtsize = 2;
    } else {
      printf("format %s not allowed\n", arg);
    }
    //TODO more formats
  } else {
    printf("option  %s  not supported\n", opt_name);
    return 1;
  }

  return 0;
}

static void server_print_help(){
  printf("Usage sb-server [--opt]\n");
  printf("available oprions:\n");
  for(int i = 0; i < SERVER_LONG_OPTS_NUM; i++){
    printf("\r--%s\n", server_long_opts[i].name);
  }
  printf("-h to print this menu\n");
}

//non zero on error
int initialize_server_from_main(int argc, char *argv[]){
  int c = 0;

  while (1){
    int opt_i = 0;
    c = getopt_long(argc, argv, server_optstring, server_long_opts, &opt_i);
    if(c == -1){
      if(opt_i != 0)
        printf("no options specified, using the default config\n");
      return 0; // all options good
    }

    switch (c) {
      case 0:
        c = update_server_conf(server_long_opts[opt_i].name, optarg);
        if(c) return c; // option not supported - abort
        break;
      case 'h':
        server_print_help();
        return 'h'; // print help. then abort
      default:
        printf("?? getopt returned character code 0%o ??\n", c);
        return c; // not supported option - abort
    }
  }
  
  return c;
}



static int update_client_conf(const char *opt_name, const char *arg){
  if (!strcmp(opt_name, "port")) {
    client_conf.port = calloc(1, strlen(arg)+1);
    strcpy(client_conf.port, arg);
  } else if (!strcmp(opt_name, "alsa-sink")) {
    client_conf.alsa_sink = calloc(1, strlen(arg)+1);
    strcpy(server_conf.alsa_sink, arg);
  } else if (!strcmp(opt_name, "alsa-rate")) {
    char *end;
    long r = strtol(arg, &end, 10);
    if(*end != '\0' || r < 0 || r > UINT32_MAX){
      printf("rate: %s not supported\n", arg);
      return 1;
    }
    client_conf.alsa_rate = r;
  } else if (!strcmp(opt_name, "alsa-period-time")) {
    char *end;
    uint64_t r = strtoul(arg, &end, 10);
    if(*end != '\0'){
      printf("period time: %s not supported\n", arg);
      return 1;
    }
    client_conf.alsa_period_time = r;
  } else if (!strcmp(opt_name, "alsa-channels")) {
    char *end;
    long r = strtol(arg, &end, 10);
    if(*end != '\0' || r < 0 || r > UINT32_MAX){
      printf("channel num: %s not supported\n", arg);
      return 1;
    }
    client_conf.alsa_channels_n = r;
  } else if (!strcmp(opt_name, "alsa-format")) {
    if(!strcmp(arg, "s16_le")){
      client_conf.alsa_fmt = SND_PCM_FORMAT_S16_LE;
      client_conf.alsa_fmtsize = 2;
    } else {
      printf("format %s not allowed\n", arg);
    }
    //TODO more formats
  } else {
    printf("option  %s  not supported\n", opt_name);
    return 1;
  }

  return 0;
}

static void client_print_help(){
  printf("Usage sb-client [--opt]\n");
  printf("available oprions:\n");
  for(int i = 0; i < CLIENT_LONG_OPTS_NUM; i++){
    printf("\r--%s\n", client_long_opts[i].name);
  }
  printf("-h to print this menu\n");
}

//non zero on error
int initialize_client_from_main(int argc, char *argv[]){
  int c = 0;

  while (1){
    int opt_i = 0;
    c = getopt_long(argc, argv, client_optstring, client_long_opts, &opt_i);
    if(c == -1){
      if(opt_i != 0)
        printf("no options, using the default config\n");
      return 0; // all options good
    }

    switch (c) {
      case 0:
        c = update_client_conf(client_long_opts[opt_i].name, optarg);
        if(c) return c; // option not supported - abort
        break;
      case 'h':
        client_print_help();
        return 'h'; // print help. then abort
      default:
        printf("?? getopt returned character code 0%o ??\n", c);
        return c; // not supported option - abort
    }
  }
  
  return c;
}
