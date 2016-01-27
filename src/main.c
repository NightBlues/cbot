#include <stdio.h>
#include "timerloop.c"
#include "tcp_repairer.c"



int main() {
  timerloop * loop = timerloop_create();
  struct hosts hosts[2] = {{"ya.ru", "80"}, {"google.com", "80"}};
  tcp_repairer * rep = tcp_repairer_create(hosts, 2, "echo repairing...", 3);

  loopswinger(loop, (repairer *) rep);
  int counter = 10;
  loop_limiter(loop, &counter);
  timerloop_start(loop);
  
  if(repairer_destroy((repairer *) rep) != 0) {
    printf("Couldn't free memory for repairer\n");
  }
  timerloop_destroy(loop);
  
  return 0;
}
