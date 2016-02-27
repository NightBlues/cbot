#pragma once
#include "repairer.h"


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


int tcp_repairer_check(repairer * rep);
tcp_repairer * tcp_repairer_create(struct hosts * hosts, int hosts_count, char * repair_command, int poll_interval);
