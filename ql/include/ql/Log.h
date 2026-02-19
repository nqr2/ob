#ifndef QL_LOG_H_INCLUDED
#define QL_LOG_H_INCLUDED

/*
 * Copyright (C) 2025-2026 nqr2
 *
 * This library is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library. If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdarg.h>
#include <time.h>

#ifndef QL_LOG_MODULE
#define QL_LOG_MODULE "*unknown*"
#endif

#ifndef QL_LOG_DISABLE
#define QL_LOG(Level, Message, ...)                                            \
  do {                                                                         \
    ql_log__handle((Level), QL_LOG_MODULE, __FILE__, __LINE__, __func__,       \
                   Message __VA_OPT__(, ) __VA_ARGS__);                        \
  } while (0)
#else
#define QL_LOG(Level, Message, ...)
#endif

#define QL_DEBUG(M, ...) QL_LOG(QL_LOG_DEBUG, M __VA_OPT__(, ) __VA_ARGS__)
#define QL_INFO(M, ...) QL_LOG(QL_LOG_INFO, M __VA_OPT__(, ) __VA_ARGS__)
#define QL_WARN(M, ...) QL_LOG(QL_LOG_WARN, M __VA_OPT__(, ) __VA_ARGS__)
#define QL_ERROR(M, ...) QL_LOG(QL_LOG_ERROR, M __VA_OPT__(, ) __VA_ARGS__)

typedef enum {
  QL_LOG_DEBUG = 4,
  QL_LOG_INFO = 3,
  QL_LOG_WARN = 2,
  QL_LOG_ERROR = 1,
  QL_LOG_DISABLE = 0,
} ql_LogLevel;

typedef struct {
  ql_LogLevel level;
  time_t time;
  const char *module;
  const char *file;
  int line;
  const char *function;
  const char *message;
  va_list *arguments;
} ql_LogData;

typedef void (*ql_FnLogHandle)(void *userdata, ql_LogData *data);

typedef struct {
  ql_FnLogHandle handle;
  void *userdata;
} ql_LogHandler;

ql_LogHandler ql_log_create_handler();

void ql_log_set_handler(ql_LogHandler *handler);
void ql_log_set_level(ql_LogLevel level);

void ql_log__handle(ql_LogLevel level, const char *module, const char *file,
                    int line, const char *function, const char *message, ...);

#endif
