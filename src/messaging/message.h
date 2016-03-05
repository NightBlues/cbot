#pragma once
#include <msgpack.h>

#include "../constants.h"

#define RET_MESSAGE_ENCODE_ERROR 30
#define RET_MESSAGE_DECODE_ERROR 31
#define HEADER_SIZE 8

typedef enum {
  MESSAGE_TYPE_REQUEST = 0,
  MESSAGE_TYPE_RESPONSE = 1
} message_type;

typedef enum {
  MESSAGE_ACTION_IDENTITY = 0,
  MESSAGE_ACTION_ECHO = 1,
  MESSAGE_ACTION_DISCOVER = 2
} message_action;


typedef struct {
  int port;
  char * name;
} message_data_identity;


typedef struct message message;
struct message {
  message_type type;
  message_action action;
  int (* to_msgpack)(message*, msgpack_object*);
  union {
    char * echo;
    message_data_identity identity;
  } data;
};


message * message_create(message_type type, message_action action);
message * message_decode(char * data, int len);
int message_encode(message * msg, char ** result, int * len);
int message_cksum(char * data, int len);
int message_read(int sock, void ** result);
int message_send(void * msg);
int message_from_msgpack_base(msgpack_object *, message **);
int message_to_msgpack_base(message *, msgpack_object *);

message * message_create_echo(message_type type, char * text);
int message_from_msgpack_echo(msgpack_object *, message *);
int message_to_msgpack_echo(message *, msgpack_object *);


message * message_create_identity(message_type type, char * name, int port);
int message_from_msgpack_identity(msgpack_object *, message *);
int message_to_msgpack_identity(message *, msgpack_object *);

