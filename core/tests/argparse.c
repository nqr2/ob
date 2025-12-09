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

const Test SUITE[] = {
    {"empty flag list", empty_flags, false},
    {"FLAG_SET and FLAG_UNSET", f_set_unset, false},
    {"FLAG_INT", f_int, false},
    SUITE_END,
};

int main() {
  assert_add_handler(assert_failure);

  test(SUITE);

  return 0;
}
