#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "sharedbuffer.h"
#include "debug.h"

#define SINGLE_OP_MAX_SIZE 256

struct shared_buffer *shared_buffer_init(uint32_t shbuff_size){
  struct shared_buffer *shbuff = malloc(sizeof(*shbuff));
  if(!shbuff){
    perror("malloc");
    exit(-1);
  }

  pthread_mutex_init(&shbuff->mutex, NULL);
  pthread_cond_init(&shbuff->not_empty_condition, NULL);
  pthread_cond_init(&shbuff->not_full_condition, NULL);

  shbuff->buffersize = shbuff_size;
  shbuff->buffer = malloc(shbuff_size);
  if(!shbuff->buffer){
    perror("malloc");
    exit(-1);
  }

  shbuff->tail = 0;
  shbuff->head = 0;

  return shbuff;

  //from utils/debug.h
  //debug_type_bitmap = 0x0F; //0b00001111 all debug levels on 
}

void shared_buffer_free(struct shared_buffer *shbuff){
  pthread_mutex_destroy(&shbuff->mutex);

  free(shbuff->buffer);
}

static int __shared_buffer_write(struct shared_buffer *shbuff, uint8_t *src_buff, uint32_t count, uint8_t smemb){
  int ret;
  ret = pthread_mutex_lock(&shbuff->mutex);
  if(ret){
    perror("pthread_mutex_lock");
    exit(-1);
  }

  uint32_t to_write = smemb * count; 

  if((shbuff->head+1) % shbuff->buffersize == shbuff->tail){ //full
    ret = pthread_cond_wait(&shbuff->not_full_condition, &shbuff->mutex);
    if(ret) {
      perror("pthread_cond_wait");
      exit(1);
    }
  }

  if(shbuff->head >= shbuff->tail){
    uint32_t space_free = shbuff->buffersize - (shbuff->head - shbuff->tail+1);
    
    if(to_write > space_free){
      to_write = space_free;
    }

    uint32_t normal_write = shbuff->buffersize - shbuff->head;
    if(normal_write > to_write){
      normal_write = to_write;
    }
    uint32_t circular_write = to_write - normal_write;

    memcpy(shbuff->buffer + shbuff->head, src_buff, normal_write);
    memcpy(shbuff->buffer, src_buff + normal_write, circular_write);
    shbuff->head = (shbuff->head + to_write) % shbuff->buffersize;
  } else if (shbuff->head < shbuff->tail) {
    uint32_t space_free = shbuff->tail - shbuff->head - 1;

    if(to_write > space_free){
      to_write = space_free;
    }

    memcpy(shbuff->buffer + shbuff->head, src_buff, to_write);
    shbuff->head += to_write;
  }

buff_write_done:
  ret = pthread_cond_signal(&shbuff->not_empty_condition);
  if(ret) {
    perror("pthread_cond_signal");
    exit(-1);
  }
  ret = pthread_mutex_unlock(&shbuff->mutex);
  if(ret){
    perror("pthread_mutex_unlock");
    exit(-1);
  }  

  dbg_printf(DEBUG_LOG_SHAREDBUFFER_IO, "sharedbuffer: written %d\n", to_write);
  return to_write;
}

//returns num of read bytes
static int __shared_buffer_read(struct shared_buffer *shbuff, uint8_t *dest_buff, uint32_t count, uint8_t smemb){
  int ret;
  ret = pthread_mutex_lock(&shbuff->mutex);
  if(ret){
    perror("pthread_mutex_lock");
    exit(-1);
  }
  uint32_t to_read = smemb * count; 

  if(shbuff->tail == shbuff->head){ //empty
    pthread_cond_wait(&shbuff->not_empty_condition, &shbuff->mutex);
    if(ret) {
      perror("pthread_cond_wait");
      exit(1);
    }
  }

  if(shbuff->tail > shbuff->head){
    uint32_t capacity = shbuff->buffersize - (shbuff->tail - shbuff->head);

    dbg_printf(DEBUG_LOG_SHAREDBUFFER_IO, "capacity: %d\n", capacity);

    if(capacity < to_read){
      to_read = capacity;
    }

    uint32_t normal_read = shbuff->buffersize - shbuff->tail;
    if(normal_read > to_read){
      normal_read = to_read;
    }
    uint32_t circular_read = to_read - normal_read;
  
    memcpy(dest_buff, shbuff->buffer + shbuff->tail, normal_read);
    memcpy(dest_buff + normal_read, shbuff->buffer, circular_read);
    shbuff->tail = (shbuff->tail + to_read) % shbuff->buffersize;
  } else if (shbuff->tail < shbuff->head){
    uint32_t capacity = shbuff->head - shbuff->tail;

    //dbg_printf(DEBUG_LOG_SHAREDBUFFER_IO, "capacity: %d\n", capacity);

    if(capacity < to_read){
      to_read = capacity;
    }

    memcpy(dest_buff, shbuff->buffer + shbuff->tail, to_read);
    shbuff->tail += to_read;
  }

buff_read_done:
  ret = pthread_cond_signal(&shbuff->not_full_condition);
  if(ret) {
    perror("pthread_cond_signal");
    exit(-1);
  }
  ret = pthread_mutex_unlock(&shbuff->mutex);
  if(ret){
    perror("pthread_mutex_unlock");
    exit(-1);
  }  

  dbg_printf(DEBUG_LOG_SHAREDBUFFER_IO, "sharedbuffer: read %d\n", to_read);
  return to_read;
}


