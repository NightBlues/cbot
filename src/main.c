#include <stdio.h>
#include <ev.h>
#include "tcp_repairer.c"
#include "messaging.c"


void stdin_cb(EV_P_ ev_io * w, int revents) {
  char buf[5];
  ssize_t rnum = read(w->fd, buf, 4);
  buf[rnum] = 0x00;
  printf("You entered %s\n", buf);
  /* ev_io_stop(EV_A_ w); */
  /* ev_io_start(EV_A_ w); */
  /* ev_break(EV_A_ EVBREAK_ONE); */
}


int main() {
  struct ev_loop * loop = ev_default_loop(0);

  struct hosts hosts[2] = {{"localhost", "8000"}, {"ya.ru", "80"}}; /*, {"google.com", "80"}};*/
  tcp_repairer * rep = tcp_repairer_create(hosts, 2, "python mycmd.py", 3);

  ev_timer repairer_watcher;
  ev_timer_init(&repairer_watcher, ev_loop_swinger, 3, 0);
  repairer_watcher.data = rep;
  ev_timer_start(loop, &repairer_watcher);

  ev_io loop_stdin_watcher;
  ev_io_init(&loop_stdin_watcher, stdin_cb, 0, EV_READ);
  ev_io_start(loop, &loop_stdin_watcher);

  messaging * msg = start_messaging(loop, "tcp://localhost:32535");
  
  ev_loop(loop, 0);

  if(repairer_destroy((repairer *) rep) != 0) {
    printf("Couldn't free memory for repairer\n");
  }
  stop_messaging(loop, msg);
  
  return 0;
}
