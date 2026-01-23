#include "Tap.h"

#include <ob/Argparse.h>
#include <ob/Assert.h>

void assert_failure() {
  fail_with("assertion failed");
}

void empty_flags() {
  auto parser = obarg_create_parser(NULL);
  obarg_parse(&parser, 0, NULL);
}

void f_set_unset() {
  auto flag = false;

  auto parser = obarg_create_parser((ob_Flag[]){
      obarg_create_flag('s', "set", OBARG_FLAG_SET, &flag),
      obarg_create_flag('u', "unset", OBARG_FLAG_UNSET, &flag),
      OB_FLAGS_END,
  });

  obarg_parse(&parser, 2, (const char *[]){"arg", "-s"});
  QL_ASSERT(flag == true, "expected FLAG_SET to set a flag to true");

  obarg_parse(&parser, 2, (const char *[]){"arg", "-u"});
  QL_ASSERT(flag == false, "expected FLAG_UNSET to set a flag to false");

  obarg_parse(&parser, 2, (const char *[]){"arg", "--set"});
  QL_ASSERT(flag == true, "expected long FLAG_SET to set a flag to true");

  obarg_parse(&parser, 2, (const char *[]){"arg", "--unset"});
  QL_ASSERT(flag == false, "expected long FLAG_UNSET to set a flag to false");
}

void f_int() {
  auto flag = 0;

  auto parser = obarg_create_parser((ob_Flag[]){
      obarg_create_flag('i', "int", OBARG_FLAG_INT, &flag),
      OB_FLAGS_END,
  });

  obarg_parse(&parser, 3, (const char *[]){"arg", "-i", "1"});
  QL_ASSERT(flag == 1, "expected FLAG_INT to set a value to 1");

  obarg_parse(&parser, 3, (const char *[]){"arg", "--int", "11"});
  QL_ASSERT(flag == 11, "expected long FLAG_INT to set a value to 11");
}

static void pos_arg__pa(void *udata, const char *arg) {
  *((const char **)udata) = arg;
}

void pos_arg() {
  const char *known = "this is a known value!";
  const char *arg = NULL;

  auto parser = obarg_create_parser((ob_Flag[]){OB_FLAGS_END});
  parser.userdata = (void *)&arg;
  parser.positional_arg = pos_arg__pa;

  obarg_parse(&parser, 2, (const char *[]){"arg", known});
  QL_ASSERT(arg == known, "expected a positional argument to be set");
}

void f_string() {
  const char *k_short = "this is short";
  const char *k_long = "this is long";

  const char *arg = NULL;

  auto parser = obarg_create_parser((ob_Flag[]){
      obarg_create_flag('s', "string", OBARG_FLAG_STRING, (void *)&arg),
      OB_FLAGS_END,
  });

  obarg_parse(&parser, 3, (const char *[]){"arg", "-s", k_short});
  QL_ASSERT(arg == k_short, "expected FLAG_STRING to set a value");

  obarg_parse(&parser, 3, (const char *[]){"arg", "--string", k_long});
  QL_ASSERT(arg == k_long, "expected long FLAG_STRING to set a value");
}

void f_subcommand() {
  auto flag = false;

  auto subparser = obarg_create_parser((ob_Flag[]){
      obarg_create_flag(0, "set", OBARG_FLAG_SET, &flag),
      obarg_create_flag(0, "unset", OBARG_FLAG_UNSET, &flag),
      OB_FLAGS_END,
  });

  auto parser = obarg_create_parser((ob_Flag[]){
      obarg_create_flag('S', NULL, OBARG_FLAG_SUBCOMMAND, &subparser),
      OB_FLAGS_END,
  });

  obarg_parse(&parser, 3, (const char *[]){"arg", "-S", "--set"});
  QL_ASSERT(flag == true,
            "expected a FLAG_SET in FLAG_SUBCOMMAND to set a flag");

  obarg_parse(&parser, 3, (const char *[]){"arg", "-S", "--unset"});
  QL_ASSERT(flag == false,
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
  ql_assert_add_handler(assert_failure);

  test(SUITE);

  return 0;
}
