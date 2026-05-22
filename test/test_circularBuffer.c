#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sharedbuffer.h>


int main(){
  struct shared_buffer *shbuff;

  shbuff = shared_buffer_init(11);

  shared_buffer_write_bulk(shbuff, (uint8_t*)"fff", 3, 1);
  if(memcmp(shbuff->buffer, "fff\0\0\0\0\0\0\0\0", 11) || 
     shbuff->tail != 0 || shbuff->head != 3){
    printf("write test failed\n");
    for(int i = 0; i < 11; i++){
      printf("%d  ", shbuff->buffer[i]);
    }
    printf("\n");
    printf("tail: %d  head %d\n", shbuff->tail, shbuff->head);

    exit(-1);
  }

  char tst_buff[11];
  shared_buffer_read_bulk(shbuff, (uint8_t*)tst_buff, 3, 1);
  if(memcmp(tst_buff, "fff", 3) || 
     shbuff->tail != 3 || shbuff->head != 3){
    printf("read test failed\n");
    printf("tail: %d  head %d\n", shbuff->tail, shbuff->head);

    exit(-1);
  }

  shared_buffer_write_bulk(shbuff, (uint8_t*)"gfhdf", 5, 1);

  if(memcmp(shbuff->buffer, "fffgfhdf\0\0\0", 11) || 
     shbuff->tail != 3 || shbuff->head != 8){
    printf("write test failed\n");
    for(int i = 0; i < 11; i++){
      printf("%d  ", shbuff->buffer[i]);
    }
    printf("\n");
    printf("tail: %d  head %d\n", shbuff->tail, shbuff->head);

    exit(-1);
  }

  shared_buffer_write_bulk(shbuff, (uint8_t*)"sssss", 5, 1);
  if(memcmp(shbuff->buffer, "ssfgfhdfsss", 11) || 
     shbuff->tail != 3 || shbuff->head != 2){
    printf("write test failed\n");
    for(int i = 0; i < 11; i++){
      printf("%d  ", shbuff->buffer[i]);
    }
    printf("\n");
    printf("tail: %d  head %d\n", shbuff->tail, shbuff->head);

    exit(-1);
  }


  shared_buffer_read_bulk(shbuff, (uint8_t*)tst_buff, 11, 1);
  if(memcmp(tst_buff, "gfhdfsssss", 10) || 
     shbuff->tail != 2 || shbuff->head != 2){
    printf("read test failed\n");
    printf("tail: %d  head %d\n", shbuff->tail, shbuff->head);

    exit(-1);
  }


  shared_buffer_write_bulk(shbuff, (uint8_t*)"\0\0\0\0\0\0\0\0\0\0\0", 11, 1);
  if(memcmp(shbuff->buffer, "\0s\0\0\0\0\0\0\0\0", 11) || 
     shbuff->tail != 2 || shbuff->head != 1){
    printf("write test failed\n");
    for(int i = 0; i < 11; i++){
      printf("%d  ", shbuff->buffer[i]);
    }
    printf("\n");
    printf("tail: %d  head %d\n", shbuff->tail, shbuff->head);

    exit(-1);
  }

  shared_buffer_read_bulk(shbuff, (uint8_t*)tst_buff, 1, 1);
  if(memcmp(tst_buff, "\0", 1) ||
     shbuff->tail != 3 || shbuff->head != 1){
    printf("read test failed\n");
    printf("%b\n", tst_buff[0]);
    printf("tail: %d  head %d\n", shbuff->tail, shbuff->head);

    exit(-1);
  }

  shared_buffer_free(shbuff);
}