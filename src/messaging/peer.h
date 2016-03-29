#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/queue.h>

#include "../constants.h"
#include "message.h"


#define RET_PEER_RESOLVE_ERROR 40
#define RET_PEER_ADDR_INFO_ERROR 41
#define RET_PEER_ALREADY_CONNECTED 42
#define RET_PEER_COULD_NOT_CONNECT 43


typedef struct peer peer;
struct peer {
  char * name;
  char * host;
  int sock;
  struct addrinfo * _addr;
  SIMPLEQ_HEAD(message_queue, message) requests;
};


peer * peer_create(char * name, char * host);
int peer_add_request(peer * p, message * msg);
int peer_pop_request(peer * p, message ** msg);
/* Note: we have no "default" port number */
int peer_split_host(char * src_host, char ** host, int * port);
int peer_resolve(peer * p);
/* Return 0 if we connected at this call. */
int peer_connect(peer * p);
char * peer_to_string(peer * p);
