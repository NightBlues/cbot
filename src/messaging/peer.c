#include <string.h>
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
  char * tmpbuf, * port_s;
  tmpbuf = GC_malloc(strlen(src_host));
  strcpy(tmpbuf, src_host);
  *host = strtok(tmpbuf, ":");
  if(*host == NULL) {
    return RET_PEER_RESOLVE_ERROR;
  }
  port_s = strtok(NULL, ":");
  if(port_s == NULL) {
    port_s = *host;
    *host = NULL;
    *port = 0;
  } else {
    *port = atoi(port_s);
  }
  if(strcmp(*host, "*") == 0) {
    *host = NULL;
  }
  if(*port == 0) {
    return RET_PEER_RESOLVE_ERROR;
  }

  return RET_OK;
}

int peer_resolve(peer * p){
  if(p->_addr != NULL) {
    return RET_OK;
  }

  return RET_OK;
}


char * peer_to_string(peer * p) {
  int size_ = strlen(p->name) + strlen(p->host) + 20;
  size_ = size_ > 79 ? 79 : size_;
  char * res = GC_malloc(size_);
  snprintf(res, size_, "Peer<%s>(%s, %d)", p->name, p->host, p->sock);

  return res;
}
