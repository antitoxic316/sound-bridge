#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <stdio.h>
#include <stdint.h>

//least significant bit is first enum value
extern uint8_t debug_type_bitmap;

enum dbg_log_type {
  DEBUG_LOG_SHAREDBUFFER_IO = 0, //bitmap least significant bit
  DEBUG_LOG_MUTEX,
  DEBUG_LOG_ALSA_IO,
  DEBUG_LOG_INET_IO,
};

void dbg_printf(int dbg_log_type, const char *fmt, ...);

#endif