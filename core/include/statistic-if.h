#ifndef _STATISTIC_IF_H_
#define _STATISTIC_IF_H_

#include <sys/socket.h>
#include <sys/un.h>
#include <stdint.h>

#define TMP_DIR "/tmp/virtsb/"
#define SOCK_FILE "stats.sock"

//TODO make checks for that
#define MAX_DATA_SIZE 1024

struct stats_if {
  int sockfd;
  struct sockaddr_un addr;
};

//up to 255 events
typedef enum {NET_SENT, NET_RECV, SND_DATA} StatsEvent;

int stats_if_init(struct stats_if *sif);
void stats_if_send_data(struct stats_if *sif, StatsEvent ev, uint8_t *buff, uint16_t n);
void stats_if_send_event(struct stats_if *sif, StatsEvent ev);
void stats_if_exit(struct stats_if *sif);

uint8_t *__priv_stats_if_datamsg_new(StatsEvent ev, uint8_t *buff, uint16_t n);
uint8_t *__priv_stats_if_eventmsg_new(StatsEvent ev);

#endif