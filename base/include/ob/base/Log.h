#ifndef OB_BASE_LOG_H_INCLUDED
#define OB_BASE_LOG_H_INCLUDED

#include <stdarg.h>
#include <time.h>

#ifdef QL_LOG_MODULE
#warning "define OB_LOG_MODULE instead"
#endif

#ifndef OB_LOG_MODULE
#define OB_LOG_MODULE "*unknown*"
#endif

#ifndef OB_LOG_DISABLE
#define OB_LOG(Level, Message, ...)                                            \
  do {                                                                         \
    ql_log__handle((Level), OB_LOG_MODULE, __FILE__, __LINE__, __func__,       \
                   Message __VA_OPT__(, ) __VA_ARGS__);                        \
  } while (0)
#else
#define OB_LOG(Level, Message, ...)
#endif

#define QL_LOG(...) OB_LOG(__VA_ARGS__)

#define QL_DEBUG(M, ...) OB_LOG(QL_LOG_DEBUG, M __VA_OPT__(, ) __VA_ARGS__)
#define QL_INFO(M, ...) OB_LOG(QL_LOG_INFO, M __VA_OPT__(, ) __VA_ARGS__)
#define QL_WARN(M, ...) OB_LOG(QL_LOG_WARN, M __VA_OPT__(, ) __VA_ARGS__)
#define QL_ERROR(M, ...) OB_LOG(QL_LOG_ERROR, M __VA_OPT__(, ) __VA_ARGS__)

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
  char const *module;
  char const *file;
  int line;
  char const *function;
  char const *message;
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

void ql_log__handle(ql_LogLevel level, char const *module, char const *file,
                    int line, char const *function, char const *message, ...);

#endif
