#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <gc.h>
#include <check.h>

#include "../src/messaging/message.h"
#include "../src/messaging/peer.h"

#ifndef TEST_PORT
#define TEST_PORT "60000"
#endif

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
  uint32_t len;

  msg = message_create_echo(MESSAGE_TYPE_REQUEST, "hey");
  ck_assert_int_eq(message_encode(msg, &res, &len), 0);
  ck_assert_int_eq(len, 7);
  ck_assert_int_eq(memcmp(res, "\x93\x00\x01\xa3hey", len), 0);
  msg = message_decode("\x93\x00\x01\xa3hey", 7);
  ck_assert(msg != NULL);
  ck_assert_str_eq(msg->data.echo, "hey");

  msg = message_create_echo(MESSAGE_TYPE_RESPONSE, "hey");
  ck_assert_int_eq(message_encode(msg, &res, &len), 0);
  ck_assert_int_eq(len, 7);
  ck_assert_int_eq(memcmp(res, "\x93\x01\x01\xa3hey", len), 0);
  msg = message_decode("\x93\x01\x01\xa3hey", 7);
  ck_assert(msg != NULL);
  ck_assert_str_eq(msg->data.echo, "hey");

  msg = message_create_identity(MESSAGE_TYPE_REQUEST, NULL, 0);
  ck_assert_int_eq(message_encode(msg, &res, &len), 0);
  ck_assert_int_eq(len, 4);
  ck_assert_int_eq(memcmp(res, "\x93\x00\x00\xc0", len), 0);

  msg = message_create_identity(MESSAGE_TYPE_RESPONSE, "localhost", 10000);
  ck_assert_int_eq(message_encode(msg, &res, &len), 0);
  ck_assert_int_eq(len, 17);
  ck_assert_int_eq(memcmp(res, "\x93\x01\x00\x92\xa9localhost\xcd'\x10", len), 0);
  msg = message_decode("\x93\x01\x00\x92\xa9localhost\xcd'\x10", 17);
  ck_assert(msg != NULL);
  /* ck_assert_int_eq(msg->data.identity.port , 10000); */
  /* ck_assert_str_eq(msg->data.identity.name, "localhost"); */

  msg = message_create(MESSAGE_TYPE_REQUEST, MESSAGE_ACTION_DISCOVER);
  ck_assert_int_eq(message_encode(msg, &res, &len), 0);
  ck_assert_int_eq(len, 4);
  ck_assert_int_eq(memcmp(res, "\x93\x00\x02\xc0", len), 0);
  msg = message_decode("\x93\x00\x02\xc0", 4);
  ck_assert(msg != NULL);
  ck_assert_int_eq(msg->type, MESSAGE_TYPE_REQUEST);
  ck_assert_int_eq(msg->action, MESSAGE_ACTION_DISCOVER);


}
END_TEST


START_TEST(test_message_read) {
  message * msg;
  int * sock = GC_malloc(sizeof(int) * 2);
  socketpair(AF_UNIX, SOCK_STREAM, 0, sock);
  uint32_t size = HEADER_SIZE + sizeof("\x93\x00\x01\xa3hey");
  char * src_buf = GC_malloc(size);
  memcpy(src_buf, "\x00\x00\x00\x07\xfb\x65\x78\xa3", HEADER_SIZE);
  memcpy(src_buf + HEADER_SIZE, "\x93\x00\x01\xa3hey", sizeof("\x93\x00\x01\xa3hey"));

  ck_assert_int_eq(message_read(sock[0], &msg), RET_MESSAGE_HEADER_ERROR);
  send(sock[1], src_buf, HEADER_SIZE, MSG_DONTWAIT);
  ck_assert_int_eq(message_read(sock[0], &msg), RET_MESSAGE_RECEIVE_ERROR);
  send(sock[1], src_buf, size, MSG_DONTWAIT);
  ck_assert_int_eq(message_read(sock[0], &msg), RET_OK);
  ck_assert(msg != NULL);
  ck_assert_str_eq(msg->data.echo, "hey");
}
END_TEST


