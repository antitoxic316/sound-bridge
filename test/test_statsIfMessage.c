#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <statistic-if.h>

int main(){
  uint8_t *msg = malloc(sizeof(uint16_t));
  msg = __priv_stats_if_eventmsg_new(NET_RECV);
  if(msg[0] != 0 || msg[1] != 0x20){
    printf("net recv header test failed\n");
    exit(-1);
  }
  free(msg);

  msg = __priv_stats_if_eventmsg_new(NET_SENT);
  if(msg[0] != 0 || msg[1] != 0){
    printf("net recv header test failed\n");
    exit(-1);
  }
  free(msg);

  int n = 128;
  uint8_t *snd_data = calloc(n, 1);
  memset(snd_data, 5, n);
  msg = __priv_stats_if_datamsg_new(SND_DATA, snd_data, n);

  uint32_t header = (msg[0] << 24) | (msg[1] << 16) | (msg[2] << 8) | (msg[3]);
  if(header != 0xD0){
    printf("snd data header not rigth\n");
    exit(-1);
  }
  for(int i = sizeof(header); i < sizeof(uint32_t) + n; i++){
    if(msg[i] != 5){
      printf("data test failed");
      exit(-1);
    }
  }

  free(msg);

  return 0;
}