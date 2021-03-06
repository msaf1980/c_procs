/**
 * Macroses for Linux epoll
 */

/**
 * Example :
 * EPOLL_EVENT_SET(event, srv_fd, EPOLLIN | EPOLLET);
 */
#define EPOLL_EVENT_SET(epoll_event, sockfd, event)                            \
  do {                                                                         \
    (epoll_event).data.fd = sockfd;                                            \
    (epoll_event).events = event;                                              \
  } while (0)
