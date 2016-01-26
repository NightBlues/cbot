#include <stdlib.h>
#include <stdio.h>
#include "timerloop.c"

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


int repairer_dumb_check(repairer * rep) {
  return 0;
}


repairer * repairer_create(char * repair_command, int poll_interval) {
  repairer * rep = malloc(sizeof(repairer));
  rep->repair_command = repair_command;
  rep->__poll_interval = rep->__base_poll_interval = poll_interval;
  rep->poll_min_interval = 1;
  rep->max_errors_count = 3;
  rep->repair_timeout = 5;
  rep->errors_count = 0;
  rep->_check_func = repairer_dumb_check;

  return rep;
}


int repairer_destroy(repairer * rep) {
  free(rep);
  return 0;
}


int repairer_need_repair(repairer * rep) {
  return (rep->errors_count >= rep->max_errors_count &&
     rep->errors_count % rep->max_errors_count == 0);
}

void repairer_repair(repairer * rep) {
  printf("Command: '%s'\n", rep->repair_command);
  FILE * repair_proc = popen(rep->repair_command, "r");
  char in_buf[500];
  printf("Command output:\n");
  while(fgets(in_buf, sizeof(in_buf), repair_proc)) {
    printf("%s", in_buf);
  }
  printf("Command exited with %d\n", pclose(repair_proc));
}

void repairer_handle_error(repairer * rep) {
  printf("Error\n");
  rep->errors_count++;
  if(repairer_need_repair(rep)) {
    repairer_repair(rep);
  }
}

void repairer_handle_success(repairer * rep) {
  printf("Success\n");
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
  printf("Checking...\n");
  if(rep->_check_func(rep) != 0) {
    repairer_handle_error(rep);
  } else {
    repairer_handle_success(rep);
  }

  return repairer__calc_poll_interval(rep);
}


void loopswinger(timerloop *, repairer *);
void loopswinger(timerloop * loop, repairer * rep) {
  timerloop_add_timeout(loop, repairer_check(rep), loopswinger, rep);
}


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
