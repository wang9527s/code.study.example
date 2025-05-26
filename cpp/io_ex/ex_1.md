// select 示例
fd_set readfds;
FD_ZERO(&readfds);
FD_SET(sockfd, &readfds);
select(sockfd+1, &readfds, NULL, NULL, &timeout);

// poll 示例
struct pollfd pfds[10];
pfds[0].fd = sockfd;
pfds[0].events = POLLIN;
poll(pfds, 1, timeout);

// epoll 示例
int epfd = epoll_create(10);
struct epoll_event ev;
ev.events = EPOLLIN;
ev.data.fd = sockfd;
epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);
epoll_wait(epfd, events, maxevents, timeout);

---
int efd = epoll_create1(0);

// 添加 eventfd
int evfd = eventfd(0, EFD_NONBLOCK);
struct epoll_event ev = {.events = EPOLLIN, .data.fd = evfd};
epoll_ctl(efd, EPOLL_CTL_ADD, evfd, &ev);

// 添加 timerfd
int tfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
struct itimerspec ts = {{1,0},{1,0}};  // 每秒触发
timerfd_settime(tfd, 0, &ts, NULL);
ev = (struct epoll_event){.events = EPOLLIN, .data.fd = tfd};
epoll_ctl(efd, EPOLL_CTL_ADD, tfd, &ev);