//returns num of written bytes
int shared_buffer_write_bulk(struct shared_buffer *shbuff, uint8_t *src_buff, uint32_t count, uint8_t smemb){
  size_t op_full_cnt = count * smemb / SINGLE_OP_MAX_SIZE;
  size_t op_remain_size = (count * smemb) % SINGLE_OP_MAX_SIZE;

  size_t written = 0;

  for(size_t op_i = 0; op_i < op_full_cnt; op_i++){
    written += __shared_buffer_write(shbuff, src_buff + written, SINGLE_OP_MAX_SIZE, 1);
  }
  written += __shared_buffer_write(shbuff, src_buff + written, op_remain_size, 1);

  return written;
}

//returns num of read bytes
int shared_buffer_read_bulk(struct shared_buffer *shbuff, uint8_t *dest_buff, uint32_t count, uint8_t smemb){
  size_t op_full_cnt = count * smemb / SINGLE_OP_MAX_SIZE;
  size_t op_remain_size = (count * smemb) % SINGLE_OP_MAX_SIZE;

  size_t read = 0;

  for(size_t op_i = 0; op_i < op_full_cnt; op_i++){
    read += __shared_buffer_read(shbuff, dest_buff + read, SINGLE_OP_MAX_SIZE, 1);
  }
  read += __shared_buffer_read(shbuff, dest_buff + read, op_remain_size, 1);

  return read;  
}




/*
int main(void){
  struct shared_buffer *shbuff;

  shbuff = shared_buffer_init(11);

  shared_buffer_write(shbuff, "fff", 3, 1);
  for(int i = 0; i < 11; i++){
    printf("%d  ", shbuff->buffer[i]);
  }
  printf("\n");
  printf("tail: %d  head %d\n", shbuff->tail, shbuff->head);

  char tst_buff[11];
  shared_buffer_read(shbuff, tst_buff, 3, 1);
  for(int i = 0; i < 11; i++){
    printf("%d  ", tst_buff[i]);
  }
  printf("\n");
  printf("tail: %d  head %d\n", shbuff->tail, shbuff->head);


  shared_buffer_write(shbuff, "gfhdf", 5, 1);
  for(int i = 0; i < 11; i++){
    printf("%d  ", shbuff->buffer[i]);
  }
  printf("\n");
  printf("tail: %d  head %d\n", shbuff->tail, shbuff->head);


  shared_buffer_write(shbuff, "sssss", 5, 1);
  for(int i = 0; i < 11; i++){
    printf("%d  ", shbuff->buffer[i]);
  }
  printf("\n");
  printf("tail: %d  head %d\n", shbuff->tail, shbuff->head);



  shared_buffer_read(shbuff, tst_buff, 11, 1);
  for(int i = 0; i < 11; i++){
    printf("%d  ", tst_buff[i]);
  }
  printf("\n");
  printf("tail: %d  head %d\n", shbuff->tail, shbuff->head);



  shared_buffer_write(shbuff, "\0\0\0\0\0\0\0\0", 11, 1);
  for(int i = 0; i < 11; i++){
    printf("%d  ", shbuff->buffer[i]);
  }
  printf("\n");
  printf("tail: %d  head %d\n", shbuff->tail, shbuff->head);


  shared_buffer_read(shbuff, tst_buff, 1, 1);
  for(int i = 0; i < 11; i++){
    printf("%d  ", tst_buff[i]);
  }
  printf("\n");
  printf("tail: %d  head %d\n", shbuff->tail, shbuff->head);


  shared_buffer_free(shbuff);
}
*/