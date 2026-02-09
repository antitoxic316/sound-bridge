#include <stdarg.h>

#include "debug.h"

uint8_t debug_type_bitmap = 0;

#define PRINT_IF_ENABLED do { \
          if (debug_type_bitmap & (1 << dbg_log_type)){ \
            va_start(args, fmt); \
            vprintf(fmt, args); \
            va_end(args); \
          } \
        } while(0);

void dbg_printf(int dbg_log_type, const char *fmt, ...){
  va_list args;

  switch (dbg_log_type) {
  case DEBUG_LOG_INET_IO:
    PRINT_IF_ENABLED
    break;
  case DEBUG_LOG_MUTEX:
    PRINT_IF_ENABLED
    break;
  case DEBUG_LOG_ALSA_IO:
    PRINT_IF_ENABLED
    break;
  case DEBUG_LOG_SHAREDBUFFER_IO:
    PRINT_IF_ENABLED
    break;
  default:
    break;
  }
}