#include <ob/core/Context.h>

#include <ob/Std.h>

#include <ql/Argparse.h>
#include <ql/Log.h>

#include <stdio.h>
#include <string.h>

// TODO: now in std, move to core?
void obj_print(ob_Ctx ctx, ob_Obj receiver);

void dofile(ob_Ctx ctx, char const *path) {
  auto file = fopen(path, "r");

  if (file == NULL) {
    return;
  }

  auto data = (ql_Array){};
  auto length = 0L;

  fseek(file, 0, SEEK_END);

  length = ftell(file);

  fseek(file, 0, SEEK_SET);

  ql_array_init(&data, ctx->allocator);
  ql_array_reserve(&data, length);
  data.size = length;

  fread(data.data, sizeof(char), data.size, file);

  ob_run_ext(ctx, path, data.size, data.data);

  // exit:
  ql_array_free(&data);
  fclose(file);
}

void repl(ob_Ctx ctx) {
  auto line = (ql_Array){};
  ql_array_init(&line, ctx->allocator);

  ql_array_reserve(&line, 256);

  while (true) {
    ql_array_clear(&line);
    ql_array_clear(&ctx->stack);

    if (feof(stdin)) {
      break;
    }

    printf("* ");
    fflush(stdout);

    int tmp = 0;
    while (tmp != '\n') {
      tmp = getchar();

      if (tmp == -1) {
        puts("bye");
        break;
      }

      auto tmp2 = (char)tmp;
      ql_array_push(&line, sizeof(char), &tmp2);
    }

    ob_run_ext(ctx, "*repl*", line.size, line.data);

    if (ctx->stack.size > 0) {
      putchar('=');
      putchar('\t');

      auto obj = ob_pop(ctx);
      obj_print(ctx, obj);

      putchar('\n');
    }
  }

  ql_array_free(&line);
}

int main(int argn, char const *argv[]) {
  bool is_interactive = argn == 1;
  char const *execute = NULL;

  char const *stdout_capture = nullptr;
  char const *stderr_capture = nullptr;
  char const *stdin_capture = nullptr;

  int loglevel = QL_LOG_ERROR;

  auto f_interactive =
      ql_create_flag('i', "interactive", QL_FLAG_SET, &is_interactive);
  f_interactive.description = "Open the interactive shell";

  auto f_exec =
      ql_create_flag('e', "execute", QL_FLAG_STRING, (void *)&execute);
  f_exec.description = "Run a string passed in the command line";

  auto f_log = ql_create_flag('v', "verbose", QL_FLAG_INT, (void *)&loglevel);
  f_log.description = "Set the log level (0 to disable, 4 to allow everything)";

  auto f_capture_stdin = ql_create_flag(0, "capture-stdin", QL_FLAG_STRING,
                                        (void *)&stdin_capture);
  f_capture_stdin.description = "Read standard input from a file.";

  auto f_capture_stderr = ql_create_flag(0, "capture-stderr", QL_FLAG_STRING,
                                         (void *)&stderr_capture);
  f_capture_stderr.description = "Capture standard error into a file.";

  auto f_capture_stdout = ql_create_flag(0, "capture-stdout", QL_FLAG_STRING,
                                         (void *)&stdout_capture);
  f_capture_stdout.description = "Capture standard output into a file.";

  auto parser = ql_create_parser((ql_Flag[]){f_interactive, f_exec, f_log,
                                             f_capture_stdin, f_capture_stderr,
                                             f_capture_stdout, QL_FLAGS_END});

  ql_parse(&parser, argn, argv);

  if (stdin_capture != nullptr) {
    auto new_stdin = freopen(stdin_capture, "r", stdin);

    if (new_stdin == NULL) {
      perror("failed to capture stdin");
      return 1;
    }

    stdin = new_stdin;
  }

  if (stderr_capture != nullptr) {
    auto new_stderr = freopen(stderr_capture, "w", stderr);

    if (new_stderr == NULL) {
      perror("cannot capture stderr");
      return 1;
    }

    stderr = new_stderr;
  }

  if (stdout_capture != nullptr) {
    auto new_stdout = freopen(stdout_capture, "w", stdout);

    if (new_stdout == NULL) {
      perror("cannot capture stdout");
      return 1;
    }

    stdout = new_stdout;
  }

  auto log = ql_log_create_handler();
  ql_log_set_handler(&log);
  ql_log_set_level(loglevel);

  auto alloc = ql_alloc_create();

  auto ctx = ob_create(&alloc);
  oblib_load_all(ctx);

  if (execute != NULL) {
    ob_run_ext(ctx, "*command*", strlen(execute), execute);
  }

  if (argn != 1) {
    for (int i = 1; i < argn; i++) {
      dofile(ctx, argv[i]);
    }
  }

  if (is_interactive) {
    repl(ctx);
  }

  ob_destroy(ctx);

  if (stdin_capture != nullptr) {
    fclose(stdin);
  }

  if (stderr_capture != nullptr) {
    fclose(stderr);
  }

  if (stdout_capture != nullptr) {
    fclose(stdout);
  }

  return 0;
}
