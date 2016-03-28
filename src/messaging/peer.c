#include <stdlib.h>
#include <gc.h>
#include "peer.h"


peer * peer_create(char * name, char * host) {
  peer * p = GC_malloc(sizeof(peer));
  p->name = name;
  p->host = host;
  p->sock = -1;
  p->_addr = NULL;
  SIMPLEQ_INIT(&(p->requests));

  return p;  
}


int peer_add_request(peer * p, message * msg) {
  SIMPLEQ_INSERT_TAIL(&(p->requests), msg, _queue);

  return RET_OK;
}


int peer_pop_request(peer * p, message ** msg) {
  *msg  = SIMPLEQ_FIRST(&(p->requests));
  SIMPLEQ_REMOVE_HEAD(&(p->requests), _queue);

  return RET_OK;
}

int peer_split_host(char * src_host, char ** host, int * port) {
  /* splitting host to host and port */
  char * tmpbuf, * port_s, * garbage;
  tmpbuf = GC_malloc(strlen(src_host));
  strcpy(tmpbuf, src_host);
  /* TODO: USE strtok_r */
  *host = strtok(tmpbuf, ":");
  if(*host == NULL) {
    return RET_PEER_RESOLVE_ERROR;
  }
  if((port_s = strtok(NULL, ":")) == NULL) {
    port_s = *host;
    *host = NULL;
  }
  *port = strtol(port_s, &garbage, 10);
  if(*host != NULL && strcmp(*host, "*") == 0) {
    *host = NULL;
  }
  if(*port == 0 || !(garbage == NULL || strlen(garbage) == 0)) {
    return RET_PEER_RESOLVE_ERROR;
  }

  return RET_OK;
}

int peer_resolve(peer * p){
  if(p->_addr != NULL) {
    return RET_OK;
  }
  char * host, * port_s = GC_malloc(6);
  int port;
  struct addrinfo hint;
  int ainfo_ret;
  int ret_code = RET_OK;

  if((ret_code = peer_split_host(p->host, &host, &port)) != 0) {
    return ret_code;
  }

  sprintf(port_s, "%d", port);
  memset(&hint, 0, sizeof(struct addrinfo));
  hint.ai_family = AF_UNSPEC;
  hint.ai_socktype = SOCK_STREAM;
  hint.ai_flags = AI_PASSIVE; /* for NULLed host */
  hint.ai_protocol = IPPROTO_TCP;
  hint.ai_canonname = NULL;
  hint.ai_addr = NULL;
  hint.ai_next = NULL;

  if((ainfo_ret = getaddrinfo(host, port_s, &hint, &p->_addr)) != 0) {
    p->_addr = NULL;
    printf("getaddrinfo: %s\n", gai_strerror(ainfo_ret));
    ret_code = RET_PEER_ADDR_INFO_ERROR;
  }

  return ret_code;
}


char * peer_to_string(peer * p) {
  int size_ = strlen(p->name) + strlen(p->host) + 20;
  size_ = size_ > 79 ? 79 : size_;
  char * res = GC_malloc(size_);
  snprintf(res, size_, "Peer<%s>(%s, %d)", p->name, p->host, p->sock);

  return res;
}
