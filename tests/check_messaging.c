#include <stdlib.h>
#include <check.h>

#include "../src/messaging/message.h"


START_TEST(test_message_cksum)
{
  ck_assert_int_eq(message_cksum("asdfgh", sizeof("asdfgh")), 102458470);
  ck_assert_int_eq(message_cksum("qqqqqqqqqqqqqqqqqqqq", sizeof("qqqqqqqqqqqqqqqqqqqq")), 1903260017);
  ck_assert_int_eq(message_cksum("", 0), 0);
  ck_assert_int_eq(sizeof("1"), 2);
  ck_assert_int_eq(message_cksum("1", sizeof("1")), 822083584);
}
END_TEST


Suite * message_suite(void) {
  Suite * s;
  TCase * tc_core;
  s = suite_create("Message");
  tc_core = tcase_create("Core");
  tcase_add_test(tc_core, test_message_cksum);
  suite_add_tcase(s, tc_core);

  return s;
}

 int main(void)
 {
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = message_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
 }
