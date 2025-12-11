#include "Tap.h"

#include <ob/Argparse.h>
#include <ob/Assert.h>

void assert_failure() {
  fail_with("assertion failed");
}

void empty_flags() {
  auto parser = arg_create_parser(NULL);
  arg_parse(&parser, 0, NULL);
}

void f_set_unset() {
  auto flag = false;

  auto parser = arg_create_parser((Flag[]){
      arg_create_flag('s', "set", FLAG_SET, &flag),
      arg_create_flag('u', "unset", FLAG_UNSET, &flag),
      FLAGS_END,
  });

  arg_parse(&parser, 2, (const char *[]){"arg", "-s"});
  ASSERT(flag == true, "expected FLAG_SET to set a flag to true");

  arg_parse(&parser, 2, (const char *[]){"arg", "-u"});
  ASSERT(flag == false, "expected FLAG_UNSET to set a flag to false");

  arg_parse(&parser, 2, (const char *[]){"arg", "--set"});
  ASSERT(flag == true, "expected long FLAG_SET to set a flag to true");

  arg_parse(&parser, 2, (const char *[]){"arg", "--unset"});
  ASSERT(flag == false, "expected long FLAG_UNSET to set a flag to false");
}

void f_int() {
  auto flag = 0;

  auto parser = arg_create_parser((Flag[]){
      arg_create_flag('i', "int", FLAG_INT, &flag),
      FLAGS_END,
  });

  arg_parse(&parser, 3, (const char *[]){"arg", "-i", "1"});
  ASSERT(flag == 1, "expected FLAG_INT to set a value to 1");

  arg_parse(&parser, 3, (const char *[]){"arg", "--int", "11"});
  ASSERT(flag == 11, "expected long FLAG_INT to set a value to 11");
}

static void pos_arg__pa(void *udata, const char *arg) {
  *((const char **)udata) = arg;
}

void pos_arg() {
  const char *known = "this is a known value!";
  const char *arg = NULL;

  auto parser = arg_create_parser((Flag[]){FLAGS_END});
  parser.userdata = (void *)&arg;
  parser.positional_arg = pos_arg__pa;

  arg_parse(&parser, 2, (const char *[]){"arg", known});
  ASSERT(arg == known, "expected a positional argument to be set");
}

void f_string() {
  const char *k_short = "this is short";
  const char *k_long = "this is long";

  const char *arg = NULL;

  auto parser = arg_create_parser((Flag[]){
      arg_create_flag('s', "string", FLAG_STRING, (void *)&arg),
      FLAGS_END,
  });

  arg_parse(&parser, 3, (const char *[]){"arg", "-s", k_short});
  ASSERT(arg == k_short, "expected FLAG_STRING to set a value");

  arg_parse(&parser, 3, (const char *[]){"arg", "--string", k_long});
  ASSERT(arg == k_long, "expected long FLAG_STRING to set a value");
}

void f_subcommand() {
  auto flag = false;

  auto subparser = arg_create_parser((Flag[]){
      arg_create_flag(0, "set", FLAG_SET, &flag),
      arg_create_flag(0, "unset", FLAG_UNSET, &flag),
      FLAGS_END,
  });

  auto parser = arg_create_parser((Flag[]){
      arg_create_flag('S', NULL, FLAG_SUBCOMMAND, &subparser),
      FLAGS_END,
  });

  arg_parse(&parser, 3, (const char *[]){"arg", "-S", "--set"});
  ASSERT(flag == true, "expected a FLAG_SET in FLAG_SUBCOMMAND to set a flag");

  arg_parse(&parser, 3, (const char *[]){"arg", "-S", "--unset"});
  ASSERT(flag == false,
         "expected a FLAG_UNSET in FLAG_SUBCOMMAND to unset a flag");
}

const Test SUITE[] = {
    {"empty flag list", empty_flags, false},
    {"FLAG_SET and FLAG_UNSET", f_set_unset, false},
    {"FLAG_INT", f_int, false},
    {"positional arguments", pos_arg, false},
    {"FLAG_STRING", f_string, false},
    {"FLAG_SUBCOMMAND", f_subcommand, false},
    SUITE_END,
};

int main() {
  assert_add_handler(assert_failure);

  test(SUITE);

  return 0;
}
