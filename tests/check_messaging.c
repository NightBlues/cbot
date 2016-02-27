#include <stdlib.h>
#include <string.h>
#include <check.h>

#include "../src/messaging/message.h"

void print(char const* buf, unsigned int len)
{
    size_t i = 0;
    for(; i < len ; ++i) {
        printf("\\x%02x", 0xff & buf[i]);
    }
    printf("\n");
}


START_TEST(test_message_cksum)
{
  ck_assert_int_eq(message_cksum("asdfgh", sizeof("asdfgh")), 102458470);
  ck_assert_int_eq(message_cksum("qqqqqqqqqqqqqqqqqqqq", sizeof("qqqqqqqqqqqqqqqqqqqq")), 1903260017);
  ck_assert_int_eq(message_cksum("", 0), 0);
  ck_assert_int_eq(sizeof("1"), 2);
  ck_assert_int_eq(message_cksum("1", sizeof("1")), 822083584);
}
END_TEST


START_TEST(test_message_encode)
{
  message * msg;
  char * res;
  int len;

  msg = message_create_echo(MESSAGE_REQUEST, "hey");
  ck_assert_int_eq(message_encode(msg, &res, &len), 0);
  ck_assert_int_eq(len, 7);
  ck_assert_int_eq(memcmp(res, "\x93\x00\x01\xa3hey", len), 0);

  msg = message_create_echo(MESSAGE_RESPONSE, "hey");
  ck_assert_int_eq(message_encode(msg, &res, &len), 0);
  ck_assert_int_eq(len, 7);
  ck_assert_int_eq(memcmp(res, "\x93\x01\x01\xa3hey", len), 0);

  msg = message_create_identity(MESSAGE_REQUEST, NULL, 0);
  ck_assert_int_eq(message_encode(msg, &res, &len), 0);
  ck_assert_int_eq(len, 4);
  ck_assert_int_eq(memcmp(res, "\x93\x00\x00\xc0", len), 0);

  msg = message_create_identity(MESSAGE_RESPONSE, "localhost", 10000);
  ck_assert_int_eq(message_encode(msg, &res, &len), 0);
  ck_assert_int_eq(len, 17);
  ck_assert_int_eq(memcmp(res, "\x93\x01\x00\x92\xa9localhost\xcd'\x10", len), 0);

  msg = message_create(MESSAGE_REQUEST, MESSAGE_DISCOVER);
  ck_assert_int_eq(message_encode(msg, &res, &len), 0);
  ck_assert_int_eq(len, 4);
  ck_assert_int_eq(memcmp(res, "\x93\x00\x02\xc0", len), 0);

}
END_TEST


Suite * message_suite(void) {
  Suite * s;
  TCase * tc_core;
  s = suite_create("Message");
  tc_core = tcase_create("Core");
  tcase_add_test(tc_core, test_message_cksum);
  tcase_add_test(tc_core, test_message_encode);
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
