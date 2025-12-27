#ifndef OB_CORE_LOG_H_INCLUDED
#define OB_CORE_LOG_H_INCLUDED

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
  OB_LOG_DEBUG,
  OB_LOG_INFO,
  OB_LOG_WARN,
  OB_LOG_ERROR,
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
