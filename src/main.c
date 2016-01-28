#include <stdio.h>
#include <ev.h>
#include "tcp_repairer.c"


int main() {
  struct ev_loop * loop = ev_default_loop(0);

  struct hosts hosts[2] = {{"localhost", "8000"}, {"ya.ru", "80"}}; /*, {"google.com", "80"}};*/
  tcp_repairer * rep = tcp_repairer_create(hosts, 2, "python mycmd.py", 3);

  ev_timer loop_limit_watcher;
  ev_timer_init(&loop_limit_watcher, ev_loop_swinger, 3, 0);
  loop_limit_watcher.data = rep;
  ev_timer_start(loop, &loop_limit_watcher);
  ev_loop(loop, 0);
  if(repairer_destroy((repairer *) rep) != 0) {
    printf("Couldn't free memory for repairer\n");
  }
  
  return 0;
}
