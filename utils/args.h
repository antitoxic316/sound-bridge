#ifndef _ARGS_H_
#define _ARGS_H_

#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>

struct server_config {
  char *cli_hostname;
  char *port;
};

extern struct server_config server_conf;

#define SERVER_LONG_OPTS_NUM 2
struct option server_long_opts[SERVER_LONG_OPTS_NUM + 1] = {
  {
    .name = "cli-hostname",
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
  {
    0,0,0,0  
  }
};
const char *server_optstring = "h0";



struct client_config {
  char *port;
};
extern struct client_config client_conf;

#define CLIENT_LONG_OPTS_NUM 1
struct option client_long_opts[CLIENT_LONG_OPTS_NUM + 1] = {
  {
    .name = "port",
    .has_arg = required_argument,
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