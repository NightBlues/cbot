#include <stdlib.h>
#include <msgpack.h>

#include "message.h"

#define addByte(data, i) ((char *)data)[i] << (3 - i) * 0x8
#define charsToInt(data) addByte(data, 3) | addByte(data, 2) | addByte(data, 1) | addByte(data, 0)


message * message_create(message_type type, message_action action) {
  message * mess = malloc(sizeof(message));
  mess->type = type;
  mess->action = action;
  mess->to_msgpack = message_to_msgpack_base;

  return mess;
}

int message_cksum(char * data, int len) {
  int res = 0;
  int buf = 0;
  int garbage_size = (4 - (len % 4)) * 0x8;

  for(int i=0; i < len; i+=4) {
    buf = charsToInt(data + i);
    /* we should remove garbage on the last step */
    if(i + 4 >= len) {
      buf = buf >> garbage_size << garbage_size;
    }
    res ^= buf;
  }

  return res;
}


int message_encode(message * msg, char ** result, int * len) {
  int ret_code = 0;
  msgpack_sbuffer sbuf;
  msgpack_sbuffer_init(&sbuf);
  msgpack_packer packer;
  msgpack_packer_init(&packer, &sbuf, msgpack_sbuffer_write);
  msgpack_object packed_obj;
  if(msg->to_msgpack(msg, &packed_obj) != 0) {
    ret_code = 1;
  } else {
    msgpack_pack_object(&packer, packed_obj);
    *result = malloc(sbuf.size);
    memcpy(*result, sbuf.data, sbuf.size);
    *len = sbuf.size;
  }

  msgpack_sbuffer_destroy(&sbuf);

  return ret_code;
}


int message_to_msgpack_base(message * msg, msgpack_object * obj) {
  obj->type = MSGPACK_OBJECT_ARRAY;
  obj->via.array.size = 3;
  /* TODO: use libgc or free it */
  obj->via.array.ptr = malloc(sizeof(msgpack_object) * 3);
  obj->via.array.ptr[0].type = MSGPACK_OBJECT_POSITIVE_INTEGER;
  obj->via.array.ptr[0].via.u64 = msg->type;
  obj->via.array.ptr[1].type = MSGPACK_OBJECT_POSITIVE_INTEGER;
  obj->via.array.ptr[1].via.u64 = msg->action;
  obj->via.array.ptr[2].type = MSGPACK_OBJECT_NIL;

  return 0;
}



message * message_create_echo(message_type type, char * text, int len) {
  message * mess = message_create(type, MESSAGE_ECHO);
  mess->to_msgpack = message_to_msgpack_echo;
  mess->data.echo.len = len;
  mess->data.echo.text = text;

  return mess;
}

int message_to_msgpack_echo(message * msg, msgpack_object * obj) {
  int res = message_to_msgpack_base(msg, obj);
  if(res == 0) {
    msgpack_object * data = &obj->via.array.ptr[2];
    data->type = MSGPACK_OBJECT_STR;
    data->via.str.size = msg->data.echo.len;
    data->via.str.ptr = msg->data.echo.text;
  }

  return res;
}
