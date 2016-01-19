#include <stdio.h>
#include "timerloop.c"
#include "repairer.c"


int main() {
  printf("Hello, world!\n");
  timerloop * loop = timerloop_create();
  repairer * rep = repairer_create("echo \"hello\"", 3);
  int my_check(repairer * rep) { return 1;}
  rep->_check_func = my_check;
  loopswinger(loop, rep);
  timerloop_start(loop);
  timerloop_destroy(loop);
  
  
  return 0;
}