START_TEST(test_message_send) {
  message * msg;
  int * sock = GC_malloc(sizeof(int) * 2);
  socketpair(AF_UNIX, SOCK_STREAM, 0, sock);
  msg = message_create_echo(MESSAGE_TYPE_REQUEST, "hey");
  message_send(sock[0], msg);
  char * header_buf = GC_malloc(8);
  char * data = GC_malloc(7);
  ck_assert_int_eq(recv(sock[1], header_buf, 8, MSG_DONTWAIT), 8);
  ck_assert_int_eq(memcmp(header_buf, "\x00\x00\x00\x07\xfb\x65\x78\xa3", 8), 0);
  ck_assert_int_eq(recv(sock[1], data, 7, MSG_DONTWAIT), 7);
  ck_assert_int_eq(memcmp(data, "\x93\x00\x01\xa3hey", 7), 0);
}
END_TEST


START_TEST(test_message_send_read) {
  message * msg, * msg2;
  int * sock = GC_malloc(sizeof(int) * 2);
  socketpair(AF_UNIX, SOCK_STREAM, 0, sock);
  msg = message_create_echo(MESSAGE_TYPE_REQUEST, "hey");
  message_send(sock[0], msg);
  ck_assert_int_eq(message_read(sock[1], &msg2), RET_OK);
  ck_assert(msg2 != NULL);
  ck_assert_str_eq(msg2->data.echo, "hey");
}
END_TEST


START_TEST(test_peer_requests) {
  /* printf("Called peer test\n"); */
  message * msg;
  peer * localhost;

  localhost = peer_create("localhost", "localhost");
  ck_assert_int_eq(peer_add_request(localhost, \
      message_create_echo(MESSAGE_TYPE_REQUEST, "hey")), RET_OK);
  ck_assert_int_eq(peer_add_request(localhost, \
      message_create_echo(MESSAGE_TYPE_REQUEST, "mess2")), RET_OK);
  ck_assert_int_eq(peer_add_request(localhost, \
      message_create_echo(MESSAGE_TYPE_REQUEST, "message3")), RET_OK);
  ck_assert_int_eq(peer_pop_request(localhost, &msg), RET_OK);
  ck_assert(msg != NULL);
  ck_assert_str_eq(msg->data.echo, "hey");
  ck_assert_int_eq(peer_pop_request(localhost, &msg), RET_OK);
  ck_assert(msg != NULL);
  ck_assert_str_eq(msg->data.echo, "mess2");
  ck_assert_int_eq(peer_pop_request(localhost, &msg), RET_OK);
  ck_assert(msg != NULL);
  ck_assert_str_eq(msg->data.echo, "message3");
}
END_TEST

START_TEST(test_peer_to_string) {
  peer * p = peer_create("local", "127.0.0.1");
  ck_assert_str_eq(peer_to_string(p), "Peer<local>(127.0.0.1, -1)");
}
END_TEST


START_TEST(test_peer_split_host) {
  char * host;
  int port;

  ck_assert_int_eq(peer_split_host("localhost:8000", &host, &port), RET_OK);
  ck_assert_int_eq(port, 8000);
  ck_assert(host != NULL);
  ck_assert_str_eq(host, "localhost");
  ck_assert_int_eq(peer_split_host("*:8000", &host, &port), RET_OK);
  ck_assert_int_eq(port, 8000);
  ck_assert(host == NULL);
  ck_assert_int_eq(peer_split_host(":8000", &host, &port), RET_OK);
  ck_assert_int_eq(port, 8000);
  ck_assert(host == NULL);
  ck_assert_int_eq(peer_split_host("8000", &host, &port), RET_OK);
  ck_assert_int_eq(port, 8000);
  ck_assert(host == NULL);
  ck_assert_int_eq(peer_split_host("8000localhost", &host, &port), RET_PEER_RESOLVE_ERROR);
  ck_assert_int_eq(peer_split_host("localhost", &host, &port), RET_PEER_RESOLVE_ERROR);
  ck_assert_int_eq(peer_split_host("localhost:http", &host, &port), RET_PEER_RESOLVE_ERROR);
}
END_TEST


