#include <nanomsg/nn.h>
#include <nanomsg/pubsub.h>
#include <ev.h>


typedef struct messaging messaging;
struct messaging {
  ev_io * watcher;
  int * nn_sock;
};

static void msg_cb(EV_P_ ev_io * w, int revents) {
  int * nn_sock = (int *) w->data;
  void * buf = NULL;
  int nbytes = nn_recv(*nn_sock, &buf, NN_MSG, 0);
  printf("messaging: got message: %s\n", (char * )buf);
  nn_freemsg(buf);

}

messaging * start_messaging(struct ev_loop * loop, char * address) {
  messaging * msg = malloc(sizeof(messaging));
  msg->watcher = malloc(sizeof(ev_io));
  msg->nn_sock = malloc(sizeof(int));
  *msg->nn_sock = nn_socket(AF_SP, NN_SUB);
  nn_setsockopt(*msg->nn_sock, NN_SUB, NN_SUB_SUBSCRIBE, "", 0);
  nn_connect(*msg->nn_sock, address);
  /* nn_bind(*msg->nn_sock, address); */
  int nn_sock_fd;
  size_t fd_size = sizeof(nn_sock_fd);
  if(nn_getsockopt(*msg->nn_sock, NN_SOL_SOCKET, NN_RCVFD, &nn_sock_fd, &fd_size) != 0) {
    printf("messaging: Could'not retrive fd for polling, err=%d\n", nn_errno());
    printf("messaging: EBADF = %d; ENOPROTOOPT = %d; ETERM = %d\n", EBADF, ENOPROTOOPT, ETERM);
  }

  ev_io_init(msg->watcher, msg_cb, nn_sock_fd, EV_READ);
  msg->watcher->data = msg->nn_sock;
  ev_io_start(loop, msg->watcher);
  return msg;
}

void stop_messaging(struct ev_loop * loop, messaging * msg) {
  ev_io_stop(loop, msg->watcher);
  free(msg->nn_sock);
  free(msg->watcher);
  free(msg);
}
