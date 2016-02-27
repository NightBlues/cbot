#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>


#include "tcp_repairer.h"



int tcp_repairer_check(repairer * rep) {
  tcp_repairer * trep = (tcp_repairer *) rep;
  printf("tcp_repairer: We called tcp repairer, hosts_count: %d\n", trep->hosts_count);
  struct addrinfo * addr, * rp;
  struct addrinfo hints;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
  hints.ai_protocol = 0;          /* Any protocol */
  hints.ai_canonname = NULL;
  hints.ai_addr = NULL;
  hints.ai_next = NULL;
  if(getaddrinfo(trep->hosts[0].hostname, trep->hosts[0].port, &hints, &addr) != 0) {
    printf("tcp_repairer: Failed to call getaddrinfo\n");
  }
  int sfd;
  for(rp = addr; rp != NULL; rp = rp->ai_next) {
    printf("tcp_repairer: Trying addr: %s, protocol: %d, family: %d\n", rp->ai_canonname, rp->ai_protocol, rp->ai_family);
    sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if(sfd == -1) {
      continue;
    }
    if(connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1) {
      break; /* Success */
    }
    close(sfd);
  }
  if(rp == NULL) {
    printf("tcp_repairer: Couldn't connect\n");
    freeaddrinfo(addr);
    return 1;
  }
  printf("tcp_repairer: Connected to %s\n", rp->ai_canonname);
  freeaddrinfo(addr);
  close(sfd);
  
  return 0;
}


tcp_repairer * tcp_repairer_create(struct hosts * hosts, int hosts_count, char * repair_command, int poll_interval) {
  repairer * base = repairer_create(repair_command, poll_interval);
  base->_check_func = tcp_repairer_check;
  tcp_repairer * res = realloc(base, sizeof(tcp_repairer));
  res->hosts = hosts;
  res->hosts_count = hosts_count;

  return res;
}
