#include <gc.h>
#include <stdio.h>

#include "repairer.h"


int repairer_dumb_check(repairer * rep) {
  return 0;
}


repairer * repairer_create(char * repair_command, int poll_interval) {
  repairer * rep = GC_malloc(sizeof(repairer));
  rep->repair_command = repair_command;
  rep->__poll_interval = rep->__base_poll_interval = poll_interval;
  rep->poll_min_interval = 1;
  rep->max_errors_count = 3;
  rep->repair_timeout = 5;
  rep->errors_count = 0;
  rep->_check_func = repairer_dumb_check;

  return rep;
}


int repairer_need_repair(repairer * rep) {
  return (rep->errors_count >= rep->max_errors_count &&
     rep->errors_count % rep->max_errors_count == 0);
}

void repairer_repair(repairer * rep) {
  printf("repairer: Command: '%s'\n", rep->repair_command);
  FILE * repair_proc = popen(rep->repair_command, "r");
  char in_buf[500];
  printf("repairer: Command output:\n");
  while(fgets(in_buf, sizeof(in_buf), repair_proc)) {
    printf("repairer: %s", in_buf);
  }
  printf("repairer: Command exited with %d\n", pclose(repair_proc));
}

void repairer_handle_error(repairer * rep) {
  printf("repairer: Error\n");
  rep->errors_count++;
  if(repairer_need_repair(rep)) {
    repairer_repair(rep);
  }
}

void repairer_handle_success(repairer * rep) {
  printf("repairer: Success\n");
  rep->errors_count = 0;
  rep->__poll_interval = rep->__base_poll_interval;
}


int repairer__calc_poll_interval(repairer * rep) {
  if(rep->errors_count == 1) {
    rep->__poll_interval = rep->poll_min_interval;
  } else {
    rep->__poll_interval = MIN(rep->__poll_interval * 2, rep->__base_poll_interval);
  }
  return rep->__poll_interval + (repairer_need_repair(rep) ?  rep->repair_timeout : 0);
}

int repairer_check(repairer * rep) {
  printf("repairer: Checking...\n");
  if(rep->_check_func(rep) != 0) {
    repairer_handle_error(rep);
  } else {
    repairer_handle_success(rep);
  }

  int res = repairer__calc_poll_interval(rep);
  printf("repairer: Next check after %d sec\n", res);
  return res;
}


void ev_loop_swinger(EV_P_ struct ev_timer * w, int revents) {
  repairer * rep = (repairer * )w->data;
  ev_timer_stop(EV_A_ w);
  ev_timer_set(w, repairer_check(rep), 0);
  ev_timer_start(EV_A_ w);
}

/* void loopswinger(timerloop *, repairer *); */
/* void loopswinger(timerloop * loop, repairer * rep) { */
/*   timerloop_add_timeout(loop, repairer_check(rep), loopswinger, rep); */
/* } */
