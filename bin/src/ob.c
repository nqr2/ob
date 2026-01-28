#include <ob/core/Context.h>

#include <ob/Std.h>

#include <ql/Argparse.h>
#include <ql/Log.h>

#include <stdio.h>
#include <string.h>

// TODO: now in std, move to core?
void obj_print(ob_Ctx ctx, ob_Obj receiver);

void dofile(ob_Ctx ctx, const char *path) {
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

  ob_run(ctx, data.size, data.data);

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

    ob_run(ctx, line.size, line.data);

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

int main(int argn, const char *argv[]) {
  bool is_interactive = argn == 1;
  const char *execute = NULL;
  int loglevel = QL_LOG_ERROR;

  auto f_interactive =
      ql_create_flag('i', "interactive", QL_FLAG_SET, &is_interactive);
  f_interactive.description = "Open the interactive shell";

  auto f_exec = ql_create_flag('e', NULL, QL_FLAG_STRING, (void *)&execute);
  f_exec.description = "Run a string passed in the command line";

  auto f_log = ql_create_flag('v', NULL, QL_FLAG_INT, (void *)&loglevel);
  f_log.description = "Set the log level (0 to disable, 4 to allow everything)";

  auto parser =
      ql_create_parser((ql_Flag[]){f_interactive, f_exec, f_log, QL_FLAGS_END});

  ql_parse(&parser, argn, argv);

  auto log = ql_log_create_handler();
  ql_log_set_handler(&log);
  ql_log_set_level(loglevel);

  auto alloc = ql_alloc_create();

  auto ctx = ob_create(&alloc);
  oblib_load_all(ctx);

  if (execute != NULL) {
    ob_run(ctx, strlen(execute), execute);
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

  return 0;
}
