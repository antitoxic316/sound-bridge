#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "statistic-if.h"

int stats_if_init(struct stats_if *sif){
  int err;

  err = mkdir(TMP_DIR, 0755);
  if(err && errno != EEXIST){
    perror("mkdir");
    return -1;
  }

  sif->sockfd = socket(PF_UNIX, SOCK_DGRAM, 0);
  if(sif->sockfd < 0){
    perror("socket");
    return -1;
  }

  memset(&sif->addr, 0, sizeof(sif->addr));
  sif->addr.sun_family = AF_UNIX;
  int r = snprintf(sif->addr.sun_path, sizeof(sif->addr.sun_path), "%s%d.%s", TMP_DIR, getpid(), SOCK_FILE);
  if(r >= sizeof(sif->addr.sun_path) || r < 0){
    perror("sprintf");
    close(sif->sockfd);
    return -1;
  }

  err = bind(sif->sockfd, (struct sockaddr*)&sif->addr, sizeof(sif->addr));
  if(err == -1){
    perror("bind");
    printf("ffffffff\n");
    printf("%s\n", sif->addr.sun_path);
    close(sif->sockfd);
    return -1;
  }

  err = connect(sif->sockfd, (struct sockaddr *)&sif->addr, sizeof(sif->addr));
  if(err == -1) {
		perror("connect");
    close(sif->sockfd);
		return -1;
	}

  return 0;
}

void stats_if_exit(struct stats_if *sif){
  close(sif->sockfd);
  unlink(sif->addr.sun_path);
}

void stats_if_send_data(struct stats_if *sif, StatsEvent ev, uint8_t *buff, uint16_t n){
  uint8_t *msg = __priv_stats_if_datamsg_new(ev, buff, n);

  int r = sendto(sif->sockfd, msg, sizeof(uint32_t) + n, 0, \
                 (struct sockaddr*)&sif->addr, sizeof(sif->addr));
  if(r == -1){
    perror("sendto");
  }

  free(msg);
}

void stats_if_send_event(struct stats_if *sif, StatsEvent ev){
  uint8_t *msg = __priv_stats_if_eventmsg_new(ev);

  int r = sendto(sif->sockfd, msg, sizeof(uint16_t), 0, \
                 (struct sockaddr*)&sif->addr, sizeof(sif->addr));
  if(r == -1){
    perror("sendto");
  }

  free(msg);
}


uint8_t *__priv_stats_if_datamsg_new(StatsEvent ev, uint8_t *buff, uint16_t n){
  uint32_t snd_bit;
  uint32_t data_bit;
  uint32_t ev_type;
  uint32_t header = 0;

  uint8_t *msg = malloc(sizeof(header) + n);

  switch (ev)
  {
  case SND_DATA:
    snd_bit = 1 << 31;
    break;
  default:
    printf("not recoknized ev type %d\n", ev); //TODO dbg
    snd_bit = 0;
  }

  data_bit = 1 << 30;
  ev_type = ev << 29;

  uint32_t data_size = n << 21;

  header |= data_size;
  header |= ev_type;
  header |= data_bit;
  header |= snd_bit;

  memcpy(msg, &header, sizeof(header));
  memcpy(msg+sizeof(header), buff, n);

  return msg;
}


uint8_t *__priv_stats_if_eventmsg_new(StatsEvent ev){
  uint16_t snd_bit;
  uint16_t data_bit;
  uint16_t ev_type;
  uint16_t header = 0;
  uint8_t *msg = malloc(sizeof(header));

  switch (ev)
  {
  case NET_RECV:
  case NET_SENT:
    snd_bit = 0;
    break;
  default:
    printf("not recoknized ev type %d\n", ev); //TODO dbg
    snd_bit = 0;
  }

  data_bit = 0;
  ev_type = ev << 13;

  header |= ev_type;
  header |= data_bit;
  header |= snd_bit;

  memcpy(msg, &header, sizeof(header));

  return msg;
}
