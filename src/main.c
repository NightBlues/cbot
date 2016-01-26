#include <stdio.h>
#include "timerloop.c"
#include "repairer.c"



int main() {
  printf("Hello, world!\n");
  timerloop * loop = timerloop_create();
  int counter = 10;
  void loop_limiter(timerloop * loop, int * count) {
    (*count)--;
    printf("Called loop limiter, counter = %d\n", *count);
    if(*count <= 0) {
      /* loop->stopped = 1; */
      loop->gracefully_stopped = 1;
      return;
    }
    timerloop_add_timeout(loop, 1, loop_limiter, count);
  }
  timerloop_add_timeout(loop, 1, loop_limiter, &counter);
  struct hosts hosts[2] = {{"ya.ru", "80"}, {"google.com", "80"}};
  tcp_repairer * rep = tcp_repairer_create(hosts, 2, "echo repairing...", 3);

  loopswinger(loop, (repairer *) rep);
  timerloop_start(loop);
  
  if(repairer_destroy((repairer *) rep) != 0) {
    printf("Couldn't free memory for repairer\n");
  }
  timerloop_destroy(loop);
  
  return 0;
}
