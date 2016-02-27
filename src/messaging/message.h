#pragma once
#include <msgpack.h>
#define HEADER_SIZE 8;

typedef enum {
  MESSAGE_REQUEST = 0,
  MESSAGE_RESPONSE = 1
} message_type;

typedef enum {
  MESSAGE_IDENTITY = 0,
  MESSAGE_ECHO = 1,
  MESSAGE_DISCOVER = 2
} message_action;


typedef struct message_data_echo {
  int len;
  char * text;
} message_data_echo;


typedef struct message message;
struct message {
  message_type type;
  message_action action;
  int (* to_msgpack)(message*, msgpack_object*);
  union {
    message_data_echo echo;
  } data;
};


message * message_create(message_type type, message_action action);
message * message_decode(char * msg, int len);
int message_encode(message * msg, char ** result, int * len);
int message_cksum(char * data, int len);
int message_read(int sock, void ** result);
int message_send(void * msg);
int message_to_msgpack_base(message *, msgpack_object *);

message * message_create_echo(message_type type, char * text, int len);
int message_to_msgpack_echo(message *, msgpack_object *);
