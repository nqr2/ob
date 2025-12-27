#include <ob/Log.h>

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

static ob_LogHandler *cur_handler = NULL;
static ob_LogLevel cur_level = OB_LOG_ERROR;

static void dflt_handle(void *userdata, ob_LogData *data) {
  (void)userdata;

  char buffer[256];

  const char *level_name = "???";

  switch (data->level) {
  case OB_LOG_DEBUG:
    level_name = "DEBUG";
    break;
  case OB_LOG_INFO:
    level_name = "INFO";
    break;
  case OB_LOG_WARN:
    level_name = "WARN";
    break;
  case OB_LOG_ERROR:
    level_name = "ERROR";
    break;
  }

  strftime(buffer, sizeof(buffer), "%Y/%m/%d %H:%M:%S", localtime(&data->time));

  fprintf(stderr, "[%s] %s:%d (%s) %s : ", buffer, data->file, data->line,
          data->function, level_name);

  vfprintf(stderr, data->message, *data->arguments);

  fputc('\n', stderr);
}

ob_LogHandler oblog_create_handler() {
  return (ob_LogHandler){
      .handle = dflt_handle,
      .userdata = NULL,
  };
}

void oblog_set_handler(ob_LogHandler *handler) {
  cur_handler = handler;
}

void oblog_set_level(ob_LogLevel level) {
  cur_level = level;
}

void oblog__handle(const ob_LogLevel level, const char *module,
                   const char *file, int line, const char *function,
                   const char *message, ...) {
  if (cur_handler == NULL) {
    return;
  }

  if (level > cur_level) {
    return;
  }

  va_list args;
  va_start(args, message);

  auto now = time(NULL);

  auto data = (ob_LogData){
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
