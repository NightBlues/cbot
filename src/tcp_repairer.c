#include "repairer.c"


struct hosts {
    char hostname[255];
    char port[6];
};

typedef struct tcp_repairer tcp_repairer;
struct tcp_repairer {
  repairer base;
  struct hosts * hosts;
  int hosts_count;
};


int tcp_repairer_check(repairer * rep) {
  tcp_repairer * trep = (tcp_repairer *) rep;
  printf("We called tcp repairer, hosts_count: %d\n", trep->hosts_count);
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