START_TEST(test_peer_resolve) {
  peer * p = peer_create("local", "127.0.0.1:8080");

  ck_assert_int_eq(peer_resolve(p), RET_OK);
  ck_assert(p->_addr != NULL);
  char host[255], port[6];
  getnameinfo(p->_addr->ai_addr, p->_addr->ai_addrlen, host, 255, port, 6, NI_NUMERICHOST | NI_NUMERICSERV);
  ck_assert_str_eq("127.0.0.1", host);
  ck_assert_str_eq("8080", port);
}
END_TEST


START_TEST(test_peer_connect) {
  char host[100] = "127.0.0.1";
  char port[6] = TEST_PORT;
  char hostport[107];
  sprintf(hostport, "%s:%s", host, port);
  peer * p = peer_create("local", hostport);

  /* creating server for test */
  int server_sock = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
  struct addrinfo * ainfo, hint;
  memset(&hint, 0, sizeof(struct addrinfo));
  hint.ai_family = AF_UNSPEC;
  hint.ai_socktype = SOCK_STREAM;
  hint.ai_flags = AI_PASSIVE; /* for NULLed host */
  hint.ai_protocol = IPPROTO_TCP;
  hint.ai_canonname = NULL;
  hint.ai_addr = NULL;
  hint.ai_next = NULL;
  ck_assert_int_eq(getaddrinfo(host, port, &hint, &ainfo), 0);
  ck_assert_int_eq(bind(server_sock, ainfo->ai_addr, ainfo->ai_addrlen), 0);
  ck_assert_int_eq(listen(server_sock, 10), 0);

  ck_assert_int_eq(peer_connect(p), 0);
  int client_sock = accept(server_sock, NULL, NULL);
  char * msg = "testmsg";
  send(p->sock, msg, 8, MSG_DONTWAIT);
  char buf[25];
  recv(client_sock, buf, 8, MSG_DONTWAIT);
  ck_assert_str_eq(buf, "testmsg");

  close(p->sock);
  close(client_sock);
  close(server_sock);
}
END_TEST


Suite * message_suite(void) {
  Suite * s;
  TCase * tc_core;
  s = suite_create("Message");
  tc_core = tcase_create("Core");
  tcase_add_test(tc_core, test_message_cksum);
  tcase_add_test(tc_core, test_message_encode);
  tcase_add_test(tc_core, test_message_read);
  tcase_add_test(tc_core, test_message_send);
  tcase_add_test(tc_core, test_message_send_read);
  suite_add_tcase(s, tc_core);

  return s;
}

Suite * peer_suite(void) {
  Suite * s;
  TCase * tc_core;
  s = suite_create("Peer");
  tc_core = tcase_create("Core");
  tcase_add_test(tc_core, test_peer_requests);
  tcase_add_test(tc_core, test_peer_to_string);
  tcase_add_test(tc_core, test_peer_split_host);
  tcase_add_test(tc_core, test_peer_resolve);
  tcase_add_test(tc_core, test_peer_connect);
  suite_add_tcase(s, tc_core);

  return s;
}

#define run_suite(suite_runner, suite, number_failed)        \
  suite_runner = srunner_create(suite);                      \
  srunner_run_all(suite_runner, CK_NORMAL);                  \
  number_failed += srunner_ntests_failed(suite_runner);      \
  srunner_free(suite_runner)


 int main(void)
 {
    int number_failed = 0;
    SRunner *sr;

    run_suite(sr, message_suite(), number_failed);
    run_suite(sr, peer_suite(), number_failed);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
 }
