#include <string.h>

#include <gc.h>
#include <msgpack.h>

#include "message.h"

#define addByte(data, i) ((char *)data)[i] << (3 - i) * 0x8
#define charsToInt(data) addByte(data, 3) | addByte(data, 2) | addByte(data, 1) | addByte(data, 0)
#define msg_array_(obj, i) obj->via.array.ptr[i]
#define msg_array(obj, i) obj.via.array.ptr[i]
#define msg_int_(obj) obj->via.u64
#define msg_int(obj) obj.via.u64
#define msg_str_set_(obj, src_str)      \
  obj->type = MSGPACK_OBJECT_STR;      \
  obj->via.str.size = strlen(src_str); \
  obj->via.str.ptr = src_str;
#define msg_str_set(obj, src_str)      \
  obj.type = MSGPACK_OBJECT_STR;      \
  obj.via.str.size = strlen(src_str); \
  obj.via.str.ptr = src_str;

char * msg_str(msgpack_object * obj) {
  int size = obj->via.str.size;
  if(obj->via.str.ptr[size] != 0x00) {
    size +=1;
  }
  char * res = GC_malloc(obj->via.str.size);
  memcpy(res, obj->via.str.ptr, obj->via.str.size);
  res[size] = 0x00;

  return res;
}


message * message_create(message_type type, message_action action) {
  message * mess = GC_malloc(sizeof(message));
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


message * message_decode(char * data, int len) {
  message * msg = NULL;
  msgpack_zone mempool;
  msgpack_zone_init(&mempool, MSGPACK_ZONE_CHUNK_SIZE);

  msgpack_object deserialized;
  msgpack_unpack(data, len, NULL, &mempool, &deserialized);
  if(message_from_msgpack_base(&deserialized, &msg) != RET_OK) {
    /* TODO: error handling */
    msg = NULL;
  }

  msgpack_zone_destroy(&mempool);

  return msg;
}


int message_encode(message * msg, char ** result, int * len) {
  int ret_code = RET_OK;
  msgpack_sbuffer sbuf;
  msgpack_sbuffer_init(&sbuf);
  msgpack_packer packer;
  msgpack_packer_init(&packer, &sbuf, msgpack_sbuffer_write);
  msgpack_object packed_obj;
  if(msg->to_msgpack(msg, &packed_obj) != RET_OK) {
    ret_code = RET_MESSAGE_ENCODE_ERROR;
  } else {
    msgpack_pack_object(&packer, packed_obj);
    *result = GC_malloc(sbuf.size);
    memcpy(*result, sbuf.data, sbuf.size);
    *len = sbuf.size;
  }

  msgpack_sbuffer_destroy(&sbuf);

  return ret_code;
}


int message_from_msgpack_base(msgpack_object * obj, message ** msg) {
  if(obj->type != MSGPACK_OBJECT_ARRAY || obj->via.array.size != 3) {
    return RET_MESSAGE_DECODE_ERROR;
  }
  if(obj->via.array.ptr[0].type != MSGPACK_OBJECT_POSITIVE_INTEGER ||
     obj->via.array.ptr[1].type != MSGPACK_OBJECT_POSITIVE_INTEGER) {
    return RET_MESSAGE_DECODE_ERROR;
  }
  message_type type = msg_int(msg_array_(obj, 0));
  message_action action = msg_int(msg_array_(obj, 1));
  *msg = message_create(type, action);
  int ret_code = RET_OK;
  switch(action) {
  case MESSAGE_ACTION_ECHO:
    ret_code = message_from_msgpack_echo(obj, *msg);
    break;
  case MESSAGE_ACTION_IDENTITY:
    ret_code = message_from_msgpack_identity(obj, *msg);
    break;
  case MESSAGE_ACTION_DISCOVER:
    /* ret_code = RET_MESSAGE_DECODE_ERROR; */
    break; /* TODO: implement */
  default: break;
  }

  return ret_code;
}


int message_to_msgpack_base(message * msg, msgpack_object * obj) {
  obj->type = MSGPACK_OBJECT_ARRAY;
  obj->via.array.size = 3;
  obj->via.array.ptr = GC_malloc(sizeof(msgpack_object) * 3);
  msg_array_(obj, 0).type = MSGPACK_OBJECT_POSITIVE_INTEGER;
  msg_int(msg_array_(obj, 0)) = msg->type;
  msg_array_(obj, 1).type = MSGPACK_OBJECT_POSITIVE_INTEGER;
  msg_array_(obj, 1).via.u64 = msg->action;
  msg_array_(obj, 2).type = MSGPACK_OBJECT_NIL;

  return RET_OK;
}



message * message_create_echo(message_type type, char * text) {
  message * mess = message_create(type, MESSAGE_ACTION_ECHO);
  mess->to_msgpack = message_to_msgpack_echo;
  mess->data.echo = text;

  return mess;
}

int message_from_msgpack_echo(msgpack_object * obj, message * msg) {
  int ret_code = RET_OK;
  msgpack_object * data = &obj->via.array.ptr[2];
  msg->data.echo = msg_str(data);

  return ret_code;
}

int message_to_msgpack_echo(message * msg, msgpack_object * obj) {
  int ret_code = message_to_msgpack_base(msg, obj);
  if(ret_code == RET_OK) {
    msgpack_object * data = &obj->via.array.ptr[2];
    msg_str_set_(data, msg->data.echo);
  }

  return ret_code;
}


message * message_create_identity(message_type type, char * name, int port) {
  message * mess = message_create(type, MESSAGE_ACTION_IDENTITY);
  mess->to_msgpack = message_to_msgpack_identity;
  if(type == MESSAGE_TYPE_RESPONSE) {
    mess->data.identity.port = port;
    mess->data.identity.name = name;
  }

  return mess;
}

int message_from_msgpack_identity(msgpack_object * obj, message * msg) {
  int ret_code = RET_OK;
  msgpack_object * data = &obj->via.array.ptr[2];
  if(data->via.array.size != 2) {
    return RET_MESSAGE_DECODE_ERROR;
  }
  if(msg_array_(data, 0).type != MSGPACK_OBJECT_STR ||
     msg_array_(data, 1).type != MSGPACK_OBJECT_POSITIVE_INTEGER) {
    return RET_MESSAGE_DECODE_ERROR;
  }
  msgpack_object name = msg_array_(data, 0);
  msgpack_object port = msg_array_(data, 1);
  msg->data.identity.name = msg_str(&name);
  msg->data.identity.port = port.via.u64;

  return ret_code;
}

int message_to_msgpack_identity(message * msg, msgpack_object * obj) {
  int ret_code = message_to_msgpack_base(msg, obj);
  if(ret_code == RET_OK && msg->type == MESSAGE_TYPE_RESPONSE) {
    msgpack_object * data = &obj->via.array.ptr[2];
    data->type = MSGPACK_OBJECT_ARRAY;
    data->via.array.size = 2;
    data->via.array.ptr = GC_malloc(sizeof(msgpack_object)*2);
    msg_str_set(msg_array_(data, 0), msg->data.identity.name);
    msg_array_(data, 1).type = MSGPACK_OBJECT_POSITIVE_INTEGER;
    msg_array_(data, 1).via.u64 = msg->data.identity.port;
  }

  return ret_code;
}
