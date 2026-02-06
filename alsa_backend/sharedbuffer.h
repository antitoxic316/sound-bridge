#ifndef _SHAREDBUFFER_H_
#define _SHAREDBUFFER_H_

#include <stdint.h>
#include <pthread.h>

struct shared_buffer {
  pthread_mutex_t mutex;
  uint32_t buffersize;
  uint32_t tail;
  uint32_t head;
  uint8_t *buffer;

  pthread_cond_t not_full_condition;
  pthread_cond_t not_empty_condition;
};


struct shared_buffer *shared_buffer_init(uint32_t shbuff_size);
void shared_buffer_free(struct shared_buffer *shbuff);
int shared_buffer_write(struct shared_buffer *shbuff, uint8_t *src_buff, uint32_t count, uint8_t smemb);
int shared_buffer_read(struct shared_buffer *shbuff, uint8_t *dest_buff, uint32_t count, uint8_t smemb);

#endif