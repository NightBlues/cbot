#include <stdlib.h>
#include <stdio.h>

typedef struct {
  int priority;
  void * data;
} priority_queue_entry;

typedef struct {
  priority_queue_entry * queue;
  int len;
  int size;
} priority_queue;


/* Heapify array in queue */
void keepheap(priority_queue * queue) {
  priority_queue_entry tmp_e;
  for(int i=queue->len - 1; i > 0; i--) {
    
    int parent_idx = i / 2;
    if(parent_idx != i && queue->queue[parent_idx].priority > queue->queue[i].priority) {
      tmp_e = queue->queue[parent_idx];
      queue->queue[parent_idx] = queue->queue[i];
      queue->queue[i] = tmp_e;
    }
  }
}

/* Create new instance of queue */
priority_queue * priority_queue_create(int size) {
  priority_queue * q = malloc(sizeof(priority_queue));
  q->queue = malloc(sizeof(priority_queue_entry) * size);
  q->len = 0;
  q->size = size;

  return q;
}

/* Destroy instance of queue */
int priority_queue_destroy(priority_queue * queue) {
  /*destroy all entries*/
  if(queue->len > 0) {
    return -1;
  }
  free(queue->queue);
  free(queue);
  return 0;
}

/* Insert new element into queue */
int priority_queue_insert (priority_queue * queue, priority_queue_entry entry) {
  if(queue->len < queue->size) {
    queue->queue = realloc(queue->queue, sizeof(priority_queue_entry) * queue->size * 2);
  }
  queue->queue[queue->len] = entry;
  queue->len++;
  keepheap(queue);

  return 0;
}

/* Pop element from queue */
int priority_queue_pop (priority_queue * queue, priority_queue_entry * res_entry) {
 /* TODO: reallocate */
  /* if(queue->len < queue->size) { */
  /*   queue->queue = realloc(queue->queue, sizeof(priority_queue_entry) * queue->size * 2); */
  /* } */
  if(queue->len == 0) {
    return -1;
  }
  *res_entry = queue->queue[0];
  queue->queue[0] = queue->queue[--queue->len];
  keepheap(queue);

  return 0;
}


/* Pretty print queue */
void priority_queue_print(priority_queue * queue) {
  for(int i=0; i<queue->len; i++) {
    printf("queue[%d] = (%d, %s)\n", i, queue->queue[i].priority, (char *)queue->queue[i].data);
  }
}

/* Tests */
  /* priority_queue * q = priority_queue_create(10); */
  /* priority_queue_insert(q, (priority_queue_entry) {0, "hello"}); */
  /* priority_queue_insert(q, (priority_queue_entry) {3, "bye"}); */
  /* priority_queue_insert(q, (priority_queue_entry) {2, "kitty"}); */
  /* priority_queue_insert(q, (priority_queue_entry) {2, "kitty2"}); */
  /* priority_queue_insert(q, (priority_queue_entry) {10, "entryX"}); */
  /* priority_queue_insert(q, (priority_queue_entry) {11, "entryY"}); */
  /* priority_queue_insert(q, (priority_queue_entry) {12, "entryZ"}); */
  /* priority_queue_insert(q, (priority_queue_entry) {5, "entryA"}); */
  /* priority_queue_insert(q, (priority_queue_entry) {6, "entryB"}); */
  /* priority_queue_print(q); */
  /* priority_queue_entry e; */
  /* while(priority_queue_pop(q, &e) == 0){ */
  /*   printf("Pop from queue: (%d, %s)\n", e.priority, (char *) e.data); */
  /* } */
  /* priority_queue_destroy(q); */
