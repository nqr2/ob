#include <ql/Log.h>

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

static ql_LogHandler *cur_handler = NULL;
static ql_LogLevel cur_level = QL_LOG_ERROR;

static void dflt_handle(void *userdata, ql_LogData *data) {
  (void)userdata;

  char buffer[256];

  const char *level_name = "???";

  switch (data->level) {
  case QL_LOG_DEBUG:
    level_name = "DEBUG";
    break;
  case QL_LOG_INFO:
    level_name = "INFO";
    break;
  case QL_LOG_WARN:
    level_name = "WARN";
    break;
  case QL_LOG_ERROR:
    level_name = "ERROR";
    break;
  case QL_LOG_DISABLE:
    return;
  }

  strftime(buffer, sizeof(buffer), "%Y/%m/%d %H:%M:%S", localtime(&data->time));

  fprintf(stderr, "[%s] %s:%d (%s) %s : ", buffer, data->file, data->line,
          data->function, level_name);

  vfprintf(stderr, data->message, *data->arguments);

  fputc('\n', stderr);
}

ql_LogHandler ql_log_create_handler() {
  return (ql_LogHandler){
      .handle = dflt_handle,
      .userdata = NULL,
  };
}

void ql_log_set_handler(ql_LogHandler *handler) {
  cur_handler = handler;
}

void ql_log_set_level(ql_LogLevel level) {
  cur_level = level;
}

void ql_log__handle(const ql_LogLevel level, const char *module,
                    const char *file, int line, const char *function,
                    const char *message, ...) {
  if (cur_handler == NULL) {
    return;
  }

  if (level >= cur_level) {
    return;
  }

  va_list args;
  va_start(args, message);

  auto now = time(NULL);

  auto data = (ql_LogData){
      .level = level,
      .time = now,
      .module = module,
      .file = file,
      .line = line,
      .function = function,
      .message = message,
      .arguments = &args,
  };

  (cur_handler->handle)(cur_handler->userdata, &data);

  va_end(args);
}
