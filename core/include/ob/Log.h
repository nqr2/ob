#ifndef OB_CORE_LOG_H_INCLUDED
#define OB_CORE_LOG_H_INCLUDED

/*
 * Copyright (C) 2025 nqr2
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

#ifndef OB_LOG_MODULE
#define OB_LOG_MODULE "*unknown*"
#endif

#ifndef OB_LOG_DISABLE
#define OB_LOG(Level, Message, ...)                                            \
  do {                                                                         \
    oblog__handle((Level), OB_LOG_MODULE, __FILE__, __LINE__, __func__,        \
                  Message __VA_OPT__(, ) __VA_ARGS__);                         \
  } while (0)
#else
#define OB_LOG(Level, Message, ...)
#endif

#define OB_DEBUG(M, ...) OB_LOG(OB_LOG_DEBUG, M __VA_OPT__(, ) __VA_ARGS__)
#define OB_INFO(M, ...) OB_LOG(OB_LOG_INFO, M __VA_OPT__(, ) __VA_ARGS__)
#define OB_WARN(M, ...) OB_LOG(OB_LOG_WARN, M __VA_OPT__(, ) __VA_ARGS__)
#define OB_ERROR(M, ...) OB_LOG(OB_LOG_ERROR, M __VA_OPT__(, ) __VA_ARGS__)

typedef enum {
  OB_LOG_DEBUG = 4,
  OB_LOG_INFO = 3,
  OB_LOG_WARN = 2,
  OB_LOG_ERROR = 1,
  OB_LOG_DISABLE = 0,
} ob_LogLevel;

typedef struct {
  ob_LogLevel level;
  time_t time;
  const char *module;
  const char *file;
  int line;
  const char *function;
  const char *message;
  va_list *arguments;
} ob_LogData;

typedef void (*ob_FnLogHandle)(void *userdata, ob_LogData *data);

typedef struct {
  ob_FnLogHandle handle;
  void *userdata;
} ob_LogHandler;

ob_LogHandler oblog_create_handler();

void oblog_set_handler(ob_LogHandler *handler);
void oblog_set_level(ob_LogLevel level);

void oblog__handle(ob_LogLevel level, const char *module, const char *file,
                   int line, const char *function, const char *message, ...);

#endif
