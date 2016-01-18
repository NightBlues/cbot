#include <stdio.h>
#include "timerloop.c"


int main() {
  printf("Hello, world!\n");

  priority_queue * q = priority_queue_create(10);
  priority_queue_insert(q, (priority_queue_entry) {0, "hello"});
  priority_queue_insert(q, (priority_queue_entry) {3, "bye"});
  priority_queue_insert(q, (priority_queue_entry) {2, "kitty"});
  priority_queue_insert(q, (priority_queue_entry) {2, "kitty2"});
  priority_queue_insert(q, (priority_queue_entry) {10, "entryX"});
  priority_queue_insert(q, (priority_queue_entry) {11, "entryY"});
  priority_queue_insert(q, (priority_queue_entry) {12, "entryZ"});
  priority_queue_insert(q, (priority_queue_entry) {5, "entryA"});
  priority_queue_insert(q, (priority_queue_entry) {6, "entryB"});
  priority_queue_print(q);
  priority_queue_entry e;
  while(priority_queue_pop(q, &e) == 0){
    printf("Pop from queue: (%d, %s)\n", e.priority, (char *) e.data);
  }
  
  
  return 0;
}
