#include <Test.h>

#include <ob/base/Argparse.h>
#include <ob/base/Assert.h>

DEFTEST(empty_flags) {
  auto parser = ql_create_parser(NULL);
  ql_parse(&parser, 0, NULL);
}

DEFTEST(f_set_unset) {
  auto flag = false;

  auto parser = ql_create_parser((ql_Flag[]){
      ql_create_flag('s', "set", QL_FLAG_SET, &flag),
      ql_create_flag('u', "unset", QL_FLAG_UNSET, &flag),
      QL_FLAGS_END,
  });

  ql_parse(&parser, 2, (char const *[]){"arg", "-s"});
  QL_ASSERT(flag == true, "expected FLAG_SET to set a flag to true");

  ql_parse(&parser, 2, (char const *[]){"arg", "-u"});
  QL_ASSERT(flag == false, "expected FLAG_UNSET to set a flag to false");

  ql_parse(&parser, 2, (char const *[]){"arg", "--set"});
  QL_ASSERT(flag == true, "expected long FLAG_SET to set a flag to true");

  ql_parse(&parser, 2, (char const *[]){"arg", "--unset"});
  QL_ASSERT(flag == false, "expected long FLAG_UNSET to set a flag to false");
}

DEFTEST(f_int) {
  auto flag = 0;

  auto parser = ql_create_parser((ql_Flag[]){
      ql_create_flag('i', "int", QL_FLAG_INT, &flag),
      QL_FLAGS_END,
  });

  ql_parse(&parser, 3, (char const *[]){"arg", "-i", "1"});
  QL_ASSERT(flag == 1, "expected FLAG_INT to set a value to 1");

  ql_parse(&parser, 3, (char const *[]){"arg", "--int", "11"});
  QL_ASSERT(flag == 11, "expected long FLAG_INT to set a value to 11");
}

static void pos_arg__pa(void *udata, char const *arg) {
  *((char const **)udata) = arg;
}

DEFTEST(pos_arg) {
  char const *known = "this is a known value!";
  char const *arg = NULL;

  auto parser = ql_create_parser((ql_Flag[]){QL_FLAGS_END});
  parser.userdata = (void *)&arg;
  parser.positional_arg = pos_arg__pa;

  ql_parse(&parser, 2, (char const *[]){"arg", known});
  QL_ASSERT(arg == known, "expected a positional argument to be set");
}

DEFTEST(f_string) {
  char const *k_short = "this is short";
  char const *k_long = "this is long";

  char const *arg = NULL;

  auto parser = ql_create_parser((ql_Flag[]){
      ql_create_flag('s', "string", QL_FLAG_STRING, (void *)&arg),
      QL_FLAGS_END,
  });

  ql_parse(&parser, 3, (char const *[]){"arg", "-s", k_short});
  QL_ASSERT(arg == k_short, "expected FLAG_STRING to set a value");

  ql_parse(&parser, 3, (char const *[]){"arg", "--string", k_long});
  QL_ASSERT(arg == k_long, "expected long FLAG_STRING to set a value");
}

DEFTEST(f_subcommand) {
  auto flag = false;

  auto subparser = ql_create_parser((ql_Flag[]){
      ql_create_flag(0, "set", QL_FLAG_SET, &flag),
      ql_create_flag(0, "unset", QL_FLAG_UNSET, &flag),
      QL_FLAGS_END,
  });

  auto parser = ql_create_parser((ql_Flag[]){
      ql_create_flag('S', NULL, QL_FLAG_SUBCOMMAND, &subparser),
      QL_FLAGS_END,
  });

  ql_parse(&parser, 3, (char const *[]){"arg", "-S", "--set"});
  QL_ASSERT(flag == true,
            "expected a FLAG_SET in FLAG_SUBCOMMAND to set a flag");

  ql_parse(&parser, 3, (char const *[]){"arg", "-S", "--unset"});
  QL_ASSERT(flag == false,
            "expected a FLAG_UNSET in FLAG_SUBCOMMAND to unset a flag");
}

DEFSUITE(argparse, SUITES(),
         TESTS(TEST(empty_flags), TEST(f_set_unset), TEST(f_int), TEST(pos_arg),
               TEST(f_string), TEST(f_subcommand)));
