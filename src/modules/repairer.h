#pragma once

#include <ev.h>


#define MAX(a,b) ((a) > (b) ? a : b)
#define MIN(a,b) ((a) < (b) ? a : b)


typedef struct repairer repairer;
struct repairer {
  char * repair_command;
  int __poll_interval;
  int __base_poll_interval;
  int poll_min_interval;
  int max_errors_count;
  int repair_timeout;
  int errors_count;
  int (* _check_func)(repairer *);
};

repairer * repairer_create(char * repair_command, int poll_interval);
int repairer_dumb_check(repairer * rep);
int repairer_need_repair(repairer * rep);
void repairer_repair(repairer * rep);
void repairer_handle_error(repairer * rep);
int repairer__calc_poll_interval(repairer * rep);
int repairer_check(repairer * rep);
void ev_loop_swinger(EV_P_ struct ev_timer * w, int revents);
