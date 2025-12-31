#include <ob/Argparse.h>
#include <ob/Context.h>
#include <ob/Log.h>
#include <ob/Parse.h>

#include <ob/Std.h>

#include <stdio.h>
#include <string.h>

// TODO: now in std, move to core?
void obj_print(ob_Context ctx, ob_Obj receiver);

void dofile(ob_Context ctx, const char *path) {
  auto file = fopen(path, "r");

  if (file == NULL) {
    return;
  }

  auto data = (ob_Array){};
  auto length = 0L;

  fseek(file, 0, SEEK_END);

  length = ftell(file);

  fseek(file, 0, SEEK_SET);

  obarr_init(&data, ctx->allocator);
  obarr_reserve(&data, length);
  data.size = length;

  fread(data.data, sizeof(char), data.size, file);

  ob_run(ctx, data.size, data.data);

  // exit:
  obarr_free(&data);
  fclose(file);
}

void repl(ob_Context ctx) {
  auto line = (ob_Array){};
  obarr_init(&line, ctx->allocator);

  obarr_reserve(&line, 256);

  while (true) {
    obarr_clear(&line);
    obarr_clear(&ctx->stack);

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
      obarr_push(&line, sizeof(char), &tmp2);
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

  obarr_free(&line);
}

int main(int argn, const char *argv[]) {
  bool is_interactive = argn == 1;
  const char *execute = NULL;
  int loglevel = OB_LOG_ERROR;

  auto f_interactive =
      obarg_create_flag('i', "interactive", OBARG_FLAG_SET, &is_interactive);
  f_interactive.description = "Open the interactive shell";

  auto f_exec =
      obarg_create_flag('e', NULL, OBARG_FLAG_STRING, (void *)&execute);
  f_exec.description = "Run a string passed in the command line";

  auto f_log = obarg_create_flag('v', NULL, OBARG_FLAG_INT, (void *)&loglevel);
  f_log.description = "Set the log level (0 to disable, 4 to allow everything)";

  auto parser = obarg_create_parser(
      (ob_Flag[]){f_interactive, f_exec, f_log, OB_FLAGS_END});

  obarg_parse(&parser, argn, argv);

  auto log = oblog_create_handler();
  oblog_set_handler(&log);
  oblog_set_level(loglevel);

  auto alloc = oballoc_create();

  auto ctx = obctx_create(&alloc);
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

  obctx_destroy(ctx);

  return 0;
}
