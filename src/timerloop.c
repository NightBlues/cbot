#ifndef TIMERLOOP_SRC
#define TIMERLOOP_SRC
#include <time.h>
#include <unistd.h>
#include "priority_queue.c"

/* The important part of time. For priority queue conversions. */
#define RUN_TIME_MASK 0xffffffff

typedef struct {
  priority_queue * queue;
} timerloop;

typedef struct {
  time_t run_time;
  void (* func)(timerloop *, void *);
  void * ctx;
} LoopCall;


int LoopCall_is_ready(LoopCall * call) {
  return call->run_time <= time(NULL);
}


int LoopCall_advice_sleep(LoopCall * call) {
  time_t cur_time = time(NULL);
  if(call->run_time - cur_time > 0) {
    return call->run_time - cur_time;
  }
  return 0;
}


int timerloop_add_timeout(timerloop * loop, int timeout, void * func, void * ctx) {
  time_t run_time = (time_t) timeout + time(NULL);
  LoopCall * call = malloc(sizeof(LoopCall));
  call->run_time = run_time;
  call->func = func;
  call->ctx = ctx;
  priority_queue_insert(loop->queue, (priority_queue_entry) {(int) (run_time & RUN_TIME_MASK), call});
  return 0;
}


timerloop * timerloop_create() {
  timerloop * loop = malloc(sizeof(timerloop));
  loop->queue = priority_queue_create(30);
  return loop;
}

int timerloop_destroy(timerloop * loop) {
  if(priority_queue_destroy(loop->queue) != 0) {
    return -1;
  }
  free(loop);

  return 0;
}


int timerloop_start(timerloop * loop) {
  priority_queue_entry call;
  while(priority_queue_pop(loop->queue, &call) == 0) {
    if(LoopCall_is_ready(call.data)) {
      LoopCall lcall = *((LoopCall *) call.data);
      free(((LoopCall *) call.data));
      lcall.func(loop, lcall.ctx);
    } else {
      priority_queue_insert(loop->queue, call);
      sleep(LoopCall_advice_sleep(call.data));
    }
  }

  return 0;
}

/* Tests */

/* void func1(timerloop *, int *); */
/* void func1(timerloop * loop, int * ctx) { */
/*   (*ctx)++; */
/*   printf("%d: Called func1, counter(%p) = %d\n", (int)time(NULL), ctx, *ctx); */
/*   if(*ctx < 5) { */
/*     timerloop_add_timeout(loop, 0.5, func1, ctx); */
/*   } */
/* } */
/*   timerloop * loop = timerloop_create(); */
/*   int counter = 0; */
/*   func1(loop, &counter); */
/*   timerloop_start(loop); */
/*   timerloop_destroy(loop); */

#endif /* TIMERLOOP_SRC */
